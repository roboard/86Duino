/*****************************************************************************
 *
 * File name   : sendmail.c
 * Function    : SMTP client
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 29-Nov-02, LVR, Created.
 * 28-Mar-05, LVR, Minor tweaks to work with smtpd forwarding
 * 15-Nov-09, LVR, Added Login
 *
 *****************************************************************************/

/* Compilation options */

/* Exports */
#include "sendmail.h"

/* Imports */

/* ANSI C library */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>
#include <limits.h>
#if _MSC_VER >= 1000
#include <crtdbg.h> /* Heap tracking */
#endif

#include "sws_net.h"
#include "sws_str.h"   /* SWS_stricmp */
#include "sws_dns.h"
#include "sws_cfg.h"


/***************************************************************************
 * Macros & constants
 ***************************************************************************/

/* Expand and stringize a macro name */
#define STR(s) #s
#define STRING(s) STR(s)

#ifdef __GNUC__
#define FORMAT(a,f,l)  __attribute__ ((format (a, f, l)))
#else
#define FORMAT(a,f,l)
#endif

/* Log file prefix */
#define kszPrefix "SENDMAIL: "

#define kszApp "Software Systems (www.softsystem.co.uk) SendMail"
#define kszVer STRING( SENDMAIL_VER_MAJOR) "." STRING( SENDMAIL_VER_MINOR)

#define DEF_SMTP_PORT   25U

/* SMTP timeouts RFC 2821 p57 */
#define INIT_TIMEOUT (30) /* RFC 2821 says (5 * 60) */
#define GEN_TIMEOUT  (1 * 60)
#define MAIL_TIMEOUT (5 * 60)
#define RCPT_TIMEOUT (5 * 60)
#define DATA_TIMEOUT (2 * 60)
#define TERM_TIMEOUT (10 * 60)
#if UINT_MAX > 0xffff
#define SEND_TIMEOUT (3 * 60 * 1000U) /* millisecs */
#else
#define SEND_TIMEOUT UINT_MAX /* millisecs */
#endif

/* Special mailboxes for envelope info */
#define kszCC "cc:"
#define kszBCC "bcc:"
#define CRLF "\x0d\x0a"

#define EINTERRUPTED 11000

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#define MAX_HOST 64       /* Max len of MX name */
#define BUFFER_SIZE 512   /* Size of SMTP output buffer */
#define STRFMT "%.255s"   /* Max len string in output buffer */

/* Ini file items */
#define kszMailSect "mail"
#define kszServer     "server"
#define kszPort       "port"
#define kszFrom       "from"


/***************************************************************************
 * Data types
 ***************************************************************************/

/* Log level */
enum { kLogError = -1, kLogWarn = 0, kLogInfo, kLogMore, kLogMost };

typedef struct SConn
  {
  SOCKET s;
  struct sockaddr_in server;
  FILE* logfile;
  int iVerbosity;
  int isInteractive;
  } SConn;


/***************************************************************************
 * Local function prototypes
 ***************************************************************************/

static int MailFile(
  SConn*, const char*, const char*, FILE*, const char*, const char*,
  const char*, int, const char* const [], int, char[], size_t
);
static int GetMx( struct sockaddr_in*, const char* pszMailbox);
static void MimeEncodeBase64( char [], size_t, const unsigned char [], size_t);
static void Log( int, const struct SConn*, const char [], ...) FORMAT(printf,3,4);

static int DoCommand( SConn*, const char*, char[], size_t, unsigned);
static int GetResponse( SConn*, char[], size_t, unsigned);
static int ReadLine( SConn*, char [], size_t, unsigned);

static int ConnectAsync( SOCKET, struct sockaddr_in*, int);
static int RawRead( SConn*, void*, size_t, unsigned);
static int RawWrite( SConn*, const void*, size_t);


/***************************************************************************
 * Module data
 ***************************************************************************/

extern sig_atomic_t volatile g_exit;

static char s_szHost[ MAX_HOST];    /* HELO name */
static char s_szBuf[ BUFFER_SIZE];  /* Output buffer */


/***************************************************************************
 * Functions
 ***************************************************************************/

/*
 * Mail a file
 */
int SendMail(
  FILE* inputfile,                /* File to mail */
  int isInteractive,
  int iRecipients,                /* Count of recipients */
  const char* const pszToArray[], /* Array of recipients, cc: or bcc: */
  const char* pszFrom,            /* From, maybe empty or NULL */
  const char* pszReply,           /* ReplyTo, maybe NULL */
  const char* pszSubject,         /* Subject. NULL or empty= none */
  const char* pszSmtpServer,      /* SMTP server, NULL= default, empty= use MX for each recipient */
  unsigned short unPort,          /* SMTP port */
  const char* pszHost,            /* Name for HELO, NULL= gethostname, empty= use IP address*/
  const char* pszLogin,           /* Login: usr:pwd or S(echo -ne '\0acct\0passwd'|base64) */
  FILE* logfile,                  /* Logfile, NULL= none */
  int iVerbosity                  /* Log verbosity */
) {
  char szFrom[ 128];
  int iRet = EXIT_SUCCESS;
  int iRecipient;
  struct sockaddr_in server;
  struct hostent* pHost;
  u_long ulVal;

  assert( NULL != inputfile);
  assert( iRecipients > 0);
  assert( NULL != pszToArray);

  if ( NULL == pszHost)
    {
    gethostname( s_szHost, sizeof s_szHost);
    pszHost = s_szHost;
    }

  if ( NULL == pszSmtpServer)
    {
    SWS_CfgGetString( kszMailSect, kszServer, "", s_szBuf, sizeof s_szBuf );
    pszSmtpServer = s_szBuf;
    }

  if ( '\0' == pszSmtpServer[0])
    {
    /* Use MX for each recipient */
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    }
  /* Try an IP address */
  else if ( INADDR_ANY == (ulVal = inet_addr( pszSmtpServer)))
    {
    Log( kLogError, NULL, "ERROR: Invalid host %s\n", pszSmtpServer );
    return EXIT_FAILURE;
    }
  else if ( INADDR_NONE != ulVal)
    {
    server.sin_addr.s_addr = ulVal;
    server.sin_family = AF_INET;
    }
  /* Try a hostname */
  else if ( NULL != (pHost = gethostbyname( pszSmtpServer)) )
    {
    server.sin_family = pHost->h_addrtype;
    memcpy( &server.sin_addr, pHost->h_addr, pHost->h_length);
    }
  else
    {
    Log( kLogWarn, NULL, "WARN: Unable to resolve %s\n", pszSmtpServer);
    return EXIT_FAILURE;
    }

  /* Server port specifed? */
  if ( 0 != unPort)
    server.sin_port = htons( unPort);
  else
    {
    server.sin_port = htons(
      (u_short) SWS_CfgGetValue( kszMailSect, kszPort, DEF_SMTP_PORT) );
    }

  if ( NULL == pszFrom)
    {
    SWS_CfgGetString( kszMailSect, kszFrom, "", szFrom, sizeof( szFrom) );
    pszFrom = szFrom;
    }

  for ( iRecipient = 0; iRecipient < iRecipients; ++iRecipient)
    {
    SConn conn;
    int i, iOnlyRecipient;
    socklen_t iLen;

    if ( 0 == SWS_stricmp( kszCC, pszToArray[ iRecipient] )
      || 0 == SWS_stricmp( kszBCC, pszToArray[ iRecipient] )
    ) {
      continue;
      }

    /* Create a socket */
    conn.logfile = logfile;
    conn.iVerbosity = iVerbosity;
    conn.isInteractive = isInteractive;
    conn.server = server;
    conn.s = socket( conn.server.sin_family, SOCK_STREAM, 0);
    if ( INVALID_SOCKET == conn.s)
      {
      iRet = GetLastError();
      Log( kLogError, &conn, "ERROR %d: socket: %s\n",
        iRet, strerror( iRet));
      break;
      }


    /* Set a send timeout */
    { DECL_TIMEOUT( tv, SEND_TIMEOUT);
    setsockopt(
      conn.s,
      SOL_SOCKET, SO_SNDTIMEO,
      (char*)&tv, sizeof tv
    ); }

    /* SMTP server specified? */
    if ( INADDR_ANY == conn.server.sin_addr.s_addr)
      {
      /* No, use MX for each recipient */
      if ( !GetMx( &conn.server, pszToArray[ iRecipient] ))
        {
        Log( kLogError, &conn, "ERROR: Failed to find MX for %s\n",
          pszToArray[ iRecipient] );
        iRet = EHOST_NOT_FOUND;
        closesocket( conn.s);
        continue;
        }
      iOnlyRecipient = iRecipient;
      Log( kLogInfo, &conn, "Connect to MX for <%s>\n", pszToArray[ iRecipient]);
      }
    else
      {
      /* Send everything to the specified SMTP server */
      iOnlyRecipient = -1;
      Log( kLogInfo, &conn, "Connect to server\n");
      }

    if ( isInteractive)
      fprintf( stderr, "Connecting to %s:%u...",
        inet_ntoa( conn.server.sin_addr), ntohs( conn.server.sin_port)
      );

    /* Connect to server */
    i = ConnectAsync( conn.s, &conn.server, isInteractive);
    if ( 0 != i)
      {
      closesocket( conn.s);

      switch ( i)
        {
      case EINTERRUPTED:
        fputs( " Interrupted.\n", stderr);
        break;
      case ETIMEDOUT:
      case EWOULDBLOCK:
#if EAGAIN != EWOULDBLOCK
      case EAGAIN:
#endif
        fputs( "Host not responding.\n", stdout);
        if ( NULL != logfile)
          Log( kLogError, &conn, "ERROR %d: Connection timed out\n", i);
        break;
      case ECONNREFUSED:
        fputs( "Not a mail server.\n", stdout);
        if ( NULL != logfile)
          Log( kLogError, &conn, "ERROR %d: Connection refused\n", i);
        break;
      default:
        fprintf( stdout, "Error %d: %s\n", i, strerror( i));
        if ( NULL != logfile)
          Log( kLogError, &conn, "ERROR %d: Connection failed: %s\n", i, strerror( i));
        break;
        }

      iRet = i;
      continue;
      }

    if ( isInteractive)
      fputs( " ok\n", stderr);

    iLen = sizeof conn.server;
    getpeername( conn.s, (struct sockaddr*)&conn.server, &iLen);
    Log( kLogMore, &conn, "Connected\n");

    if ( '\0' == *pszHost)
      {
      /* Set host name to IP address */
      struct sockaddr_in addr;
      iLen = sizeof addr;
      getsockname( conn.s, (struct sockaddr*)&addr, &iLen);
      strcpy( s_szHost, inet_ntoa( addr.sin_addr));
      pszHost = s_szHost;
      }

    /* Get signon */
    i = GetResponse( &conn, s_szBuf, sizeof( s_szBuf), INIT_TIMEOUT);
    if ( 220 == i)
      {
      rewind( inputfile);

      /* Send the item */
      i = MailFile(
        &conn,
        pszHost,
        pszLogin,
        inputfile,
        pszFrom,
        pszReply,
        pszSubject,
        iRecipients, pszToArray, iOnlyRecipient,
        s_szBuf, sizeof s_szBuf
      );
      if ( 250 != i)
        {
        /* Failed */
        fputs( s_szBuf, stdout);
        iRet = i;
        }

      DoCommand( &conn, "QUIT", s_szBuf, sizeof s_szBuf, GEN_TIMEOUT );
      }
    else
      {
      /* Bad login, probably not an SMTP service */
      fprintf( stdout, "Login failed: %s\n", s_szBuf);
      iRet = ECONNABORTED;
      }

    /* Disconnect server */
    closesocket( conn.s);
    if ( -1 == iOnlyRecipient)
      break;
    }

  return iRet;
  }


/*
 * Mail a file
 */
int MailFile(
  SConn* pConn,
  const char* pszHost,            /* HELO name */
  const char* pszLogin,           /* Login: NULL, empty, usr:pwd or base64 */
  FILE* file,                     /* File to mail */
  const char* pszFrom,            /* From, maybe empty */
  const char* pszReply,           /* Reply, maybe NULL */
  const char* pszSubject,         /* Subject. NULL or empty= none */
  int iRecipients,                /* Count of recipients */
  const char* const pszToArray[], /* Array of recipients */
  int iThisRecipient,             /* Recipient index or -1 if all */
  char szBuf[],                   /* IN: buffer */
  size_t buffersize               /* IN: buffer size */
) {
  int iRet, eol, i;
  time_t const t = time( NULL); /* Submission time */
  const char* pcsz;

  assert( NULL != file);
  assert( NULL != pszHost);
  assert( NULL != pszFrom);
  assert( iRecipients > 0);
  assert( NULL != pszToArray);
  assert( NULL != szBuf);
  assert( buffersize > MAX_HOST);
  assert( buffersize >= BUFFER_SIZE);

  /* Send login */
  sprintf( szBuf, NULL != pszLogin ? "EHLO " STRFMT : "HELO " STRFMT, pszHost);
  iRet = DoCommand( pConn, szBuf, szBuf, buffersize, GEN_TIMEOUT);
  if ( 250 != iRet)
    return iRet;

  if ( NULL != pszLogin && *pszLogin)
    {
    /* AUTH: RFC 2554 and later 4954, SASL RFC 4422 */
    pcsz = strchr( pszLogin, ':');
    if ( NULL == pcsz)
      sprintf( szBuf, "AUTH PLAIN " STRFMT, pszLogin);
    else
      {
      size_t usrlen, pwdlen;
      char* psz;

      /* Convert 'usr:pwd' to '\0usr\0pwd' */
      usrlen = pcsz - pszLogin;
      pwdlen = strlen( ++pcsz);
      psz = &szBuf[ buffersize - 2 - usrlen - pwdlen];
      psz[ 0] = '\0';
      memcpy( &psz[ 1], pszLogin, usrlen);
      psz[ usrlen + 1] = '\0';
      memcpy( &psz[ usrlen + 2], pcsz, pwdlen);

      strcpy( szBuf, "AUTH PLAIN ");
      MimeEncodeBase64( &szBuf[ 11], buffersize - 11, (unsigned char*)psz, 2 + usrlen + pwdlen);
      }

    iRet = DoCommand( pConn, szBuf, szBuf, buffersize, GEN_TIMEOUT );
    if ( 235 != iRet)
      return iRet;
    }

#if 0
  iRet = DoCommand( pConn, "RSET", szBuf, buffersize, GEN_TIMEOUT);
  if ( 250 != iRet)
    return iRet;
#endif

  /* Specify return path */
  pcsz = strchr( pszFrom, '<');
  if ( NULL == pcsz)
    sprintf( szBuf, "MAIL FROM:<" STRFMT ">", pszFrom);
  else
    sprintf( szBuf, "MAIL FROM:" STRFMT, pcsz);

  iRet = DoCommand( pConn, szBuf, szBuf, buffersize, MAIL_TIMEOUT);
  if ( 250 != iRet)
    return iRet;

  /* Specify recipients */
  if ( iThisRecipient >= 0)
    {
    sprintf( szBuf, "RCPT TO:<" STRFMT ">", pszToArray[ iThisRecipient] );
    iRet = DoCommand( pConn, szBuf, szBuf, buffersize, RCPT_TIMEOUT);
    if ( iRet >= 400)
      return iRet;
    }
  else for ( i = 0; i < iRecipients; ++i)
    {
    /* Ignore Cc and Bcc directives */
    if ( !SWS_stricmp( kszCC, pszToArray[ i] )
      || !SWS_stricmp( kszBCC, pszToArray[ i] )
    ) {
      continue;
      }

    sprintf( szBuf, "RCPT TO:<" STRFMT ">", pszToArray[ i] );
    iRet = DoCommand( pConn, szBuf, szBuf, buffersize, RCPT_TIMEOUT);
    if ( iRet >= 400)
      return iRet;
    }

  iRet = DoCommand( pConn, "DATA", szBuf, buffersize, DATA_TIMEOUT);
  if ( 354 != iRet)
    return iRet;

  if ( NULL != pszSubject)
    {
    /* Create envelope - RFC 2822 p18 */
    const struct tm local = *localtime( &t);
    char* psz = szBuf;

    /* Origination date (required) */
#if __STDC_VERSION__ >= 199901L
    psz += strftime(
      psz, buffersize, "Date: %a, %d %b %Y %H:%M:%S %z" CRLF, &local );
#else
    {
    struct tm gmt = *gmtime( &t);
    psz += strftime( psz, buffersize, "Date: %a, %d %b %Y %H:%M:%S", &local );
    i = local.tm_hour - gmt.tm_hour;
    if ( i <= -12)
      i += 24;
    else if ( i > 12)
      i -= 24;
    i *= 100;
    if ( local.tm_min >= gmt.tm_min)
      i += local.tm_min - gmt.tm_min;
    else
      i += (60 + local.tm_min) - gmt.tm_min - 100;
    psz += sprintf( psz, " %+05d" CRLF, i);
    }
#endif
    RawWrite( pConn, szBuf, psz - szBuf);

    /* Originator from (required) */
    if ( *pszFrom)
      RawWrite( pConn, szBuf, sprintf( szBuf, "From: " STRFMT CRLF, pszFrom) );

    /* Originator reply-to */
    if ( NULL != pszReply && *pszReply)
      RawWrite( pConn, szBuf, sprintf( szBuf, "Reply-To: " STRFMT CRLF, pszReply) );

    /* Destination addresses */
    psz = szBuf;
    eol = 0;
    for ( i = 0; i < iRecipients; ++i)
      {
      if ( !SWS_stricmp( kszCC, pszToArray[ i]) )
        {
        /* Start a cc address field */
        if ( eol)
          {
          psz += sprintf( psz, CRLF);
          RawWrite( pConn, szBuf, psz - szBuf);
          eol = 0;
          }

        psz = szBuf + sprintf( szBuf, "Cc: ");
        }
      else if ( !SWS_stricmp( kszBCC, pszToArray[ i]) )
        {
        /* Bcc's are not added to the envelope */
        break;
        }
      else if ( 0 == i)
        {
        /* Start a to address field */
        eol = 1;
        psz += sprintf( psz, "To: " STRFMT, pszToArray[ 0]);
        }
      else if ( eol)
        {
        ++eol;
        psz += sprintf( psz, "," STRFMT, pszToArray[ i]);
        }
      else
        {
        eol = 1;
        psz += sprintf( psz, STRFMT, pszToArray[ i]);
        }
      }

    if ( eol)
      {
      psz += sprintf( psz, CRLF);
      RawWrite( pConn, szBuf, psz - szBuf);
      }

    /* Informational */
    if ( '\0' != *pszSubject)
      RawWrite( pConn, szBuf, sprintf( szBuf, "Subject: " STRFMT CRLF, pszSubject) );

    /* MIME header RFC 2045 */
    iRet = RawWrite( pConn, szBuf,
      sprintf( szBuf,
        "MIME-Version: 1.0" CRLF
        "Content-Type: text/plain; charset=\"us-ascii\"" CRLF
#if 0
        "X-Mailer:" kszApp " " kszVer CRLF
#endif
        CRLF
      )
    );
    if ( SOCKET_ERROR == iRet )
      {
      strcpy( szBuf, "Connection reset\n");
      return 1000;
      }
    }

  /* Body */
  if ( pConn->isInteractive)
    fputs( "Enter the message terminated by <eof>:", stderr);

  eol = 1; /* CRLF sent */

  /* Read a line at a time until EOF */
  while ( NULL != fgets( szBuf, buffersize - 1, file) )
    {
    size_t count = strlen( szBuf);

    /* Transparency */
    if ( eol && '.' == szBuf[0] )
      {
      /* Prepend a period */
      memmove( szBuf + 1, szBuf, count);
      ++count;
      }

    /* Remove trailing '\n' */
    if ( count > 0 && '\n' == szBuf[ count - 1] )
      {
      /* Replace '\n' with CR LF */
      strcpy( &szBuf[ count - 1], CRLF);
      ++count;

      eol = 1;
      }
    else
      {
      eol = 0;
      }

    if ( SOCKET_ERROR == RawWrite( pConn, szBuf, count) )
      {
      strcpy( szBuf, "Connection reset\n");
      return 1000;
      }
    }

  if ( pConn->isInteractive)
    fputs( "\n", stderr);

  /* Send 'end of data' */
  if ( eol)
    {
    static const char kszEOM[] = "." CRLF;
    RawWrite( pConn, kszEOM, sizeof( kszEOM) - 1);
    }
  else
    {
    static const char kszNLEOM[] = CRLF "." CRLF;
    RawWrite( pConn, kszNLEOM, sizeof( kszNLEOM) - 1);
    }

  /* Get response */
  return GetResponse( pConn, szBuf, buffersize, TERM_TIMEOUT);
  }


/*
 * Issue an SMTP command
 */
static int DoCommand(
  SConn* pConn,
  const char* pszCmd,
  char szBuf[],
  size_t buffersize,
  unsigned timeout
) {
  Log( kLogInfo, pConn, "%s\n", pszCmd);

  if ( SOCKET_ERROR == RawWrite( pConn, pszCmd, strlen( pszCmd) )
    || SOCKET_ERROR == RawWrite( pConn, CRLF, sizeof( CRLF) - 1)
  ) {
    strcpy( szBuf, "Connection reset\n");
    return 1000;
    }

  return GetResponse( pConn, szBuf, buffersize, timeout);
  }


/*
 * Get a server response
 */
static int GetResponse(
  SConn* pConn,
  char szBuf[],
  size_t buffersize,
  unsigned timeout
) {
  int iRet = -1, eol = 1;

  for ( ;;)
    {
    char c;
    int iCode;

    iCode = ReadLine( pConn, szBuf, buffersize, timeout );
    if ( SOCKET_ERROR == iCode)
      {
      strcpy( szBuf, "No data during read\n");
      iRet = 1000;
      break;
      }

    if ( !eol)
      {
      /* Discard remainder of long line */
      if ( iCode > 0 && '\n' == szBuf[ iCode - 1])
        eol = 1;
      continue;
      }

    if ( iCode > 0 && '\n' == szBuf[ iCode - 1] )
      {
      --iCode;
      eol = 1;
      }
    else
      {
      eol = 0;
      }

    Log( kLogInfo, pConn, "%.*s\n", iCode, szBuf);

    /* Check for response code */
    c = ' ';
    if ( sscanf( szBuf, "%d%c", &iCode, &c) >= 1)
      {
      switch ( c)
        {
      case ' ':
        if ( -1 == iRet)
          return iCode; /* Single line response */

        return iRet; /* End of multi-line response */

      case '-':
        /* Part of multi-line response */
        if ( -1 == iRet)
          iRet = iCode;
        break;

      default:
        break;
        }
      }
    }

  return iRet;
  }


/*
 * Read a line
 */
static int ReadLine(
  SConn* pConn,
  char pBuf[],
  size_t size,
  unsigned timeout
) {
  unsigned char c;
  size_t count = 0;
  int iRet = 0;
  int gotCR = 0;

  while (
    count < size
    && SOCKET_ERROR != (iRet = RawRead( pConn, &c, 1, timeout) )
  ) {
    if ( gotCR)
      {
      gotCR = 0;
      switch ( c)
        {
      case 0:
      case 10:
        pBuf[ count++ ] = '\n';
        pBuf[ count] = '\0';
        iRet = count;
        size = 0; /* Force exit */
        break;

      default:
        pBuf[ count = 0] = (char)c;
        break;
        }
      }
    else switch ( c)
      {
    default:
      if ( isprint( c) && count + 1 < size)
        pBuf[ count++] = (char)c;
      break;

    case 10:
      pBuf[ count++ ] = '\n';
      pBuf[ count] = '\0';
      iRet = count;
      size = 0; /* Force exit */
      break;

    case 13:
      gotCR = 1;
      break;
      }
    }

  return iRet;
  }


/*
 * Non-blocking read data
 */
static int RawRead(
  SConn* pConn,
  void* pv,
  size_t size,
  unsigned timeout
) {
  time_t const t = time( NULL) + timeout;

  /* Until SIGINT or timeout */
  while ( !g_exit && time( NULL) <= t )
    {
    fd_set set;
    int iRet;
    struct timeval tval;

    FD_ZERO( &set);
    FD_SET( pConn->s, &set);

    /* Set timeout for select */
    tval.tv_usec = 333 * 1000UL;
    tval.tv_sec = 0;

    /* Wait for a receive event or timeout */
    iRet = select( FD_SETSIZE, &set, NULL, NULL, &tval);
    if ( 0 == iRet)
      continue; /* Timeout */

    if ( SOCKET_ERROR == iRet)
      return SOCKET_ERROR;

    /* Process receive event */
    if ( FD_ISSET( pConn->s, &set))
      {
      iRet = recv( pConn->s, (char*)pv, size, 0);

      if ( SOCKET_ERROR == iRet)
        {
        int err = GetLastError();
        switch ( err)
          {
        case ECONNRESET:
          Log( kLogError, pConn, "ERROR %d: Connection reset while reading\n", err);
          break;
        case ECONNABORTED:
          Log( kLogError, pConn, "ERROR %d: Connection aborted while reading\n", err);
          break;
        case ENOTCONN:
          Log( kLogError, pConn, "ERROR %d: Disconnected while reading\n", err);
          break;
        case EWOULDBLOCK:
#if EAGAIN != EWOULDBLOCK
        case EAGAIN:
#endif
          continue;
        default:
          Log( kLogError, pConn, "ERROR %d: Receive failed: %s\n", err, strerror( err));
          break;
          }
        return SOCKET_ERROR;
        }
      else if ( 0 == iRet)
        {
        /* Graceful disconnect */
        Log( kLogMost, pConn, "Disconnected\n" );
        return SOCKET_ERROR;
        }

      return iRet;
      }
    }

  return SOCKET_ERROR;
  }


/*
 * Write raw data
 */
static int RawWrite(
  SConn* pConn,
  const void* pv,
  size_t size
) {
  int iRet;

  iRet = send( pConn->s, (const char*)pv, size, MSG_NOSIGNAL);
  if ( SOCKET_ERROR == iRet)
    {
    int err = GetLastError();
    switch ( err)
      {
    case ECONNRESET:
      Log( kLogError, pConn, "ERROR %d: Connection reset while writing\n", err);
      break;
    case ECONNABORTED:
      Log( kLogError, pConn, "ERROR %d: Connection aborted while writing\n", err);
      break;
    case ETIMEDOUT:
      Log( kLogError, pConn, "ERROR %d: Connection timed out while writing\n", err);
      break;
    case EWOULDBLOCK:
#if EAGAIN != EWOULDBLOCK
    case EAGAIN:
#endif
      /* Win98se sometimes returns EWOULDBLOCK even though select()
      * previously indicated socket is ready to be written */
      return 0;
    default:
      Log( kLogError, pConn, "ERROR %d: Send failed: %s\n", err, strerror( err));
      break;
      }
    }

  return iRet;
  }


/*
 * Non-blocking connect
 */
int ConnectAsync(
  SOCKET s,
  struct sockaddr_in* pTo,
  int isInteractive
) {
  int iRet = 0;

  /* Enable non-blocking mode */
  setsockasync( s, 1);

  if ( connect( s, (struct sockaddr*)pTo, sizeof(*pTo))
    && (EWOULDBLOCK == (iRet = GetLastError()) || EINPROGRESS == iRet || EAGAIN == iRet)
  ) {
    int count;

    for ( count = 0; ; ++count)
      {
      fd_set setw, sete;
      struct timeval tval;

      /* Check for SIGINT */
      if ( g_exit)
        {
        iRet = EINTERRUPTED;
        break;
        }

      FD_ZERO( &setw);
      FD_SET( s, &setw);
      FD_ZERO( &sete);
      FD_SET( s, &sete);

      /* Set timeout for select */
      tval.tv_usec = 333 * 1000UL;
      tval.tv_sec = 0;

      /* Wait for a connect, error or timeout */
      iRet = select( FD_SETSIZE, NULL, &setw, &sete, &tval);
      if ( 0 == iRet)
        {
        /* Timeout */
        if ( isInteractive)
          {
          static const char progress[] = "|/-\\";
          fprintf( stderr, "%c\b", progress[ count % (sizeof progress -1)] );
          }
        continue;
        }
      else if ( SOCKET_ERROR == iRet)
        {
        iRet = GetLastError();
        assert( iRet);
        break;
        }
      else if ( FD_ISSET( s, &sete) || FD_ISSET( s, &setw))
        {
        /* Get error */
        socklen_t len = sizeof iRet;
        getsockopt( s, SOL_SOCKET, SO_ERROR, (char*)&iRet, &len);
        break;
        }
      }
    }

  /* Disable non-blocking mode */
  setsockasync( s, 0);

  return iRet;
  }


/*
 * Get the preferred mail server for a mailbox
 */
int GetMx(
  struct sockaddr_in* pAddr,  /* OUT */
  const char* pszMailbox      /* IN */
) {
  u_long addr;

  pszMailbox = strchr( pszMailbox, '@');
  if ( NULL == pszMailbox)
    return 0;
  ++pszMailbox;

  /* Try a simple IPv4 address */
  addr = inet_addr( pszMailbox);
  if ( INADDR_NONE != addr && INADDR_ANY != addr)
    {
    pAddr->sin_addr.s_addr = addr;
    }
  else
    {
    SWS_DnsQHandle hQuery;
    char szHost[ MAX_HOST] = "";
    ui16 pref = 0xffff;
    struct hostent* pHost;

    /* Lookup MX record for domain */
    hQuery = SWS_DnsOpenQuery(
      SWS_DNS_OP_QUERY | SWS_DNS_RD_MASK,
      pszMailbox,
      SWS_DNS_TYPE_MX, /* was SWS_DNS_QTYPE_STAR */
      SWS_DNS_CLASS_IN,
      INADDR_ANY,
      0
    );
    if ( NULL != hQuery)
      {
      SWS_SDnsQRecord rec;

      /* Search for MX records */
      while ( !SWS_DnsQueryRead( hQuery, &rec))
        {
        if ( SWS_kDnsAnswer == rec.eRecordType
          && SWS_DNS_CLASS_IN == rec.rclass
        ) {
          ui16 u1;

          switch ( rec.rtype)
            {
          case SWS_DNS_TYPE_MX:
            /* Get MX preference */
            SWS_DnsQueryData( &u1, sizeof( u1), hQuery, rec.dataIndex);
            u1 = ntohs( u1);
            if ( u1 < pref) /* Lower is preferred */
              {
              pref = u1;
              SWS_DnsQueryName(
                szHost, sizeof( szHost), hQuery, rec.dataIndex + 2);
              }
            break;

          case SWS_DNS_TYPE_A:
            /* If no MX then use this */
            if ( 0xffff == pref)
              SWS_DnsQueryName( szHost, sizeof( szHost), hQuery, rec.nameIndex);
            break;

          case SWS_DNS_TYPE_CNAME:
            /* If no MX then lookup alias */
            if ( 0xffff == pref)
              {
              SWS_DnsQueryName( szHost, sizeof( szHost), hQuery, rec.dataIndex);
              SWS_DnsCloseQuery( hQuery);

              hQuery = SWS_DnsOpenQuery(
                SWS_DNS_OP_QUERY | SWS_DNS_RD_MASK,
                szHost,
                SWS_DNS_QTYPE_STAR,
                SWS_DNS_CLASS_IN,
                INADDR_ANY,
                0
              );
              if ( NULL == hQuery)
                return 0; /* MX not found */

              szHost[ 0] = '\0';
              }
            break;

          default:
            break;
            }
          }
        }
      SWS_DnsCloseQuery( hQuery);
      }

    if ( '\0' == szHost[ 0])
      return 0; /* MX not found */

    pHost = gethostbyname( szHost);
    if ( NULL == pHost)
      return 0;

    pAddr->sin_addr = *(const struct in_addr*)pHost->h_addr_list[0];
    }

  return 1;
  }


/*
 * Convert a buffer to base64
 */
void MimeEncodeBase64( char szBuf[], size_t size, const unsigned char src[], size_t len)
  {
  enum { kBits = 6, kBase = 1 << kBits };
  static const char lut[ kBase + 1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz" "0123456789+/";

  while ( len && size >= 4)
    {
    unsigned long ulSum;

    /* Pack 3 bytes into 4 chars */
    ulSum = *src++;
    if ( --len)
      {
      ulSum <<= 8;
      ulSum |= *src++;
      if ( --len)
        {
        ulSum <<= 8;
        ulSum |= *src++;
        --len;

        szBuf[3] = lut[ (unsigned)(ulSum % kBase)];
        ulSum /= kBase;
        }
      else
        {
        ulSum <<= 3 * kBits - 16; /* Align 16 to 18 */
        szBuf[3] = '=';
        }

      szBuf[2] = lut[ (unsigned)(ulSum % kBase)];
      ulSum /= kBase;
      szBuf[1] = lut[ (unsigned)(ulSum % kBase)];
      ulSum /= kBase;
      }
    else
      {
      ulSum <<= 2 * kBits - 8; /* Align 8 to 12 */
      szBuf[3] = szBuf[2] = '=';
      szBuf[1] = lut[ (unsigned)(ulSum % kBase)];
      ulSum /= kBase;
      }

    szBuf[0] = lut[ (unsigned)(ulSum % kBase)];

    szBuf += 4;
    size -= 4;
    }

  if ( size)
    szBuf[ 0] = '\0';
  }


/*
 * Log a message
 */
static void Log(
  int iLevel,
  const struct SConn* pConn,
  const char szFormat[], ...
) {
  FILE* file;
  char szTime[ 32];
  time_t t;
  va_list ap;

  if ( kLogError == iLevel)
    file = stderr;
  else if ( kLogWarn == iLevel)
    file = (pConn && pConn->logfile) ? pConn->logfile : stderr;
  else if ( pConn && pConn->iVerbosity >= iLevel && pConn->logfile)
    file = pConn->logfile;
  else
    return;

  va_start( ap, szFormat);

  t = time( NULL);
  strftime( szTime, sizeof szTime, "%Y/%m/%d %H:%M:%S", localtime( &t));
  fprintf( file, "%s " kszPrefix, szTime);
  if ( NULL != pConn)
    fprintf( file, "%s:%u ", inet_ntoa( pConn->server.sin_addr), ntohs( pConn->server.sin_port));
  vfprintf( file, szFormat, ap);

  va_end( ap);
  }

/* End of file */
