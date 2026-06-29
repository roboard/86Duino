/*****************************************************************************
 *
 * File name   : httpcgi.c
 * Function    : web server cgi RFC 3875
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 07-Dec-09, LVR, v1.00 - Created.
 *
 *****************************************************************************/

/* Compilation options */
#ifndef OPT_ENVIRON
#define OPT_ENVIRON 1 /* !0 to use environ global */
#endif
#ifndef OPT_STDIN
#define OPT_STDIN 1 /* !0 to use dup2 to restore stdin after pclose */
#endif

/* Exports */
#include "httpcgi.h"

/* Imports */

/* ANSI C library */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#if OPT_STDIN
# if defined WIN32 || defined _WIN32 || defined __WIN32__ || defined _DOS || defined _DOS16
#  include <io.h> /* _dup2 */
#  define popen  _popen
#  define pclose _pclose
# else
#  include <unistd.h> /* dup, dup2 */
# endif
#endif

#if defined WIN32 || defined _WIN32 || defined __WIN32__ || defined WIN16 \
 || defined __MINGW32__\
 || defined _MSDOS || defined _DOS || defined _DOS16 || defined __DJGPP__
#include <malloc.h> /* for alloca */
#endif

#include "httpd.h"
#include "httpfile.h"
#include "sws_cfg.h"
#include "sws_str.h"  /* SWS_strnicmp */


/***************************************************************************
 * Macros & constants
 ***************************************************************************/

/* Length of an array - sorely missing from stddef.h */
#ifndef lengthof
#define lengthof( _a) ((size_t)( sizeof(_a)/sizeof((_a)[0]) ))
#endif

/* Expand and stringize a macro name */
#define STR(s) #s
#define STRING(s) STR(s)

/* Compile time assertion
 * Generates 'invalid array dimension' compiler error if condition false
 */
#undef STATIC_ASSERT
#ifdef __GNUC__
# define STATIC_ASSERT(e) extern char _dummy[(e) ? 1 : -1] __attribute__((unused))
#else
# define STATIC_ASSERT(e) extern char _dummy[(e) ? 1 : -1]
#endif

#if defined WIN32 || defined _WIN32 || defined __WIN32__
# define MAX_CMDLINE 1024
#elif defined _MSDOS || defined _DOS || defined _DOS16 || defined WIN16
# define MAX_CMDLINE 128
#else
# define MAX_CMDLINE HTTP_BUFFER_SIZE
#endif

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

#define L_UNIQUE 48

#define INVALID_TIME ((time_t)-1)
#define LBLEN 511 /* Stack based buffer size */
#define LBLENS STRING(LBLEN)

#define CRLF "\x0d\x0a"
#define kszUpDir ".."
#define kszCgiPrefix "/cgi-bin" /* default URI prefix for CGI */

/* ini file values */
#define kszCgiRoot    "cgi_root"
#define kszAuthorize  "cgi_authorize"


/***************************************************************************
 * Data types
 ***************************************************************************/

typedef struct SContext
  {
  char szTmpFile[ L_UNIQUE];
  FILE* fileBody;
  FILE* filePipe;
  int isText;
  } SContext;
STATIC_ASSERT( sizeof( SContext) <= sizeof( ((SHttpRequest*)0))->reserve);


/***************************************************************************
 * Function prototypes
 ***************************************************************************/

static FHttpMethod HttpCgiGet;
static FHttpMethod HttpCgiPost;
static FHttpMethod Forbidden;
static FHttpMethod Options;
static FHttpMethod CgiRecvBody;
static FHttpMethod CgiSendBody;

static void UriToPathName( char*, const char*, const char*);
static char* UniqueName( char [ L_UNIQUE] );
static int CgiProcess( SHttpConnection*, SHttpRequest*);
static int CgiSetup( SHttpConnection*, SHttpRequest*);

static SWS_CB( int, EnumRestricted( void* pv, const char* pszVal));

static int PlatformIsDir( const char*);
static int PlatformChdir( const char*, char*, size_t);
#if defined WIN32 || defined _WIN32 || defined __WIN32__ || defined WIN16 \
 || defined __MINGW32__\
 || defined _MSDOS || defined _DOS || defined _DOS16 || defined __DJGPP__
static int Setenv( const char *envname, const char *envval, int overwrite);
#define setenv Setenv
static int Unsetenv( const char *envname);
#define unsetenv Unsetenv
#endif


/***************************************************************************
 * Module data
 ***************************************************************************/

#if OPT_ENVIRON
# ifdef __cplusplus
extern "C" {
# endif /* __cplusplus */
extern char** environ;
# ifdef __cplusplus
}
# endif /* __cplusplus */
#endif

static char s_szRoot[ MAX_PATH]; /* File system root */
static char s_szCgiBin[ MAX_PATH]; /* Filesystem path prefix for all CGI scripts */

/* Table of known URI actions */
static const SHttpAction actionArray[] = {
  { kszCgiPrefix "/?*",
    {
      {kszHttpGet,     &HttpCgiGet, NULL},
      {kszHttpHead,    &HttpCgiGet, NULL},
      {kszHttpPut,     &Forbidden, NULL},
      {kszHttpDelete,  &Forbidden, NULL},
      {kszHttpPost,    &HttpCgiPost, NULL},
      {kszHttpOpts,    &Options, NULL}
    }
  }
};


/***************************************************************************
 * Functions
 ***************************************************************************/

/*
 * Install CGI actions
 */
int HttpCgiInit( void)
  {
  char szBuf[ sizeof s_szRoot];

  /* CGI filesystem root */
  if ( 0 == SWS_CfgGetString( kszHttpSect, kszHttpRoot, NULL, s_szRoot, sizeof s_szRoot))
    PlatformChdir( NULL, s_szRoot, sizeof s_szRoot);
  SWS_CfgGetString( kszHttpSect, kszCgiRoot, "", s_szCgiBin, sizeof s_szCgiBin);

  /* Enum stems that require authorization */
  SWS_CfgEnum( kszHttpSect, &EnumRestricted, NULL);

  HttpActions( actionArray, lengthof( actionArray) );

  /* Verify cgi-bin exists */
  strcat( strcpy( szBuf, s_szRoot), s_szCgiBin);
  return PlatformIsDir( szBuf);
  }


/*
 * Process a GET or HEAD request
 */
int HttpCgiGet(
  SHttpConnection* pConn,
  SHttpRequest* pRqst
) {
  SContext* const pContext = (SContext*)pRqst->reserve;
  int iRet;

  if ( NULL == pConn)
    return HTTPD_BAD_REQUEST;

  if ( NULL != pRqst->pszParam)
    return HttpReply( pConn, HTTPD_BAD_REQUEST, "Unexpected parameters", 0, NULL);

  if ( pRqst->lContent > 0)
    return HttpReply( pConn, HTTPD_BAD_REQUEST, "Unexpected content", 0, NULL);

  pContext->isText = 0;

  /* Setup EV's from headers */
  if ( 0 != (iRet = CgiSetup( pConn, pRqst)))
    return iRet;

  pContext->fileBody = NULL;
  return CgiProcess( pConn, pRqst);
  }


/*
 * Process a POST request
 */
int HttpCgiPost(
  SHttpConnection* pConn,
  SHttpRequest* pRqst
) {
  SContext* const pContext = (SContext*)pRqst->reserve;
  int iRet;

  if ( NULL == pConn)
    return 0;

  if ( NULL != pRqst->pszParam)
    return HttpReply( pConn, HTTPD_BAD_REQUEST, "Unexpected parameters", 0, NULL);

  if ( pRqst->lContent > 30000000L)
    return HttpReply( pConn, HTTPD_BAD_REQUEST, "Content too big", 0, NULL);

  pContext->isText = 0;

  /* Setup EV's from headers */
  if ( 0 != (iRet = CgiSetup( pConn, pRqst)))
    return iRet;

  if ( pRqst->lContent > 0)
    {
    /* Create a temp file to receive the post data */
    if ( NULL == UniqueName( pContext->szTmpFile))
      return HttpReply( pConn, HTTPD_INTERN_ERROR, "No resources", 0, NULL);

    pContext->fileBody = fopen( pContext->szTmpFile, "wb");
    if ( NULL == pContext->fileBody)
      {
      HttpLog( kHttpWarn, pConn, "WARN %d: HttpCgiPost fopen(%s): %s\n",
        errno, pContext->szTmpFile, strerror( errno));
      return HttpReply( pConn, HTTPD_INTERN_ERROR, "No resources", 0, NULL);
      }

    /* Recv the post data */
    return HttpRecv( pConn, &CgiRecvBody);
    }

  pContext->fileBody = NULL;
  return CgiProcess( pConn, pRqst);
  }


/*
 * Receive request body
 */
int CgiRecvBody(
  SHttpConnection* pConn,
  SHttpRequest* pRqst
) {
  SContext* const pContext = (SContext*)pRqst->reserve;
  int iRet;

  if ( NULL == pConn)
    {
    /* Abort */
    iRet = 0;
    }
  else if ( 0 == pRqst->sizeAvail)
    {
    /* Body complete */
    if ( 0 == fflush( pContext->fileBody) )
      iRet = CgiProcess( pConn, pRqst);
    else
      {
      HttpLog( kHttpError, pConn, "ERROR %d: CgiRecvBody fflush: %s\n", errno, strerror( errno));
      iRet = HttpReply( pConn, HTTPD_INTERN_ERROR, "No resources", 0, NULL);
      }
    }
  /* Save the content */
  else if ( pRqst->sizeAvail != fwrite( pRqst->buffer, 1, pRqst->sizeAvail, pContext->fileBody))
    {
    /* Write error */
    HttpLog( kHttpError, pConn, "ERROR %d: CgiRecvBody fwrite: %s\n", errno, strerror( errno));
    iRet = HttpReply( pConn, HTTPD_INTERN_ERROR, "No resources", 0, NULL);
    }
  else
    return HTTPD_OK;

  if ( NULL != pContext->fileBody)
    {
    if ( fclose( pContext->fileBody))
      HttpLog( kHttpError, pConn, "ERROR %d: CgiRecvBody fclose: %s\n", errno, strerror( errno));
    pContext->fileBody = NULL;
    if ( remove( pContext->szTmpFile))
      HttpLog( kHttpError, pConn, "ERROR %d: CgiRecvBody remove: %s\n", errno, strerror( errno));
    }
  return iRet;
  }


/*
 * Setup EV's
 */
int CgiSetup( SHttpConnection* pConn, SHttpRequest* pRqst)
  {
  char szBuf[ LBLEN + 1], szPrefix[64], szName[ 64], *pszName;
  const char* pszVal;
  int n;
  unsigned u;

  if ( pRqst->lContent > 0)
    {
    sprintf( szBuf, "%ld", pRqst->lContent);
    setenv( "CONTENT_LENGTH", szBuf, 1);

    if ( NULL != (pszVal = strstr( pRqst->pszHeaders, kszHttpContentType ":")))
      {
      (void)sscanf( pszVal, kszHttpContentType ": %"LBLENS"[^"CRLF"]", szBuf);
      setenv( "CONTENT_TYPE", szBuf, 1);
      }
    else
      unsetenv( "CONTENT_TYPE");
    }
  else
    {
    unsetenv( "CONTENT_LENGTH");
    unsetenv( "CONTENT_TYPE");
    }

  setenv( "GATEWAY_INTERFACE", "CGI/1.1", 0); /* RFC3875 */

  /* Parse the request into action prefix and script suffix */
  if ( 2 == sscanf( pRqst->pszPath, "/%63[^/]/%*[^/]%"LBLENS"[^"CRLF"]", szPrefix, szBuf))
    {
    size_t len;

    HttpDecodeString( szBuf, szBuf);
    if ( NULL != strstr( szBuf, kszUpDir))
      return HttpReply( pConn, HTTPD_FORBIDDEN, "Forbidden .. in path", 0, NULL);

    len = strlen( szBuf);
    while ( len-- > 0)
      {
      if ( !isgraph( szBuf[ len]))
        return HttpReply( pConn, HTTPD_FORBIDDEN, "Forbidden character in path", 0, NULL);
      }

    setenv( "PATH_INFO", szBuf, 1);

    len = strlen( szBuf) + 1;
    if ( 0 == HttpFilePath( szBuf + len, szBuf, sizeof szBuf - len))
      setenv( "PATH_TRANSLATED", szBuf + len, 1);
    else
      return HttpReply( pConn, HTTPD_BAD_REQUEST, "URI too long", 0, NULL);
    }
  else
    {
    unsetenv( "PATH_INFO");
    unsetenv( "PATH_TRANSLATED");
    }

  if ( NULL != pRqst->pszQuery && '\0' != *pRqst->pszQuery)
    setenv( "QUERY_STRING", pRqst->pszQuery, 1);
  else
    setenv( "QUERY_STRING", "", 1);

  setenv( "REMOTE_ADDR", HttpClientAddr( pConn), 1);
#if 0
  setenv( "REMOTE_HOST", HttpClientAddr( pConn), 1);
#endif

  if ( NULL != (pszVal = strstr( pRqst->pszHeaders, kszHttpAuthorization ":")))
    {
    char szAuth[ 64];

    (void)sscanf( pszVal, kszHttpAuthorization ": %63s %"LBLENS"s", szAuth, szBuf);
    setenv( "AUTH_TYPE", szAuth, 1);
    setenv( "REMOTE_USER", szBuf, 1);
    }
  else
    {
    unsetenv( "AUTH_TYPE");
    unsetenv( "REMOTE_USER");
    }

  setenv( "REQUEST_METHOD", pRqst->pszMethod, 1);

  /* Parse script name from request */
  pszName = szBuf + sprintf( szBuf, "/%s/", szPrefix);
  if ( 1 == sscanf( pRqst->pszPath, "/%*[^/]/%"LBLENS"[^/]", pszName))
    HttpDecodeString( pszName, pszName);
  setenv( "SCRIPT_NAME", szBuf, 1);

  u = HTTPD_PORT;
  pszVal = strstr( pRqst->pszHeaders, kszHttpHost":");
  if ( NULL != pszVal)
    pszVal += sizeof kszHttpHost":" -1;
  if ( NULL != pszVal && 1 <= sscanf( pszVal, " %"LBLENS"[^:"CRLF"]:%u", szBuf, &u))
    {
    setenv( "SERVER_NAME", szBuf, 1);
    sprintf( szBuf, "%u", u);
    setenv( "SERVER_PORT", szBuf, 1);
    }
  else
    {
    unsetenv( "SERVER_NAME");
    unsetenv( "SERVER_PORT");
    }

  setenv( "SERVER_PROTOCOL", "HTTP/1.0", 0);
  setenv( "SERVER_SOFTWARE", "sws_cgi/1.0", 0);

#if OPT_ENVIRON
  /* Unset all HTTP EV's */
  for ( pszName = environ[ n = 0]; NULL != pszName;  pszName = environ[ ++n])
    {
    if ( 0 == strncmp( pszName, "HTTP_", 5))
      {
      (void)sscanf( pszName, "%"LBLENS"[^=]", szBuf);
      unsetenv( szBuf);
      if ( pszName != environ[ n])
        --n;
      }
    }
#endif

  /* Parse HTTP headers and convert to HTTP_XXX_XXX form EV's */
  pszVal = pRqst->pszHeaders;
  pszName = szName + sprintf( szName, "HTTP_");
  while ( n = 0, 2 <= sscanf( pszVal, " %58[^:]: %"LBLENS"[^"CRLF"]%n", pszName, szBuf, &n)
    && n > 0
  ) {
    char* psz;

    pszVal += n;

    /* Uppercase name and convert - to _ */
    for ( psz = pszName; *psz; ++psz)
      {
      if ( '-' == *psz)
        *psz = '_';
      else
        *psz = (char)toupper( *psz);
      }
    setenv( szName, szBuf, 1);
    }

#if 1 /* Common extras */
  if ( 0 == HttpFilePath( szBuf, "", sizeof szBuf))
    setenv( "DOCUMENT_ROOT", szBuf, 0);

  sprintf( pRqst->buffer, "%s%s%s%s%s", pRqst->pszPath,
    NULL != pRqst->pszQuery ? "?" : "", NULL != pRqst->pszQuery ? pRqst->pszQuery : "",
    NULL != pRqst->pszParam ? "?" : "", NULL != pRqst->pszParam ? pRqst->pszParam : ""
  );
  setenv( "REQUEST_URI", pRqst->buffer, 1);

  sprintf( szBuf, "%u", HttpClientPort( pConn));
  setenv( "REMOTE_PORT", szBuf, 1);
  setenv( "SERVER_ADDR", HttpServerAddr( pConn), 1);
  setenv( "REDIRECT_STATUS", "OK", 0);
#endif

  return 0;
  }


int CgiProcess(
  SHttpConnection* pConn,
  SHttpRequest* pRqst
) {
  SContext* const pContext = (SContext*)pRqst->reserve;
  int iStatus = HTTPD_OK, len;
#if OPT_STDIN
  int cstdin = -1;
#endif
  char szBuf[ LBLEN + 1], szCwd[ MAX_PATH];
  char* psz;
  const char* pszQuery;

  /* Get script pathname */
  szBuf[0] = '\0';
  (void)sscanf( pRqst->pszPath, "/%*[^/]/%"LBLENS"[^/]", szBuf);
  UriToPathName( pRqst->buffer, szBuf, HttpRealmPath( pConn));

  /* Disallow .. in pathname */
  if ( strstr( pRqst->buffer, kszUpDir))
    return HttpReply( pConn, HTTPD_FORBIDDEN, "Forbidden .. in URI", 0, NULL);

  len = 0;
  (void)sscanf( pRqst->buffer, "%*[/a-zA-Z0-9_.-]%n", &len);
  if ( strlen( pRqst->buffer) != (size_t)len)
    return HttpReply( pConn, HTTPD_FORBIDDEN, "Forbidden characters in URI", 0, NULL);

#if defined WIN32 || defined _WIN32 || defined __WIN32__ || defined WIN16 \
 || defined __MINGW32__\
 || defined _MSDOS || defined _DOS || defined _DOS16 || defined __DJGPP__
  /* Windows & MS-DOS require .exe/com/bat extension */
  if ( NULL != (psz = strstr( pRqst->buffer, ".cgi"))
    && '\0' == psz[4]
  ) {
    *psz = '\0';
    }
#else
  /* Verify that the cgi program exists */
  { FILE* file = fopen( pRqst->buffer, "r");
  if ( NULL == file)
    {
    HttpLog( kHttpWarn, pConn, "WARN %d: CgiProcess fopen(%s): %s\n",
      errno, pRqst->buffer, strerror( errno));
    return HttpReply( pConn, HTTPD_NOT_FOUND, "CGI not found", 0, NULL);
    }
  fclose( file); }
#endif

#if 1 /* For PHP */
  setenv( "SCRIPT_FILENAME", pRqst->buffer, 1);
#endif

  /* Is there any post data? */
  if ( NULL != pContext->fileBody)
    {
#if OPT_STDIN
    /* Save current stdin */
    cstdin = dup( fileno( stdin) );
#endif

    /* Redirect stdin to read the post data */
    if ( NULL == freopen( pContext->szTmpFile, "r", stdin))
      {
      HttpLog( kHttpError, pConn, "ERROR %d: CgiProcess freopen(%s): %s\n",
        errno, pContext->szTmpFile, strerror( errno));
#if OPT_STDIN
      /* Close stdin and restore it */
      if ( -1 != cstdin)
        dup2( cstdin, fileno( stdin) );
#endif
      return HttpReply( pConn, HTTPD_INTERN_ERROR, "No resources", 0, NULL);
      }
    rewind( stdin);
    }

  /* Change working directory to that containing the script */
  strncpy( szBuf, pRqst->buffer, sizeof szBuf);
  if ( NULL != (psz = (char*)strrchr( szBuf, '/')))
    *psz = '\0';
  if ( PlatformChdir( szBuf, szCwd, sizeof szCwd))
    HttpLog( kHttpError, pConn, "ERROR %d: CgiProcess chdir(%s): %s\n", errno, szBuf, strerror( errno));

  /* Add command line args RFC 3875 4.4 */
  if ( NULL != (pszQuery = pRqst->pszQuery)
    && NULL == strchr( pszQuery, '=')
    && len + strlen( pszQuery) < MAX_CMDLINE
  ) {
    char* pszArgs = pRqst->buffer + len;
    int n;

    while ( 1 <= sscanf( pszQuery, "%"LBLENS"[^+]%n", szBuf, &n))
      {
      pszQuery += n;
      if ( '\0' != *pszQuery)
        ++pszQuery; /* step over + */

      HttpDecodeString( szBuf, szBuf);
      pszArgs += sprintf( pszArgs, " %s", szBuf);
      }
    }

  /* Start the cgi program */
  HttpLog( kHttpMore, pConn, "CgiProcess popen(%s)\n", pRqst->buffer);
  pContext->filePipe = popen( pRqst->buffer, "r");
  if ( NULL != pContext->filePipe)
    {
    char szStatus[ 64] = "OK";
    int iLines, isNph = 0;
    char* pszHdr;
    long lContent = -1;

    *(pszHdr = pRqst->buffer) = '\0';

    /* Parse cgi headers */
    for ( iLines = 0; NULL != fgets( szBuf, sizeof szBuf, pContext->filePipe); ++iLines)
      {
      static const char kszType[] = "Content-Type:";
      static const char kszLocn[] = "Location:";
      static const char kszStat[] = "Status:";

      /* Test for end of headers */
      if ( 0 == strcmp( szBuf, "\n") || 0 == strcmp( szBuf, CRLF))
        break;

      /* Strip any trailing newline */
      szBuf[ strcspn( szBuf, CRLF)] = '\0';
      HttpLog( kHttpMost, pConn, "CgiProcess header:%s\n", szBuf);

      if ( pszHdr + strlen( szBuf) > pRqst->buffer + sizeof pRqst->buffer)
        continue;

      if ( 0 == iLines
        && 3 <= sscanf( szBuf, "HTTP/%u.%u %d %63[^"CRLF"]",
          &pRqst->uMajor, &pRqst->uMinor, &iStatus, szStatus)
      ) {
        /* Non-parsed header */
        isNph = 1;
        }
      else if ( isNph)
        {
        pszHdr += sprintf( pszHdr, "%s" CRLF, szBuf);
        }
      /* Check for CGI headers */
      else if ( 0 == SWS_strnicmp( szBuf, kszType, sizeof kszType -1))
        {
        psz = &szBuf[ sizeof kszType -1];
        while ( isspace( *psz))
          ++psz;

        if ( 0 == strncmp( "text/", psz, 5))
          pContext->isText = 1;

        pszHdr += sprintf( pszHdr, kszHttpContentType ":%s" CRLF, psz);
        }
      else if ( 0 == SWS_strnicmp( szBuf, kszLocn, sizeof kszLocn -1))
        {
        psz = &szBuf[ sizeof kszLocn -1];
        while ( isspace( *psz))
          ++psz;

        if ( '/' != *psz)
          {
          /* Client re-direction */
          pszHdr += sprintf( pszHdr, kszHttpLocation ":%s" CRLF, psz);
          if ( HTTPD_OK == iStatus)
            {
            /* Temporary re-direction */
            iStatus = HTTPD_FOUND;
            strcpy( szStatus, "Found");
            }
          }
        else
          {
          int err;
          /* Server re-direction
           * Wait for program completion */
          HttpLog( kHttpMore, pConn, "CgiProcess calling pclose\n");
          err = pclose( pContext->filePipe);
          pContext->filePipe = NULL;
          if ( !err)
            {
            HttpLog( kHttpMore, pConn, "CgiProcess pclose OK\n");
            iStatus = HttpRedirect( pConn, psz);
            }
          else
            {
            HttpLog( kHttpWarn, pConn, "WARN %#x: CgiProcess pclose\n", err);
            sprintf( pRqst->buffer, "Status %#x", err);
            iStatus = HttpReply( pConn, HTTPD_INTERN_ERROR, pRqst->buffer, 0, NULL);
            }
          goto done;
          }
        }
      else if ( 0 == SWS_strnicmp( szBuf, kszStat, sizeof kszStat -1))
        {
        psz = &szBuf[ sizeof kszStat -1];
        (void)sscanf( psz, "%d %63[^"CRLF"]", &iStatus, szStatus);
        }
      /* Check for 'special' HTTP headers */
      else if ( 0 == SWS_strnicmp( szBuf, kszHttpContentLen ":", sizeof kszHttpContentLen))
        {
        (void)sscanf( szBuf + sizeof kszHttpContentLen, "%ld", &lContent);
        }
      else if ( 0 == SWS_strnicmp( szBuf, kszHttpConnection ":", sizeof kszHttpConnection))
        {
        /* Discard */
        }
      /* CGI extension header? */
      else if ( strncmp( szBuf, "X-CGI-", 6))
        {
        /* No, assume it's an HTTP header and forward it */
        pszHdr += sprintf( pszHdr, "%s" CRLF, szBuf);
        }
      }

    HttpLog( kHttpMore, pConn, "CgiProcess header complete\n");
    if ( NULL != pContext->filePipe)
      {
      pRqst->uMajor = 1, pRqst->uMinor = 0;
      HttpReply( pConn, iStatus, szStatus, lContent, pRqst->buffer);

      /* Send the body */
      CgiSendBody( pConn, pRqst);
      }
    }
  else
    {
    HttpLog( kHttpError, pConn, "ERROR %d: CgiProcess popen(%s): %s\n",
      errno, pRqst->buffer, strerror( errno));
    iStatus = HttpReply( pConn, HTTPD_INTERN_ERROR, "CGI failure", 0, NULL);
    }

done:
  /* Restore working directory */
  PlatformChdir( szCwd, NULL, 0);

#if OPT_STDIN
  if ( -1 != cstdin)
    {
    /* Close stdin and restore it */
    dup2( cstdin, fileno( stdin) );
    rewind( stdin);
    }
#else
  if ( NULL != pContext->fileBody)
    fclose( stdin);
#endif

  return iStatus;
  }


int CgiSendBody(
  SHttpConnection* pConn,
  SHttpRequest* pRqst
) {
  SContext* const pContext = (SContext*)pRqst->reserve;
  int err;

  if ( pConn)
    {
    size_t len;

    if ( NULL == pContext->filePipe)
      len = 0;
    else
      {
      /* Would like to poll for data but select never returns ready */
#if 0 && (defined _POSIX_SOURCE || defined _POSIX_C_SOURCE)
      static fd_set setRd;
      struct timeval tval;
      const int fd = fileno( pContext->filePipe);

      FD_ZERO( &setRd);
      FD_SET( fd, &setRd);

      /* Set timeout for select */
      tval.tv_usec = 1000000;
      tval.tv_sec = 0;

      /* Wait for pipe ready */
      switch ( select( 1, &setRd, NULL, NULL, &tval))
        {
      case -1: /* Signal etc */
        HttpLog( kHttpError, pConn, "CgiSendBody select: %s\n", strerror( errno));
        return HttpSend( pConn, pRqst->buffer, 0, &CgiSendBody);
      case 0:
        HttpLog( kHttpInfo, pConn, "CgiSendBody pipe not ready\n");
        return HttpSend( pConn, pRqst->buffer, 0, &CgiSendBody);
      default:
        HttpLog( kHttpInfo, pConn, "CgiSendBody pipe ready\n");
        break;
        }
#endif

      if ( !pContext->isText)
        len = fread( pRqst->buffer, 1, sizeof pRqst->buffer, pContext->filePipe);
      else if ( NULL != fgets( pRqst->buffer, sizeof pRqst->buffer, pContext->filePipe))
        len = strlen( pRqst->buffer);
      else
        len = 0;

      if ( len || !feof( pContext->filePipe))
        return HttpSend( pConn, pRqst->buffer, len, &CgiSendBody);
      }
    }

  /* Wait for cgi program completion */
  HttpLog( kHttpMore, pConn, "CgiSendBody calling pclose\n");
  err = pclose( pContext->filePipe);
  pContext->filePipe = NULL;
  if ( err)
    HttpLog( kHttpWarn, pConn, "WARN %#x: CgiSendBody pclose\n", err);
  else
    HttpLog( kHttpMore, pConn, "CgiSendBody pclose OK\n");

  /* Close the connection */
  return HttpSend( pConn, NULL, 0, 0);
  }


/*
 * Handle a forbidden request
 */
int Forbidden(
  SHttpConnection* pConn,
  SHttpRequest* pRqst
) {
  (void)pRqst;
  return HttpReply( pConn, HTTPD_FORBIDDEN, "Forbidden", 0, NULL);
  }


/*
 * OPTIONS request *
 */
int Options(
  SHttpConnection* pConn,
  SHttpRequest* pRqst
) {
  pRqst->iConnClose = 1;
  return HttpReply( pConn, HTTPD_OK, "OK", 0, kszHttpAllow
    ": " kszHttpGet
    ", " kszHttpHead
    ", " kszHttpPost
    ", " kszHttpOpts
    CRLF
  );
  }


/*
 * Convert URI to local pathname
 */
void UriToPathName( char* pszBuf, const char* pszPath, const char* pszPrefix)
  {
  /* Form local pathname ending with a single slash */
  pszBuf += sprintf( pszBuf, "%s%s/", s_szRoot, NULL == pszPrefix ? s_szCgiBin : pszPrefix);

  /* Remove all leading slashes from the URI */
  while( '/' == *pszPath)
    ++pszPath;

  /* Append URI to local pathname */
  HttpDecodeString( pszBuf, pszPath);
  }


/*
 * Generate a unique filename
 */
static char* UniqueName( char szBuf[ L_UNIQUE] )
  {
  static unsigned long s_u;
  long i;
  const char* pszPath;

  if ( ( NULL == (pszPath = getenv( "TMPDIR"))
      && NULL == (pszPath = getenv( "KDETMP"))
      && NULL == (pszPath = getenv( "TMP"))
      && NULL == (pszPath = getenv( "TEMP")) )
    || strlen( pszPath) + 10 > L_UNIQUE
  )
    pszPath = ".";

  for ( i = 0; i < 1000000L; ++i)
    {
    FILE* f;

    sprintf( szBuf, "%s/CGI%05lu", pszPath, s_u++);
    f = fopen( szBuf, "r");
    if ( NULL == f)
      return szBuf;

    fclose( f);
    }
  errno = ENFILE;
  return NULL;
  }


/*
 * SWS_CfgEnum callback for restricted realms
 */
SWS_CB( int, EnumRestricted( void* pv, const char* pszVal))
  {
  char szStem[ 256], szAuth[ 32];
  int n;
  (void)pv;

  /* Parse entry */
  if ( 2 <= (n = sscanf( pszVal, " "kszAuthorize" = %255s %31[^ ;]",
      szStem, szAuth))
  ) {
    SHttpAction action = { NULL,
      {
        {kszHttpGet,     &HttpCgiGet, NULL},
        {kszHttpHead,    &HttpCgiGet, NULL},
        {kszHttpPut,     &Forbidden, NULL},
        {kszHttpDelete,  &Forbidden, NULL},
        {kszHttpPost,    &HttpCgiPost, NULL},
        {kszHttpOpts,    &Options, NULL}
      }
    };
    size_t size = strlen( szStem) + 1;
    size_t size2 = strlen( szAuth) + 1;
    /* NB this memory is never freed */
    char* psz = (char*) malloc( size + size2);
    if ( NULL == psz)
      {
      HttpLog( kHttpError, NULL, "ERROR %d: EnumRestricted malloc: %s\n",
        errno, strerror( errno));
      return -1;
      }

    action.pszStem = strcpy( psz, szStem);
    psz = (char*)strcpy( psz + size, szAuth);
    action.methods[0].pszRealm = psz; /* Get */
    action.methods[1].pszRealm = psz; /* Head */

    HttpActions( &action, 1);
    }

  return 0;
  }


/****************************************************************************
 * Platform specific code
 ****************************************************************************/

#if defined WIN32 || defined _WIN32 || defined __WIN32__ || defined WIN16 \
 || defined _MSDOS || defined _DOS || defined _DOS16
/****************************************************************************
 * Win32/16 & MSDOS
 ****************************************************************************/

#include <sys/stat.h> /* stat */
#include <direct.h>

#ifdef __WATCOMC__
#define _chdir chdir

#elif defined __MWERKS__
#define _S_IFDIR S_IFDIR

#endif


/*
 * Test if directory exists
 */
int PlatformIsDir( const char* pszPath)
  {
  struct _stat info;

  if ( _stat( pszPath, &info))
    return 0; /* Doesn't exist */

  return (info.st_mode & _S_IFDIR) ? 1 : 0;
  }


/*
 * Change working directory
 */
int PlatformChdir(
  const char* pszPath,
  char* pszCwd,
  size_t size
) {
  if ( NULL != pszCwd)
    _getcwd( pszCwd, size);
  return NULL != pszPath ? _chdir( pszPath) : 0;
  }


#elif defined _POSIX_SOURCE || defined _POSIX_C_SOURCE || defined __DJGPP__
/****************************************************************************
 * POSIX
 ****************************************************************************/

#include <sys/stat.h> /* stat */
#include <unistd.h>   /* chdir */


/*
 * Test if directory exists
 */
int PlatformIsDir( const char* pszPath)
  {
  struct stat info;

  if ( stat( pszPath, &info))
    return 0; /* Doesn't exist */

  return S_ISDIR( info.st_mode) ? 1 : 0;
  }


/*
 * Change working directory
 */
int PlatformChdir(
  const char* pszPath,
  char* pszCwd,
  size_t size
) {
  if ( NULL != pszCwd && NULL == getcwd( pszCwd, size))
    return -1;
  return NULL != pszPath ? chdir( pszPath) : 0;
  }


#else
/****************************************************************************
 * ANSI Standard C
 ****************************************************************************/

int PlatformIsDir( const char* pszPath)
  {
  {int WARN_PlatformIsDir_STUB;}
  return 0 == strcmp( s_szCgiBin, pszPath);
  }


int PlatformChdir( const char* pszPath, char* pszCwd, size_t size)
  {
  (void)pszPath;
  (void)pszCwd;
  (void)size;
  {int WARN_PlatformChdir_STUB;}
  return -1;
  }

#endif


#if defined WIN32 || defined _WIN32 || defined __WIN32__ || defined WIN16 \
 || defined __MINGW32__\
 || defined _MSDOS || defined _DOS || defined _DOS16 || defined __DJGPP__
static int Setenv( const char *envname, const char *envval, int overwrite)
  {
  char *pszOld, *pszNew;
  int ret;

  pszOld = getenv( envname);
  if ( NULL != pszOld && *pszOld && !overwrite)
    return -1; /* Exists, no overwrite */

  if ( NULL != pszOld && 0 == strcmp( pszOld, envval))
    return 0;

  pszNew = (char*)alloca( strlen( envname) + strlen( envval) + 2);
  if ( NULL == pszNew)
    return -1;

  sprintf( pszNew, "%s=%s", envname, envval);
  ret = putenv( pszNew);
  if ( ret)
    HttpLog( kHttpWarn, NULL, "WARN %d: Setenv(%s=%s): putenv: %s\n",
      errno, envname, envval, strerror( errno));

  return ret;
  }


static int Unsetenv( const char *envname)
  {
  char* psz = getenv( envname);
  if ( NULL != psz)
    {
    char szBuf[ 64];

    /* putenv("xxx") unsets the ev */
    strncpy( szBuf, envname, sizeof szBuf);
    if ( putenv( szBuf)
      || (NULL != (psz = getenv( envname)) && *psz)
    ) {
      /* For MSVC lib */
      strcat( szBuf, "=");
      if ( putenv( szBuf))
        {
        HttpLog( kHttpWarn, NULL, "WARN %d: Unsetenv(%s): putenv: %s\n",
          errno, envname, strerror( errno));
        *psz = '\0';
        }
      }
    }
  return 0;
  }
#endif

/* End of file */
