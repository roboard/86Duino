/*****************************************************************************
 *
 * File name   : loganaly.c
 * Function    : Http/ftp/smtp server log file report generator
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 19-Oct-09, LVR, v1.00 Created.
 * 10-Jan-10, LVR, v1.01 Added httpd URI stats
 *
 * Comments
 * --------
 *
 *****************************************************************************/
#define kszApp "loganaly"
#define kszVer "1.01"
#define kszBanner kszApp " " kszVer ", " TIMESTAMP "."
#define kszAbout "- SwsSock server log file analyser."

/* Compilation options */


/* Exports */

/* Imports */

/* ANSI C library */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <assert.h>
#include <limits.h>

#include "sws_str.h" /* SWS_StrMatch */


/***************************************************************************
 * Macros & constants
 ***************************************************************************/

#ifdef __TIMESTAMP__
# define TIMESTAMP "edited " __TIMESTAMP__
#else
# define TIMESTAMP "built " __TIME__ " " __DATE__
#endif

/* Expand and stringize a macro name */
#define STR(s) #s
#define STRING(s) STR(s)

#define lengthof(a) (sizeof(a) / sizeof((a)[0]))

#ifndef MAX_LINE
#define MAX_LINE 2048 /* Max length of a log line */
#endif

#define DEFAULT_COUNT 20 /* Default report size */

#define SIZE_PCOL 31
#define SIZE_DATE 10 /* yyyy/mm/dd */
#define SIZE_TIME 12 /* hh:mm:ss.ttt */
#define SIZE_ADDR 15 /* xxx.yyy.zzz.xxx */

#ifndef INITIAL_IPS
#define INITIAL_IPS   512 /* Initial IP table size */
#endif

/* Known log sources */
#define kszSMTP     "SMTPD"
#define kszSendmail "SENDMAIL"
#define kszPOP3     "POP3D"
#define kszFTP      "FTPD"
#define kszHTTP     "HTTPD"
#define kszIdent    "IDNTD"
#define kszTelnet   "TLNTD"
#define kszEcho     "ECHOD"
#define kszDiscard  "DSCRD"

/* Platforms */
#define kszWindowsNt7 "Windows NT 6.1"
#define kszWindows7 "Windows 7"
#define kszWindowsNtVista "Windows NT 6"
#define kszWindowsVista "Windows Vista"
#define kszWindowsNt2003 "Windows NT 5.2"
#define kszWindows2003 "Windows 2003"
#define kszWindowsNtXP "Windows NT 5.1"
#define kszWindowsXP "Windows XP"
#define kszWindowsNt2000 "Windows NT 5.0"
#define kszWindows2000 "Windows 2000"
#define kszWindowsNt4 "Windows NT 4" /* Normally "Windows NT 4.0" */
#define kszWindowsME "Windows ME"
#define kszWindows98 "Windows 98"
#define kszWindows95 "Windows 95"
#define kszWindowsCE "Windows CE"
#define kszWindows16 "Win16"

#define kszWebbot "Webbot"
#define ISBOT( p)\
  strstr( p, "http://") ||\
  SWS_stristr( p, "bot") ||\
  SWS_stristr( p, "crawl") ||\
  SWS_stristr( p, "spider") ||\
  SWS_stristr( p, "scan")


/***************************************************************************
 * Data types
 ***************************************************************************/

/* Type of log filter */
enum EFilter
  {
  kFilterNone,
  kFilterError,
  kFilterWarn,
  kFilterSmtp,
  kFilterSmtpOpen,
  kFilterSmtpHelo,
  kFilterSmtpAuth,
  kFilterSmtpFrom,
  kFilterSmtpTo,
  kFilterSmtpData,
  kFilterFtp,
  kFilterFtpPut,
  kFilterFtpGet,
  kFilterHttp,
  kFilterAll
  };

/* Parameters */
typedef struct SConfig
  {
  int iVerbosity;
  FILE* input;
  FILE* output;
  const char* pszProtocol;
  const char* pszFormat;
  const char* pszSearch;
  enum EFilter eFilter;
  unsigned long ulIpMask;
  unsigned long ulSrcAddr, ulSrcSubnetMask;
  unsigned long ulExSrcAddr, ulExSrcSubnetMask;
  int iCode, iCodeMax;
  unsigned uTopCount;
  } SConfig;

/* A dynamic list */
struct SArray {
  void* array;
  size_t size;            /* Currently allocated array size */
  unsigned long ulCount;  /* No. items in array */
  unsigned long ulTotal;  /* Total items added */
};
#define STAT_ITEM( T) struct SArray
#define STAT_COUNT( r) (r).ulCount
#define STAT_TOTAL( r) (r).ulTotal

/* Stats per IP address */
typedef struct SIpAddress
  {
  unsigned long ulAddr;
  unsigned long ulCount;
  unsigned long ulBytes;
  char szDate[ SIZE_DATE + 1], szTime[ SIZE_TIME + 1];
  } SIpAddress;

typedef struct SEcode
  {
  int iValue;
  unsigned long ulCount;
  } SEcode;

/* Stats per http URI */
typedef struct SUri
  {
  char szName[ 512];
  unsigned long ulCount;
  unsigned long ulBytes;
  } SUri;

/* Stats per http referer */
typedef struct SRef
  {
  char szName[ 512];
  unsigned long ulCount;
  } SRef;

/* Stats per http user-agent, platform, locale, method, accept-lang */
typedef struct SAgent
  {
  char szName[ 64];
  unsigned long ulCount;
  } SAgent;

/* http request info */
typedef struct SHttpRqst
  {
  const char* pszMethod;
  const char* pszURI;
  const char* pszUserAgent;
  const char* pszBrowser;
  const char* pszPlatform;
  const char* pszBrowseLang;
  const char* pszAcceptLang;
  const char* pszReferer;
  const char* pszVia;
  const char* pszFrom;
  int iResponse;
  const char* pszResponse;
  const char* pszRspHdrs;
  long lBytes;
  } SHttpRqst;

/* smtp info */
enum ESmtpType
  {
  kSmtpNone,
  kSmtpOpen, kSmtpHelo, kSmtpEhlo, kSmtpAuth, kSmtpFrom, kSmtpTo, kSmtpData, kSmtpQuit,
  kSmtpReset, kSmtpVerify, kSmtpHelp, kSmtpNoop, kSmtpUnknown
 };
typedef struct SSmtpRqst
  {
  int iResponse;
  enum ESmtpType eType;
  const char* pszArg;
  const char* pszInfo;
  unsigned long ulBytes;
  } SSmtpRqst;

/* Stats per mail address */
typedef struct SSmtpAddress
  {
  char szName[ 256];
  unsigned long ulCount;
  } SSmtpAddress;

/* Stats per smtp helo */
typedef struct SHostname
  {
  char szName[ 256];
  unsigned long ulCount;
  } SHostname;

/* Ftp info */
enum EFtpType
  {
  kFtpNone,
  kFtpOpen,
  kFtpUser, kFtpPass, kFtpAcct,
  kFtpCwd,  kFtpCdup,
  kFtpRmd,  kFtpMkd, kFtpPwd,
  kFtpRein, kFtpQuit,
  kFtpPort, kFtpPasv, kFtpType, kFtpStru, kFtpMode,
  kFtpRetr, kFtpStor, kFtpAppe, kFtpRest, kFtpabor,
  kFtpRnfr, kFtpRnto, kFtpDele,
  kFtpList, kFtpNlst,
  kFtpSite, kFtpSiteChmd, kFtpSiteHelp,
  kFtpSyst, kFtpStat, kFtpHelp,
  kFtpFeat, kFtpSize, kFtpMdtm,
  kFtpNoop, kFtpUnknown
  };
typedef struct SFtpRqst
  {
  int iResponse;
  enum EFtpType eType;
  const char* pszArg;
  unsigned long ulTxBytes, ulRxBytes;
  } SFtpRqst;
typedef struct SFtpFname
  {
  char szName[ 256];
  unsigned long ulCount;
  } SFtpFname;

/* Data gleaned from log */
typedef struct SStats
  {
  char szStartDate[ SIZE_DATE + 1], szEndDate[ SIZE_DATE + 1];
  char szStartTime[ SIZE_TIME + 1], szEndTime[ SIZE_TIME + 1];

  unsigned long ulProtocolPackets, ulMatchingPackets;

  size_t sizeIps;
  unsigned long ulIps;
  SIpAddress* ipAddressArray;

  STAT_ITEM( SEcode) ecode;
  STAT_ITEM( SAgent) prtcl;

  /* HTTP */
  STAT_ITEM( SAgent) method;
  STAT_ITEM( SUri) uri;
  STAT_ITEM( SRef) referer;
  STAT_ITEM( SAgent) agent;
  STAT_ITEM( SAgent) platform;
  STAT_ITEM( SAgent) locale;
  STAT_ITEM( SRef) via;
  STAT_ITEM( SAgent) accept;

  /* SMTP */
  STAT_ITEM( SHostname) hostname;
  STAT_ITEM( SSmtpAddress) mailfrom;
  STAT_ITEM( SSmtpAddress) mailto;
  unsigned long ulMailBytes;

  /* FTP */
  STAT_ITEM( SFtpFname) fread, fwrite;
  STAT_ITEM( SFtpFname) dread, dwrite;
  STAT_ITEM( SFtpFname) user, pass;
  unsigned long ulFtpTxBytes;
  unsigned long ulFtpRxBytes;
  } SStats;


/***************************************************************************
 * Local function prototypes
 ***************************************************************************/

#ifdef SIGINT
static void OnSigInt( int);
#endif

static int ParseArgs( SConfig* const, int, const char* const [] );
static void Help( const char* const);

static int Parse( SStats*, const SConfig*);
static int Report( FILE*, const SStats*, unsigned);

static int AddIpAddress( SStats*, unsigned long, unsigned long,
  const char*, const char*, unsigned long);
static int CmpIpCount( const void*, const void*);
static int CmpSEcode( const void*, const void*);

static int ParseHttpRequest( SHttpRqst*, const char* pszTail);
static int AddHttpRequest( SStats*, const SHttpRqst*);
static void DecodeUserAgent( SHttpRqst*, char*);
static int CmpSUri( const void*, const void*);
static int CmpSRef( const void*, const void*);
static int CmpSAgent( const void*, const void*);

static int ParseSmtpRequest( SSmtpRqst*, const char* pszTail);
static int AddSmtpRequest( SStats*, const SSmtpRqst*);
static int CmpSSmtpAddress( const void*, const void*);
static int CmpSHostname( const void*, const void*);

static int ParseFtpRequest( SFtpRqst*, const char* pszTail);
static int AddFtpRequest( SStats*, const SFtpRqst*);
static int CmpSFtpFname( const void*, const void*);

static unsigned MonthNum( const char*);


/***************************************************************************
 * Module data
 ***************************************************************************/

sig_atomic_t volatile g_exit;

/* Protocols */
struct SProtocol { const char* pszName; const char* pszId; enum EFilter eFilter; };
static const struct SProtocol kProtocols[] = {
  { "all",      NULL,       kFilterAll},
  { "error",    NULL,       kFilterError},
  { "warn",     NULL,       kFilterWarn},
  { "smtp",     kszSMTP,    kFilterSmtp},
  { "mail",     kszSMTP,    kFilterSmtp},
  { "mailopen", kszSMTP,    kFilterSmtpOpen},
  { "mailhelo", kszSMTP,    kFilterSmtpHelo},
  { "mailauth", kszSMTP,    kFilterSmtpAuth},
  { "mailfrom", kszSMTP,    kFilterSmtpFrom},
  { "mailto",   kszSMTP,    kFilterSmtpTo},
  { "mailin",   kszSMTP,    kFilterSmtpData},
  { "mailout",  kszSendmail, kFilterNone},
  { "pop3",     kszPOP3,    kFilterNone},
  { "ftp",      kszFTP,     kFilterFtp},
  { "ftpput",   kszFTP,     kFilterFtpPut},
  { "ftpget",   kszFTP,     kFilterFtpGet},
  { "http",     kszHTTP,    kFilterHttp},
  { "ident",    kszIdent,   kFilterNone},
  { "telnet",   kszTelnet,  kFilterNone},
  { "echo",     kszEcho,    kFilterNone},
  { "discard",  kszDiscard, kFilterNone}
};


/***************************************************************************
 * Functions
 ***************************************************************************/

/*
 * Show command line syntax
 */
void Help( const char* const pszArg)
  {
  const char* pszCmd;
  const char* pszDot;
  size_t c;
  unsigned u;

  fputs( kszBanner "\n" kszAbout "\n", stderr);

  /* Get command name without path */
  if ( NULL != pszArg)
    {
    pszCmd = strrchr( pszArg, '/');
    if ( NULL != pszCmd)
      ++pszCmd;
    else
      {
      pszCmd = strrchr( pszArg, '\\');
      if ( NULL != pszCmd)
        ++pszCmd;
      else
        pszCmd = pszArg;
      }
    }
  else
    {
    pszCmd = kszApp;
    }

  /* Count command name length less any '.' extension */
  pszDot = strrchr( pszCmd, '.');
  if ( NULL != pszDot)
    c = (size_t)(pszDot - pszCmd);
  else
    c = strlen( pszCmd);

  fprintf( stderr,
    "\nSyntax: %.*s [options] [log file]\n"
    "-i<s> : Source IP address filter e.g. 192.168.0.2/24\n"
    "-x<s> : Source IP address exclusion filter e.g. 192.168.0.2/24\n"
    "-m<m> : CIDR mask (1..32)\n"
    "-r<s> : Protocol [",
    (int)c, pszCmd
  );
  for ( u = 0; u < lengthof( kProtocols); ++u)
    printf( "%s%s", u ? "|" : "", kProtocols[ u].pszName);
  puts( "]\n"
    "-f<s> : Protocol tail wildcard filter e.g. \"user*@my???.com\"\n"
    "-s<s> : Protocol tail filter e.g. user@my.com\n"
    "-e<n> : Error code filter e.g. 200 or 400-599\n"
    "-n<n> : Max number of entries in result table ("STRING(DEFAULT_COUNT)")\n"
    "-     : Read log file from stdin (filter)\n"
    "-o<s> : Set output file\n"
    "-t    : Decrease verbosity of output\n"
    "-v    : Increase verbosity of output\n"
    "-V    : Show version and exit\n"
    "-h    : Help"
  );
  }


/*
 * M A I N
 */
int main( int argc, const char* argv[] )
  {
  SConfig cfg;
  int iRet;
  SStats stats;

  /* Set default options */
  cfg.iVerbosity = 1;
  cfg.input = NULL;
  cfg.output = stdout;
  cfg.pszProtocol = NULL;
  cfg.eFilter = kFilterAll;
  cfg.pszFormat = NULL;
  cfg.pszSearch = NULL;
  cfg.ulIpMask = 0xffffffffL;
  cfg.ulSrcAddr = 0;
  cfg.ulSrcSubnetMask = 0;
  cfg.ulExSrcAddr = 0;
  cfg.ulExSrcSubnetMask = 0;
  cfg.uTopCount = DEFAULT_COUNT;
  cfg.iCode = cfg.iCodeMax = -1;

  /* Parse args */
  iRet = ParseArgs( &cfg, argc, argv );
  if ( EXIT_SUCCESS != iRet)
    return iRet;

  if ( NULL == cfg.input
#if 0
    && NULL == (cfg.input = fopen( "/var/log/sws_inetd.log", "r"))
#endif
  ) {
    fputs( "No log file specified\n", stderr);
    return EXIT_FAILURE;
    }

#ifdef SIGINT
  /* Register a handler for ctrl-c to ensure a clean exit */
  signal( SIGINT, OnSigInt);
#endif

  /* Parse the log */
  memset( &stats, 0, sizeof stats);
  iRet = Parse( &stats, &cfg);
  if ( !iRet && stats.szStartDate[0])
    iRet = Report( cfg.output, &stats, cfg.uTopCount);

  if ( stdout != cfg.output)
    fclose( cfg.output);
  if ( stdin != cfg.input)
    fclose( cfg.input);

  return iRet ? EXIT_FAILURE : EXIT_SUCCESS;
  }


/*
 * A linear list search/insert function
 */
#define STAT_FIND_STR( T, icnt, F)\
T* F( const char* psz, struct SArray* pList)\
  {\
  unsigned long ulCount;\
  T* p;\
\
  /* Find match */\
  for (\
    ulCount = pList->ulCount, p = (T*)(pList->array);\
    ulCount-- > 0;\
    ++p\
  ) {\
    if ( 0 == SWS_strnicmp( p->szName, psz, sizeof p->szName))\
      return p;\
    }\
\
  ulCount = pList->ulCount;\
  if ( ulCount * sizeof (T) >= pList->size)\
    {\
    /* Extend the array */\
    size_t size = pList->size ? pList->size * 2 : icnt * sizeof(T);\
    void* pv = realloc( pList->array, size);\
    if ( NULL == pv)\
      return NULL;\
\
    pList->array = pv;\
    pList->size = size;\
    }\
\
  p = &((T*)(pList->array))[ ulCount];\
  memset( p, 0, sizeof(T));\
  strncpy( p->szName, psz, sizeof(p->szName) -1);\
  pList->ulCount = 1 + ulCount;\
  return p;\
  }

#define STAT_FIND_INT( T, icnt, F)\
T* F( const int iTag, struct SArray* pList)\
  {\
  unsigned long ulCount;\
  T* p;\
\
  /* Find match */\
  for (\
    ulCount = pList->ulCount, p = (T*)(pList->array);\
    ulCount-- > 0;\
    ++p\
  ) {\
    if ( iTag == p->iValue)\
      return p;\
    }\
\
  ulCount = pList->ulCount;\
  if ( ulCount * sizeof (T) >= pList->size)\
    {\
    /* Extend the array */\
    size_t size = pList->size ? pList->size * 2 : icnt * sizeof(T);\
    void* pv = realloc( pList->array, size);\
    if ( NULL == pv)\
      return NULL;\
\
    pList->array = pv;\
    pList->size = size;\
    }\
\
  p = &((T*)(pList->array))[ ulCount];\
  memset( p, 0, sizeof(T));\
  p->iValue = iTag;\
  pList->ulCount = 1 + ulCount;\
  return p;\
  }

#ifdef __cplusplus
template< typename T, int icnt > STAT_FIND_STR( T, icnt, Find)
# define STAT_DEFFIND( T, icnt)\
  const int k##T = icnt;\
  template static T* Find< T, icnt > ( const char*, struct SArray*);
# define STAT_FIND( T) Find< T, k##T >

template< typename T, int icnt > STAT_FIND_INT( T, icnt, Find)
# define STAT_DEFFINDI( T, icnt)\
  const int k##T = icnt;\
  template static T* Find< T, icnt > ( const int, struct SArray*);
# define STAT_FIND( T) Find< T, k##T >
#else
# define STAT_FIND( T) Find##T
# define STAT_DEFFIND( T, icnt) static STAT_FIND_STR( T, icnt, STAT_FIND( T))
# define STAT_DEFFINDI( T, icnt) static STAT_FIND_INT( T, icnt, STAT_FIND( T))
#endif

STAT_DEFFIND( SUri, 64)
STAT_DEFFIND( SRef, 4)
STAT_DEFFIND( SAgent, 16)
STAT_DEFFINDI( SEcode, 16)


/*
 * Parse the log file
 */
int Parse( SStats* pStats, const SConfig* pCfg)
  {
  static char szBuf[ MAX_LINE];
  int iFirst = 1, iLines = 0;

  /* Read a line at a time until EOF */
  while ( NULL != fgets( szBuf, sizeof szBuf, pCfg->input) )
    {
    int i, iTail;
    char szProtocol[ SIZE_PCOL + 1];
    char szDate[ SIZE_DATE + 1], szTime[ SIZE_TIME + 1], szSrc[ SIZE_ADDR + 1];
    char* psz, *pszTail;
    unsigned uSrc = 0;
    unsigned ip[4];
    unsigned long ulSrcAddr;
    SHttpRqst rqst;
    SSmtpRqst smtp;
    SFtpRqst ftp;
    enum { kLines = 512 };
    long ulBytes = 0;

    if ( pCfg->iVerbosity > 0
      && (iLines++ % kLines) == 0
    ) {
      static const char table[] = "|/-\\";
      fprintf( stderr, "%c\r", table[ (iLines / kLines) % 4] );
      }

    if ( g_exit)
      {
      fputs( "Aborted\n", stderr);
      return 1;
      }

    /* Log format:
      Logging(1) started Thu Oct 15 14:01:24 2009
      2009/10/15 14:01:24 SMTPD: 0.0.0.0:25 LISTENING
      2009/10/15 14:26:15 SMTPD: 213.56.203.230:3194 OPEN : 220 softsystem.homeip.net [77.200.166.134] ESMTP; Thu, 15 Oct 2009 14:26:15 +0200
      2009/10/15 14:26:16 SMTPD: 213.56.203.230:3194 EHLO smtp5.lettra.fr : 250 OK smtp5.lettra.fr [213.56.203.230]
      2009/10/15 14:26:16 SMTPD: 213.56.203.230:3194 MAIL FROM:<ec50@email-cible.com> SIZE=15641 : 250 Sender ec50@email-cible.com OK
      2009/10/15 14:26:16 SMTPD: 213.56.203.230:3194 RCPT TO:<ann@softsystem.co.uk> : 250 Recipient ann@softsystem.co.uk OK
      2009/10/15 14:26:16 SMTPD: 213.56.203.230:3194 DATA : 354 Start mail input; end with <CRLF>.<CRLF>
      2009/10/15 14:26:16 SMTPD: 213.56.203.230:3194 STORED /var/lib/sws_inetd/inbox/ann/00000001.eml
      2009/10/15 14:26:16 SMTPD: 213.56.203.230:3194 DATA : 250 OK
      2009/10/15 14:26:16 SMTPD: 213.56.203.230:3194 QUIT : 221 softsystem.homeip.net closing
      2009/10/15 14:26:16 SMTPD: 213.56.203.230:3194 CLOSE
      2009/10/17 03:10:34 HTTPD: 61.136.93.18:4241 GET http://www.baidu.com/ HTTP/1.1 501 Method not implemented for URI
      2009/10/17 03:10:35 HTTPD: 61.136.93.18:4241 Connection reset in read
      2009/10/15 15:03:33 FTPD: 192.168.1.20:55306 OPEN : 220 Proceed to login.
      2009/10/15 15:03:35 FTPD: 192.168.1.20:55306 USER lvr : 331 Password required.
      2009/10/15 15:03:35 FTPD: 192.168.1.20:55306 PASS #### : 230 lvr login. Full access
      2009/10/15 15:03:35 FTPD: 192.168.1.20:55306 SYST : 215 UNIX Type: L8
      2009/10/15 15:03:35 FTPD: 192.168.1.20:55306 PWD : 257 "/"
      2009/10/15 15:06:35 FTPD: 192.168.1.20:55306 QUIT : 221 Goodbye.
      2009/10/15 15:06:35 FTPD: 192.168.1.20:55306 CLOSE
    */
    if ( iFirst)
      {
      unsigned uYr;

      /* Parse the start line */
      i = sscanf( szBuf,
        "Logging(%*u) started %*s %3s %u %" STRING(SIZE_TIME) "s %u",
        szProtocol, &uSrc, szTime, &uYr
      );
      if ( 4 == i)
        {
        iFirst = 0;
        sprintf( pStats->szStartDate, "%u/%02u/%02u",
          uYr, MonthNum( szProtocol), uSrc);
        strncpy( pStats->szStartTime, szTime, sizeof  pStats->szStartTime);
        }
      continue;
      }

    /* Parse a log line */
    i = sscanf( szBuf,
      " %" STRING(SIZE_DATE) "s %" STRING(SIZE_TIME) "s"
      " %" STRING(SIZE_PCOL) "[^ :] :"
      " %" STRING(SIZE_ADDR) "[0-9.]:%u %n",
      szDate, szTime, szProtocol, szSrc, &uSrc, &iTail
    );
    if ( i < 5)
      continue;

    /* Update end date & time */
    i = strcmp( szDate, pStats->szEndDate);
    if ( i > 0)
      {
      strncpy( pStats->szEndDate, szDate, sizeof pStats->szEndDate);
      strncpy( pStats->szEndTime, szTime, sizeof pStats->szEndDate);
      }
    else if ( 0 == i && strcmp( szTime, pStats->szEndTime) > 0)
      {
      strncpy( pStats->szEndTime, szTime, sizeof pStats->szEndTime);
      }

    /* Parse dotted IP src address */
    if ( 4 != sscanf( szSrc, "%u.%u.%u.%u", &ip[0], &ip[1], &ip[2], &ip[3]))
      continue;
    ulSrcAddr = ((unsigned long)ip[0] << 24)
      + ((unsigned long)ip[1] << 16)
      + (ip[2] << 8)
      + ip[3];
    if ( 0 == ulSrcAddr)
      continue;

    /* Skip ANSI SGR prefix on protocol */
    if ( '\x1b' == szProtocol[0] && NULL != (psz = strchr( &szProtocol[1], 'm')) )
      strcpy( szProtocol, psz + 1);
    else
      psz = szProtocol;

    /* Apply protocol filter */
    if ( NULL != pCfg->pszProtocol && strcmp( szProtocol, pCfg->pszProtocol))
      continue;

    /* Skip ANSI SGR prefix on tail */
    pszTail = &szBuf[ iTail];
    if ( '\x1b' == pszTail[0] && NULL != (psz = strchr( &pszTail[1], 'm')) )
      pszTail = psz + 1;

    /* Exclude ANSI SGR sufffix on tail */
    if ( NULL != (psz = strchr( &pszTail[1], '\x1b')))
      strcpy( psz, "\n");

    smtp.iResponse = -1;
    ftp.iResponse = -1;
    rqst.iResponse = -1;

    /* Protocol specific action */
    switch ( pCfg->eFilter)
      {
    case kFilterNone:
      pStats->ulProtocolPackets++;
      break;

    case kFilterError:
      if ( strncmp( "ERROR", pszTail, 5))
        continue;
      pStats->ulProtocolPackets++;
      break;

    case kFilterWarn:
      if ( strncmp( "WARN", pszTail, 4))
        continue;
      pStats->ulProtocolPackets++;
      break;

    case kFilterSmtp:
    case kFilterSmtpOpen:
    case kFilterSmtpHelo:
    case kFilterSmtpAuth:
    case kFilterSmtpFrom:
    case kFilterSmtpTo:
    case kFilterSmtpData:
      if ( ParseSmtpRequest( &smtp, pszTail))
        continue;

      switch ( pCfg->eFilter)
        {
      case kFilterSmtpOpen:
        if ( smtp.eType != kSmtpOpen)
          continue;
        break;
      case kFilterSmtpHelo:
        if ( smtp.eType != kSmtpHelo && smtp.eType != kSmtpEhlo)
          continue;
        break;
      case kFilterSmtpAuth:
        if ( smtp.eType != kSmtpAuth)
          continue;
        break;
      case kFilterSmtpFrom:
        if ( smtp.eType != kSmtpFrom)
          continue;
        break;
      case kFilterSmtpTo:
        if ( smtp.eType != kSmtpTo)
          continue;
        break;
      case kFilterSmtpData:
        if ( smtp.eType != kSmtpData)
          continue;
        break;
      default:
        smtp.pszInfo = NULL;
        break;
        }

      pStats->ulProtocolPackets++;
      if ( pCfg->iCode >= 0)
        {
        if ( smtp.iResponse < pCfg->iCode || smtp.iResponse > pCfg->iCodeMax)
          continue;
        }

      ulBytes = smtp.ulBytes;
      break;

    case kFilterFtp:
    case kFilterFtpPut:
    case kFilterFtpGet:
      if ( ParseFtpRequest( &ftp, pszTail))
        continue;

      if ( kFilterFtpPut == pCfg->eFilter) switch ( ftp.eType)
        {
      case kFtpStor:
      case kFtpAppe:
      case kFtpDele:
      case kFtpCwd:
      case kFtpCdup:
      case kFtpMkd:
      case kFtpRmd:
        break;
      default:
        continue;
        }
      else if ( kFilterFtpGet == pCfg->eFilter) switch ( ftp.eType)
        {
      case kFtpRetr:
      case kFtpList:
      case kFtpNlst:
      case kFtpCwd:
      case kFtpCdup:
        break;
      default:
        continue;
        }

      pStats->ulProtocolPackets++;
      if ( pCfg->iCode >= 0)
        {
        if ( ftp.iResponse < pCfg->iCode || ftp.iResponse > pCfg->iCodeMax)
          continue;
        }

      ulBytes = ftp.ulRxBytes + ftp.ulTxBytes;
      break;

    case kFilterHttp:
      if ( ParseHttpRequest( &rqst, pszTail))
        continue;

      pStats->ulProtocolPackets++;
      if ( pCfg->iCode >= 0)
        {
        if ( rqst.iResponse < pCfg->iCode || rqst.iResponse > pCfg->iCodeMax)
          continue;
        }

      if ( rqst.lBytes > 0)
        ulBytes = rqst.lBytes;
      break;

    case kFilterAll:
      if ( 0 == strncmp( "ERROR", pszTail, 5))
        {
        strcpy( szProtocol, "ERROR");
        pStats->ulProtocolPackets++;
        if ( pCfg->iCode >= 0)
          continue;
        }
      else if ( 0 == strncmp( "WARN", pszTail, 4))
        {
        strcpy( szProtocol, "WARN");
        pStats->ulProtocolPackets++;
        if ( pCfg->iCode >= 0)
          continue;
        }
      else if ( 0 == strcmp( kszSMTP, szProtocol))
        {
        if ( ParseSmtpRequest( &smtp, pszTail))
          continue;
        pStats->ulProtocolPackets++;
        if ( pCfg->iCode >= 0)
          {
          if ( smtp.iResponse < pCfg->iCode || smtp.iResponse > pCfg->iCodeMax)
            continue;
          }
        ulBytes = smtp.ulBytes;
        smtp.pszInfo = NULL;
        }
      else if ( 0 == strcmp( kszHTTP, szProtocol))
        {
        if ( ParseHttpRequest( &rqst, pszTail))
          continue;
        pStats->ulProtocolPackets++;
        if ( pCfg->iCode >= 0)
          {
          if ( rqst.iResponse < pCfg->iCode || rqst.iResponse > pCfg->iCodeMax)
            continue;
          }
        if ( rqst.lBytes > 0)
          ulBytes = rqst.lBytes;
        }
      else if ( 0 == strcmp( kszFTP, szProtocol))
        {
        if ( ParseFtpRequest( &ftp, pszTail))
          continue;
        pStats->ulProtocolPackets++;
        if ( pCfg->iCode >= 0)
          {
          if ( ftp.iResponse < pCfg->iCode || ftp.iResponse > pCfg->iCodeMax)
            continue;
          }
        ulBytes += ftp.ulTxBytes + ftp.ulRxBytes;
        }
      else
        {
        if ( pCfg->iCode >= 0)
          continue;
        pStats->ulProtocolPackets++;
        }
      break;
      }

    /* Apply tail filter */
    if ( NULL != pCfg->pszFormat && !SWS_StrMatch( pCfg->pszFormat, pszTail, 0))
      continue;
    if ( NULL != pCfg->pszSearch && NULL == SWS_stristr( pszTail, pCfg->pszSearch))
      continue;

    /* Apply source address filter */
    if ( (ulSrcAddr & pCfg->ulSrcSubnetMask) != pCfg->ulSrcAddr)
      continue;
    if ( pCfg->ulExSrcAddr
      && (ulSrcAddr & pCfg->ulExSrcSubnetMask) == pCfg->ulExSrcAddr
    )
      continue;

    if ( pCfg->iVerbosity > 1)
      {
      /* Display packet info */
      fprintf( pCfg->output, "%s %.8s %s: %15s:%-5u %s",
        szDate, szTime, szProtocol, szSrc, uSrc, pszTail);
      }

    /* IP specific action */
    if ( AddIpAddress(
        pStats,
        ulSrcAddr,
        pCfg->ulIpMask,
        szDate, szTime,
        ulBytes
      )
    ) {
      return 1;
      }

    /* Filter specific action */
    switch ( pCfg->eFilter)
      {
    case kFilterHttp:
      AddHttpRequest( pStats, &rqst);
      break;

    case kFilterSmtp:
    case kFilterSmtpOpen:
    case kFilterSmtpHelo:
    case kFilterSmtpAuth:
    case kFilterSmtpFrom:
    case kFilterSmtpTo:
    case kFilterSmtpData:
      AddSmtpRequest( pStats, &smtp);
      break;

    case kFilterFtp:
    case kFilterFtpPut:
    case kFilterFtpGet:
      AddFtpRequest( pStats, &ftp);
      break;

    case kFilterAll:
    case kFilterError:
    case kFilterWarn:
      {SAgent* pAgent = STAT_FIND( SAgent)( szProtocol, &pStats->prtcl);
      if ( NULL != pAgent)
        {
        ++(pAgent->ulCount);
        STAT_TOTAL( pStats->prtcl)++;
        }
      }
      if ( rqst.iResponse >= 0)
        AddHttpRequest( pStats, &rqst);
      else if ( smtp.iResponse >= 0)
        AddSmtpRequest( pStats, &smtp);
      else if ( ftp.iResponse >= 0)
        AddFtpRequest( pStats, &ftp);
      break;

    case kFilterNone:
      break;
      }

    pStats->ulMatchingPackets++;
    }

  if ( pCfg->iVerbosity > 1)
    fputs( "\n", pCfg->output);

  return 0;
  }


#define STAT_URIREPORT( T, r, cnt, output, pkt) do\
  {\
  const T* p = &((const T*)((r).array))[ (r).ulCount];\
  unsigned long ul;\
  qsort( (r).array, (r).ulCount, sizeof(T), Cmp##T);\
  for ( ul = 0; ul < (cnt) && ul < (r).ulCount; ++ul)\
    {\
    --p;\
    fprintf( output, "%2lu %5.1f %5lu %10lu  %s\n",\
      ul +1, (100. * p->ulCount) / pkt,\
      p->ulCount, p->ulBytes, p->szName);\
    }\
  } while(0)

#define STAT_REPORT( T, r, cnt, output) do\
  {\
  const T* p = &((const T*)((r).array))[ (r).ulCount];\
  unsigned long ul;\
  qsort( (r).array, (r).ulCount, sizeof(T), Cmp##T);\
  for ( ul = 0; ul < (cnt) && ul < (r).ulCount; ++ul)\
    {\
    --p;\
    fprintf( output, "%2lu %5.1f %5lu  %s\n",\
      ul +1, (100. * p->ulCount) / (r).ulTotal,\
      p->ulCount, p->szName);\
    }\
  } while(0)

#define STAT_EREPORT( T, r, cnt, output) do\
  {\
  const T* p = &((const T*)((r).array))[ (r).ulCount];\
  unsigned long ul;\
  qsort( (r).array, (r).ulCount, sizeof(T), Cmp##T);\
  for ( ul = 0; ul < (cnt) && ul < (r).ulCount; ++ul)\
    {\
    --p;\
    fprintf( output, "%2lu %5.1f %5lu  %d\n",\
      ul +1, (100. * p->ulCount) / (r).ulTotal,\
      p->ulCount, p->iValue);\
    }\
  } while(0)


/*
 * Generate a report
 */
int Report( FILE* output, const SStats* pStats, unsigned uCount)
  {
  if ( output != stdout)
    fputs( kszBanner "\n", output);

  fprintf( output, "%lu/%lu matches from %s %s to %s %s\n",
    pStats->ulMatchingPackets, pStats->ulProtocolPackets,
    pStats->szStartDate, pStats->szStartTime,
    pStats->szEndDate, pStats->szEndTime
  );

  /* List top IP addreses */
  if ( pStats->ulIps)
    {
    SIpAddress* pSrc;
    unsigned long ulIndex;

    qsort(
      pStats->ipAddressArray,
      pStats->ulIps,
      sizeof pStats->ipAddressArray[0],
      CmpIpCount
    );

    fprintf( output, "\nRank  %%  Count  IP address (%-3lu)  Time of last packet   KBytes\n",
      pStats->ulIps);

    pSrc = &pStats->ipAddressArray[ pStats->ulIps];
    for ( ulIndex = 0;
      ulIndex < uCount && ulIndex < pStats->ulIps;
      ++ulIndex
    ) {
      unsigned ip[4];
      char szAddr[ SIZE_ADDR + 1];

      --pSrc;

      ip[3] = (unsigned)(pSrc->ulAddr >> 24);
      ip[2] = (unsigned)(pSrc->ulAddr >> 16) & 0xff;
      ip[1] = (unsigned)(pSrc->ulAddr >> 8) & 0xff;
      ip[0] = (unsigned)(pSrc->ulAddr) & 0xff;
      sprintf( szAddr, "%u.%u.%u.%u", ip[3], ip[2], ip[1], ip[0]);

      fprintf( output, "%2lu %5.1f %5lu  %-15s : %s %s  %7.1f\n",
        ulIndex +1, (100. * pSrc->ulCount) / pStats->ulMatchingPackets,
        pSrc->ulCount,
        szAddr,
        pSrc->szDate, pSrc->szTime,
        pSrc->ulBytes / 1000.
      );
      }
    }

  /** Browser */
  /* HTTP top method */
  if ( STAT_COUNT( pStats->method))
    {
    fprintf( output, "\nRank  %%  Count  %lu HTTP methods (from %lu requests)\n",
      STAT_COUNT( pStats->method), STAT_TOTAL( pStats->method));
    STAT_REPORT( SAgent, pStats->method, uCount, output);
    }

  /* HTTP top URI's */
  if ( STAT_COUNT( pStats->uri))
    {
    fprintf( output, "\nRank  %%  Count      Bytes  %lu HTTP URIs totalling %.3f Mbytes\n",
      STAT_COUNT( pStats->uri), STAT_TOTAL( pStats->uri) / 1e6);
    STAT_URIREPORT( SUri, pStats->uri, uCount, output, pStats->ulMatchingPackets);
    }

  /* HTTP top referers */
  if ( STAT_COUNT( pStats->referer))
    {
    fprintf( output, "\nRank  %%  Count  %lu HTTP referers (from %lu headers)\n",
      STAT_COUNT( pStats->referer), STAT_TOTAL( pStats->referer));
    STAT_REPORT( SRef, pStats->referer, uCount, output);
    }

  /* HTTP top vias */
  if ( STAT_COUNT( pStats->via))
    {
    fprintf( output, "\nRank  %%  Count  %lu HTTP vias (from %lu headers)\n",
      STAT_COUNT( pStats->via), STAT_TOTAL( pStats->via));
    STAT_REPORT( SRef, pStats->via, uCount, output);
    }

  /* HTTP top user-agents */
  if ( STAT_COUNT( pStats->agent))
    {
    fprintf( output, "\nRank  %%  Count  %lu Browser User-Agents (from %lu headers)\n",
      STAT_COUNT( pStats->agent), STAT_TOTAL( pStats->agent));
    STAT_REPORT( SAgent, pStats->agent, uCount, output);
    }

  /* HTTP top platforms */
  if ( STAT_COUNT( pStats->platform))
    {
    fprintf( output, "\nRank  %%  Count  %lu Browser platforms (from %lu headers)\n",
      STAT_COUNT( pStats->platform), STAT_TOTAL( pStats->platform));
    STAT_REPORT( SAgent, pStats->platform, uCount, output);
    }

  /* HTTP top browser locales */
  if ( STAT_COUNT( pStats->locale))
    {
    fprintf( output, "\nRank  %%  Count  %lu Browser locales (from %lu headers)\n",
      STAT_COUNT( pStats->locale), STAT_TOTAL( pStats->locale));
    STAT_REPORT( SAgent, pStats->locale, uCount, output);
    }

  /* HTTP top accepted languages */
  if ( STAT_COUNT( pStats->accept))
    {
    fprintf( output, "\nRank  %%  Count  %lu HTTP accepted languages (from %lu headers)\n",
      STAT_COUNT( pStats->accept), STAT_TOTAL( pStats->accept));
    STAT_REPORT( SAgent, pStats->accept, uCount, output);
    }


  /** SMTP */
  /* SMTP top client domains */

  if ( STAT_COUNT( pStats->hostname))
    {
    fprintf( output, "\nRank  %%  Count  %lu Mail client domains (from %lu hellos)\n",
      STAT_COUNT( pStats->hostname), STAT_TOTAL( pStats->hostname));
    STAT_REPORT( SHostname, pStats->hostname, uCount, output);
    }

  /* SMTP top senders */
  if ( STAT_COUNT( pStats->mailfrom))
    {
    fprintf( output, "\nRank  %%  Count  %lu Mail sources (from %lu)\n",
      STAT_COUNT( pStats->mailfrom), STAT_TOTAL( pStats->mailfrom));
    STAT_REPORT( SSmtpAddress, pStats->mailfrom, uCount, output);
    }

  /* SMTP top recipients */
  if ( STAT_COUNT( pStats->mailto))
    {
    fprintf( output, "\nRank  %%  Count  %lu Mail recipients (from %lu)\n",
      STAT_COUNT( pStats->mailto), STAT_TOTAL( pStats->mailto));
    STAT_REPORT( SSmtpAddress, pStats->mailto, uCount, output);
    }
  if ( pStats->ulMailBytes)
    fprintf( output, "\nMail total received: %.1f KBytes\n", pStats->ulMailBytes / 1000.);

  /** FTP */
  if ( STAT_COUNT( pStats->user))
    {
    fprintf( output, "\nRank  %%  Count  %lu FTP user (from %lu)\n",
      STAT_COUNT( pStats->user), STAT_TOTAL( pStats->user));
    STAT_REPORT( SFtpFname, pStats->user, uCount, output);
    }
  if ( STAT_COUNT( pStats->pass))
    {
    fprintf( output, "\nRank  %%  Count  %lu FTP password (from %lu)\n",
      STAT_COUNT( pStats->user), STAT_TOTAL( pStats->pass));
    STAT_REPORT( SFtpFname, pStats->pass, uCount, output);
    }
  if ( STAT_COUNT( pStats->fread))
    {
    fprintf( output, "\nRank  %%  Count  %lu FTP files read (from %lu)\n",
      STAT_COUNT( pStats->fread), STAT_TOTAL( pStats->fread));
    STAT_REPORT( SFtpFname, pStats->fread, uCount, output);
    }
  if ( STAT_COUNT( pStats->fwrite))
    {
    fprintf( output, "\nRank  %%  Count  %lu FTP files written (from %lu)\n",
      STAT_COUNT( pStats->fwrite), STAT_TOTAL( pStats->fwrite));
    STAT_REPORT( SFtpFname, pStats->fwrite, uCount, output);
    }
  if ( STAT_COUNT( pStats->dread))
    {
    fprintf( output, "\nRank  %%  Count  %lu FTP directories read (from %lu)\n",
      STAT_COUNT( pStats->dread), STAT_TOTAL( pStats->dread));
    STAT_REPORT( SFtpFname, pStats->dread, uCount, output);
    }
  if ( STAT_COUNT( pStats->dwrite))
    {
    fprintf( output, "\nRank  %%  Count  %lu FTP directories written (from %lu)\n",
      STAT_COUNT( pStats->dwrite), STAT_TOTAL( pStats->dwrite));
    STAT_REPORT( SFtpFname, pStats->dwrite, uCount, output);
    }
  if ( pStats->ulFtpRxBytes)
    fprintf( output, "\nFTP total received: %.1f KBytes\n", pStats->ulFtpRxBytes / 1000.);
  if ( pStats->ulFtpTxBytes)
    fprintf( output, "\nFTP total sent: %.1f KBytes\n", pStats->ulFtpTxBytes / 1000.);

  /** All protocols */
  /* Top response codes */
  if ( STAT_COUNT( pStats->ecode))
    {
    fprintf( output, "\nRank  %%  Count  %lu Response codes (from %lu requests)\n",
      STAT_COUNT( pStats->ecode), STAT_TOTAL( pStats->ecode));
    STAT_EREPORT( SEcode, pStats->ecode, uCount, output);
    }

  /* Top protocol */
  if ( STAT_COUNT( pStats->prtcl))
    {
    fprintf( output, "\nRank  %%  Count  %lu Protocols (from %lu requests)\n",
      STAT_COUNT( pStats->prtcl), STAT_TOTAL( pStats->prtcl));
    STAT_REPORT( SAgent, pStats->prtcl, uCount, output);
    }

  return 0;
  }


/*
 * Add an IP address
 */
int AddIpAddress(
  SStats* pStats,
  unsigned long ulAddr,
  unsigned long ulIpMask,
  const char* pszDate,
  const char* pszTime,
  unsigned long ulBytes
) {
  unsigned long ulCount;
  SIpAddress* pSrc;

  /* Find matching IP */
  for ( ulCount = pStats->ulIps, pSrc = pStats->ipAddressArray;
    ulCount > 0;
    --ulCount, ++pSrc
  ) {
    if ( (pSrc->ulAddr & ulIpMask) == (ulAddr & ulIpMask))
      {
      ++(pSrc->ulCount);
      break;
      }
    }

  /* IP matched? */
  if ( 0 == ulCount)
    {
    /* No, add it */
    ulCount = pStats->ulIps;
    if ( ulCount * sizeof pStats->ipAddressArray[0] >= pStats->sizeIps)
      {
      /* Extend the array */
      size_t size = pStats->sizeIps ? pStats->sizeIps * 2
        : INITIAL_IPS * sizeof pStats->ipAddressArray[0];
      void* pv = realloc( pStats->ipAddressArray, size);
      if ( NULL == pv)
        return 1;

      pStats->ipAddressArray = (SIpAddress*)pv;
      pStats->sizeIps = size;
      }

    pSrc = &pStats->ipAddressArray[ ulCount];
    pSrc->ulAddr = ulAddr;
    pSrc->ulCount = 1;
    pSrc->ulBytes = 0;
    strncpy( pSrc->szDate, pszDate, sizeof pSrc->szDate);
    strncpy( pSrc->szTime, pszTime, sizeof pSrc->szTime);

    pStats->ulIps = 1 + ulCount;
    }
  else
    {
    /* Update date & time if later */
    int i = strcmp( pSrc->szDate, pszDate);
    if ( i < 0)
      {
      strncpy( pSrc->szDate, pszDate, sizeof pSrc->szDate);
      strncpy( pSrc->szTime, pszTime, sizeof pSrc->szTime);
      }
    else if ( 0 == i && strcmp( pSrc->szTime, pszTime) < 0)
      {
      strncpy( pSrc->szTime, pszTime, sizeof pSrc->szTime);
      }
    }

  pSrc->ulBytes += ulBytes;

  return 0;
  }


/*
 * Parse an http request log line tail
 * GET /... [[...]] 200 "OK" [[...]]...
 */
int ParseHttpRequest( SHttpRqst* pRqst, const char* pszRqst)
  {
  static char s_szBuf[ MAX_LINE];
  char* psz, *pszTail;
  const char* pcsz;
  int n;

  pRqst->pszMethod = NULL;
  pRqst->pszURI = NULL;
  pRqst->pszUserAgent = NULL;
  pRqst->pszBrowser = NULL;
  pRqst->pszPlatform = NULL;
  pRqst->pszBrowseLang = NULL;
  pRqst->pszAcceptLang = NULL;
  pRqst->pszReferer = NULL;
  pRqst->pszVia = NULL;
  pRqst->pszFrom = NULL;
  pRqst->iResponse = -1;
  pRqst->pszResponse = NULL;
  pRqst->pszRspHdrs = NULL;
  pRqst->lBytes = -1;

#if 0
  if ( 1 == sscanf( pszRqst, " ERROR %d", &pRqst->iResponse))
    return 0;
  else if ( 1 == sscanf( pszRqst, " WARN %d", &pRqst->iResponse))
    return 0;
#endif

  /* Copy the request */
  pszTail = strncpy( s_szBuf, pszRqst, sizeof s_szBuf);

  /* Parse the method */
  while ( isspace( *pszTail))
    ++pszTail;
  pRqst->pszMethod = pszTail;
  do
    {
    if ( '\0' == *pszTail)
      return -1;
    ++pszTail;
    }
  while ( !isspace( *pszTail));
  *pszTail++ = '\0';

  if ( strcmp( "GET", pRqst->pszMethod)
    && strcmp( "HEAD", pRqst->pszMethod)
    && strcmp( "POST", pRqst->pszMethod)
    && strcmp( "PUT", pRqst->pszMethod)
    && strcmp( "DELETE", pRqst->pszMethod)
    && strcmp( "OPTIONS", pRqst->pszMethod)
    && strcmp( "CONNECT", pRqst->pszMethod)
    && strcmp( "TRACE", pRqst->pszMethod)
  ) return -1;

  /* Parse the URI */
  while ( isspace( *pszTail))
    ++pszTail;
  pRqst->pszURI = pszTail;
  do
    {
    if ( '\0' == *pszTail)
      return -1;
    ++pszTail;
    }
  while ( !isspace( *pszTail));
  *pszTail++ = '\0';

#if 0  /* Split off query */
  psz = (char*)strchr( pRqst->pszURI, '?');
  if ( NULL != psz)
    *psz = '\0';
#endif

  /* Split off parameters */
  psz = (char*)strchr( pRqst->pszURI, ';');
  if ( NULL != psz)
    *psz = '\0';

  /* Parse request headers */
  for (;;)
    {
    static const char kszBeginHdr[] = "[[";
    static const char kszEndHdr[] = "]] ";
    char c, *pszVal;

    while ( isspace( *pszTail))
      ++pszTail;
    if ( strncmp( kszBeginHdr, pszTail, sizeof kszBeginHdr - 1))
      break;

    pszTail += sizeof kszBeginHdr - 1;
    while ( isspace( *pszTail))
      ++pszTail;

    /* Lowercase header name */
    for ( psz = pszTail; '\0' != (c = *psz); ++psz)
      {
      if ( ':' != c)
        *psz = (char)tolower( c);
      else
        {
        *psz++ = '\0';
        while ( isspace( *psz))
          ++psz;
        break;
        }
      }
    pszVal = psz;

    psz = (char*)strstr( psz, kszEndHdr);
    if ( NULL == psz)
      return -1;
    *psz = '\0';

    if ( 0 == strcmp( pszTail, "referer"))
      pRqst->pszReferer = pszVal;
    else if ( 0 == strcmp( pszTail, "user-agent"))
      {
      pRqst->pszUserAgent = pszVal;
      DecodeUserAgent( pRqst, pszVal);
      }
    else if ( 0 == strcmp( pszTail, "accept-language"))
      pRqst->pszAcceptLang = pszVal;
    else if ( 0 == strcmp( pszTail, "via"))
      pRqst->pszVia = pszVal;
    else if ( 0 == strcmp( pszTail, "from"))
      pRqst->pszFrom = pszVal;

    pszTail = psz + sizeof kszEndHdr - 1;
    }

  /* Parse the result code */
  n = 0;
  if ( 1 > sscanf( pszTail, "%d%n", &pRqst->iResponse, &n)
    && 1 > sscanf( pszTail, " HTTP/%*u.%*u %d%n", &pRqst->iResponse, &n) /* Pre 6/1/2010 */
  ) return -1;
  if ( 0 == n)
    return -1;

  /* Response text */
  pszTail += n;
  while ( isspace( *pszTail))
    ++pszTail;
  pRqst->pszResponse = pszTail;

  /* Response headers */
  pRqst->pszRspHdrs = psz = (char*)strstr( pszTail, "[[");
  if ( NULL != psz)
    {
    pszTail = psz;
    psz[ -1] = '\0';
    }

  #define kszContentLength "Content-Length:"
  pcsz = strstr( pszTail, kszContentLength);
  if ( NULL != pcsz)
    pRqst->lBytes = strtoul( pcsz + sizeof kszContentLength -1, NULL, 10);
  else
    pRqst->lBytes = -1;

  return 0;
  }


/*
 * Decode http User-Agent header
 */
void DecodeUserAgent( SHttpRqst* pRqst, char* pszUA)
  {
  static char szPlatform[32], szSec[32], szOS[32], szLocale[32];
  int n;
  unsigned uMajor, uMinor;
  const char* pszMoz;

  pszMoz = strstr( pszUA, "Mozilla/");

  /* https://developer.mozilla.org/En/User_Agent_Strings_Reference */
  n = 0;
  if ( pszMoz && 3 <= sscanf( pszMoz,
     "Mozilla/%*u.%*u ( %31[^;)]; %31[^;)]; %31[^;)]; %31[^;)]%n%*[^)]%n",
      szPlatform, szSec, szOS, szLocale, &n, &n)
  ) {
    pRqst->pszPlatform = szOS;
    pRqst->pszBrowseLang = szLocale;

    if ( n > 0 && ')' == pszUA[ n])
      {
      do
        ++n;
      while ( isspace( pszUA[ n]));
      }

    if ( 0 == strcmp( "compatible", szPlatform))
      {
#if 0
      char szMinor[8];
      if ( 2 == sscanf( szSec, "MSIE %u.%7s", &uMajor, szMinor))
        sprintf( szSec, "Internet Explorer %u.%s", uMajor, szMinor);
#endif
      pRqst->pszBrowser = szSec;
      pRqst->pszBrowseLang = NULL;

      /* Some branded (e.g. AOL) browswers have the CPU-OS later */
      if ( strncmp( "Windows", szOS, 7)
        && NULL != (pszMoz = strstr( pszUA, "Windows NT"))
      ) {
        sscanf( pszMoz, "%31[^;()]", szOS);
        }
      }
    else if ( n > 0
      && NULL != (pszMoz = strstr( pszUA + n, "Firefox/"))
      && 2 == sscanf( pszMoz, "Firefox/%u.%u", &uMajor, &uMinor)
    ) {
      sprintf( szSec, "Firefox %u.%u", uMajor, uMinor);
      pRqst->pszBrowser = szSec;
      }
    else if ( n > 0
      && NULL != (pszMoz = strstr( pszUA + n, "Chrome/"))
      && 2 == sscanf( pszMoz, "Chrome/%u.%u", &uMajor, &uMinor)
    ) {
      sprintf( szSec, "Chrome %u.%u", uMajor, uMinor);
      pRqst->pszBrowser = szSec;
      }
    else if ( n > 0
      && NULL != (pszMoz = strstr( pszUA + n, "Safari/"))
      && 2 == sscanf( pszMoz, "Safari/%u.%u", &uMajor, &uMinor)
    ) {
      sprintf( szSec, "Safari %u", uMajor);
      pRqst->pszBrowser = szSec;
      }
    else
      {
      /* Gecko etc */
      sscanf( pszUA + n, " %31[^(]", szSec);
      pRqst->pszBrowser = szSec;
      }
    }
  else if ( pszMoz && 1 <= sscanf( pszMoz, "Mozilla/%*u.%*u ( %31[^ ;)]%n", szSec, &n))
    {
    pRqst->pszBrowser = szSec;
    pszMoz += n;

    if ( ';' == *pszMoz || ')' == *pszMoz)
      ++pszMoz;
    while ( isspace( *pszMoz))
      ++pszMoz;
    if ( '(' == *pszMoz)
      ++pszMoz;
    if ( 1 == sscanf( pszMoz, " %31[^;()]", szOS))
      pRqst->pszPlatform = szOS;
    }
  /* Non-Mozilla browsers */
  else if ( 1 == sscanf( pszUA, "%31[^(]", szSec))
    {
    if ( 2 <= sscanf( szSec, "Opera/%u%n.%u", &uMajor, &n, &uMinor))
      szSec[ n] = '\0';

    pRqst->pszBrowser = szSec;
    pRqst->pszPlatform = NULL;
    }
  else
    {
    pRqst->pszBrowser = pszUA;
    pRqst->pszPlatform = NULL;
    }

  /* Platform */
  if ( NULL != pRqst->pszPlatform)
    {
    if ( 0 == strncmp( pRqst->pszPlatform, kszWindowsNt7, sizeof kszWindowsNt7 -1))
      pRqst->pszPlatform = kszWindows7;
    else if ( 0 == strncmp( pRqst->pszPlatform, kszWindowsNtVista, sizeof kszWindowsNtVista -1))
      pRqst->pszPlatform = kszWindowsVista;
    else if ( 0 == strncmp( pRqst->pszPlatform, kszWindowsNt2003, sizeof kszWindowsNt2003 -1))
      pRqst->pszPlatform = kszWindows2003;
    else if ( 0 == strncmp( pRqst->pszPlatform, kszWindowsNtXP, sizeof kszWindowsNtXP -1)
      || 0 == strncmp( pRqst->pszPlatform, kszWindowsXP, sizeof kszWindowsXP -1)
    ) {
      pRqst->pszPlatform = kszWindowsXP;
      }
    else if ( 0 == strncmp( pRqst->pszPlatform, kszWindowsNt2000, sizeof kszWindowsNt2000 -1))
      pRqst->pszPlatform = kszWindows2000;
    else if ( 0 == strncmp( pRqst->pszPlatform, kszWindowsNt4, sizeof kszWindowsNt4 -1))
      pRqst->pszPlatform = kszWindowsNt4;
    else if ( 0 == strncmp( pRqst->pszPlatform, "Linux", 5))
      {
      /* Allow [kxl]ubuntu */
      if ( NULL != (pszMoz = SWS_stristr( pszUA, "Ubuntu")))
        {
        sscanf( pszMoz - 1, " %31[^ ;()]", szPlatform);
        pRqst->pszPlatform = szPlatform;
        }
      }
    else if ( ISBOT( pRqst->pszPlatform) || NULL != pRqst->pszFrom)
      pRqst->pszPlatform = kszWebbot;
    }
  else if ( strstr( pszUA, kszWindowsNt7))
    pRqst->pszPlatform = kszWindows7;
  else if ( strstr( pszUA, kszWindowsNtVista))
    pRqst->pszPlatform = kszWindowsVista;
  else if ( strstr( pszUA, kszWindowsNt2003))
    pRqst->pszPlatform = kszWindows2003;
  else if ( strstr( pszUA, kszWindowsNtXP) || strstr( pszUA, kszWindowsXP))
    pRqst->pszPlatform = kszWindowsXP;
  else if ( strstr( pszUA, kszWindowsNt2000))
    pRqst->pszPlatform = kszWindows2000;
  else if ( strstr( pszUA, kszWindowsNt4))
    pRqst->pszPlatform = kszWindowsNt4;
  else if ( strstr( pszUA, "Win 9x 4.90"))
    pRqst->pszPlatform = kszWindowsME;
  else if ( strstr( pszUA, kszWindows98) || strstr( pszUA, "Win98"))
    pRqst->pszPlatform = kszWindows98;
  else if ( strstr( pszUA, kszWindows95) || strstr( pszUA, "Win95"))
    pRqst->pszPlatform = kszWindows95;
  else if ( strstr( pszUA, kszWindowsCE))
    pRqst->pszPlatform = kszWindowsCE;
  else if ( strstr( pszUA, kszWindows16))
    pRqst->pszPlatform = kszWindows16;
  else if ( NULL != (pszMoz = SWS_stristr( pszUA, "Mac OS X"))
    || NULL != (pszMoz = SWS_stristr( pszUA, "Macintosh"))
    || NULL != (pszMoz = SWS_stristr( pszUA, "Linux"))
  ) {
    sscanf( pszMoz, "%31[^();]", szPlatform);
    pRqst->pszPlatform = szPlatform;
    }
  else if ( strstr( pszUA, "X11"))
    pRqst->pszPlatform = "X11";
  else if ( ISBOT( pszUA) || NULL != pRqst->pszFrom)
    pRqst->pszPlatform = kszWebbot;
  else
#if 0
    pRqst->pszPlatform = pszUA;
#else
    pRqst->pszPlatform = "Unknown";
#endif
  }


/*
 * Add an http request
 */
int AddHttpRequest( SStats* pStats, const SHttpRqst* pRqst)
  {
  SAgent* pAgent;

  if ( NULL != pRqst->pszMethod)
    {
    pAgent = STAT_FIND( SAgent)( pRqst->pszMethod, &pStats->method);
    if ( NULL == pAgent)
      return 1;

    ++(pAgent->ulCount);
    STAT_TOTAL( pStats->method)++;
    }

  if ( -1 != pRqst->iResponse)
    {
    SEcode* pEcode = STAT_FIND( SEcode)( pRqst->iResponse, &pStats->ecode);
    if ( NULL != pEcode)
      {
      ++(pEcode->ulCount);
      STAT_TOTAL( pStats->ecode)++;
      }
    }

  if ( NULL != pRqst->pszURI)
    {
    /* Find matching URI */
    SUri* pUri = STAT_FIND( SUri)( pRqst->pszURI, &pStats->uri);
    if ( NULL == pUri)
      return 1;

    ++(pUri->ulCount);
    if ( pRqst->lBytes > 0)
      {
      pUri->ulBytes += pRqst->lBytes;
      STAT_TOTAL( pStats->uri) += pRqst->lBytes;
      }
    }

  /* Find matching referer */
  if ( NULL != pRqst->pszReferer)
    {
    SRef* pRef = STAT_FIND( SRef)( pRqst->pszReferer, &pStats->referer);
    if ( NULL == pRef)
      return 1;

    ++(pRef->ulCount);
    STAT_TOTAL( pStats->referer)++;
    }

  /* Find matching user-agent */
  if ( NULL != pRqst->pszBrowser)
    {
    pAgent = STAT_FIND( SAgent)( pRqst->pszBrowser, &pStats->agent);
    if ( NULL == pAgent)
      return 1;

    ++(pAgent->ulCount);
    STAT_TOTAL( pStats->agent)++;
    }

  /* Find matching platform */
  if ( NULL != pRqst->pszPlatform)
    {
    pAgent = STAT_FIND( SAgent)( pRqst->pszPlatform, &pStats->platform);
    if ( NULL == pAgent)
      return 1;

    ++(pAgent->ulCount);
    STAT_TOTAL( pStats->platform)++;
    }

  /* Find matching browser locale */
  if ( NULL != pRqst->pszBrowseLang)
    {
    pAgent = STAT_FIND( SAgent)( pRqst->pszBrowseLang, &pStats->locale);
    if ( NULL == pAgent)
      return 1;

    ++(pAgent->ulCount);
    STAT_TOTAL( pStats->locale)++;
    }

  /* Find matching accept lang */
  if ( NULL != pRqst->pszAcceptLang)
    {
    pAgent = STAT_FIND( SAgent)( pRqst->pszAcceptLang, &pStats->accept);
    if ( NULL == pAgent)
      return 1;

    ++(pAgent->ulCount);
    STAT_TOTAL( pStats->accept)++;
    }

  /* Find matching via */
  if ( NULL != pRqst->pszVia)
    {
    SRef* pRef = STAT_FIND( SRef)( pRqst->pszVia, &pStats->via);
    if ( NULL == pRef)
      return 1;

    ++(pRef->ulCount);
    STAT_TOTAL( pStats->via)++;
    }

  return 0;
  }


/*
 * Parse an smtp request log line tail:
 *
 * OPEN ; 220 hostname [IP] greet; Wed, 20 Jan 2010 11:41:46 +0100
 * EHLO name ; 250 OK hostname [IP]
 * AUTH method : 
 * MAIL FROM:<x@y> SIZE=77828 ; nnn text
 * RCPT TO:<x@y> ; nnn text
 * STORED path n bytes
 * DATA ; 250 OK. 77994 bytes received
 * QUIT ; 221 hostname text
 * CLOSE, STORED, RELAY
 * error message
 */
int ParseSmtpRequest( SSmtpRqst* pRqst, const char* pszTail)
  {
  static char s_szBuf[ 256], s_szBuf2[ 256];
  const char* psz;

  pRqst->iResponse = -1;
  pRqst->eType = kSmtpNone;
  pRqst->pszArg = NULL;
  pRqst->pszInfo = NULL;
  pRqst->ulBytes = 0;
  s_szBuf[0] = '\0';
  s_szBuf2[0] = '\0';

  if ( 1 <= sscanf( pszTail, "HELO %*s %*[:;] %d %*s %*s %255s", &pRqst->iResponse, s_szBuf))
    {
    pRqst->eType = kSmtpHelo;
    if ( pRqst->iResponse < 300)
      pRqst->pszArg =  s_szBuf;
    else if ( 550 == pRqst->iResponse
      && NULL != (psz = strstr( pszTail, "hostname"))
      && 1 == sscanf( psz, "hostname %s", s_szBuf)
    ) {
      pRqst->pszArg =  s_szBuf;
      }
    else
      pRqst->pszArg =  "";
    }
  else if ( 1 <= sscanf( pszTail, "EHLO %*s %*[:;] %d OK %255s", &pRqst->iResponse, s_szBuf))
    {
    pRqst->eType = kSmtpEhlo;
    if ( pRqst->iResponse < 300)
      pRqst->pszArg =  s_szBuf;
    else if ( 550 == pRqst->iResponse
      && NULL != (psz = strstr( pszTail, "hostname"))
      && 1 == sscanf( psz, "hostname %s", s_szBuf)
    ) {
      pRqst->pszArg =  s_szBuf;
      }
    else
      pRqst->pszArg =  "";
    }
  else if ( 1 == sscanf( pszTail, "AUTH %*s %*[:;] %d", &pRqst->iResponse))
    pRqst->eType = kSmtpAuth;
  /* NB skip "BODY=8BITMIME SIZE=3839" after address */
  else if ( 2 == sscanf( pszTail, "MAIL %*[^:]: <%255[^>]>%*[^:;]%*c%d", s_szBuf, &pRqst->iResponse))
    {
    pRqst->eType = kSmtpFrom;
    pRqst->pszArg = s_szBuf;
    }
  else if ( 1 == sscanf( pszTail, "MAIL %*[^:]: <>%*[^:;]%*c%d", &pRqst->iResponse))
    {
    pRqst->eType = kSmtpFrom;
    pRqst->pszArg = "";
    }
  else if ( 2 <= sscanf( pszTail, "RCPT %*[^:]: <%255[^>]> %*c%d %*[^<]<%255[^>]",
    s_szBuf, &pRqst->iResponse, s_szBuf2))
    {
    /* RCPT TO:<s> ; 250 Mail from <s> OK */
    /* RCPT TO:<s> ; nnn Sender <s> ... */
    pRqst->eType = kSmtpTo;
    pRqst->pszArg = s_szBuf;
    pRqst->pszInfo = s_szBuf2;
    }
  else if ( 1 <= sscanf( pszTail, "DATA %*[:;] %d %*s %lu", &pRqst->iResponse, &pRqst->ulBytes))
    pRqst->eType = pRqst->iResponse < 300 ? kSmtpData : kSmtpNoop;
  else if ( 1 == sscanf( pszTail, "RSET %*[:;] %d", &pRqst->iResponse))
    pRqst->eType = kSmtpReset;
  else if ( 1 == sscanf( pszTail, "VRFY %*[:;] %d", &pRqst->iResponse))
    pRqst->eType = kSmtpVerify;
  else if ( 1 == sscanf( pszTail, "HELP %*[:;] %d", &pRqst->iResponse))
    pRqst->eType = kSmtpHelp;
  else if ( 1 == sscanf( pszTail, "NOOP %*[:;] %d", &pRqst->iResponse)
    || 1 == sscanf( pszTail, " %*[:;] %d", &pRqst->iResponse)
  ) {
    pRqst->eType = kSmtpNoop;
    }
  else if ( 1 == sscanf( pszTail, "QUIT %*[:;] %d", &pRqst->iResponse))
    pRqst->eType = kSmtpQuit;
  else if ( 1 == sscanf( pszTail, "OPEN %*[:;] %d", &pRqst->iResponse))
    pRqst->eType = kSmtpOpen;
#if 0
  else if ( 1 == sscanf( pszTail, "ERROR %d", &pRqst->iResponse))
    pRqst->eType = kSmtpUnknown;
  else if ( 1 == sscanf( pszTail, "WARN %d", &pRqst->iResponse))
    pRqst->eType = kSmtpUnknown;
#endif
  else if ( 1 == sscanf( pszTail, "%*[^:;] %*[:;] %d", &pRqst->iResponse))
    pRqst->eType = kSmtpUnknown;
  else if ( 0 == strncmp( "CLOSE", pszTail, 5))
    return 0;
  else if ( 0 == strncmp( "STORED", pszTail, 6))
    return 1;
  else if ( 0 == strncmp( "RELAY", pszTail, 5))
    return 1;
  else
    return -1;

  return 0;
  }


STAT_DEFFIND( SHostname, 64)
STAT_DEFFIND( SSmtpAddress, 64)

/*
 * Add an SMTP log entry
 */
static int AddSmtpRequest( SStats* pStats, const SSmtpRqst* pRqst)
  {
  SSmtpAddress* pAddr;
  SHostname* pHost;
  const char* pszDom;
  int n;

  if ( -1 != pRqst->iResponse)
    {
    SEcode* pEcode = STAT_FIND( SEcode)( pRqst->iResponse, &pStats->ecode);
    if ( NULL != pEcode)
      {
      ++(pEcode->ulCount);
      STAT_TOTAL( pStats->ecode)++;
      }
    }

  switch ( pRqst->eType)
    {
  case kSmtpHelo:
  case kSmtpEhlo:
    pszDom = pRqst->pszArg;
    if ( '\0' == pszDom[0])
      break;

    if ( '[' == pszDom[0])
      pszDom = "<unknown>";
    else
      {
      /* Separate domain name from hostname */
      n = 0;
      for ( pszDom += strlen( pszDom); pszDom > pRqst->pszArg; --pszDom)
        {
        if ( '.' == pszDom[ -1] &&  ++n >= 3)
          break;
        }

      /* Remove leading whitespace & dynamic IP numbers */
      n = 0;
      sscanf( pszDom, " %*[-._0123456789]%n", &n);
      pszDom += n;
      }

    pHost = STAT_FIND( SHostname)( pszDom, &pStats->hostname);
    if ( NULL == pHost)
      return 1;

    ++(pHost->ulCount);
    STAT_TOTAL( pStats->hostname)++;
    break;

  case kSmtpFrom:
    pAddr = STAT_FIND( SSmtpAddress)( pRqst->pszArg, &pStats->mailfrom);
    if ( NULL == pAddr)
      return 1;

    ++(pAddr->ulCount);
    STAT_TOTAL( pStats->mailfrom)++;
    break;

  case kSmtpTo:
    pAddr = STAT_FIND( SSmtpAddress)( pRqst->pszArg, &pStats->mailto);
    if ( NULL == pAddr)
      return 1;

    ++(pAddr->ulCount);
    STAT_TOTAL( pStats->mailto)++;

    if ( NULL != pRqst->pszInfo
      && '\0' != pRqst->pszInfo[0]
      && NULL != (pAddr = STAT_FIND( SSmtpAddress)( pRqst->pszInfo, &pStats->mailfrom))
    ) {
      ++(pAddr->ulCount);
      STAT_TOTAL( pStats->mailfrom)++;
      }
    break;

  case kSmtpData:
    pStats->ulMailBytes += pRqst->ulBytes;
    break;

  case kSmtpNone:
  case kSmtpOpen:
  case kSmtpAuth:
  case kSmtpQuit:
  case kSmtpReset:
  case kSmtpVerify:
  case kSmtpHelp:
  case kSmtpNoop:
  case kSmtpUnknown:
    break;
    }

  return 0;
  }


/*
 * Parse a FTP log entry
 */
int ParseFtpRequest( SFtpRqst* pRqst, const char* pszTail)
  {
  static char s_szBuf[ 256];

  pRqst->iResponse = -1;
  pRqst->eType = kFtpNone;
  pRqst->pszArg = NULL;
  pRqst->ulTxBytes = pRqst->ulRxBytes = 0;

  if ( 2 <= sscanf( pszTail, "USER %255s %*[:;] %d", s_szBuf, &pRqst->iResponse))
    {
    pRqst->eType = kFtpUser;
    pRqst->pszArg = s_szBuf;
    }
  else if ( 2 <= sscanf( pszTail, "PASS %255s %*[:;] %d", s_szBuf, &pRqst->iResponse))
    {
    pRqst->eType = kFtpPass;
    pRqst->pszArg = s_szBuf;
    }
  else if ( 2 <= sscanf( pszTail, "CWD %255s %*[:;] %d", s_szBuf, &pRqst->iResponse))
    {
    pRqst->eType = kFtpCwd;
    pRqst->pszArg = s_szBuf;
    }
  else if ( 1 <= sscanf( pszTail, "CDUP %*[:;] %d", &pRqst->iResponse))
    {
    pRqst->eType = kFtpCdup;
    pRqst->pszArg = "..";
    }
  else if ( 2 <= sscanf( pszTail, "RMD %255s %*[:;] %d", s_szBuf, &pRqst->iResponse))
    {
    pRqst->eType = kFtpRmd;
    pRqst->pszArg = s_szBuf;
    }
  else if ( 2 <= sscanf( pszTail, "MKD %255s %*[:;] %d", s_szBuf, &pRqst->iResponse))
    {
    pRqst->eType = kFtpMkd;
    pRqst->pszArg = s_szBuf;
    }
  else if ( 2 <= sscanf( pszTail, "RETR %255s %*[:;] %d %lu", s_szBuf, &pRqst->iResponse, &pRqst->ulTxBytes))
    {
    pRqst->eType = kFtpRetr;
    pRqst->pszArg = s_szBuf;
    }
  else if ( 2 <= sscanf( pszTail, "STOR %255s %*[:;] %d %lu", s_szBuf, &pRqst->iResponse, &pRqst->ulRxBytes))
    {
    pRqst->eType = kFtpStor;
    pRqst->pszArg = s_szBuf;
    }
  else if ( 2 <= sscanf( pszTail, "APPE %255s %*[:;] %d %lu", s_szBuf, &pRqst->iResponse, &pRqst->ulRxBytes))
    {
    pRqst->eType = kFtpAppe;
    pRqst->pszArg = s_szBuf;
    }
  else if ( 2 <= sscanf( pszTail, "DELE %255s %*[:;] %d", s_szBuf, &pRqst->iResponse))
    {
    pRqst->eType = kFtpDele;
    pRqst->pszArg = s_szBuf;
    }
  else if ( 2 <= sscanf( pszTail, "RNFR %255s %*[:;] %d", s_szBuf, &pRqst->iResponse))
    {
    pRqst->eType = kFtpRnfr;
    pRqst->pszArg = s_szBuf;
    }
  else if ( 2 <= sscanf( pszTail, "RNTO %255s %*[:;] %d", s_szBuf, &pRqst->iResponse))
    {
    pRqst->eType = kFtpRnto;
    pRqst->pszArg = s_szBuf;
    }
  else if ( 2 <= sscanf( pszTail, "PORT %255s %*[^:;] %*[:;] %d", s_szBuf, &pRqst->iResponse))
    {
    pRqst->eType = kFtpPort;
    pRqst->pszArg = s_szBuf;
    }
  /* PASV ; 227 Entering Passive Mode (192,168,1,17,142,98) */
  else if ( 1 <= sscanf( pszTail, "PASV %*[:;] %d %*[^(] ( %255[^)]", &pRqst->iResponse, s_szBuf))
    {
    pRqst->eType = kFtpPasv;
    pRqst->pszArg = s_szBuf;
    }
  else if ( 2 <= sscanf( pszTail, "LIST %255[^:;]%*[:;] %d", s_szBuf, &pRqst->iResponse))
    {
    pRqst->eType = kFtpList;
    pRqst->pszArg = s_szBuf;
    }
  else if ( 1 <= sscanf( pszTail, "LIST %*[:;] %d", &pRqst->iResponse))
    pRqst->eType = kFtpList;
  else if ( 2 <= sscanf( pszTail, "NLST %255[^:;]%*[:;] %d", s_szBuf, &pRqst->iResponse))
    {
    pRqst->eType = kFtpNlst;
    pRqst->pszArg = s_szBuf;
    }
  else if ( 1 <= sscanf( pszTail, "NLST %*[:;] %d", &pRqst->iResponse))
    pRqst->eType = kFtpNlst;
  else if ( 1 <= sscanf( pszTail, "QUIT %*[:;] %d", &pRqst->iResponse))
    pRqst->eType = kFtpQuit;
  else if ( 1 <= sscanf( pszTail, "OPEN %*[:;] %d", &pRqst->iResponse))
    pRqst->eType = kFtpOpen;
#if 0
  else if ( 1 == sscanf( pszTail, "ERROR %d", &pRqst->iResponse))
    pRqst->eType = kFtpUnknown;
  else if ( 1 == sscanf( pszTail, "WARN %d", &pRqst->iResponse))
    pRqst->eType = kFtpUnknown;
#endif
  else if ( 1 <= sscanf( pszTail, "%*[^:;] %*[:;] %d", &pRqst->iResponse))
    pRqst->eType = kFtpUnknown;
  else if ( 0 == strncmp( "CLOSE", pszTail, 5))
    return 0;
  else
    return -1;

  return 0;
  }


STAT_DEFFIND( SFtpFname, 64)

/*
 * Add an FTP log entry
 */
static int AddFtpRequest( SStats* pStats, const SFtpRqst* pRqst)
  {
  SFtpFname* pFname;

  pStats->ulFtpTxBytes += pRqst->ulTxBytes;
  pStats->ulFtpRxBytes += pRqst->ulRxBytes;

  if ( -1 != pRqst->iResponse)
    {
    SEcode* pEcode = STAT_FIND( SEcode)( pRqst->iResponse, &pStats->ecode);
    if ( NULL != pEcode)
      {
      ++(pEcode->ulCount);
      STAT_TOTAL( pStats->ecode)++;
      }
    }

  switch ( pRqst->eType)
    {
  case kFtpUser:
    pFname = STAT_FIND( SFtpFname)( pRqst->pszArg, &pStats->user);
    if ( NULL == pFname)
      return 1;

    ++(pFname->ulCount);
    STAT_TOTAL( pStats->user)++;
    break;

  case kFtpPass:
    pFname = STAT_FIND( SFtpFname)( pRqst->pszArg, &pStats->pass);
    if ( NULL == pFname)
      return 1;

    ++(pFname->ulCount);
    STAT_TOTAL( pStats->pass)++;
    break;

  case kFtpRetr:
  case kFtpRnfr:
    pFname = STAT_FIND( SFtpFname)( pRqst->pszArg, &pStats->fread);
    if ( NULL == pFname)
      return 1;

    ++(pFname->ulCount);
    STAT_TOTAL( pStats->fread)++;
    break;

  case kFtpStor:
  case kFtpAppe:
  case kFtpRnto:
  case kFtpDele:
    pFname = STAT_FIND( SFtpFname)( pRqst->pszArg, &pStats->fwrite);
    if ( NULL == pFname)
      return 1;

    ++(pFname->ulCount);
    STAT_TOTAL( pStats->fwrite)++;
    break;

  case kFtpCwd:
  case kFtpCdup:
    pFname = STAT_FIND( SFtpFname)( pRqst->pszArg, &pStats->dread);
    if ( NULL == pFname)
      return 1;

    ++(pFname->ulCount);
    STAT_TOTAL( pStats->dread)++;
    break;

  case kFtpMkd:
  case kFtpRmd:
    pFname = STAT_FIND( SFtpFname)( pRqst->pszArg, &pStats->dwrite);
    if ( NULL == pFname)
      return 1;

    ++(pFname->ulCount);
    STAT_TOTAL( pStats->dwrite)++;
    break;

  default:
    break;
    }

  return 0;
  }


/*
 * qsort callbacks
 */
int CmpIpCount( const void* pv1, const void* pv2)
  {
  int i;

  if ( ((const SIpAddress*)pv1)->ulCount < ((const SIpAddress*)pv2)->ulCount)
    return -1;
  if ( ((const SIpAddress*)pv1)->ulCount > ((const SIpAddress*)pv2)->ulCount)
    return 1;

  i = strcmp( ((const SIpAddress*)pv1)->szDate, ((const SIpAddress*)pv2)->szDate );
  if ( i)
    return i;

  return strcmp( ((const SIpAddress*)pv1)->szTime, ((const SIpAddress*)pv2)->szTime );
  }

int CmpSEcode( const void* pv1, const void* pv2)
  {
  if ( ((const SEcode*)pv1)->ulCount < ((const SEcode*)pv2)->ulCount)
    return -1;
  if ( ((const SEcode*)pv1)->ulCount > ((const SEcode*)pv2)->ulCount)
    return 1;

  if ( ((const SEcode*)pv1)->iValue < ((const SEcode*)pv2)->iValue)
    return -1;
  if ( ((const SEcode*)pv1)->iValue > ((const SEcode*)pv2)->iValue)
    return 1;
  return 0;
  }

int CmpSUri( const void* pv1, const void* pv2)
  {
  if ( ((const SUri*)pv1)->ulCount < ((const SUri*)pv2)->ulCount)
    return -1;
  if ( ((const SUri*)pv1)->ulCount > ((const SUri*)pv2)->ulCount)
    return 1;

  if ( ((const SUri*)pv1)->ulBytes < ((const SUri*)pv2)->ulBytes)
    return -1;
  if ( ((const SUri*)pv1)->ulBytes > ((const SUri*)pv2)->ulBytes)
    return 1;

  return SWS_stricmp( ((const SUri*)pv2)->szName, ((const SUri*)pv1)->szName);
  }

int CmpSRef( const void* pv1, const void* pv2)
  {
  if ( ((const SRef*)pv1)->ulCount < ((const SRef*)pv2)->ulCount)
    return -1;
  if ( ((const SRef*)pv1)->ulCount > ((const SRef*)pv2)->ulCount)
    return 1;

  return SWS_stricmp( ((const SRef*)pv2)->szName, ((const SRef*)pv1)->szName);
  }

int CmpSAgent( const void* pv1, const void* pv2)
  {
  if ( ((const SAgent*)pv1)->ulCount < ((const SAgent*)pv2)->ulCount)
    return -1;
  if ( ((const SAgent*)pv1)->ulCount > ((const SAgent*)pv2)->ulCount)
    return 1;

  return SWS_stricmp( ((const SAgent*)pv2)->szName, ((const SAgent*)pv1)->szName);
  }

int CmpSSmtpAddress( const void* pv1, const void* pv2)
  {
  if ( ((const SSmtpAddress*)pv1)->ulCount < ((const SSmtpAddress*)pv2)->ulCount)
    return -1;
  if ( ((const SSmtpAddress*)pv1)->ulCount > ((const SSmtpAddress*)pv2)->ulCount)
    return 1;

  return SWS_stricmp( ((const SSmtpAddress*)pv2)->szName, ((const SSmtpAddress*)pv1)->szName);
  }

int CmpSHostname( const void* pv1, const void* pv2)
  {
  if ( ((const SHostname*)pv1)->ulCount < ((const SHostname*)pv2)->ulCount)
    return -1;
  if ( ((const SHostname*)pv1)->ulCount > ((const SHostname*)pv2)->ulCount)
    return 1;

  return SWS_stricmp( ((const SHostname*)pv2)->szName, ((const SHostname*)pv1)->szName);
  }

int CmpSFtpFname( const void* pv1, const void* pv2)
  {
  if ( ((const SFtpFname*)pv1)->ulCount < ((const SFtpFname*)pv2)->ulCount)
    return -1;
  if ( ((const SFtpFname*)pv1)->ulCount > ((const SFtpFname*)pv2)->ulCount)
    return 1;

  return strcmp( ((const SFtpFname*)pv2)->szName, ((const SFtpFname*)pv1)->szName);
  }

/*
 * Convert month name to a number (1..12)
 */
unsigned MonthNum( const char* psz)
  {
  unsigned i;
  static const char* const months[] = {
    "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
  };

  for ( i = 0; i < lengthof( months); ++i)
    {
    if ( !strcmp( months[i], psz))
      return 1 + i;
    }
  return 0;
  }


#ifdef SIGINT
/*
 * SIGINT handler (ctrl-c)
 */
static void OnSigInt( int sig)
  {
  if ( ++g_exit >= 2)
    exit( EXIT_FAILURE);

#ifdef NDEBUG
  signal( sig, OnSigInt);
#else
  (void)sig;
#endif
  }
#endif


/*
 * Scan the command line for options
 */
static int ParseArgs(                 /* Returns !0 on error */
  SConfig* const pCfg,                /* OUT: Updated parameters */
  int iArgc,                          /* IN: No. args */
  const char* const pszArgvArray[]    /* IN: Array of arg strings */
) {
  int i;
  int bHelp = 0; /*iArgc <= 1; * Do help if no args */
  int iArgs = 0;

  /* Scan args */
  for ( i = 1; i < iArgc; ++i)
    {
    char c;
    const char* pszArg;

    pszArg = pszArgvArray[i];

    /* Check for switch options ('-' or '/' prefix) */
#   define isswitch( _c) ('-' == _c)
    c = *pszArg++;
    if ( isswitch( c) )
      {
      /* Option found */
      if ( '\0' == *pszArg)
        {
        if ( NULL != pCfg->input)
          {
          fputs( "Log file previously given\n", stderr);
          return EXIT_FAILURE;
          }
        pCfg->input = stdin;
        }
      else do
        {
        char* pszEnd;
        long lVal;
        unsigned ip[4];
        unsigned uMask;

#       define GetArg() \
          ('\0' != *pszArg \
            || i + 1 >= iArgc \
            || isswitch( pszArgvArray[ i + 1][ 0] ) \
          ) ? pszArg : pszArgvArray[ ++i]

        switch ( *pszArg++)
          {
        case 'h':
        case '?':
          bHelp = !0;
          break;

        case 'V':
          fputs( kszBanner "\n", stderr);
          return EXIT_FAILURE;

        case 't':
          /* terse */
          pCfg->iVerbosity--;
          break;

        case 'v':
          /* verbose */
          pCfg->iVerbosity++;
          break;

        case 'n':
          /* count */
          pszArg = GetArg();
          lVal = strtol( pszArg, &pszEnd, 0);
          if ( *pszEnd || lVal < 0 || (unsigned long)lVal > UINT_MAX)
            {
            fprintf( stderr, "Invalid count %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          pCfg->uTopCount = (unsigned)lVal;
          break;

        case 'e':
          /* error code */
          pszArg = GetArg();
          ip[1] = 0;
          c = '-';
          switch ( sscanf( pszArg, " %u%c%u", &ip[0], &c, &ip[1]))
            {
          case 1:
            break;
          case 2:
            ip[1] = 999;
            break;
          case 3:
            break;
          default:
            fprintf( stderr, "Invalid error range %s\n", pszArg);
            return EXIT_FAILURE;
            }

          if ( ip[0] > 999
            || (ip[1] > 0 && ip[0] > ip[1]) 
            || '-' != c
          ) {
            fprintf( stderr, "Invalid error range %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          pCfg->iCode = ip[0];
          pCfg->iCodeMax = ip[1] ? ip[1] : ip[0];
          break;

        case 'i':
          /* Src IP address */
          pszArg = GetArg();
          uMask = 32;
          if ( 4 > sscanf( pszArg, "%u.%u.%u.%u/%u",
              &ip[0], &ip[1], &ip[2], &ip[3], &uMask)
          ) {
            fprintf( stderr, "Invalid IP address %s\n", pszArg);
            return EXIT_FAILURE;
            }
          pszArg = NULL;

          pCfg->ulSrcAddr = ((unsigned long)ip[0] << 24)
            + ((unsigned long)ip[1] << 16)
            + (ip[2] << 8)
            + ip[3];
          pCfg->ulSrcSubnetMask = 0xffffffffL << (32 - uMask);
          pCfg->ulSrcAddr &= pCfg->ulSrcSubnetMask;
          break;

        case 'x':
          /* Exclude src IP address range */
          pszArg = GetArg();
          uMask = 32;
          if ( 4 > sscanf( pszArg, "%u.%u.%u.%u/%u",
              &ip[0], &ip[1], &ip[2], &ip[3], &uMask)
          ) {
            fprintf( stderr, "Invalid IP address %s\n", pszArg);
            return EXIT_FAILURE;
            }
          pszArg = NULL;

          pCfg->ulExSrcAddr = ((unsigned long)ip[0] << 24)
            + ((unsigned long)ip[1] << 16)
            + (ip[2] << 8)
            + ip[3];
          pCfg->ulExSrcSubnetMask = 0xffffffffL << (32 - uMask);
          pCfg->ulExSrcAddr &= pCfg->ulExSrcSubnetMask;
          break;

        case 'm':
          /* IP address mask */
          pszArg = GetArg();
          lVal = strtol( pszArg, &pszEnd, 10);
          if ( *pszEnd || lVal < 1 || lVal > 32)
            {
            fprintf( stderr, "Invalid IP mask %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          pCfg->ulIpMask = 0xffffffffL << (32 - (int)lVal);
          break;

        case 'r':
          /* protocol */
          pszArg = GetArg();
          if ( '\0' == *pszArg)
            {
            fputs( "Missing protocol\n", stderr);
            return EXIT_FAILURE;
            }

          uMask = lengthof( kProtocols);
          while ( uMask-- > 0)
            {
            if ( 0 == strcmp( pszArg, kProtocols[ uMask].pszName))
              {
              pCfg->pszProtocol = kProtocols[ uMask].pszId;
              pCfg->eFilter = kProtocols[ uMask].eFilter;
              uMask = 0;
              break;
              }
            }
           if ( uMask)
            {
            fprintf( stderr, "Unknown protocol: %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          break;

        case 'f':
          /* Log line filter */
          pCfg->pszFormat = GetArg();
          pszArg = NULL;
          break;

        case 's':
          /* Log line filter */
          pCfg->pszSearch = GetArg();
          pszArg = NULL;
          break;

        case 'o':
          /* output file */
          if ( NULL != pCfg->output && stdout != pCfg->output)
            {
            fprintf( stderr, "Duplicate output file '%s'\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = GetArg();
          pCfg->output = fopen( pszArg, "a");
          if ( NULL == pCfg->output)
            {
            fprintf( stderr, "Failed to open '%s'\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          break;

        default:
          fprintf( stderr, "Invalid switch: '%s'\n", pszArgvArray[i] );
          return EXIT_FAILURE;
          }
        }
      while ( NULL != pszArg && '\0' != *pszArg);
      }
    else
      {
      /* Non-switch argument */
      switch ( iArgs)
        {
      case 0:
        /* input file */
        pCfg->input = fopen( pszArgvArray[i], "r");
        if ( NULL == pCfg->input)
          {
          fprintf( stderr, "Failed to open '%s'\n", pszArgvArray[i]);
          return EXIT_FAILURE;
          }

        ++iArgs;
        break;

      default:
        fprintf( stderr, "Excess argument: '%s'\n", pszArgvArray[i] );
        return EXIT_FAILURE;
        }
      }
    }

  if ( bHelp)
    {
    Help( pszArgvArray[0] );
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
  }

/* End of file */
