/*****************************************************************************
 *
 * File name   : mping.c
 * Function    : Multicast ICMP echo client & server
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, sockets
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 03-Oct-05, LVR, 1.0  Created.
 * 16-Aug-09, LVR, 1.02 Add server.
 *
 *
 * Comments
 * --------
 *
 *****************************************************************************/
#define kszApp "MPING"
#define kszVer "1.02"
#define kszBanner kszApp " " kszVer " (" SWS_NET_ID ")" ", " TIMESTAMP "."
#define kszAbout "Multicast ping client/server."

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
# define TIMESTAMP "edited " __TIMESTAMP__
#else
# define TIMESTAMP "built " __TIME__ " " __DATE__
#endif

/* Expand and stringize a macro name */
#define STR(s) #s
#define STRING(s) STR(s)

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

#define ICMP_HDR_LEN  8U
#define IP_HDR_LEN    20U
#define IP_HDR_MAXLEN 60U

/* Default parameter values */
#define DEF_PACKET_SIZE 8
#define MAX_PACKET_SIZE (0xffff - IP_HDR_LEN - ICMP_HDR_LEN)
#define DEF_REQUESTS 4
#define DEF_PERIOD  1000U /* Client send interval, millisecs */
#define DEF_TIMEOUT 1000U /* Client receive timeout, millisecs */
#define POLL_TIMEOUT 333U /* Server select timeout, millisecs  */

#define MCAST_GROUP 0xefc00001L /* 239.192.0.0 is Organization-Local Scope RFC2365 */


/***************************************************************************
 * Data types
 ***************************************************************************/

/* Parameters */
typedef struct SConfig
  {
  int iVerbosity;
#if SWS_USE_SOCKLIB && PACKET_DUMP
  FILE* logfile;
#endif
  const char* pszGroup;
  int isClient;
  u_long ulPeriod;                    /* Send interval */
  unsigned uRequests;
  unsigned uTimeout;                  /* Rx timeout */
  size_t datasize;                    /* Packet size */
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


/***************************************************************************
 * Local function prototypes
 ***************************************************************************/

static int Server( struct sockaddr_in*, int);
static int Client( struct sockaddr_in*, SConfig*);
static int ClientTransaction(
  SOCKET, const struct sockaddr*, void*, size_t, unsigned, int);
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
static int s_iSent, s_iLost;


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
    "-c :    Client mode\n"
    "-l<n> : Client packet size (default %d, max %u)\n"
    "-n<n> : Client requests to send (default %u, 0= infinite)\n"
    "-i<n> : Client send interval in milliseconds (default %u, 0= no delay)\n"
    "-w<n> : Client reply timeout in milliseconds (default %u)\n"
#if SWS_USE_SOCKLIB && PACKET_DUMP
    "-o<s> : Set logfile\n"
#endif
    "-t    : Decrease verbosity of output\n"
    "-v    : Increase verbosity of output\n"
    "-V    : Display version and exit\n"
    "-h    : Help\n",
    (int)c, pszCmd,
    DEF_PACKET_SIZE, (unsigned)MAX_PACKET_SIZE, DEF_REQUESTS,
    DEF_PERIOD, DEF_TIMEOUT
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
#if SWS_USE_SOCKLIB && PACKET_DUMP
  cfg.logfile = stdout;
#endif
  cfg.pszGroup = NULL;
  cfg.ulPeriod = DEF_PERIOD;
  cfg.datasize = DEF_PACKET_SIZE;
  cfg.uRequests = DEF_REQUESTS;
  cfg.uTimeout = DEF_TIMEOUT;
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
  group.sin_port = 0;
  group.sin_addr.s_addr = htonl( MCAST_GROUP);
  if ( NULL != cfg.pszGroup)
    {
    /* Parse destination group IP address */
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

#if SWS_USE_SOCKLIB && PACKET_DUMP
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

#if defined NDEBUG
  signal( sig, &OnSigInt);
#else
  (void)sig;
#endif
  }
#endif


/*
 * Multicast ICMP echo server
 */
static int Server( struct sockaddr_in* pGroup, int iVerbosity)
  {
  int iRet;
  SOCKET s;
  struct sockaddr_in from;
#if SWS_USE_SOCKLIB || !(defined WIN32 || defined _WIN32 || defined __WIN32__)
  struct ip_mreq mreq;
#endif

  /* Enable reception of multicast packets to the group */
#if SWS_USE_SOCKLIB || !(defined WIN32 || defined _WIN32 || defined __WIN32__)
  s = socket( PF_INET, SOCK_RAW, IPPROTO_ICMP);
  if ( INVALID_SOCKET == s)
    {
    iRet = GetLastError();
    fprintf( stderr, "Failed to create socket (%d): %s\n",
      iRet, strerror( iRet));
    return iRet;
    }

  mreq.imr_multiaddr.s_addr = pGroup->sin_addr.s_addr;
  mreq.imr_interface.s_addr = INADDR_ANY;
  iRet = setsockopt( s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof( mreq) );
  if ( iRet)
    {
    iRet = GetLastError();
    fprintf( stderr, "Failed to join multicast group (%d): %s\n",
      iRet, strerror( iRet));
    return iRet;
    }

  from = *pGroup;
  from.sin_family = AF_INET;
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
    AF_INET, SOCK_RAW, IPPROTO_ICMP, NULL, 0, WSA_FLAG_MULTIPOINT_C_LEAF | WSA_FLAG_MULTIPOINT_D_LEAF);
  if ( INVALID_SOCKET == s)
    {
    iRet = GetLastError();
    fprintf( stderr, "Failed to create socket (%d): %s\n",
      iRet, strerror( iRet));
    return iRet;
    }

  from = *pGroup;
  sLeaf = WSAJoinLeaf(
    s, (struct sockaddr*)&from, sizeof( from), NULL, &buf, NULL, NULL, JL_BOTH);
  if ( INVALID_SOCKET == sLeaf)
    {
    iRet = GetLastError();
    fprintf( stderr, "Failed to join multicast group (%d): %s\n",
      iRet, strerror( iRet));
    return iRet;
    }
  }
#endif

  if ( iVerbosity >= 0)
    {
    printf( "Multicast icmp echo server %s.  CTRL-C to exit.\n",
      inet_ntoa( from.sin_addr )
    );
    }

  /* The server loop */
  for ( ;;)
    {
    static unsigned char s_buf[ 32767];
    socklen_t sLen;
    static fd_set setRd;
    struct timeval tval;
    struct SIcmp* pIcmp;
    unsigned len;

    /* Check for ctrl-c */
    if ( s_exit)
      {
      puts( "\nInterrupted");
      break;
      }

    FD_ZERO( &setRd);
    FD_SET( s, &setRd);

    /* Set timeout for select */
    tval.tv_usec = (POLL_TIMEOUT % 1000) * 1000UL;
    tval.tv_sec = POLL_TIMEOUT / 1000;

    /* Wait for packet */
    iRet = select( FD_SETSIZE, &setRd, NULL, NULL, &tval);
    if ( SOCKET_ERROR == iRet)
      {
      iRet = GetLastError();
      fprintf( stderr, "select failed(%d):%s\n", iRet, strerror( iRet));
      continue;
      }
    else if ( 0 == iRet)
      continue;

    sLen = sizeof from;
    iRet = recvfrom( s, (char*)s_buf, sizeof s_buf, 0, (struct sockaddr*)&from, &sLen);
    if ( SOCKET_ERROR == iRet)
      {
      iRet = GetLastError();
      fprintf( stderr, "recfrom failed(%d):%s\n", iRet, strerror( iRet));
      continue;
      }

    /* Ensure ICMP packet */
    if ( (size_t)iRet < IP_HDR_LEN)
      continue;
    if ( 1 != s_buf[9])
      continue;

    /* Ensure sent to our m/cast group */
    if ( memcmp( &pGroup->sin_addr.s_addr, &s_buf[16], 4))
      continue;

    /* Skip IP header */
    len = (s_buf[0] & 0xf) * 4;
    if ( len < IP_HDR_LEN
      || len + sizeof(struct SIcmp) > (size_t)iRet
    ) {
      continue;
      }

    /* Verify ICMP checksum */
    pIcmp = (SIcmp*)&s_buf[ len];
    if ( IpChecksum( (const u_short*) pIcmp, (size_t)iRet - len))
      {
      fprintf( stderr, "Bad checksum\n");
      continue;
      }

    /* Check ICMP content */
    switch ( pIcmp->type)
      {
    case ICMP_ECHO:
      printf( "Echo request from %s\n", inet_ntoa( from.sin_addr) );
      pIcmp->type = ICMP_ECHO_REPLY;

      /* Calc checksum.  This is endian independent */
      len = (size_t)iRet - len;
      pIcmp->cksum = 0;
      pIcmp->cksum = (u_short) IpChecksum( (const u_short*) pIcmp, len);

      /* Echo the packet */
      iRet = sendto( s, (char*)pIcmp, len, 0, (struct sockaddr*)&from, sizeof from);
      if ( SOCKET_ERROR == iRet)
        {
        iRet = GetLastError();
        fprintf( stderr, "sendto failed(%d):%s\n", iRet, strerror( iRet));
        }
      break;

    default:
      printf( "From %s: Type %d, code %d\n",
        inet_ntoa( from.sin_addr), pIcmp->type, pIcmp->code );
      break;
      }
    }

#if SWS_USE_SOCKLIB || !(defined WIN32 || defined _WIN32 || defined __WIN32__)
  setsockopt( s, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof( mreq) );
#endif

  closesocket( s);

  return 0;
  }


/*
 * Multicast ICMP echo client
 */
static int Client( struct sockaddr_in* pGroup, SConfig* pCfg)
  {
  int iRet;
  SOCKET s;
  void * pv;
  unsigned u;

  /* Allocate a data buffer */
  pv = malloc( ICMP_HDR_LEN + IP_HDR_MAXLEN + pCfg->datasize );
  if ( NULL == pv)
    {
    fprintf( stderr, "Out of memory for data buffer\n");
    return 1;
    }

  /* Open client socket */
  s = socket( PF_INET, SOCK_RAW, IPPROTO_ICMP);
#if defined linux || defined __linux || defined __linux__
  setuid( getuid()); /* Drop root privileges forever */
#endif
  if ( INVALID_SOCKET == s)
    {
    iRet = GetLastError();
    fprintf( stderr, "Failed to create socket (%d): %s\n",
      iRet, strerror( iRet));

    free( pv);
    return iRet;
    }

  if ( pCfg->iVerbosity >= 0)
    printf( "Multicast ping %s with %u bytes of data:\n\n",
      inet_ntoa( pGroup->sin_addr), (unsigned)pCfg->datasize
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
      pCfg->uTimeout,
      pCfg->iVerbosity
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
  unsigned uTimeout,                  /* IN: Recv timeout */
  int iVerbosity                      /* IN: Print verbosity */
) {
  static u_short s_seq;
  const u_short kMagic = 0x3456;
  int iRet, iReplies;
  size_t len;
  SIcmp* pPacket;
  u_long tSend;

  /* Format a request packet */
  pPacket = (SIcmp*)pvData;
  pPacket->type = ICMP_ECHO;
  pPacket->code = 0;
  pPacket->cksum = 0;
  pPacket->id = kMagic;
  pPacket->seq = ++s_seq;

  /* Init ICMP data */
  for ( len = 0; len < data; ++len)
    ((u_char*)pvData)[ len + sizeof( SIcmp)] = (u_char)(len + s_seq);

  /* Calc checksum.  This is endian independent */
  pPacket->cksum = (u_short) IpChecksum(
    (const u_short*) pPacket, sizeof( *pPacket) + data);

  if ( iVerbosity > 0)
    printf( "Send"); fflush( stdout);

  GetMillisecs( tSend);

  /* Send the packet */
  iRet = sendto(
    s, (char*)pPacket, sizeof( *pPacket) + data, 0, pGroup, sizeof(*pGroup));
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
      printf( "\rSend: timed out\n" );
      return iRet;

    case ENETUNREACH:
    case EHOSTUNREACH:
    case ECONNRESET:
      printf( "\rSend: Destination host unreachable\n" );
      return 0;

    default:
      printf( "\rSend" );
      return iRet;
      }
    }

  ++s_iSent;
  if ( (size_t)iRet != sizeof( *pPacket) + data)
    printf( "\rSend truncated\n");
  else if ( iVerbosity >= 0)
    putchar( '.' ), fflush( stdout);

  iReplies = 0;

  /* Wait for reply */
  while ( !s_exit)
    {
    socklen_t sLen;
    static fd_set setRd;
    struct SIcmp* pIcmp;
    struct sockaddr_in from;
    u_long tRecv;
    struct timeval tval;

    FD_ZERO( &setRd);
    FD_SET( s, &setRd);

    if ( iVerbosity > 0)
      printf( "\rRecv" ), fflush( stdout);

    /* Set timeout for select */
    tval.tv_usec = (uTimeout % 1000) * 1000UL;
    tval.tv_sec = uTimeout / 1000;

    /* Wait for reply */
    iRet = select( FD_SETSIZE, &setRd, NULL, NULL, &tval);
    GetMillisecs( tRecv);
    if ( SOCKET_ERROR == iRet)
      {
      ++s_iLost;
      return GetLastError();
      }

    if ( 0 == iRet)
      {
      if ( iReplies)
        return 0;

      ++s_iLost;

      if ( iVerbosity > 0)
        printf( "\rRecv: timed out\n" );
      else if ( iVerbosity >= 0)
        putchar( 'X'), fflush( stdout);

      return -1;
      }

    sLen = sizeof(from);
    iRet = recvfrom(
      s,
      (char*)pvData, data + ICMP_HDR_LEN + IP_HDR_MAXLEN,
      0,
      (struct sockaddr*)&from, &sLen
    );
    if ( SOCKET_ERROR == iRet)
      {
      iRet = GetLastError();
      switch ( iRet)
        {
      case EMSGSIZE:
        printf( "\rTruncated packet received\n" );
        continue;

      case ENETUNREACH:
      case EHOSTUNREACH:
      case ECONNRESET:
        ++s_iLost;
        printf( "\rRecv: Destination host unreachable\n" );
        return -1;

      default:
        ++s_iLost;
        printf( "\rRecv:" );
        return iRet;
        }
      }

    if ( (size_t)iRet < IP_HDR_LEN)
      {
      printf( "\rRunt packet received\n" );
      continue;
      }

    /* Ensure ICMP packet */
    if ( 1 != ((const unsigned char*)pvData)[9])
      continue;

    /* Skip IP header */
    len = (*(const unsigned char*)pvData & 0xf) * 4;
    if ( len < IP_HDR_LEN
      || len + sizeof(struct SIcmp) > (size_t)iRet
    ) {
      printf( "\rInvalid packet received\n" );
      continue;
      }

    /* Check ICMP content */
    pIcmp = (SIcmp*)((char*)pvData + len);
    switch ( pIcmp->type)
      {
    default:
    case ICMP_TIME_REPLY:
    case ICMP_INFO_REPLY:
    case ICMP_AM_REPLY:
    case ICMP_ECHO:
      printf( "\rReply from %s: Type %d, code %d\n",
        inet_ntoa( from.sin_addr), pIcmp->type, pIcmp->code );
      break;

    case ICMP_ECHO_REPLY:
      if ( kMagic != pIcmp->id)
        {
        printf( "\rIncorrect ID received %u, sent %u\n", pIcmp->id, kMagic );
        break;
        }
      if ( s_seq != pIcmp->seq)
        {
        printf( "\rIncorrect sequence received %u, sent %u\n",
          pIcmp->seq, s_seq );
        break;
        }

      if ( iVerbosity > 0)
        {
        len = (unsigned)iRet - (len + sizeof( struct SIcmp));
        tRecv -= tSend;

        printf( "\rReply from %s: sequence %u, %u bytes, %lu ms\n",
          inet_ntoa( from.sin_addr),
          pIcmp->seq,
          (unsigned)len,
          tRecv
        );
        }
      else if ( iVerbosity >= 0)
        putchar( ':'), fflush( stdout);

      ++iReplies;
      break;

    case ICMP_QUENCH:
      printf( "\rReply from %s: Source quench (%d)\n",
        inet_ntoa( from.sin_addr), pIcmp->code );
      break;

    case ICMP_DEST_UNREACH:
      printf( "\rReply from %s: ", inet_ntoa( from.sin_addr));
      switch ( pIcmp->code)
        {
      case 0:
        printf( "Destination net unreachable\n");
        break;

      case 1:
        printf( "Destination host unreachable\n");
        break;

      case 2:
        printf( "Destination protocol unreachable\n");
        break;

      case 3:
        printf( "Destination port unreachable\n");
        break;

      default:
        printf( "Destination not reachable (%d)\n", pIcmp->code );
        break;
        }
      break;

    case ICMP_TIME_EXCEED:
      printf( "\rReply from %s: Time exceeded (%d)\n",
        inet_ntoa( from.sin_addr), pIcmp->code );
      break;

    case ICMP_PARAM_PROB:
      printf( "\rReply from %s: Parameter problem\n",
        inet_ntoa( from.sin_addr));
      break;
      }
    }

  return 0;
  }


/*
 * Calc IP checksum
 * This is endian independent (surprisingly) if the checksum
 * is written into an IP packet in host endian form.
 */
static unsigned IpChecksum( const u_short* buffer, size_t size)
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

        case 'w':
          /* Receive timeout */
          pszArg = GetArg();
          lVal = strtol( pszArg, &pszEnd, 10);
          if ( *pszEnd || lVal < 0 || (unsigned long)lVal > UINT_MAX)
            {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Invalid receive timeout %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          pCfg->uTimeout = (unsigned)lVal;
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

        case 'c':
          /* client */
          pCfg->isClient = 1;
          break;

#if SWS_USE_SOCKLIB && PACKET_DUMP
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
