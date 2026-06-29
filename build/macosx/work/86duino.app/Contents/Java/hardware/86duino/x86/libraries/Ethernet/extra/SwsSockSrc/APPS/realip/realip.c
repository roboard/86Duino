/*****************************************************************************
 *
 * Project     : realip
 * Function    : Discover the external IP address of the default IP gateway
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, sockets
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 11-Jul-09, LVR, v1.00 Created.
 *
 *****************************************************************************/
#define kszApp "realip"
#define kszVer "1.00"
#define kszBanner kszApp " " kszVer " (" SWS_NET_ID ")" ", " TIMESTAMP "."
#define kszAbout ": display the external IP address of the default IP gateway."

/* Compilation options */
#ifndef SWS_USE_SOCKLIB
#define SWS_USE_SOCKLIB 0 /* 0= native sockets, !0 for sws_sock */
#endif

#ifndef PACKET_DUMP
#define PACKET_DUMP 0
#endif

/* Exports */

/* Imports */

/* ANSI C library */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <limits.h>
#include <time.h>
#if _MSC_VER >= 1000
#include <crtdbg.h>
#endif
#if defined linux || defined __linux || defined __linux__
#include <unistd.h> /* getuid, seteuid */
#endif

#include "sws_net.h"
#include "sws_upnp.h"
#if SWS_USE_SOCKLIB
#include "sws_cfg.h"
#endif
#if SWS_USE_SOCKLIB && PACKET_DUMP
#include "sws_dump.h"
#endif


/***************************************************************************
 * Macros & constants
 ***************************************************************************/
#ifdef __TIMESTAMP__
# define TIMESTAMP __TIMESTAMP__
#else
# define TIMESTAMP "built " __TIME__ " " __DATE__
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

/* ICMP types */
#define ICMP_ECHO_REPLY   0
#define ICMP_DEST_UNREACH 3
#define ICMP_QUENCH       4
#define ICMP_REDIRECT     5
#define ICMP_ECHO         8
#define ICMP_TIME_EXCEED  11
#define ICMP_PARAM_PROB   12
#define ICMP_TIMESTAMP    13
#define ICMP_TIME_REPLY   14
#define ICMP_INFO_RQST    15
#define ICMP_INFO_REPLY   16
#define ICMP_ADDR_MASK    17
#define ICMP_AM_REPLY     18

#define IP_HDR_LEN    20U
#define IP_HDR_MAXLEN 60U
#define ICMP_HDR_LEN  8U
#define IP_MIN_FRAG 576

/* Default parameter values */
#define DEF_TIMEOUT 2000U  /* Rx timeout in milliseconds */


/***************************************************************************
 * Data types
 ***************************************************************************/

/* Discovery method */
typedef enum EMode
  {
  kModeAuto,
  kModeRecRoute,
  kModeUpnp
  } EMode;

/* Parameters */
typedef struct SConfig
  {
#if SWS_USE_SOCKLIB && PACKET_DUMP
  int iVerbosity;
  FILE* logfile;
#endif
  enum EMode eMode;
#define DEF_MODE kModeAuto
  } SConfig;

/* ICMP header */
typedef struct SIcmp
  {
  u_char type;
  u_char code; /* type sub code */
  u_short cksum;
  u_short id;
  u_short seq;
  } SIcmp;

/* Type requirements */
STATIC_ASSERT( sizeof(u_short) == 2);
STATIC_ASSERT( sizeof(u_long) == 4);
STATIC_ASSERT( sizeof(SIcmp) == ICMP_HDR_LEN);


/***************************************************************************
 * Local function prototypes
 ***************************************************************************/

static u_long GatewayIP( void);
static u_long DiscoverGateway( SOCKET);
static int Ping( SOCKET, const struct sockaddr_in*, void*, size_t);
static unsigned IpChecksum( const u_short*, size_t);

#ifdef SIGINT
static void OnSigInt( int);
#endif
static int ParseArgs( SConfig* const, int, const char* const [] );
static void Help( const char* const);
#if SWS_USE_SOCKLIB && PACKET_DUMP
static void LogInit( FILE*, int);
#endif


/***************************************************************************
 * Module data
 ***************************************************************************/

static sig_atomic_t volatile s_exit;


/***************************************************************************
 * Functions
 ***************************************************************************/

/*
 * Show command line syntax
 */
static void Help( const char* const pszArg)
  {
  const char* pszCmd;
  const char* pszDot;
  size_t c;

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
    "\nSyntax: %.*s [options]\n"
    "-a    : Auto (default), try record route if UPnP fails\n"
    "-u    : UPnP Internet Gateway Device method\n"
    "-r    : Record route method (root privilege reqd)\n"
#if SWS_USE_SOCKLIB && PACKET_DUMP
    "-o<s> : Set logfile\n"
    "-v    : Increase verbosity of output\n"
    "-t    : Decrease verbosity of output\n"
#endif
    "-V    : Show version and exit\n"
    "-h    : Help\n",
    (int)c, pszCmd
  );
  }


/*
 * M A I N
 */
int main( int argc, const char* argv[] )
  {
  SConfig cfg;
  int iRet;
  struct hostent* pHost;
  char szHost[256];
  struct in_addr addr;

  /* Set default options */
#if SWS_USE_SOCKLIB && PACKET_DUMP
  cfg.iVerbosity = 1;
  cfg.logfile = stdout;
#endif
  cfg.eMode = DEF_MODE;

  /* Parse args */
  iRet = ParseArgs( &cfg, argc, argv );
  if ( EXIT_SUCCESS != iRet)
    return iRet;

#if SWS_USE_SOCKLIB
  /* Specify the configuration file */
  if ( !SWS_CfgSetName( NULL, argv[0] ))
    fprintf( stderr, "WARNING: configuration file '" SWS_CFGFILE "' not found\n");
#endif

#ifdef SIGINT
  /* Register a handler for ctrl-c */
  signal( SIGINT, &OnSigInt);
#endif

  /* Initialise the sockets library */
#if PACKET_DUMP
# define GEF( _f, _v) SWS_PKTDUMP( _f, _v)
#else
# define GEF( _f, _v) 0
#endif
  SWS_SockUp( GEF( cfg.logfile, cfg.iVerbosity), NULL);

  /* Check the network's up */
  if ( gethostname( szHost, sizeof(szHost))
    || NULL == (pHost = gethostbyname( szHost))
  ) {
    fprintf( stderr, "ERROR: no network (driver not found)\n");
    return EXIT_FAILURE;
    }

  switch ( cfg.eMode)
    {
  case kModeAuto:
  case kModeUpnp:
    /* Try UPnP to get gateway's external IP */
    if ( 0 == SWS_UpnpGetExtIP( szHost))
      return printf( "%s\n", szHost), EXIT_SUCCESS;
    else if ( kModeUpnp == cfg.eMode)
      return perror( "UPnP error"), EXIT_FAILURE;
    /* Fall thru */
  case kModeRecRoute:
    /* Try ICMP ping with IP record route option */
    addr.s_addr = GatewayIP();
    if ( INADDR_NONE != addr.s_addr)
      return printf( "%s\n", inet_ntoa( addr)), EXIT_SUCCESS;
    break;
    }

  return EXIT_FAILURE;
  }


/*
 * Get the external IP address (network form) of the default gateway
 */
u_long GatewayIP( void)                 /* Returns INADDR_NONE on error */
  {
  SOCKET s;
  u_long addr;

  /* Create an ICMP socket */
  s = socket( PF_INET, SOCK_RAW, IPPROTO_ICMP);
#if 1 && (defined linux || defined __linux || defined __linux__)
  setuid( getuid()); /* Drop root privileges forever */
#endif
  if ( INVALID_SOCKET == s)
    {
    int iRet = GetLastError();
    switch ( iRet)
      {
    case ESOCKTNOSUPPORT:
      fprintf( stderr, "ERROR: RAW sockets not supported\n");
      break;
    case EPROTONOSUPPORT:
      fprintf( stderr, "ERROR: ICMP protocol is not supported\n");
      break;
    default:
      fprintf( stderr, "ERROR: unable to create socket: %s (%d)\n",
        strerror( iRet), iRet);
      break;
      }
    return INADDR_NONE;
    }

  addr = DiscoverGateway( s);
  closesocket( s);
  return addr;
  }


/*
 * Get the external IP address (network form) of the default gateway
 * Returns INADDR_NONE on error
 */
u_long DiscoverGateway( SOCKET s)
  {
#if 1 /* Record route - fails with Win2K */
  static u_char opts[ IP_HDR_MAXLEN - IP_HDR_LEN] = { 7, sizeof opts, 4 };
#else /* Timestamp - OK with Win2K */
  static u_char opts[ IP_HDR_MAXLEN - IP_HDR_LEN] = { 0x44, sizeof opts, 5, 1};
#endif
  static u_char buf[ IP_MIN_FRAG];
  struct sockaddr_in dest;
  int i, iRet;
  unsigned len, offset;
  socklen_t n;

  n = sizeof buf;
  if ( getsockopt(
    s,
    IPPROTO_IP, IP_OPTIONS,
    (char*)buf, &n
  )){
    iRet = GetLastError();
    fprintf( stderr, "ERROR: unable to read IP_OPTIONS: %s (%d)\n",
      strerror( iRet), iRet);
    return INADDR_NONE;
    }

  /* Set a short TTL to provoke a Time Exceeded reply */
  iRet = 2;
  if ( setsockopt(
    s,
    IPPROTO_IP, IP_TTL,
    (const char*)&iRet, sizeof iRet
  )){
    iRet = GetLastError();
    fprintf( stderr, "ERROR: unable to set TTL: %s (%d)\n",
      strerror( iRet), iRet);
    return INADDR_NONE;
    }

  /* Ping a host a long way away */
  dest.sin_family = AF_INET;
  dest.sin_addr.s_addr = inet_addr( "1.0.0.1");
  for ( i = 0; i < 3; ++i)
    {
    iRet = Ping( s, &dest, buf, 0);
    /* OK if any reply */
    if ( iRet >= 0)
      break;
    }
  if ( iRet < 0)
    return INADDR_NONE;

  /* Set the RecordRoute IP option */
  if ( setsockopt(
    s,
    IPPROTO_IP, IP_OPTIONS,
    (const char*)opts, sizeof opts
  )){
    iRet = GetLastError();
    fprintf( stderr, "ERROR: unable to set record route IP option: %s (%d)\n",
      strerror( iRet), iRet);
    return INADDR_NONE;
    }

  /* Reset the TTL */
  iRet = 60;
  (void) setsockopt(
    s,
    IPPROTO_IP, IP_TTL,
    (const char*)&iRet, sizeof iRet
  );

  /* Ping the host that replied */
  dest.sin_addr.s_addr = *(u_long*)&buf[ 12]; /* IP source address */
  for ( i = 0; i < 3; ++i)
    {
    iRet = Ping( s, &dest, buf, 0);
    if ( !iRet)
      break;
    }
  if ( iRet < 0)
    return INADDR_NONE;
  if ( iRet > 0)
    {
    fputs( "ERROR: network problem\n", stderr);
    return INADDR_NONE;
    }

  len = (buf[0] & 0xf) * 4; /* IP header length */
  offset = IP_HDR_LEN;

  /* Parse IP options */
  while ( offset < len)
    {
    u_char opt = buf[ offset];
    if ( 0 == opt)
      break;
    else if ( 1 == opt)
      ++offset;
    else if ( offset + 1 >= len)
      break;
    else
      {
      const unsigned optLen = buf[ 1 + offset];
      int j;

      switch ( opt)
        {
      case 7: /* Record route */
        j = buf[ 2 + offset]; /* next free word: 4,8... */
#ifdef DEBUG
        fputs( "Route:", stdout);
        for ( i = 3; i +1 < j; ++i)
          printf( "%s%u",
            3 == (i % 4) ? "  " : ".",
            buf[ i + offset]
          );
        fputs( "\n", stdout);
#endif
        if ( j >= 8)
          {
          /* If 1st != last then 1st else 2nd */
          i = 3;
          if ( j >= 12
            && 0 == memcmp( &buf[ i + offset], &buf[ j - 5 + offset], 4)
          )
            i = 7; /* 2nd host */

          return *(u_long*)&buf[ i + offset];
          }
        break;

      default:
        break;
        }
      offset += optLen;
      }
    }

  fputs( "ERROR: no route recorded\n", stderr);
  return INADDR_NONE;
  }


/*
 * Ping a host
 */
int Ping(                             /* Returns !0 on error */
  SOCKET s,                           /* IN: socket to send from */
  const struct sockaddr_in* pDest,    /* IN: Dest host and TCP src port */
  void* pvData,                       /* I/O: data buffer >= IP_MIN_FRAG bytes */
  size_t sizeData                     /* IN: Send data size */
) {
  static u_short s_seq;
  const u_short kMagic = 0x3456;
  int iRet;
  size_t len;
  struct sockaddr_in from;
  struct SIcmp* pIcmp = (struct SIcmp*) pvData;
  size_t sizePkt = sizeof( struct SIcmp) + sizeData;

  /* Init ICMP data */
  for ( len = sizeof *pIcmp; len < sizeof *pIcmp + sizeData; ++len)
    ((u_char*)pIcmp)[ len] = (u_char)len;

  /* Setup ICMP header */
  pIcmp->type = ICMP_ECHO; /* byte */
  pIcmp->code = 0;
  pIcmp->cksum = 0;
  pIcmp->id = kMagic;
  pIcmp->seq = ++s_seq;

  /* Calc ICMP checksum.  This is endian independent */
  pIcmp->cksum = (u_short) IpChecksum(
    (const u_short*) pIcmp, sizeof *pIcmp + sizeData);

  /* Send the ICMP packet */
  iRet = sendto(
    s,
    (const char*)pvData, sizePkt,
    0*MSG_DONTROUTE,
    (const struct sockaddr*)pDest, sizeof *pDest
  );
  if ( SOCKET_ERROR == iRet)
    {
    iRet = GetLastError();
    fprintf( stderr, "ERROR in sendto: %s (%d)\n", strerror( iRet), iRet);
    return -1;
    }

  /* Wait for reply */
  while ( !s_exit)
    {
    socklen_t iLen;
    static fd_set setRd;
    struct timeval tval;

    FD_ZERO( &setRd);
    FD_SET( s, &setRd);

    /* Set timeout for select */
    tval.tv_usec = 1000 * (DEF_TIMEOUT % 1000);
    tval.tv_sec = DEF_TIMEOUT / 1000;

    /* Wait for reply */
    iRet = select( FD_SETSIZE, &setRd, NULL, NULL, &tval);
    if ( SOCKET_ERROR == iRet)
      {
      iRet = GetLastError();
      fprintf( stderr, "ERROR in select: %s (%d)\n", strerror( iRet), iRet);
      return -1;
      }

    if ( !FD_ISSET( s, &setRd))
      {
      fprintf( stderr, "ERROR: time out\n" );
      return -1;
      }

    sizePkt = sizeData + ICMP_HDR_LEN + IP_HDR_MAXLEN;
    if ( sizePkt < IP_MIN_FRAG)
      sizePkt = IP_MIN_FRAG;

    iLen = sizeof from;
    iRet = recvfrom(
      s,
      (char*)pvData, sizePkt,
      0,
      (struct sockaddr*)&from, &iLen
    );
    if ( SOCKET_ERROR == iRet)
      {
      iRet = GetLastError();
      fprintf( stderr, "ERROR in recvfrom: %s (%d)\n", strerror( iRet), iRet);
      return -1;
      }

    if ( (size_t)iRet < IP_HDR_LEN)
      {
#ifdef DEBUG
      fputs( "Runt datagram\n", stderr);
#endif
      continue;
      }

    /* Ensure ICMP packet */
    if ( 1 != ((const unsigned char*)pvData)[9])
      {
#ifdef DEBUG
      fputs( "Not icmp\n", stderr);
#endif
      continue;
      }

    /* Skip IP header */
    len = (*(const unsigned char*)pvData & 0xf) * 4;
    if ( len < IP_HDR_LEN
      || len + sizeof(struct SIcmp) > (size_t)iRet
    ) {
#ifdef DEBUG
      fputs( "Truncated datagram\n", stderr);
#endif
      continue;
      }

    pIcmp = (SIcmp*)((char*)pvData + len);
    if ( IpChecksum( (const u_short*) pIcmp, iRet - len))
      {
      fputs( "Checksum error\n", stderr);
      return -1;
      }

    /* Check ICMP content */
    switch ( pIcmp->type)
      {
    case ICMP_ECHO:
      break;

    case ICMP_ECHO_REPLY:
      if ( kMagic != pIcmp->id)
        {
#ifdef DEBUG
        fputs( "Invalid icmp id\n", stderr);
#endif
        break;
        }
      if ( s_seq != pIcmp->seq)
        {
#ifdef DEBUG
        fputs( "Incorrect sequence\n", stderr);
#endif
        break;
        }
      return 0;

    case ICMP_DEST_UNREACH:
    case ICMP_TIME_EXCEED:
    case ICMP_PARAM_PROB:
      return 1; /* Unexpected reply */

    default:
      break;
      }
    }

  fputs( "ABORTED\n", stderr);
  return -1;
  }


/*
 * Calc IP checksum
 * This is endian independent (surprisingly) if the checksum
 * is written into an IP packet in host endian form.
 */
unsigned IpChecksum( const u_short* buffer, size_t size)
  {
  u_long cksum = 0;

  while ( size >= (int)sizeof( *buffer))
    {
    cksum += *(buffer++);
    size -= sizeof( *buffer);
    }

  if ( size )
    cksum += ntohs( (u_short)(*(const u_char*)buffer << 8));

  /* Deal with end around carries in high order 16-bits */
  while ( cksum > 0xffff)
    cksum = (cksum >> 16) + (cksum & 0xffff);

  return ~(unsigned)cksum & 0xffff;
  }


#ifdef SIGINT
/*
 * SIGINT handler (ctrl-c)
 */
static void OnSigInt( int sig)
  {
  if ( ++s_exit >= 2)
    exit( EXIT_FAILURE);

#ifdef NDEBUG
  signal( sig, &OnSigInt);
#else
  (void)sig;
#endif
  }
#endif


#if SWS_USE_SOCKLIB && PACKET_DUMP
/*
 * Log file initialisation
 */
static void LogInit( FILE* file, int iVerbosity)
  {
  time_t t = time( NULL);
  setvbuf( file, NULL, _IONBF, 0 ); /* Unbuffered otherwise tail doesn't follow in realtime */
  fputs( kszBanner "\n", file);
  fprintf( file, "Logging(%d) started %s", iVerbosity, ctime( &t) );
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
  int bHelp = 0; /* iArgc <= 1;  Do help if no args */
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
      do
        {
#if 0
        char* pszEnd;
        long lVal;
        unsigned long ulVal;
#endif

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

        case 'a':
          pCfg->eMode = kModeAuto;
          break;

        case 'u':
          pCfg->eMode = kModeUpnp;
          break;

        case 'r':
          pCfg->eMode = kModeRecRoute;
          break;

#if SWS_USE_SOCKLIB && PACKET_DUMP
        case 't':
          /* terse */
          pCfg->iVerbosity--;
          break;

        case 'v':
          /* verbose */
          pCfg->iVerbosity++;
          break;

        case 'o':
          /* logfile */
          pszArg = GetArg();
          if ( '\0' == *pszArg)
            {
            fputs( kszBanner "\n", stderr);
            fputs( "Missing logfile name\n", stderr);
            return EXIT_FAILURE;
            }
          else if ( stdout != pCfg->logfile)
            {
            fputs( kszBanner "\n", stderr);
            fputs( "Duplicate logfile\n", stderr);
            return EXIT_FAILURE;
            }
          else if ( NULL == (pCfg->logfile = fopen( pszArg, "a")))
            {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Failed to open logfile '%s'\n", pszArg);
            return EXIT_FAILURE;
            }

          LogInit( pCfg->logfile, pCfg->iVerbosity);
          pszArg = NULL;
          break;
#endif

        default:
          fputs( kszBanner "\n", stderr);
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
      default:
        fputs( kszBanner "\n", stderr);
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
