/*****************************************************************************
 *
 * File name   : ping.c
 * Function    : simple ping
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, sockets
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 03-May-01, LVR, v1.00 Created.
 * 03-Oct-01, LVR, v1.01 Add VRP_Open call.
 * 07-Mar-03, LVR, v2 rewrite
 * 20-Aug-03, LVR, v2.01 Enable broadcast to 255.255.255.255
 * 15-Apr-05, LVR, v2.02 Added OPT_PING_RAWIP
 * 08-Aug-06, LVR, v2.03 Linux raw sockets
 * 07-Jul-09, LVR, v2.04 Replaced OPT_PING_RAWIP with IP_HDRINCL
 * 13-Jul-09, LVR, v2.05 Added record-route option & broadcasts
 * 04-Feb-10, LVR, v2.06 Mods for CGI
 *
 * Comments
 * --------
 *
 *****************************************************************************/
#define kszApp "ping"
#define kszVer "2.06"
#define kszBanner kszApp " " kszVer " (" SWS_NET_ID ")" ", " TIMESTAMP "."
#define kszAbout "ICMP echo client."

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
#define DEF_PACKET_SIZE 32U
#define MAX_PACKET_SIZE (0xffff - IP_HDR_LEN - ICMP_HDR_LEN)
#define DEF_PERIOD  1000U
#define DEF_PINGS      4U
#define DEF_TIMEOUT 1000U  /* Rx timeout in milliseconds */

#define SEND_TIMEOUT 5000U /* Milliseconds */


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
  const char* pszDest;
  struct sockaddr_in dest;
#ifdef IP_HDRINCL
  u_long src; /* spoofed host, network form */
#endif
  u_long dst; /* Network form */
  int isBroadcast;
  u_short dstPort;
  u_long ulPeriod;
  unsigned pings;
  unsigned uTimeout;  /* Rx timeout */
  size_t datasize;
  unsigned type;
  unsigned code;
#if defined IP_HDRINCL || defined IP_TTL
  unsigned ttl;
#endif
#if defined IP_HDRINCL || defined IP_TOS
  unsigned tos;
#endif
#if defined IP_HDRINCL || defined IP_DONTFRAGMENT
  int df;
#endif
#if defined IP_HDRINCL || defined IP_OPTIONS
  int isRecordRoute;
#endif
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

/* IP header */
typedef struct Sip
  {
  u_char ihl;
  u_char tos;
  u_short len;
  u_short id;
  u_short frag;
  u_char ttl;
  u_char protocol;
  u_short checksum;
  u_long src;
  u_long dst;
  } Sip;

/* TCP header */
typedef struct Stcp
  {
  u_short src;
  u_short dst;
  u_long seq;
  u_long ack;
  u_short flags;
  u_short window;
  u_short checksum;
  u_short urgent;
  } Stcp;

/* Type requirements */
STATIC_ASSERT( sizeof(u_short) == 2);
STATIC_ASSERT( sizeof(u_long) == 4);
STATIC_ASSERT( sizeof(SIcmp) == ICMP_HDR_LEN);
STATIC_ASSERT( sizeof(Sip) == IP_HDR_LEN);


/***************************************************************************
 * Local function prototypes
 ***************************************************************************/

static SOCKET CreateSocket( SConfig*);
static int Ping( SOCKET, const struct sockaddr_in*,
  u_short, u_long,
#if defined IP_HDRINCL
  u_long, unsigned, unsigned, int, int,
#endif
  unsigned, unsigned, unsigned, void*, size_t, int
);
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
    "\nSyntax: %.*s [options] {host}\n"
    "-l{n} : Send buffer size (default %u, max %u)\n"
    "-n{n} : Number of echo requests to send (default %u, 0= infinite)\n"
    "-w{n} : Receive timeout (default %u milliseconds, 0= no reply)\n"
    "-i{n} : Send interval (default %u milliseconds, 0= no delay)\n"
    "-b    : Enable broadcasts\n"
#if defined IP_HDRINCL
    "-f{i} : Source IP address\n"
#endif
    "-d{i} : Included IP fragment destination IP address\n"
    "-p{n} : Included TCP fragment src port\n"
    "-P{n} : Included TCP fragment dst port\n",
    (int)c, pszCmd,
    DEF_PACKET_SIZE, MAX_PACKET_SIZE, DEF_PINGS, DEF_TIMEOUT, DEF_PERIOD
  );
  fprintf( stderr,
#if defined IP_HDRINCL || defined IP_DONTFRAGMENT
    "-F{n} : Set 'Dont Fragment' flag\n"
#endif
#if defined IP_HDRINCL || defined IP_TTL
    "-L{n} : Time To Live\n"
#endif
#if defined IP_HDRINCL || defined IP_OPTIONS
    "-R    : Include RecordRoute IP option\n"
#endif
#if defined IP_HDRINCL || defined IP_TOS
    "-S{n} : Type Of Service, 0x20=PRI 0x10=DLY\n"
#endif
    "-T{n} : ICMP type, DSTUNREACH=3 QUENCH=4 REDIRECT=5 ECHO=8 TEXCEED=11 TSTAMP=13\n"
    "-C{n} : ICMP code, REDIR_HOST=1 PORT_UNREACH=3. FRAG_NEEDED=4\n"
#if SWS_USE_SOCKLIB && PACKET_DUMP
    "-o{s} : Set logfile\n"
#endif
    "-t    : Decrease verbosity of output\n"
    "-v    : Increase verbosity of output\n"
    "-V    : Display version and exit\n"
    "-h    : Help\n"
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
  void * pv;
  size_t size;
  SOCKET s;
  char szHost[256];

  /* Set default options */
  cfg.iVerbosity = 1;
#if SWS_USE_SOCKLIB && PACKET_DUMP
  cfg.logfile = stdout;
#endif
  cfg.ulPeriod = DEF_PERIOD;
  cfg.pings = DEF_PINGS;
  cfg.uTimeout = DEF_TIMEOUT; /* Rx timeout */
  cfg.datasize = DEF_PACKET_SIZE;
  cfg.type = ICMP_ECHO;
  cfg.code = 0;
#if defined IP_HDRINCL || defined IP_TTL
  cfg.ttl = 0;
#endif
#if defined IP_HDRINCL || defined IP_TOS
  cfg.tos = 0x30; /* 0x20 = PRI, 0x10 = DLY */
#endif
#if defined IP_HDRINCL || defined IP_DONTFRAGMENT
  cfg.df = 1;
#endif
#if defined IP_HDRINCL || defined IP_OPTIONS
  cfg.isRecordRoute = 0;
#endif
  cfg.pszDest = NULL;
#if defined IP_HDRINCL
  cfg.src = INADDR_ANY;
#endif
  cfg.dst = INADDR_ANY;
  cfg.isBroadcast = 0;
  cfg.dest.sin_family = AF_UNSPEC;
  cfg.dest.sin_port = 0;
  cfg.dstPort = 0;

  /* Parse args */
  iRet = ParseArgs( &cfg, argc, argv );
  if ( EXIT_SUCCESS != iRet)
    return iRet;

  /* Verify required arguments */
  if ( NULL == cfg.pszDest)
    {
    fprintf( stderr, "Hostname required\n");
    return EXIT_FAILURE;
    }

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

  /* Destination address.  Try an IP address */
  cfg.dest.sin_addr.s_addr = inet_addr( cfg.pszDest);
  if ( INADDR_ANY == cfg.dest.sin_addr.s_addr)
    {
    fprintf( stderr, "Invalid host %s\n", cfg.pszDest );
    return EXIT_FAILURE;
    }
  else if ( INADDR_NONE != cfg.dest.sin_addr.s_addr
    || 0 == strcmp( cfg.pszDest, "255.255.255.255")
  ) {
    cfg.dest.sin_family = AF_INET;
    }
  /* Try a hostname */
  else if ( NULL != (pHost = gethostbyname( cfg.pszDest )) )
    {
    cfg.dest.sin_family = pHost->h_addrtype;
    memcpy( &cfg.dest.sin_addr, pHost->h_addr, pHost->h_length);
    }
  else
    {
    fprintf( stderr, "Unable to resolve %s\n", cfg.pszDest);
    return EXIT_FAILURE;
    }

  /* Allocate a data buffer */
  size = cfg.datasize + ICMP_HDR_LEN + IP_HDR_MAXLEN;
  if ( size < IP_MIN_FRAG)
    size = IP_MIN_FRAG;
  pv = malloc( size);
  if ( NULL == pv)
    {
    fprintf( stderr, "Out of memory for data buffer\n");
    return EXIT_FAILURE;
    }

  s = CreateSocket( &cfg);
#if 1 && (defined linux || defined __linux || defined __linux__)
  setuid( getuid()); /* Drop root privileges forever */
#endif
  if ( INVALID_SOCKET == s)
    {
    iRet = 1;
    }
  else
    {
    u_long tSend, tWait;
    unsigned i;

    if ( cfg.iVerbosity >= 0)
      printf( "Pinging %s [%s] with %u(%u) bytes of data:\n\n",
        cfg.pszDest, inet_ntoa( cfg.dest.sin_addr), (unsigned)cfg.datasize,
#if defined IP_HDRINCL || defined IP_OPTIONS
        (cfg.isRecordRoute ? (IP_HDR_MAXLEN - IP_HDR_LEN) : 0) +
#endif
        (unsigned)(sizeof( struct Sip) + sizeof( struct SIcmp) + cfg.datasize)
      );
      fflush( stdout);

    /* The ping loop */
    GetMillisecs( tSend);
    tWait = 0;
    for ( i = 0; 0 == cfg.pings || i < cfg.pings; ++i)
      {
      if ( tWait > 0)
        Sleep( tWait);

      /* Check for broken output pipe */
      if ( ferror( stdout))
        {
        iRet = 1;
        break;
        }

      /* Check for ctrl-c */
      if ( s_exit)
        {
        iRet = 1;
        fputs( "\nInterrupted\n", stderr);
        break;
        }

      iRet = Ping(
        s, &cfg.dest,
        cfg.dstPort, cfg.dst,
#if defined IP_HDRINCL
        cfg.src, cfg.ttl, cfg.tos, cfg.df, cfg.isRecordRoute,
#endif
        cfg.uTimeout, cfg.type, cfg.code,
        pv, cfg.datasize,
        cfg.iVerbosity
      );
      GetMillisecs( tWait);
      if ( iRet > 0)
        {
        fprintf( stderr, "FAILED %d: %s\n", iRet, strerror( iRet));
        break;
        }

      tSend += cfg.ulPeriod;
      tWait = tSend > tWait ? tSend - tWait : 0;
      fflush( stdout);
      }

    if ( s_iSent)
      printf( "\n%d packets sent, %d replies lost\n", s_iSent, s_iLost);

    closesocket( s);
    }

  free( pv);

  return iRet ? EXIT_FAILURE : EXIT_SUCCESS;
  }


/*
 * Create ICMP socket
 */
SOCKET CreateSocket( SConfig* pCfg)
  {
  SOCKET s, s2;
  int iRet;

  /* Create sending socket */
  s = socket( PF_INET, SOCK_RAW, IPPROTO_ICMP);
  if ( INVALID_SOCKET == s)
    {
    iRet = GetLastError();
    switch ( iRet)
      {
    case ESOCKTNOSUPPORT:
      fprintf( stderr, "FAILED: RAW sockets not supported\n");
      break;
    case EPROTONOSUPPORT:
      fprintf( stderr, "FAILED: ICMP protocol is not supported\n");
      break;
    default:
      fprintf( stderr, "FAILED to create socket: %s (%d)\n",
        strerror( iRet), iRet);
      break;
      }
    return INVALID_SOCKET;
    }

#if defined IP_HDRINCL
  {
  unsigned u = 1;
  iRet = setsockopt( s, IPPROTO_IP, IP_HDRINCL, (char*)&u, sizeof u);
  if ( SOCKET_ERROR == iRet)
    {
    iRet = GetLastError();
    fprintf( stderr, "FAILED to set IP_HDRINCL: %s (%d)\n",
      strerror( iRet), iRet);
    }
  }
#else
#ifdef IP_TTL
  if ( pCfg->ttl)
    {
    /* Set the TTL */
    iRet = pCfg->ttl;
    setsockopt(
      s,
      IPPROTO_IP, IP_TTL,
      (char*)&iRet, sizeof iRet
    );
    }
#endif

#ifdef IP_TOS
  /* Set the TOS */
  iRet = pCfg->tos;
  setsockopt(
    s,
    IPPROTO_IP, IP_TOS,
    (char*)&iRet, sizeof iRet
  );
#endif

#ifdef IP_DONTFRAGMENT
  /* Set the DF flag */
  iRet = pCfg->df ? 1 : 0;
  setsockopt(
    s,
    IPPROTO_IP, IP_DONTFRAGMENT,
    (char*)&iRet, sizeof iRet
  );
#endif

#ifdef IP_OPTIONS
  if ( pCfg->isRecordRoute)
    {
    /* RecordRoute IP option, max size 40 */
    static u_char opts[ IP_HDR_MAXLEN - IP_HDR_LEN] = { 7, sizeof opts, 4 };

    iRet = setsockopt(
      s,
      IPPROTO_IP, IP_OPTIONS,
      (char*)&opts, sizeof opts
    );
    if ( iRet)
      {
      iRet = GetLastError();
      fprintf( stderr, "FAILED to set IP options: %s (%d)\n",
        strerror( iRet), iRet);
      }
    }
#endif
#endif /* !IP_HDRINCL */

#if 0
  /* Enable non-blocking mode */
  setsockasync( s, 1);
#endif

  if ( pCfg->isBroadcast)
    {
    /* Enable broadcasts */
    iRet = 1;
    setsockopt(
      s,
      SOL_SOCKET, SO_BROADCAST,
      (char*)&iRet, sizeof(iRet)
    );
    }

  /* Get IP address of sending adapter */
  s2 = socket( PF_INET, SOCK_RAW, IPPROTO_ICMP);
  if ( INVALID_SOCKET != s2)
    {
    socklen_t len;
    struct sockaddr_in addr;

    /* Specify remote host. This implicitly binds localhost if not set */
    iRet = connect( s2, (struct sockaddr*)&pCfg->dest, sizeof(pCfg->dest));

    /* Get assigned local adapter */
    len = sizeof addr;
    getsockname( s2, (struct sockaddr*)&addr, &len);

    closesocket( s2);

#if defined IP_HDRINCL
    if ( INADDR_ANY == pCfg->src)
      pCfg->src = addr.sin_addr.s_addr;

    if ( INADDR_ANY == pCfg->dst)
      pCfg->dst = pCfg->src;
#else
    if ( INADDR_ANY == pCfg->dst)
      pCfg->dst = addr.sin_addr.s_addr;
#endif
    }

  return s;
  }


/*
 * Ping a host
 */
static int Ping(                      /* Returns !0 on error */
  SOCKET s,                           /* IN: socket to send from */
  const struct sockaddr_in* pDest,    /* IN: Dest host and TCP src port */
  u_short dstPort,                    /* IN: TCP dst port */
  u_long dst,                         /* IN: DstIP in returned IP header */
#if defined IP_HDRINCL
  u_long src,                         /* IN: SrcIP & interface */
  unsigned ttl,                       /* IN: Time To Live */
  unsigned tos,                       /* IN: Type Of Service */
  int df,                             /* IN: !0 to set DF flag */
  int rr,                             /* IN: !0 to RecordRoute */
#endif
  unsigned uTimeout,                  /* IN: Recv timeout */
  unsigned type,                      /* IN: ICMP type */
  unsigned code,                      /* IN: ICMP type code */
  void* pvData,                       /* IN: data buffer >= IP_MIN_FRAG bytes long */
  size_t sizeData,                    /* IN: Send data size */
  int iVerbosity                      /* IN: Print verbosity */
) {
  static u_short s_uIcmpID;
  static u_short s_uIcmpSeq;
#if defined IP_HDRINCL
  static u_short s_uHdrID;
#endif
  int iRet, iPackets;
  size_t len;
  struct sockaddr_in from;
  u_long tSend, tRecv;
  struct SIcmp* pIcmp = (struct SIcmp*) pvData;
  size_t sizePkt = sizeof( struct SIcmp) + sizeData;

  if ( 0 == s_uIcmpID)
    {
    srand( (int)time( NULL));
    s_uIcmpID = (u_short) rand();
    }

#if defined IP_HDRINCL
  {
  struct Sip* pIp = (struct Sip*) pvData;

  /* Init IP header */
  pIp->tos = (u_char)tos;
  pIp->id = htons( ++s_uHdrID);
  pIp->frag = htons( (u_short)(df ? 0x4000 : 0U));  /* Set the DF flag */
  pIp->ttl = (u_char)(ttl ? ttl : 128);
  pIp->protocol = IPPROTO_ICMP;
  pIp->checksum = 0;
  pIp->src = src;
  pIp->dst = pDest->sin_addr.s_addr;

  if ( rr)
    {
    u_char* p = (u_char*)(pIp + 1);
#if 1
    /* Record route */
    p[ 0] = 0x07;
    p[ 1] = IP_HDR_MAXLEN - sizeof( struct Sip); /* len */
    p[ 2] = 4; /* ptr: 1 + 3 + 4*n*/
    if ( INADDR_ANY != src)
      {
      p[ 2] += sizeof( u_long);
      *(u_long*)&p[3] = src;
      }
    len = IP_HDR_MAXLEN;
#else
    /* Timestamp */
    p[ 0] = 0x44;
    p[ 1] = IP_HDR_MAXLEN - sizeof( struct Sip); /* len */
    p[ 2] = 5; /* ptr: 1 + 4 + 4*n*/
    p[ 3] = 1; /* flags, 0= ms, 1=IP+ms, 3=ms if host=IP */
    len = IP_HDR_MAXLEN;
#endif
    }
  else
    len = sizeof( struct Sip);

  sizePkt += len;
  pIp->len = htons( (u_short) sizePkt);
  pIp->ihl = (u_char)((0x4 << 4) | (len / 4));
  pIp->checksum = (u_short) IpChecksum( (const u_short*)pIp, len);

  pIcmp = (struct SIcmp*)((char*)pvData + len);
  }
#endif /* def IP_HDRINCL */

  /* Init ICMP data */
  for ( len = sizeof *pIcmp; len < sizeof *pIcmp + sizeData; ++len)
    ((u_char*)pIcmp)[ len] = (u_char)len;

  switch ( type)
    {
  case ICMP_DEST_UNREACH:
  case ICMP_TIME_EXCEED:
  case ICMP_PARAM_PROB:
  case ICMP_QUENCH:
  case ICMP_REDIRECT:
    /* IP hdr + 8 bytes */
    if ( sizeData < IP_HDR_LEN + 8)
      {
      fputs( "Buffer too small\n", stderr);
      return -1;
      }
    else
      {
      struct Sip* const pHdr = (struct Sip*) (pIcmp + 1);
      struct Stcp* const pTcp = (struct Stcp*) (pHdr + 1);
      static u_short s_uSrcPort = 1024, s_uDstPort = 0;
      static u_long s_uSeq = 0x12345678L;

      /* Setup returned IP hdr */
      pHdr->ihl = (u_char)((0x4 << 4) | (IP_HDR_LEN / 4));
      pHdr->tos = 0;
      pHdr->len = htons( IP_HDR_LEN + sizeof( Stcp));
      pHdr->id = htons( s_uIcmpSeq);
      pHdr->frag = 0;
      pHdr->ttl = 128;
      pHdr->protocol = IPPROTO_TCP;
      pHdr->checksum = 0;
      pHdr->dst = dst;
      pHdr->src = pDest->sin_addr.s_addr;
      pHdr->checksum = (u_short) IpChecksum( (const u_short*) pHdr, IP_HDR_LEN);

      /* Setup returned TCP hdr */
      if ( 0 == pDest->sin_port)
        pTcp->src = htons( ++s_uSrcPort);
      else
        pTcp->src = pDest->sin_port;

      if ( 0 == dstPort)
        pTcp->dst = htons( ++s_uDstPort);
      else
        pTcp->dst = htons( dstPort);

      pTcp->seq = htonl( ++s_uSeq);
      }

    if ( ICMP_REDIRECT == type)
      {
      u_long ulGateway = ntohl( pDest->sin_addr.s_addr);
      ulGateway ^= 1;
      *(u_long*)&pIcmp->id = htonl( ulGateway);
      }
    else
      {
      pIcmp->id = 0;
      pIcmp->seq = 0;
      }
    break;

  case ICMP_ECHO:
  case ICMP_TIMESTAMP:
  case ICMP_INFO_RQST:
  case ICMP_ADDR_MASK:
    pIcmp->id = s_uIcmpID;
    pIcmp->seq = ++s_uIcmpSeq;
    break;

  default:
    break;
    }

  /* Format ICMP header */
  pIcmp->type = (u_char) type; /* byte */
  pIcmp->code = (u_char) code;
  pIcmp->cksum = 0;

  GetMillisecs( tSend);
  if ( ICMP_ECHO == type && sizeData >= 4)
    *(u_long*)(pIcmp +1) = tSend;

  /* Calc checksum.  This is endian independent */
  pIcmp->cksum = (u_short) IpChecksum(
    (const u_short*) pIcmp, sizeof *pIcmp + sizeData);

  /* Send the ICMP packet */
  iRet = sendto(
    s,
    (char*)pvData, sizePkt,
    0*MSG_DONTROUTE,
    (const struct sockaddr*)pDest, sizeof(*pDest)
  );
  if ( SOCKET_ERROR == iRet)
    {
    iRet = GetLastError();
    switch ( iRet)
      {
    case ETIMEDOUT:
      printf( "FAILED %d: send timed out\n", iRet);
      return -1;

    case ENETUNREACH:
    case EHOSTUNREACH:
    case ECONNRESET:
      printf( "FAILED %d: destination host unreachable\n", iRet);
      return -1;

    case EWOULDBLOCK:
#if EAGAIN != EWOULDBLOCK
    case EAGAIN:
#endif
      return -1;

    default:
      printf( "FAILED %d: send %s\n", iRet, strerror( iRet));
      return -1;
      }
    }

  ++s_iSent;
  if ( (size_t)iRet != sizePkt)
    printf( "Send truncated to %d bytes\n", iRet);
  else if ( 0 == iVerbosity)
    putchar( '.' ), fflush( stdout);

  /* Is a reply packet expected? */
  switch ( type)
    {
  case ICMP_ECHO:
  case ICMP_TIMESTAMP:
  case ICMP_INFO_RQST:
  case ICMP_ADDR_MASK:
    /* Yes */
    break;

  case ICMP_ECHO_REPLY:
  case ICMP_DEST_UNREACH:
  case ICMP_QUENCH:
  case ICMP_REDIRECT:
  case ICMP_TIME_EXCEED:
  case ICMP_PARAM_PROB:
  case ICMP_TIME_REPLY:
  case ICMP_INFO_REPLY:
  case ICMP_AM_REPLY:
    /* No */
    if ( iVerbosity > 0)
      puts( "OK" );
    return 0;

  default:
    /* Possibly */
    if ( 0 == uTimeout)
      {
      if ( iVerbosity > 0)
        puts( "OK" );
      return 0;
      }
    break;
    }

  /* Wait for reply/ies */
  iPackets = 0;
  while ( !s_exit)
    {
    socklen_t iLen;
    static fd_set setRd;
    struct timeval tval;
    int rxttl;

    if ( ferror( stdout))
      break;

    FD_ZERO( &setRd);
    FD_SET( s, &setRd);

    /* Set timeout for select */
    tval.tv_usec = (uTimeout % 1000) * 1000UL;
    tval.tv_sec = uTimeout / 1000;

    /* Wait for reply */
    iRet = select( FD_SETSIZE, &setRd, NULL, NULL, &tval);
    GetMillisecs( tRecv);
    if ( SOCKET_ERROR == iRet)
      {
      if ( !iPackets)
        ++s_iLost;
      return GetLastError();
      }

    if ( !FD_ISSET( s, &setRd))
      {
      if ( !iPackets)
        {
        ++s_iLost;
        if ( iVerbosity > 0)
          puts( "Recv timed out");
        else if ( iVerbosity >= 0)
          putchar( 'X'), fflush( stdout);
        return -1;
        }
      return 0;
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
      switch ( iRet)
        {
      case EMSGSIZE:
        puts( "Recv: truncated packet");
        iRet = iLen;
        break;

      case ETIMEDOUT:
      case EWOULDBLOCK:
#if EAGAIN != EWOULDBLOCK
      case EAGAIN:
#endif
        if ( !iPackets)
          {
          ++s_iLost;
          puts( "Recv: timed out");
          return -1;
          }
        return 0;

      case ENETUNREACH:
      case EHOSTUNREACH:
      case ECONNRESET:
        ++s_iLost;
        puts( "Recv: destination host unreachable");
        return -1;

      default:
        ++s_iLost;
        fputs( "Recv:", stdout );
        return iRet;
        }
      }

    if ( (size_t)iRet < IP_HDR_LEN)
      {
      puts( "Recv: runt packet" );
      continue;
      }

    /* Ensure ICMP packet */
    if ( 1 != ((const unsigned char*)pvData)[9])
      continue;

    rxttl = ((const unsigned char*)pvData)[ 8];

    /* Skip IP header */
    len = (*(const unsigned char*)pvData & 0xf) * 4;
    if ( len < IP_HDR_LEN
      || len + sizeof(struct SIcmp) > (size_t)iRet
    ) {
      puts( "Recv: invalid packet" );
      continue;
      }

    tRecv -= tSend;
    if ( uTimeout > tRecv)
      uTimeout -= (unsigned)tRecv;
    else
      uTimeout = 0;

    pIcmp = (SIcmp*)((char*)pvData + len);

    len = (unsigned)iRet - (len + sizeof( struct SIcmp));
    if ( iVerbosity > 0)
      printf( "%u(%d) bytes from %s: ",
        (unsigned)len, iRet, inet_ntoa( from.sin_addr) );

    if ( IpChecksum( (const u_short*) pIcmp, len + sizeof( struct SIcmp)))
      fputs( "BADSUM ", stdout);

    /* Check ICMP content */
    switch ( pIcmp->type)
      {
    case ICMP_ECHO:
      puts( "Echo request");
      continue;

    case ICMP_ECHO_REPLY:
    case ICMP_TIME_REPLY:
    case ICMP_INFO_REPLY:
    case ICMP_AM_REPLY:
      if ( 0 != pIcmp->code)
        fputs( "Reply code error", stdout);
      else if ( s_uIcmpID != pIcmp->id)
        fputs( "Reply ID error", stdout);
      else
        {
        if ( ICMP_ECHO_REPLY == pIcmp->type)
          tRecv += tSend - *(u_long*)(pIcmp +1);
        if ( s_uIcmpSeq == pIcmp->seq)
          {
          ++iPackets;
          uTimeout = 0;
          }
        if ( iVerbosity > 0)
          printf( "Reply # %u", pIcmp->seq);
        else if ( iVerbosity >= 0)
          putchar( s_uIcmpSeq == pIcmp->seq ? ':' : '?'), fflush( stdout);
        }
      break;

    case ICMP_QUENCH:
      printf( "Source quench (%d)\n", pIcmp->code);
      continue;

    case ICMP_DEST_UNREACH:
      if ( iVerbosity > 0) switch ( pIcmp->code)
        {
      case 0:
        puts( "Destination net unreachable");
        break;
      case 1:
        puts( "Destination host unreachable");
        break;
      case 2:
        puts( "Destination protocol unreachable");
        break;
      case 3:
        puts( "Destination port unreachable");
        break;
      default:
        printf( "Destination not reachable (%d)\n", pIcmp->code );
        break;
        }
      else
        putchar( 'D'), fflush( stdout);
      continue;

    case ICMP_REDIRECT:
      if ( iVerbosity > 0) switch ( pIcmp->code)
        {
      case 0:
        puts( "Redirect for the network");
        break;
      case 1:
        puts( "Redirect for the host");
        break;
      case 2:
        puts( "Redirect for TOS & network");
        break;
      case 3:
        puts( "Redirect for TOS & host");
        break;
      default:
        printf( "Redirect (%d)\n", pIcmp->code );
        break;
        }
      else
        putchar( 'R'), fflush( stdout);
      continue;

    case ICMP_TIME_EXCEED:
      if ( iVerbosity > 0)
        printf( "Time exceeded (%d)", pIcmp->code);
      else
        putchar( 'T'), fflush( stdout);
      break;

    case ICMP_PARAM_PROB:
      if ( iVerbosity > 0)
        puts( "Parameter problem");
      else
        putchar( 'P'), fflush( stdout);
      continue;

    default:
      printf( "Type %d, code %d\n", pIcmp->type, pIcmp->code);
      continue;
      }

    if ( iVerbosity > 0)
      {
      unsigned offset;

      printf( ", ttl %d, %lu ms\n", rxttl, tRecv);

      /* IP header length */
      len = (*(const u_char*)pvData & 0xf) * 4;

      /* Parse IP options */
      offset = IP_HDR_LEN;
      while ( offset < len)
        {
        u_char opt = ((const u_char*)pvData)[ offset];
        if ( 0 == opt)
          break;
        else if ( 1 == opt)
          ++offset;
        else if ( offset + 1 >= len)
          break;
        else
          {
          const unsigned optLen = ((const u_char*)pvData)[ 1 + offset];
          unsigned i, j;

          switch ( opt)
            {
          case 7: /* Record route */
            j = ((const u_char*)pvData)[ 2 + offset];
            fputs( "Route:", stdout);
            for ( i = 3; i +1 < j; ++i)
              printf( "%s%u",
                3 == (i % 4) ? "  " : ".",
                ((const u_char*)pvData)[ i + offset]
              );
            fputs( "\n", stdout);
            break;

          default:
            printf( "IP option %#x:", opt);
            for ( i = 2; i < optLen; ++i)
              printf( " %02x", ((const u_char*)pvData)[ i + offset]);
            fputs( "\n", stdout);
            break;
            }
          offset += optLen;
          }
        }
      }
    }

  return -1;
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
  int bHelp = iArgc <= 1; /* Do help if no args */
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
          fputs( kszBanner "\n", stderr);;
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
          /* send data size */
          pszArg = GetArg();
          lVal = strtol( pszArg, &pszEnd, 0);
          if ( *pszEnd || lVal < 0 || lVal > (long)MAX_PACKET_SIZE)
            {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Invalid data size %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          pCfg->datasize = (size_t)lVal;
          break;

        case 'n':
          /* Packets to send */
          pszArg = GetArg();
          lVal = strtol( pszArg, &pszEnd, 0);
          if ( *pszEnd
            || lVal < 0
#if UINT_MAX < ULONG_MAX
            || (unsigned long)lVal > UINT_MAX
#endif
          ) {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Invalid number of echo requests %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          pCfg->pings = (unsigned)lVal;
          break;

        case 'w':
          /* Receive timeout */
          pszArg = GetArg();
          lVal = strtol( pszArg, &pszEnd, 10);
          if ( *pszEnd
            || lVal < 0
#if UINT_MAX < ULONG_MAX
            || (unsigned long)lVal > UINT_MAX
#endif
          ) {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Invalid receive timeout %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          pCfg->uTimeout = (unsigned)lVal;
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
          break;

        case 'b':
          /* Enable broadcast */
          pCfg->isBroadcast = 1;
          break;

        case 'T':
          /* ICMP packet type */
          pszArg = GetArg();
          ulVal = strtoul( pszArg, &pszEnd, 0);
          if ( *pszEnd || ulVal > 255)
            {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Invalid packet type %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          pCfg->type = (unsigned)ulVal;

          switch ( pCfg->type)
            {
          case ICMP_DEST_UNREACH:
          case ICMP_TIME_EXCEED:
          case ICMP_PARAM_PROB:
          case ICMP_QUENCH:
          case ICMP_REDIRECT:
            if ( pCfg->datasize < IP_HDR_LEN + 8)
              pCfg->datasize = IP_HDR_LEN + 8;
            break;
          default:
            break;
            }
          break;

        case 'C':
          /* ICMP type code */
          pszArg = GetArg();
          ulVal = strtoul( pszArg, &pszEnd, 0);
          if ( *pszEnd || ulVal > 255)
            {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Invalid type code %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          pCfg->code = (unsigned)ulVal;
          break;

#if defined IP_HDRINCL || defined IP_TTL
        case 'L':
          /* TTL */
          pszArg = GetArg();
          ulVal = strtoul( pszArg, &pszEnd, 10);
          if ( *pszEnd || ulVal > 255)
            {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Invalid TTL %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          pCfg->ttl = (unsigned)ulVal;
          break;
#endif

#if defined IP_HDRINCL || defined IP_DONTFRAGMENT
        case 'F':
          pszArg = GetArg();
          ulVal = strtoul( pszArg, &pszEnd, 10);
          if ( pszEnd == pszArg)
            pCfg->df = 1;
          else if ( *pszEnd || ulVal > 1)
            {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Invalid DF value: %s\n", pszArg);
            return EXIT_FAILURE;
            }
          else if ( 0 == ulVal)
            {
            pCfg->df = 0;
            pszArg = NULL;
            }
          else
            {
            pCfg->df = 1;
            pszArg = NULL;
            }
          break;
#endif

#if defined IP_HDRINCL || defined IP_OPTIONS
        case 'R':
          /* Include RecordRoute IP option */
          pCfg->isRecordRoute = 1;
          break;
#endif

#if defined IP_HDRINCL || defined IP_TOS
        case 'S':
          /* TOS */
          pszArg = GetArg();
          ulVal = strtoul( pszArg, &pszEnd, 0);
          if ( *pszEnd || ulVal > 255)
            {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Invalid TTL %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          pCfg->tos = (unsigned)ulVal;
          break;
#endif

#if defined IP_HDRINCL
        case 'f':
          /* spoofed host */
          pszArg = GetArg();
          ulVal = inet_addr( pszArg );
          if ( INADDR_NONE == ulVal
            && strcmp( pszArg, "255.255.255.255")
          ) {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Invalid IP address %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          pCfg->src = ulVal;
          break;
#endif
        case 'd':
          /* Destination host in returned IP hdr */
          pszArg = GetArg();
          ulVal = inet_addr( pszArg );
          if ( INADDR_NONE == ulVal)
            {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Invalid IP address %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          pCfg->dst = ulVal;
          break;

        case 'p':
          /* Included TCP fragment src port */
          pszArg = GetArg();
          ulVal = strtoul( pszArg, &pszEnd, 10);
          if ( *pszEnd || ulVal > 65535UL)
            {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Invalid port %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          pCfg->dest.sin_port = htons( (u_short)ulVal);
          break;

        case 'P':
          /* Included TCP fragment dst port */
          pszArg = GetArg();
          ulVal = strtoul( pszArg, &pszEnd, 10);
          if ( *pszEnd || ulVal > 65535UL)
            {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Invalid port %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          pCfg->dstPort = (u_short)ulVal;
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
