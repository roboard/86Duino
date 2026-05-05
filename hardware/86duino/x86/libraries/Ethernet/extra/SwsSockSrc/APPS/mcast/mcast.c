/*****************************************************************************
 *
 * File name   : mcast.c
 * Function    : UDP multicast client & server test
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, Win32 Console
 * Version     :
 * Release Date:
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 22-Dec-02, LVR, Created.
 *
 *
 * Comments
 * --------
 *
 *****************************************************************************/
#define kszApp "MCAST"
#define kszVer "1.01"
#define kszBanner kszApp " " kszVer " (" SWS_NET_ID ")" ", " TIMESTAMP "."
#define kszAbout "Multicast discard client/server."

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
#include <assert.h>
#if _MSC_VER >= 1000
#include <crtdbg.h>
#endif

#if defined linux || defined __linux || defined __linux__
#include <unistd.h> /* getuid, seteuid */
#endif

#include "sws_net.h"
#if SWS_USE_SOCKLIB && PACKET_DUMP
#include "sws_dump.h"
#endif
#if SWS_USE_SOCKLIB
#include "sws_cfg.h"
#endif


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

/* Default parameter values */
#ifndef DEF_PORT
#define DEF_PORT 9 /* The discard server */
#endif
#ifndef DEF_PACKET_SIZE
#define DEF_PACKET_SIZE ((unsigned)sizeof( SPacket))
#endif
#ifndef MAX_PACKET_SIZE
#define MAX_PACKET_SIZE ((size_t)4096)
#endif
#ifndef DEF_PERIOD
#define DEF_PERIOD 1000U
#endif
#ifndef DEF_TIMEOUT
#define DEF_TIMEOUT 5000
#endif
#ifndef DEF_REQUESTS
#define DEF_REQUESTS 4
#endif

#define MCAST_GROUP 0xefc00001L /* 239.192.0.0 is Organization-Local Scope RFC2365 */


/***************************************************************************
 * Data types
 ***************************************************************************/

/* Parameters */
typedef struct SConfig
  {
  int iVerbosity;
#if PACKET_DUMP
  FILE* logfile;
#endif
  const char* pszGroup;
  u_short sink_port;                  /* Server sink port */
  u_short client_port;                /* Client src port */
  u_long ulPeriod;                    /* Send interval */
  size_t datasize;                    /* Packet size */
  unsigned uRequests;
  int isClient;
  } SConfig;

/* Packet format */
typedef struct SPacket
  {
  u_short id;
  u_short seq;
  u_long timestamp;
  } SPacket;


/***************************************************************************
 * Local function prototypes
 ***************************************************************************/

static int Server( struct sockaddr_in*, int);
static int Client( struct sockaddr_in*, SConfig*);
static int ClientTransaction( SOCKET, const struct sockaddr*, void*, size_t, int, u_short, unsigned);

#ifdef SIGINT
static void OnSigInt( int);
#endif

static int ParseArgs( SConfig* const, int, const char* const [] );
static void Help( const char* const);
#if PACKET_DUMP
static void LogInit( FILE*, int);
#endif


/***************************************************************************
 * Module data
 ***************************************************************************/

static sig_atomic_t volatile s_exit;
static int s_iSent;


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
    "\nSyntax: %.*s [options] [group]\n"
    "-p<n> : Discard server port (default %d)\n"
    "-c[n] : Client mode local port (no default)\n"
    "-l<n> : Client packet size (default %d, max %u)\n"
    "-i<n> : Client send interval in milliseconds (default %d, 0= no delay)\n"
    "-n<n> : Client requests to send (default %u, 0= infinite)\n"
#if PACKET_DUMP
    "-o<s> : Set logfile\n"
#endif
    "-t    : Decrease verbosity of output\n"
    "-v    : Increase verbosity of output\n"
    "-V    : Display version and exit\n"
    "-h    : Help\n",
    (int)c, pszCmd,
    DEF_PORT, DEF_PACKET_SIZE, (unsigned)MAX_PACKET_SIZE, DEF_PERIOD, DEF_REQUESTS
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
  struct sockaddr_in group;

  /* Set default options */
  cfg.iVerbosity = 1;
#if PACKET_DUMP
  cfg.logfile = stdout;
#endif
  cfg.pszGroup = NULL;
  cfg.sink_port = DEF_PORT;
  cfg.client_port = 0;
  cfg.ulPeriod = DEF_PERIOD;
  cfg.datasize = DEF_PACKET_SIZE;
  cfg.uRequests = DEF_REQUESTS;
  cfg.isClient = 0;

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

  /* Check network up */
  if ( gethostname( szHost, sizeof(szHost))
    || NULL == (pHost = gethostbyname( szHost))
  ) {
    fprintf( stderr, "No network (driver not found)\n");
    return EXIT_FAILURE;
    }

  group.sin_family = AF_INET;
  group.sin_port = htons( cfg.sink_port);
  group.sin_addr.s_addr = htonl( MCAST_GROUP);
  if ( NULL != cfg.pszGroup)
    {
    group.sin_addr.s_addr = inet_addr( cfg.pszGroup );
    if ( INADDR_ANY == group.sin_addr.s_addr
      || INADDR_NONE == group.sin_addr.s_addr
    ) {
      printf( "Invalid group address %s\n", cfg.pszGroup );
      return EXIT_FAILURE;
      }
    }

  if ( !cfg.isClient)
    iRet = Server( &group, cfg.iVerbosity);
  else
    iRet = Client( &group, &cfg);

#if PACKET_DUMP
  if ( stdout != cfg.logfile)
    fclose( cfg.logfile);
#endif

  return iRet ? EXIT_FAILURE : EXIT_SUCCESS;
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


/*
 * Discard server
 */
static int Server( struct sockaddr_in* pGroup, int iVerbosity)
  {
  int iRet;
  SOCKET s;
  struct sockaddr_in from;
  const size_t size = MAX_PACKET_SIZE;
  void* pv;
#if SWS_USE_SOCKLIB || !(defined WIN32 || defined _WIN32 || defined __WIN32__)
  struct ip_mreq mreq;
#endif

  /* Get a data buffer */
  pv = malloc( size);
  if ( NULL == pv)
    {
    fprintf( stderr, "Out of memory for receive buffer\n");
    return 1;
    }

#if SWS_USE_SOCKLIB || !(defined WIN32 || defined _WIN32 || defined __WIN32__)
  /* Open server socket */
  s = socket( PF_INET, SOCK_DGRAM, 0);
  if ( INVALID_SOCKET == s)
    {
    iRet = GetLastError();
    fprintf( stderr, "Failed (%d) to create socket: %s\n",
      iRet, strerror( iRet));

    free( pv);
    return iRet;
    }

  mreq.imr_multiaddr.s_addr = pGroup->sin_addr.s_addr;
  mreq.imr_interface.s_addr = INADDR_ANY;
  iRet = setsockopt( s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof( mreq) );
  if ( iRet)
    {
    iRet = GetLastError();
    fprintf( stderr, "Failed (%d) to join multicast group: %s\n",
      iRet, strerror( iRet));

    free( pv);
    return iRet;
    }

  /* Select the port to receive on */
  from = *pGroup;
  from.sin_family = AF_INET;
  from.sin_addr.s_addr = mreq.imr_interface.s_addr;
  iRet = bind( s, (struct sockaddr*)&from, sizeof(from) );
  if ( iRet)
    {
    iRet = GetLastError();
    fprintf( stderr, "Failed (%d) to bind to port %u: %s\n",
      iRet, ntohs( from.sin_port), strerror( iRet));

    closesocket( s);
    free( pv);
    return iRet;
    }

  from.sin_addr.s_addr = mreq.imr_multiaddr.s_addr;
#if defined linux || defined __linux || defined __linux__
  setuid( getuid()); /* Drop root privileges forever */
#endif
#else
  {
  SOCKET sLeaf;
  WSABUF buf;

  buf.len = 0;
  buf.buf = NULL;

  /* Open server socket */
  s = WSASocket(
    PF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_MULTIPOINT_C_LEAF | WSA_FLAG_MULTIPOINT_D_LEAF);
  if ( INVALID_SOCKET == s)
    {
    iRet = GetLastError();
    fprintf( stderr, "Failed (%d) to create socket: %s\n",
      iRet, strerror( iRet));

    free( pv);
    return iRet;
    }

  from = *pGroup;
  sLeaf = WSAJoinLeaf(
    s, (struct sockaddr*)&from, sizeof( from), NULL, &buf, NULL, NULL, JL_BOTH);
  if ( INVALID_SOCKET == sLeaf)
    {
    iRet = GetLastError();
    fprintf( stderr, "Failed (%d) to join multicast group: %s\n",
      iRet, strerror( iRet));

    free( pv);
    return iRet;
    }
  assert( sLeaf == s);
  iRet = 0;
  }
#endif

  if ( iVerbosity >= 0)
    {
    printf( "Multicast discard server listening on %s:%u \n\n",
      inet_ntoa( from.sin_addr ),
      ntohs( from.sin_port)
    );
    }

  /* The Discard server loop */
  for ( ;;)
    {
    socklen_t len;
    fd_set rd;
    struct timeval tval;

    /* Check for ctrl-c */
    if ( s_exit)
      {
      puts( "\nInterrupted");
      break;
      }

    FD_ZERO( &rd);
    FD_SET( s, &rd);

    if ( iVerbosity >= 0)
      {
      printf( "\rListening..." );
      fflush( stdout);
      }

    /* Set timeout for select */
#   define TIMEOUT 2000
    tval.tv_usec = (TIMEOUT % 1000) * 1000L;
    tval.tv_sec = TIMEOUT / 1000;

    /* Wait for receive event */
    iRet = select( FD_SETSIZE, &rd, NULL, NULL, &tval);
    if ( 0 == iRet)
      continue; /* Timeout to check ctrl-c */

    if ( SOCKET_ERROR == iRet)
      {
      iRet = GetLastError();
      printf( "\rSelect failed (%d): %s\n", iRet, strerror( iRet));
      break;
      }

    /* Process receive event */
    if ( FD_ISSET( s, &rd))
      {
      /* Receive a datagram */
      len = sizeof(from);
      iRet = recvfrom(
        s,
        (char*)pv, size,
        0,
        (struct sockaddr*)&from, &len
      );

      if ( SOCKET_ERROR == iRet)
        {
        iRet = GetLastError();
        if ( ECONNRESET == iRet)
          printf( "\rClient not reachable\n" );
        else
          printf( "\rReceive failed (%d): %s\n", iRet, strerror( iRet));
        continue;
        }

      if ( iVerbosity > 0)
        {
        printf( "\rReceived %u bytes from %s:%u\n",
          (unsigned)iRet, inet_ntoa( from.sin_addr), ntohs( from.sin_port) );
        }

      /* Discard the datagram */
      }
    }

#if SWS_USE_SOCKLIB || !(defined WIN32 || defined _WIN32 || defined __WIN32__)
  setsockopt( s, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof( mreq) );
#endif

  closesocket( s);
  free( pv);

  return iRet;
  }


/*
 * Client process
 */
static int Client( struct sockaddr_in* pGroup, SConfig* pCfg)
  {
  int iRet;
  SOCKET s;
  void * pv;
  size_t size;
  unsigned u;

  /* Allocate a data buffer */
  if ( pCfg->datasize < sizeof( SPacket))
    pCfg->datasize = sizeof( SPacket);

  pv = malloc( size = pCfg->datasize );
  if ( NULL == pv)
    {
    fprintf( stderr, "Out of memory for data buffer\n");
    return 1;
    }

  /* Open client socket */
  s = socket( PF_INET, SOCK_DGRAM, 0);
  if ( INVALID_SOCKET == s)
    {
    iRet = GetLastError();
    fprintf( stderr, "Failed (%d) to create socket: %s\n",
      iRet, strerror( iRet));

    free( pv);
    return iRet;
    }

  if ( pCfg->iVerbosity >= 0)
    printf( "Multicast discard %s:%u using %u bytes of data:\n\n",
      inet_ntoa( pGroup->sin_addr), ntohs( pGroup->sin_port), (unsigned)size
    );

  /* The client loop */
  iRet = 0;
  for ( u = 1; !pCfg->uRequests || u <= pCfg->uRequests; ++u)
    {
    /* Check for ctrl-c */
    if ( s_exit)
      {
      puts( "\nInterrupted");
      break;
      }

    iRet = ClientTransaction(
      s,
      (const struct sockaddr*)pGroup,
      pv, pCfg->datasize,
      pCfg->iVerbosity,
      pCfg->client_port,
      u
    );
    if ( iRet > 0)
      {
      printf( " FAILED (%d): %s\n", iRet, strerror( iRet));
      break;
      }

    /* Check for ctrl-c */
    if ( s_exit)
      {
      puts( "\nInterrupted");
      break;
      }

    if ( iRet >= 0)
      Sleep( pCfg->ulPeriod);
    }

  closesocket( s);
  free( pv);

  if ( s_iSent)
    printf( "\r%d packets sent\n", s_iSent);

  return iRet;
  }


/*
 * Client transaction
 */
static int ClientTransaction(                 /* Returns !0 on error */
  SOCKET s,                           /* IN: socket to send from */
  const struct sockaddr* pGroup,      /* IN  M/cast group */
  void* pvData,                       /* IN: data buffer */
  size_t data,                        /* IN: Send data size */
  int iVerbosity,                     /* IN: Print verbosity */
  u_short port,
  unsigned packet
) {
  static u_short s_seq;
  const u_short kMagic = 0x3456;
  int iRet;
  size_t len;
  socklen_t slen;
  struct sockaddr_in from;
  SPacket* pPacket;

  if ( 0 != port)
    {
    /* Check if bound */
    slen = sizeof( from);
    iRet = getsockname( s, (struct sockaddr*)&from, &slen );
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
  slen = sizeof( from);
  iRet = getpeername( s, (struct sockaddr*)&from, &slen );
  if ( iRet)
    {
    /* Connect to server */
    iRet = connect( s, pGroup, sizeof(*pGroup) );
    if ( iRet)
      {
      iRet = GetLastError();
      if ( ECONNREFUSED == iRet)
        {
        printf( "\rDiscard server not running\n");
        return 0;
        }
      else if ( ETIMEDOUT == iRet || EAGAIN == iRet || EWOULDBLOCK == iRet)
        {
        printf( "\rConnect failed, host not responding\n");
        return -1;
        }
      else
        {
        printf( "\rConnect ");
        return iRet;
        }
      }
    }

  if ( iVerbosity > 0)
    printf( "\r%u Send...", packet); fflush( stdout);

  /* Format a request packet */
  pPacket = (SPacket*)pvData;
  pPacket->seq = ++s_seq;

  for ( len = sizeof( SPacket); len < data; ++len)
    ((u_char*)pPacket)[ len] = (u_char)(len + s_seq);

  pPacket->id = kMagic; /* byte */
  GetMillisecs( pPacket->timestamp);

  /* Send the packet */
  iRet = send( s, (char*)pPacket, data, 0);
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
      printf( "\rSend timed out\n" );
      return SOCKET_ERROR;

    case ECONNRESET:
      printf( "\rServer not running\n" );
      return 0;

    default:
      printf( "\rSend" );
      return iRet;
      }
    }

  ++s_iSent;
  if ( (size_t)iRet != data)
    printf( "\rSend truncated\n");

  return 0;
  }


#if PACKET_DUMP
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
          if ( *pszEnd || lVal < 0 || (unsigned long)lVal > MAX_PACKET_SIZE)
            {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Invalid data size %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          pCfg->datasize = (size_t)lVal;
          pCfg->isClient = 1;
          break;

        case 'n':
          /* No. requests */
          pszArg = GetArg();
          lVal = strtol( pszArg, &pszEnd, 0);
          if ( *pszEnd || lVal < 0 || (unsigned long)lVal > UINT_MAX)
            {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Invalid number of requests %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          pCfg->uRequests = (unsigned)lVal;
          pCfg->isClient = 1;
          break;

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
          pCfg->isClient = 1;
          break;

        case 'p':
          /* sink port */
          pszArg = GetArg();
          lVal = strtol( pszArg, &pszEnd, 0);
          if ( *pszEnd || lVal < 0 || lVal > 65535L)
            {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Invalid port %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          pCfg->sink_port = (u_short)lVal;
          pCfg->isClient = 0;
          break;

        case 'c':
          /* client local port */
          lVal = strtol( pszArg, &pszEnd, 0);
          if ( *pszEnd)
            {
            pCfg->isClient = 1;
            }
          else if ( lVal < 0 || lVal > 65535L)
            {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Invalid port %s\n", pszArg);
            return EXIT_FAILURE;
            }
          else
            {
            pszArg = NULL;
            pCfg->client_port = (u_short)lVal;
            pCfg->isClient = 1;
            }
          break;

#if PACKET_DUMP
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
      case 0:
        /* IP group address */
        pCfg->pszGroup = pszArgvArray[i];
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
