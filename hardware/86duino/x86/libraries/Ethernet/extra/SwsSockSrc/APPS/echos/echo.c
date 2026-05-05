/*****************************************************************************
 *
 * File name   : echo.c
 * Function    : UDP/TCP echo/sink client/server
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, BSD sockets
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 04-Mar-03, LVR, Created.
 * 10-May-05, LVR, v2.01. Call SWS_CfgSetName for all builds
 * 04-Jul-05, LVR, v2.02. Merge sink.c & echo.c
 *
 *****************************************************************************/
#ifndef OPT_ECHO
# define OPT_ECHO 1        /* 0= sink, !0= echo client/server */
#endif

#if OPT_ECHO
# define kszApp "Echo"
# define kszAbout "Echo client/service."
#else
# define kszApp "Sink"
# define kszAbout "Discard client/service."
#endif
#define kszVer "2.02"
#define kszBanner kszApp " " kszVer " (" SWS_NET_ID ")" ", " TIMESTAMP "."

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

#include "sws_net.h"
#if SWS_USE_SOCKLIB && PACKET_DUMP
#include "sws_dump.h"
#endif

#include "sws_cfg.h"
#include "echod.h"


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

#define SEND_TIMEOUT  15000U /* Send timeout in millisecs */

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

/* Default parameter values */
#if OPT_ECHO
#define DEF_PACKET_SIZE ((unsigned)sizeof( SPacket))
#define DEF_TIMEOUT     5000 /* Reply timeout in ms */
#define DEF_PORT ECHOD_PORT
#else
#define DEF_PORT DSCRD_PORT
#define DEF_PACKET_SIZE 8U
#endif

#define DEF_TYPE        SOCK_STREAM
#define MAX_PACKET_SIZE UINT_MAX
#define DEF_PERIOD      1000U   /* Inter-packet gap in ms */
#define DEF_REQUESTS    4


/***************************************************************************
 * Data types
 ***************************************************************************/

/* Parameters */
typedef struct SConfig
  {
  int iVerbosity;
  FILE* logfile;

  const char* pszDest;                /* Echo server, NULL= server mode */
  int iSockType;                      /* Socket type */
  u_short uPort;                      /* Echo port */
  u_short uClientPort;                /* Client src port */

  unsigned requests;                  /* No. requests */
  u_long ulPeriod;                    /* Send interval */
#if OPT_ECHO
  int iTimeout;                       /* Rx timeout */
#endif

  FILE* datafile;
  size_t datasize;                    /* Packet size */
  } SConfig;

#if OPT_ECHO
/* Packet format */
typedef struct SPacket
  {
  u_short id;
  u_short seq;
  u_long timestamp;
  } SPacket;
#endif


/***************************************************************************
 * Local function prototypes
 ***************************************************************************/

static int Client( SConfig*);
static int ClientTransaction( SOCKET, const struct sockaddr*, void*, size_t, int, int, u_short, unsigned);

#ifdef SIGINT
static void OnSigInt( int);
#endif

static int ParseArgs( SConfig* const, int, const char* const [] );
static void Help( const char* const);
static void LogInit( FILE*, int);


/***************************************************************************
 * Module data
 ***************************************************************************/

sig_atomic_t volatile g_exit;
sig_atomic_t volatile g_running;
static double s_dBytes;
static unsigned long s_ulSent;
#if OPT_ECHO
static unsigned long s_ulLost;
#endif


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
    "\nSyntax: %.*s [options] [host]\n"
    "-c    : Streaming (TCP) mode (default)\n"
    "-d    : Datagram (UDP) mode\n"
    "-p<n> : Port, default " STRING( DEF_PORT) "\n"
#if OPT_ECHO
    "-w<n> : Receive timeout in milliseconds (default " STRING( DEF_TIMEOUT) ", 0= none)\n"
#endif
    "-l<n> : Client packet size (default %u, max %u)\n"
    "-i<n> : Client send interval in milliseconds (default " STRING( DEF_PERIOD) ", 0= no delay)\n"
    "-k<n> : Client source port\n"
    "-n<n> : Number of requests to send (default " STRING( DEF_REQUESTS) ", 0= infinite)\n"
    "-f<s> : Client file to send\n",
    (int)c, pszCmd,
    DEF_PACKET_SIZE, MAX_PACKET_SIZE
  );
  fprintf( stderr,
    "-o<s> : Set logfile\n"
    "-t    : Decrease verbosity of output\n"
    "-v    : Increase verbosity of output\n"
    "-h    : Help\n"
  );
  }


/*
 * M A I N
 */
int main( int const argc, const char* const argv[] )
  {
  SConfig cfg;
  int iRet;
  struct hostent* pHost;
  char szHost[256];

  /* Set default options */
  cfg.iVerbosity= 1;
  cfg.logfile   = stdout;
  cfg.pszDest   = NULL;
  cfg.iSockType = DEF_TYPE;
  cfg.uPort     = DEF_PORT;
  cfg.uClientPort = 0;
  cfg.requests  = DEF_REQUESTS;
  cfg.ulPeriod  = DEF_PERIOD;
#if OPT_ECHO
  cfg.iTimeout  = DEF_TIMEOUT;
#endif
  cfg.datafile  = NULL;
  cfg.datasize  = DEF_PACKET_SIZE;

  /* Parse args */
  iRet = ParseArgs( &cfg, argc, argv );
  if ( EXIT_SUCCESS != iRet)
    return iRet;

  if ( cfg.iVerbosity >= 0)
    fputs( kszBanner "\n", stderr);

  /* Specify the configuration file */
  if ( !SWS_CfgSetName( NULL, argv[0] ))
    fprintf( stderr, "WARNING: configuration file '" SWS_CFGFILE "' not found\n");

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

  /* Check network up */
  if ( gethostname( szHost, sizeof(szHost))
    || NULL == (pHost = gethostbyname( szHost))
  ) {
    fprintf( stderr, "No network (driver not found)\n");
    return EXIT_FAILURE;
    }

  if ( cfg.iVerbosity >= 0)
    {
    int i;

    fprintf( stderr, "This host is %s", szHost);

    /* Display the list of local IP addresses */
    for ( i = 0; NULL != pHost->h_addr_list[i]; ++i)
      {
      fprintf( stderr, " [%s]",
        inet_ntoa( *(const struct in_addr*)pHost->h_addr_list[i])
      );
      }
    fputs( "\n", stderr);
    }

  if ( NULL == cfg.pszDest)
    {
    struct sockaddr_in host;

    if ( cfg.iVerbosity > 0)
      fputs( "CTRL-C to exit\n\n", stderr);

    host.sin_family = AF_INET;
    host.sin_port = htons( cfg.uPort);
    host.sin_addr.s_addr = INADDR_ANY;

    iRet = Echod( (struct sockaddr*)&host, OPT_ECHO, cfg.iVerbosity, cfg.logfile);
    }
  else
    {
    iRet = Client( &cfg);
    }

  if ( NULL != cfg.datafile)
    fclose( cfg.datafile);

  if ( stdout != cfg.logfile)
    fclose( cfg.logfile);

  return iRet ? EXIT_FAILURE : EXIT_SUCCESS;
  }


/*
 * Client process
 */
static int Client( SConfig* pCfg)
  {
  int iRet;
  SOCKET s;
  void * pv;
  size_t size;
  unsigned u;
  struct sockaddr_in host;
  struct hostent* pHost;
  u_long tReport, tStart;
  unsigned long ulBytesSent;
  const u_long kInterval = 1000;

  host.sin_port = htons( pCfg->uPort );
  host.sin_addr.s_addr = inet_addr( pCfg->pszDest );
  if ( INADDR_ANY == host.sin_addr.s_addr)
    {
    printf( "Invalid host %s\n", pCfg->pszDest );
    return EXIT_FAILURE;
    }
  else if ( INADDR_NONE != host.sin_addr.s_addr)
    {
    host.sin_family = AF_INET;
    }
  /* Try a hostname */
  else if ( NULL != (pHost = gethostbyname( pCfg->pszDest )) )
    {
    host.sin_family = pHost->h_addrtype;
    memcpy( &host.sin_addr, pHost->h_addr, pHost->h_length );
    }
  else
    {
    printf( "Unable to resolve %s\n", pCfg->pszDest);
    return EXIT_FAILURE;
    }

#if OPT_ECHO
  if ( pCfg->datasize < sizeof(SPacket))
    size = sizeof(SPacket);
#endif

  /* Allocate a data buffer */
  pv = malloc( size = pCfg->datasize );
  if ( NULL == pv)
    {
    fprintf( stderr, "Out of memory for data buffer\n");
    return 1;
    }

  /* Init the data */
  if ( NULL != pCfg->datafile)
    {
#if OPT_ECHO
    size_t n = fread( (char*)pv + sizeof(SPacket), 1, pCfg->datasize - sizeof(SPacket), pCfg->datafile);
#else
    size_t n = fread( pv, 1, pCfg->datasize, pCfg->datafile);
#endif
    if ( 0 == n)
      {
      perror( "Failed to read data");
      return EXIT_FAILURE;
      }
    }
  else
    {
    unsigned long ms;
    GetMillisecs( ms);
    srand( (unsigned)ms);
    size = 0;
    while ( size < pCfg->datasize)
      {
      ((u_char*)pv)[ size++] = (u_char)rand();
      }
    }

  s = socket( host.sin_family, pCfg->iSockType, 0);
  if ( INVALID_SOCKET == s)
    {
    iRet = GetLastError();
    switch ( iRet)
      {
    case ESOCKTNOSUPPORT:
      fprintf( stderr, "The requested protocol is not installed\n");
      break;
    default:
      fprintf( stderr, "Failed (%d) to create socket: %s\n",
        iRet, strerror( iRet));
      break;
      }

    free( pv);
    return iRet;
    }

#if OPT_ECHO
  if ( pCfg->iTimeout)
    {
    /* Set recv timeout */
    DECL_TIMEOUT( tv, pCfg->iTimeout);
    iRet = setsockopt(
      s,
      SOL_SOCKET, SO_RCVTIMEO,
      (char*)&tv, sizeof tv
    );
    if ( iRet)
      {
      iRet = GetLastError();
      fprintf( stderr, "Failed (%d) to set receive timeout: %s\n",
        iRet, strerror( iRet));
      }
    }
#endif

  /* Set send timeout */
  { DECL_TIMEOUT( tv, SEND_TIMEOUT);
  setsockopt(
    s,
    SOL_SOCKET, SO_SNDTIMEO,
    (char*)&tv, sizeof tv
  ); }

  if ( SOCK_STREAM == pCfg->iSockType)
    {
    /* Disable Nagle */
    iRet = 1;
    setsockopt(
      s,
      IPPROTO_TCP, TCP_NODELAY,
      (char*)&iRet, sizeof(iRet)
    );

    /* Set send buffer size */
    iRet = (int)pCfg->datasize;
    setsockopt(
      s,
      SOL_SOCKET, SO_SNDBUF,
      (char*)&iRet, sizeof(iRet)
    );

#if OPT_ECHO
    /* NB For echo client, receive buffer must be >= datasize
     * otherwise deadlock can occur */
    iRet = (int)pCfg->datasize;
    setsockopt(
      s,
      SOL_SOCKET, SO_RCVBUF,
      (char*)&iRet, sizeof(iRet)
    );
#endif
    }

#if 0 && defined UDP_NOCHECKSUM /* Disable UDP checksum */
  if ( SOCK_DGRAM == pCfg->iSockType)
    {
    iRet = 1;
    setsockopt(
      s,
      IPPROTO_UDP, UDP_NOCHECKSUM,
      (char*)&iRet, sizeof(iRet)
    );
    }
#endif

  if ( pCfg->iVerbosity >= 0)
    printf( "\n%s " kszApp " %s:%u with %u bytes:\n\n",
      SOCK_STREAM == pCfg->iSockType ? "TCP" : "UDP",
      inet_ntoa( host.sin_addr), ntohs( host.sin_port), (unsigned)size
    );

  ulBytesSent = 0;
  GetMillisecs( tStart);
  tReport = tStart + kInterval;

  /* The client loop */
  iRet = 0;
  for ( u = 1; !pCfg->requests || u <= pCfg->requests; ++u)
    {
    if ( u > 1
      && pCfg->ulPeriod > 0
    ) {
      Sleep( pCfg->ulPeriod);

      if ( NULL == pCfg->datafile)
        {
        /* Shuffle the data */
        memmove( (char*)pv + 1, pv, pCfg->datasize - 1);
        *(u_char*)pv = (u_char) rand();
        }
      }

    /* Check for ctrl-c */
    if ( g_exit)
      {
      puts( "\nInterrupted");
      break;
      }

    /* Send */
    iRet = ClientTransaction(
      s,
      (const struct sockaddr*)&host,
      pv, pCfg->datasize,
      pCfg->iVerbosity,
      pCfg->iSockType,
      pCfg->uClientPort,
      u
    );
    if ( iRet > 0)
      {
      printf( " FAILED (%d): %s\n", iRet, strerror( iRet));
      break;
      }

    if ( 0 == iRet)
      ulBytesSent += pCfg->datasize;

    if ( pCfg->iVerbosity < 1 && pCfg->ulPeriod < kInterval)
      {
      u_long tNow;
      GetMillisecs( tNow);
      if ( tReport <= tNow)
        {
        double dBps = (8000.0 * ulBytesSent) / (tNow - tReport + kInterval);

        if ( dBps >= 1.e9)
          printf( "\r%.3f Gbps\n", dBps / 1.e9);
        else if ( dBps >= 1.e6)
          printf( "\r%.3f Mbps\n", dBps / 1.e6);
        else if ( dBps >= 1.e3)
          printf( "\r%.3f Kbps\n", dBps / 1.e3);
        else
          printf( "\r%.3f bps\n", dBps);

        ulBytesSent = 0;
        tReport = tNow + kInterval;
        }
      }
    }

  GetMillisecs( tReport);
  tReport -= tStart;

  closesocket( s);
  free( pv);

  if ( s_ulSent > 0)
    {
    double dKbps = tReport > 0 ? (8.0 * s_dBytes) / tReport : 0;
#if OPT_ECHO
    printf( "\r%lu packets sent (%.3f MB), %lu lost in %.3f seconds (%.3f kbps)\n",
      s_ulSent, s_dBytes / 1e6, s_ulLost, tReport / 1000.0, dKbps);
#else
    printf( "\r%.3f Mbytes sent in %.3f seconds (%.3f Kbps)\n",
      s_dBytes / 1e6, tReport / 1000.0, dKbps);
#endif
    }

  return iRet;
  }


/*
 * Client transaction
 */
static int ClientTransaction(         /* Returns !0 on error */
  SOCKET s,                           /* IN: socket to send from */
  const struct sockaddr* pHost,       /* IN  Server */
  void* pvData,                       /* IN: data buffer */
  size_t data,                        /* IN: Send data size */
  int iVerbosity,                     /* IN: Print verbosity */
  int iSockType,
  u_short port,
  unsigned packet
) {
  int iRet;
  socklen_t len;
  struct sockaddr_in from;
#if OPT_ECHO
  static u_short s_seq;
  const u_short kMagic = 0x3456;
  SPacket* pPacket;
  size_t sofar;
#endif

  if ( 0 != port)
    {
    /* Check if bound */
    len = sizeof( from);
    iRet = getsockname( s, (struct sockaddr*)&from, &len );
    if ( iRet || 0 == from.sin_port)
      {
      /* Bind to local port */
      from.sin_family = AF_INET;
      from.sin_addr.s_addr = INADDR_ANY;
      from.sin_port = htons( port);
      iRet = bind( s, (struct sockaddr*)&from, sizeof(from) );
      if ( iRet)
        {
        iRet = GetLastError();
        fprintf( stderr, "\rFailed (%d) to bind to port %u: %s\n",
          iRet, ntohs( from.sin_port), strerror( iRet));
        }
      }
    }

  /* Check if connected */
  len = sizeof( from);
  iRet = getpeername( s, (struct sockaddr*)&from, &len );
  if ( iRet)
    {
    /* Connect to server */
    if ( iVerbosity > 0)
      {
      printf( "\r%u Connect %s:%u",
        packet,
        inet_ntoa( ((const struct sockaddr_in*)pHost)->sin_addr),
        ntohs( ((const struct sockaddr_in*)pHost)->sin_port)
      );
      fflush( stdout);
      }

    iRet = connect( s, pHost, sizeof(*pHost) );
    if ( iRet)
      {
      if ( iVerbosity > 0)
        printf( " failed. ");

      iRet = GetLastError();
      if ( ECONNREFUSED == iRet)
        {
        printf( "Server not running\n");
        return 0;
        }
      else if ( ETIMEDOUT == iRet || EAGAIN == iRet || EWOULDBLOCK == iRet)
        {
        printf( "Host not responding\n");
        return -1;
        }
      else
        {
        return iRet;
        }
      }

    if ( iVerbosity > 0)
      {
      printf( "\rConnected to %s:%u\n",
        inet_ntoa( ((const struct sockaddr_in*)pHost)->sin_addr),
        ntohs( ((const struct sockaddr_in*)pHost)->sin_port)
      );
      }
    }

  if ( iVerbosity > 0)
    printf( "\r%u Send...", packet ); fflush( stdout);

#if OPT_ECHO
  /* Format a request packet */
  pPacket = (SPacket*)pvData;
  pPacket->id = kMagic;
  pPacket->seq = ++s_seq;
  GetMillisecs( pPacket->timestamp);
#endif

  /* Send the packet */
  iRet = send( s, (char*)pvData, data, MSG_NOSIGNAL);
  if ( SOCKET_ERROR == iRet)
    {
    iRet = GetLastError();
    switch ( iRet)
      {
    case ETIMEDOUT:
    case EWOULDBLOCK:
#if EAGAIN != EWOULDBLOCK
    case EAGAIN:
#endif
      printf( "\rSend timed out" );
      return iRet;

    case ECONNRESET:
      if ( SOCK_STREAM == iSockType)
        {
        printf( "\rConnection reset" );
        return iRet;
        }

    case ENETUNREACH:
    case EHOSTUNREACH:
      printf( "\rServer not reachable\n" );
      return -1;

    case ECONNABORTED:
      printf( "\rConnection aborted" );
      return iRet;

    default:
      printf( "\rSend" );
      return iRet;
      }
    }

  ++s_ulSent;
  s_dBytes += data;
  if ( (size_t)iRet != data)
    printf( "\rSend truncated\n");

#if OPT_ECHO
  sofar = 0;
  pPacket->id = 0;
  pPacket->seq = 0;

  for ( ;;)
    {
    u_long ticks;

    /* Get the reply */
    if ( iVerbosity > 0)
      printf( "\r%u Recv...", packet ); fflush( stdout);

    len = sizeof(from);
    iRet = recvfrom(
      s,
      (char*)pPacket + sofar, data - sofar,
      0,
      (struct sockaddr*)&from, &len
    );

    GetMillisecs( ticks);

    if ( SOCKET_ERROR == iRet)
      {
      ++s_ulLost;
      iRet = GetLastError();
      switch ( iRet)
        {
      case ETIMEDOUT:
      case EWOULDBLOCK:
#if EAGAIN != EWOULDBLOCK
      case EAGAIN:
#endif
        printf( "\rRecv timed out (%u)\n", (unsigned)sofar );
        return -1;

      case ECONNABORTED:
        printf( "\rConnection closed\n" );
        return 0;

      case ECONNRESET:
        printf( "\r%u Recv reset\n", packet );
        return 0;

      default:
        printf( "\rRecv" );
        return iRet;
        }
      }

    if ( SOCK_STREAM == iSockType)
      {
      sofar += (size_t)iRet;
      if ( sofar < data)
        continue;

      iRet = (int)sofar;
      sofar = 0;
      }
    else if ( (size_t)iRet != data )
      {
      printf( "\rTruncated packet received\n" );
      continue;
      }

    /* Verify packet header */
    if ( kMagic != pPacket->id)
      {
      printf( "\rIncorrect packet ID. Sent %u, received %u\n",
        kMagic, pPacket->id );
      continue;
      }

    if ( s_seq != pPacket->seq)
      {
      printf( "\rIncorrect sequence. Sent %u, received %u\n",
        s_seq, pPacket->seq );
      continue;
      }

    if ( iVerbosity > 0)
      {
      printf( "\rReply from %s:%u sequence %u, %u bytes, %ld ms\n",
        inet_ntoa( from.sin_addr),
        ntohs( from.sin_port),
        pPacket->seq,
        (unsigned)iRet,
        ticks - pPacket->timestamp
      );
      }
    else if ( iVerbosity >= 0)
      {
      printf( "."); fflush( stdout);
      }
    break;
    }
#endif

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
  signal( sig, &OnSigInt);
#else
  (void)sig;
#endif
  }
#endif


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


/*
 * Scan the command line for options
 */
static int ParseArgs(                 /* Returns !0 on error */
  SConfig* const pCfg,                /* OUT: Updated parameters */
  int iArgc,                          /* IN: No. args */
  const char* const pszArgvArray[]    /* IN: Array of arg strings */
) {
  int i;
  int bHelp = 0;
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
        char* pszEnd;
        long lVal;
        unsigned long ulVal;

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

        case 'l':
          /* packet size */
          pszArg = GetArg();
          lVal = strtol( pszArg, &pszEnd, 0);
          if ( *pszEnd
            || lVal < 0
#if MAX_PACKET_SIZE < ULONG_MAX
            || (unsigned long)lVal > MAX_PACKET_SIZE
#endif
          ) {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Invalid data size %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          pCfg->datasize = (size_t)lVal;
          break;

        case 'n':
          /* No. requests */
          pszArg = GetArg();
          lVal = strtol( pszArg, &pszEnd, 10);
          if ( *pszEnd
            || lVal < 0
#if UINT_MAX < ULONG_MAX
            || (unsigned long)lVal > UINT_MAX
#endif
          ) {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Invalid number of requests %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          pCfg->requests = (unsigned)lVal;
          break;

#if OPT_ECHO
        case 'w':
          /* Receive timeout */
          pszArg = GetArg();
          lVal = strtol( pszArg, &pszEnd, 10);
          if ( *pszEnd
            || lVal < 0
#if INT_MAX < LONG_MAX
            || lVal > INT_MAX
#endif
          ) {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Invalid receive timeout %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          pCfg->iTimeout = (int)lVal;
          break;
#endif

        case 'i':
          /* Send delay */
          pszArg = GetArg();
          ulVal = strtoul( pszArg, &pszEnd, 10);
          if ( *pszEnd)
            {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Invalid period %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          pCfg->ulPeriod = ulVal;
          break;

        case 'p':
          /* port */
          pszArg = GetArg();
          lVal = strtol( pszArg, &pszEnd, 10);
          if ( *pszEnd || lVal < 0 || lVal > 65535L)
            {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Invalid port %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          pCfg->uPort = (u_short)lVal;
          break;

        case 'k':
          /* client local port */
          pszArg = GetArg();
          lVal = strtol( pszArg, &pszEnd, 10);
          if ( *pszEnd || lVal < 0 || lVal > 65535L)
            {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Invalid port %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          pCfg->uClientPort = (u_short)lVal;
          break;

        case 'd':
          /* UDP */
          pCfg->iSockType = SOCK_DGRAM;
          break;

        case 'c':
          /* TCP */
          pCfg->iSockType = SOCK_STREAM;
          break;

        case 'f':
          /* datafile */
          if ( NULL != pCfg->datafile)
            {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Duplicate datafile '%s'\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = GetArg();
          pCfg->datafile = fopen( pszArg, "rb");
          if ( NULL == pCfg->datafile)
            {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Failed to open datafile '%s'\n", pszArg);
            return EXIT_FAILURE;
            }

          fseek( pCfg->datafile, 0 , SEEK_END);
          pCfg->datasize = (size_t) ftell( pCfg->datafile);
          fseek( pCfg->datafile, 0 , SEEK_SET);
#if OPT_ECHO
          pCfg->datasize += sizeof( SPacket);
#endif
          pszArg = NULL;
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
      case 0:
        /* Server address */
        pCfg->pszDest = pszArgvArray[i];
        ++iArgs;
        break;

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
