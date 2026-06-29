/*****************************************************************************
 *
 * File name   : syslogr.c
 * Function    : Syslog report generastor
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 26-Aug-03, LVR, v1.00 Created.
 * 22-Jan-04, LVR, v1.01 Set default log filename for NetCenturion.
 * 16-Apr-05, LVR, v1.02 Add -m option (CIDR IP mask) for grouping packets
 *                       Add code to deal with NetCenturion log wrapping.
 * 23-Jul-05, LVR, v1.03 AddIpAddress only updates date&time if later
 * 04-Jul-09, LVR, v1.04 Add TTL to statistics. Rename ipdump to pktdump
 *
 * Comments
 * --------
 *
 *****************************************************************************/
#define kszApp "Syslogr"
#define kszVer "1.04"
#define kszBanner kszApp " " kszVer ", " TIMESTAMP "."
#define kszAbout "Log file analyser."

/* Compilation options */


/* Exports */

/* Imports */

/* ANSI C library */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <assert.h>
#include <limits.h>
#include <errno.h>


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
#define MAX_LINE 1024 /* Max length of a log line */
#endif

#define DEFAULT_COUNT 20 /* Default report size */

#define SIZE_DATE 10 /* yyyy/mm/dd */
#define SIZE_TIME 12 /* hh:mm:ss.ttt */
#define SIZE_ADDR 15 /* xxx.yyy.zzz.xxx */
#define SIZE_PRTCL 4 /* TCP, UDP, ICMP etc */

#ifndef INITIAL_IPS
#define INITIAL_IPS   512 /* Initial IP table size */
#endif
#ifndef INITIAL_PORTS
#define INITIAL_PORTS 32  /* Initial port table size */
#endif
#ifndef INITIAL_TTLS
#define INITIAL_TTLS 8  /* Initial TTL table size */
#endif


/***************************************************************************
 * Data types
 ***************************************************************************/

/* Protocol types */
typedef enum EProtocol
  {
  kAny, kIcmp, kUdp, kTcp
  } EProtocol;

/* Log file types */
typedef enum ELogFormat
  {
  kSyslog, kPktdump, kNetcenturion
  } ELogFormat;

/* Parameters */
typedef struct SConfig
  {
  int iVerbosity;
  FILE* input;
  FILE* output;
  ELogFormat eLogFormat;
  EProtocol eProtocol;
  unsigned uDstPort, uDstPortMax;
  unsigned uSrcPort, uSrcPortMax;
  int isSortByDstIp;
  unsigned long ulIpMask;
  unsigned long ulSrcAddr, ulSrcSubnetMask;
  unsigned long ulDstAddr, ulDstSubnetMask;
  unsigned long ulExSrcAddr, ulExSrcSubnetMask;
  unsigned long ulExDstAddr, ulExDstSubnetMask;
  unsigned uTopCount;
  } SConfig;

/* Stats per IP address */
typedef struct SIpAddress
  {
  unsigned long ulAddr;
  unsigned long ulCount;
  unsigned long ulIcmpCount;
  unsigned long ulUdpCount;
  unsigned long ulTcpCount;
  unsigned long ulTtl;
  char szDate[ SIZE_DATE + 1], szTime[ SIZE_TIME + 1];
  } SIpAddress;

/* Stats per port */
typedef struct SDestPort
  {
  unsigned uPort;
  unsigned long ulCount;
  unsigned long ulTcpCount;
  } SDestPort;

/* Stats per TTL */
typedef struct STtl
  {
  unsigned uTtl;
  unsigned long ulCount;
  } STtl;

/* Data gleaned from log */
typedef struct SStats
  {
  char szLogType[ 32];
  char szStartDate[ SIZE_DATE + 1], szEndDate[ SIZE_DATE + 1];
  char szStartTime[ SIZE_TIME + 1], szEndTime[ SIZE_TIME + 1];

  unsigned long ulPackets;
  unsigned long ulTcpPackets;
  unsigned long ulUdpPackets;
  unsigned long ulIcmpPackets;

  size_t sizeIps;
  unsigned long uIps;
  SIpAddress* ipAddressArray;

  size_t sizePorts;
  unsigned uDestPorts;
  SDestPort* portArray;

  size_t sizeTtls;
  unsigned uTtls;
  STtl* ttlArray;
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
static int AddIpAddress( SStats*, unsigned long, unsigned long, EProtocol, unsigned, const char*, const char*);
static int AddPort( SStats*, unsigned, EProtocol);
static int AddTtl( SStats*, unsigned);
static int CmpIpCount( const void*, const void*);
static int CmpPortCount( const void*, const void*);
static int CmpTtlCount( const void*, const void*);
static unsigned MonthNum( const char*);


/***************************************************************************
 * Module data
 ***************************************************************************/

sig_atomic_t volatile g_exit;


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
    "-t    : Decrease verbosity of output\n"
    "-v    : Increase verbosity of output\n"
    "-o<s> : Set output file\n"
    "-n<n> : Max number of entries per table\n"
    "-r<s> : Set protocol filter (tcp, udp, icmp)\n"
    "-p<n> : Set destination port filter range e.g. 135 or 135-139\n"
    "-P<n> : Set source port filter range e.g. 135 or 135-139\n",
    (int)c, pszCmd
  );
  fprintf( stderr,
    "-i<s> : Set source IP address filter e.g. 192.168.0.2/24 \n"
    "-I<s> : Set destination IP address filter e.g. 192.168.0.2/24 \n"
    "-x<s> : Set source IP address exclusion filter e.g. 192.168.0.2/24 \n"
    "-X<s> : Set destination IP address exclusion filter e.g. 192.168.0.2/24 \n"
    "-m<m> : Set CIDR mask (1..32)\n"
    "-D    : Sort on destination IP address\n"
    "-l<s> : Set log file format (syslog, pktdump, netcen)\n"
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
  SStats stats;

  /* Set default options */
  cfg.iVerbosity = 1;
  cfg.input = NULL;
  cfg.eLogFormat = kPktdump;
  cfg.output = stdout;
  cfg.eProtocol = kAny;
  cfg.uDstPort = 0;
  cfg.uDstPortMax = 65535U;
  cfg.uSrcPort = 0;
  cfg.uSrcPortMax = 65535U;
  cfg.isSortByDstIp = 0;
  cfg.ulIpMask = 0xffffffffL;
  cfg.ulSrcAddr = 0;
  cfg.ulSrcSubnetMask = 0;
  cfg.ulDstAddr = 0;
  cfg.ulDstSubnetMask = 0;
  cfg.ulExSrcAddr = 0;
  cfg.ulExSrcSubnetMask = 0;
  cfg.ulExDstAddr = 0;
  cfg.ulExDstSubnetMask = 0;
  cfg.uTopCount = DEFAULT_COUNT;

  /* Parse args */
  iRet = ParseArgs( &cfg, argc, argv );
  if ( EXIT_SUCCESS != iRet)
    return iRet;

  if ( cfg.iVerbosity >= 0)
    {
    fputs( kszBanner "\n", stderr);
    if ( stdout == cfg.output)
      putchar( '\n');
    }

  if ( NULL == cfg.input)
    {
    const char* pszLog;

    switch ( cfg.eLogFormat)
      {
    case kSyslog:
      pszLog = "c:\\syslogsvc.log";
      break;
    case kNetcenturion:
      pszLog = "c:\\netcenturion.log";
      break;
    case kPktdump:
    default:
      fprintf( stderr, "Log file not specified\n");
      return EXIT_FAILURE;
      }

    cfg.input = fopen( pszLog, "r");
    if ( NULL == cfg.input)
      {
      fprintf( stderr, "Unable to open %s: %s\n", pszLog, strerror( errno) );
      return EXIT_FAILURE;
      }
    }

#ifdef SIGINT
  /* Register a handler for ctrl-c to ensure a clean exit */
  signal( SIGINT, OnSigInt);
#endif

  /* Parse the log */
  memset( &stats, 0, sizeof stats);
  iRet = Parse( &stats, &cfg);
  if ( !iRet)
    iRet = Report( cfg.output, &stats, cfg.uTopCount);

  if ( stdout != cfg.output)
    fclose( cfg.output);

  return iRet ? EXIT_FAILURE : EXIT_SUCCESS;
  }


/*
 * Parse the syslog file
 */
int Parse( SStats* pStats, const SConfig* pCfg)
  {
  static char szBuf[ MAX_LINE];
  int iFirst = 1, iLines = 0, iWrapped = 0;
  char szDate[ SIZE_DATE + 1], szTime[ SIZE_TIME + 1];

  szDate[0] = '\0';
  szTime[0] = '\0';

  if ( pCfg->iVerbosity > 1)
    fputs( "Date       Time     Source          Destination  Protocol  Src     Dst  TTL\n", pCfg->output);

  /* Read a line at a time until EOF */
  while ( NULL != fgets( szBuf, sizeof( szBuf), pCfg->input) )
    {
    int i;
    unsigned uSrc = 0, uDst = 0, uTtl = 0;
    char szSrc[ SIZE_ADDR + 1], szDst[ SIZE_ADDR + 1];
    char szProtocol[ SIZE_PRTCL + 1];
    unsigned ip[4];
    unsigned long ulSrcAddr, ulDstAddr;
    EProtocol eProtocol = kAny;
    const char* psz;
    char cType;
    enum { kLines = 512 };
    struct tm date;

    if ( pCfg->iVerbosity > 0 && (iLines++ % kLines) == 0)
      {
      static const char table[] = "|/-\\";
      printf( "%c\r", table[ (iLines / kLines) % 4] );
      }

    if ( g_exit)
      {
      fputs( "Aborted\n", stderr);
      return 1;
      }

    switch ( pCfg->eLogFormat)
      {
    case kSyslog:
      /* Syslogsvc log format:
       *
       * Syslogd version 1.00 started Mon Aug 18 09:50:50 2003
       * 2003/08/26 08:43:25 SYSLOGD: 192.168.0.1 local(1) notice IP[Src=62.255.12.68 Dst=192.168.0.64 ICMP]}S04>R02mN
       * 2003/08/26 09:03:48 SYSLOGD: 192.168.0.1 local(1) notice IP[Src=207.44.210.28 Dst=192.168.0.64 TCP spo=00080  dpo=01906]}S04>R02mN
       */
      if ( iFirst)
        {
        /* Parse the start line */
        i = sscanf( szBuf,
          "Syslogd version %*u.%*u started %*s %3s %u %" STRING(SIZE_TIME) "s %u",
          szProtocol, &uSrc, szTime, &uDst
        );
        if ( 4 == i)
          {
          iFirst = 0;
          strcpy( pStats->szLogType, "Syslogd");
          sprintf( pStats->szStartDate, "%u/%02u/%02u",
            uDst, MonthNum( szProtocol), uSrc);
          strcpy( pStats->szStartTime, szTime);
          continue;
          }
        }

      /* Parse a log line */
      i = sscanf( szBuf,
        " %" STRING(SIZE_DATE) "s %" STRING(SIZE_TIME) "s SYSLOGD: %*s %*s %*s "
        "IP[ Src= %" STRING(SIZE_ADDR) "s Dst= %" STRING(SIZE_ADDR) "s %" STRING(SIZE_PRTCL) "[A-Z] spo= %u dpo= %u",
        szDate, szTime, szSrc, szDst, szProtocol, &uSrc, &uDst
      );
      if ( i < 5)
        continue;

      /* Determine protocol */
      if ( !strcmp( szProtocol, "ICMP"))
        eProtocol = kIcmp;
      else if ( uSrc > 0 && uDst > 0 && !strcmp( szProtocol, "UDP"))
        eProtocol = kUdp;
      else if ( uSrc > 0 && uDst > 0 && !strcmp( szProtocol, "TCP"))
        eProtocol = kTcp;
      else
        continue;
      break;

    case kPktdump:
      /* PKTDUMP log format
       *
       * INETD version 1.00 (SwsSock), built 15:12:40 Jun 27 2003.
       * Logging(1) started Sat Aug 23 09:06:47 2003
       * 09:07:05.612 R:92 62.253.52.213 >192.168.0.64 TTL:120 ICMP ECHO
       * 09:07:29.940 R:52 62.254.17.102 >192.168.0.64 TTL:56 DF TCP 4823 >135 SYN WND:60352 SEQ:1467085265 MSS:1460 WS:2 SACKP
       * 2003/08/23 09:07:31 DISCARD: 62.254.17.102:4823 TCP(135) RECV 72
       * 09:07:59.614 R:294 62.134.73.14 >192.168.0.64 TTL:106 UDP 2438 >135
       * 19:41:41.051 R:46(40) 127.0.0.1 >192.168.0.64 TTL:117 TCP 80 >1948 RST ACK:1572208641 WND:0 SEQ:0
       */

      /* Parse the start line */
      i = sscanf( szBuf,
        "Logging(%*u) started %*s %3s %u %" STRING(SIZE_TIME) "s %u",
        szProtocol, &uSrc, szTime, &uDst
      );
      if ( 4 == i)
        {
        if ( iFirst)
          {
          iFirst = 0;
          strcpy( pStats->szLogType, "Pktdump");
          sprintf( pStats->szStartDate, "%u/%02u/%02u",
            uDst, MonthNum( szProtocol), uSrc);
          strcpy( pStats->szEndDate, pStats->szStartDate);
          strcpy( pStats->szStartTime, szTime);
          strcpy( pStats->szEndTime, szTime);
          }
        else
          {
          sprintf( pStats->szEndDate, "%u/%02u/%02u",
            uDst, MonthNum( szProtocol), uSrc);
          strcpy( pStats->szEndTime, szTime);
          }
        continue;
        }

      /* Parse a log line */
      i = sscanf( szBuf,
        " %" STRING(SIZE_TIME) "s %c:%*u(%*u) %" STRING(SIZE_ADDR) "s > %" STRING(SIZE_ADDR) "s",
        szTime, &cType, szSrc, szDst
      );
      if ( 2 == i && ('R' == cType || 'B' == cType))
        {
        i = sscanf( szBuf,
          " %" STRING(SIZE_TIME) "s %c:%*u %" STRING(SIZE_ADDR) "s > %" STRING(SIZE_ADDR) "s",
          szTime, &cType, szSrc, szDst
        );
        }
      if ( i < 4)
        continue;

      if ( strcmp( pStats->szEndTime, szTime) > 0)
        {
        /* Time rollover so bump date */
        date.tm_sec = 0;
        date.tm_min = 0;
        date.tm_hour = 0;
        sscanf( pStats->szEndDate, "%d/%d/%d", &date.tm_year, &date.tm_mon, &date.tm_mday);
        ++date.tm_mday;
        date.tm_isdst = -1;
        mktime( &date);
        sprintf( pStats->szEndDate, "%d/%02d/%02d", date.tm_year, date.tm_mon, date.tm_mday);
        strcpy( pStats->szEndTime, szTime);
        }

      if ( NULL != (psz = strstr( szBuf, "TTL:")))
        sscanf( psz, "TTL: %u", &uTtl);

      if ( 'T' == cType)
        continue;

      /* Determine protocol */
      if ( strstr( szBuf, "ICMP"))
        {
        eProtocol = kIcmp;
        }
      else if ( NULL != (psz = strstr( szBuf, "UDP")))
        {
        i = sscanf( psz, "UDP %u > %u", &uSrc, &uDst);
        if ( i < 2)
          {
          i = sscanf( psz, "UDP LEN: %*u %u > %u", &uSrc, &uDst);
          if ( i < 2)
            continue;
          }

        eProtocol = kUdp;
        }
      else if ( NULL != (psz = strstr( szBuf, "TCP")))
        {
        i = sscanf( psz, "TCP %u > %u", &uSrc, &uDst);
        if ( i < 2)
          continue;

        eProtocol = kTcp;
        }
      else
        continue;

      strcpy( szDate, pStats->szEndDate);
      break;

    case kNetcenturion:
      /* Netcenturion log format:
       *
       * NetCenturion version 1.00
       * Log started Wed Aug 27 09:25:56 2003
       * Applying rules to Intel(R) PRO Adapter 192.168.0.2
       * 14:12:04.114 192.168.0.2 >62.253.142.55 28 ICMP port unreachable UDP 137 >1265 discarded by rule13
       * 17:11:20.289 0.0.0.0 >255.255.255.255 72 UDP 68 >67 discarded by default
       */
      if ( 0 == strncmp( szBuf, ">>>", 3))
        {
        iWrapped = 1;
        strcpy( szDate, "");
        continue;
        }

      /* Parse the start line */
      i = sscanf( szBuf,
        "NetCenturion version %*u.%*u log started %*s %3s %u %" STRING(SIZE_TIME) "s %u",
        szProtocol, &uSrc, szTime, &uDst
      );
      if ( 0 == i)
        {
        i = sscanf( szBuf,
          "Log started %*s %3s %u %" STRING(SIZE_TIME) "s %u",
          szProtocol, &uSrc, szTime, &uDst
        );
        }

      if ( 4 == i)
        {
        if ( iFirst)
          {
          iFirst = 0;
          strcpy( pStats->szLogType, "NetCenturion");
          sprintf( szDate, "%u/%02u/%02u", uDst, MonthNum( szProtocol), uSrc);
          strcpy( pStats->szStartDate, szDate);
          strcpy( pStats->szEndDate, szDate);
          strcpy( pStats->szStartTime, szTime);
          strcpy( pStats->szEndTime, szTime);
          }
        else
          {
          sprintf( szDate, "%u/%02u/%02u", uDst, MonthNum( szProtocol), uSrc);
          if ( (i = strcmp( szDate, pStats->szStartDate)) < 0)
            {
            strcpy( pStats->szStartDate, szDate);
            strcpy( pStats->szStartTime, szTime);
            }
          else if (0 == i && strcmp( szTime, pStats->szStartTime) < 0)
            {
            strcpy( pStats->szStartTime, szTime);
            }
          else if ( (i = strcmp( szDate, pStats->szEndDate)) > 0)
            {
            strcpy( pStats->szEndDate, szDate);
            strcpy( pStats->szEndTime, szTime);
            }
          else if ( 0 == i && strcmp( szTime, pStats->szEndTime) > 0)
            {
            strcpy( pStats->szEndTime, szTime);
            }
          }

        iWrapped = 0;
        continue;
        }

      /* Parse a log line */
      i = sscanf( szBuf,
        " %" STRING(SIZE_TIME) "s %" STRING(SIZE_ADDR) "s > %" STRING(SIZE_ADDR) "s %*u %" STRING(SIZE_PRTCL) "[A-Z] %u > %u",
        szTime, szSrc, szDst, szProtocol, &uSrc, &uDst
      );
      if ( i < 4)
        continue;

      if ( !iWrapped
        && strcmp( pStats->szEndDate, szDate) == 0
        && strcmp( pStats->szEndTime, szTime) > 0
      ) {
        /* Time rollover so bump date */
        date.tm_sec = 0;
        date.tm_min = 0;
        date.tm_hour = 0;
        sscanf( pStats->szEndDate, "%d/%d/%d", &date.tm_year, &date.tm_mon, &date.tm_mday);
        ++date.tm_mday;
        date.tm_isdst = -1;
        mktime( &date);
        sprintf( szDate, "%d/%02d/%02d", date.tm_year, date.tm_mon, date.tm_mday);
        strcpy( pStats->szEndDate, szDate);
        strcpy( pStats->szEndTime, szTime);
        }

      /* Determine protocol */
      if ( !strcmp( szProtocol, "ICMP"))
        eProtocol = kIcmp;
      else if ( uSrc > 0 && uDst > 0 && !strcmp( szProtocol, "UDP"))
        eProtocol = kUdp;
      else if ( uSrc > 0 && uDst > 0 && !strcmp( szProtocol, "TCP"))
        eProtocol = kTcp;
      else
        continue;

      break;
      }

    /* Update end date & time */
    i = strcmp( szDate, pStats->szEndDate);
    if ( i > 0)
      {
      strcpy( pStats->szEndDate, szDate);
      strcpy( pStats->szEndTime, szTime);
      }
    else if ( 0 == i && strcmp( szTime, pStats->szEndTime) > 0)
      {
      strcpy( pStats->szEndTime, szTime);
      }

    /* Parse dotted IP src address */
    if ( 4 != sscanf( szSrc, "%u.%u.%u.%u", &ip[0], &ip[1], &ip[2], &ip[3]))
      continue;
    ulSrcAddr = ((unsigned long)ip[0] << 24)
      + ((unsigned long)ip[1] << 16)
      + (ip[2] << 8)
      + ip[3];

    /* Apply source addres filter */
    if ( (ulSrcAddr & pCfg->ulSrcSubnetMask) != pCfg->ulSrcAddr)
      continue;
    if ( pCfg->ulExSrcAddr && (ulSrcAddr & pCfg->ulExSrcSubnetMask) == pCfg->ulExSrcAddr)
      continue;

    /* Parse dotted IP dst address */
    if ( 4 != sscanf( szDst, "%u.%u.%u.%u", &ip[0], &ip[1], &ip[2], &ip[3]))
      continue;
    ulDstAddr = ((unsigned long)ip[0] << 24)
      + ((unsigned long)ip[1] << 16)
      + (ip[2] << 8)
      + ip[3];

    /* Apply destination addres filter */
    if ( (ulDstAddr & pCfg->ulDstSubnetMask) != pCfg->ulDstAddr)
      continue;
    if ( pCfg->ulExDstAddr && (ulDstAddr & pCfg->ulExDstSubnetMask) == pCfg->ulExDstAddr)
      continue;

    /* Protocol specific action */
    if ( pCfg->eProtocol != kAny && pCfg->eProtocol != eProtocol)
      continue;
    switch ( eProtocol)
      {
    default:
      break;

    case kIcmp:
      if ( 0 != pCfg->uDstPort || 0 != pCfg->uSrcPort)
        continue;

      pStats->ulIcmpPackets++;
      break;

    case kUdp:
      if ( uSrc < pCfg->uSrcPort || uSrc > pCfg->uSrcPortMax)
        continue;
      if ( uDst < pCfg->uDstPort || uDst > pCfg->uDstPortMax)
        continue;

      pStats->ulUdpPackets++;
      if ( AddPort( pStats, uDst, kUdp))
        return 1;
      break;

    case kTcp:
      if ( uSrc < pCfg->uSrcPort || uSrc > pCfg->uSrcPortMax)
        continue;
      if ( uDst < pCfg->uDstPort || uDst > pCfg->uDstPortMax)
        continue;

      pStats->ulTcpPackets++;
      if ( AddPort( pStats, uDst, kTcp))
        return 1;
      break;
      }

    if ( pCfg->iVerbosity > 1)
      {
      /* Display packet info */
      static const char* protocols[] = { "?","ICMP","UDP","TCP" };

      fprintf( pCfg->output, "%s %.8s %-15s %-15s %-4s",
        szDate, szTime, szSrc, szDst, protocols[ eProtocol]);

      if ( kTcp == eProtocol || kUdp == eProtocol)
        fprintf( pCfg->output, " %5u > %5u", uSrc, uDst);
      else
        fputs( "              ", pCfg->output);

      if ( uTtl)
        fprintf( pCfg->output, "  %3u\n", uTtl);
      else
        fputs( "\n", pCfg->output);
      }

    /* IP specific action */
    if ( AddIpAddress(
        pStats,
        !pCfg->isSortByDstIp ? ulSrcAddr : ulDstAddr,
        pCfg->ulIpMask,
        eProtocol, uTtl,
        szDate,
        szTime
      )
    ) {
      return 1;
      }

    /* TTL */
    if ( uTtl && AddTtl( pStats, uTtl) )
      return 1;

    pStats->ulPackets++;
    }

  assert( pStats->ulPackets == pStats->ulIcmpPackets \
    + pStats->ulUdpPackets + pStats->ulTcpPackets );

  if ( pCfg->iVerbosity > 1)
    fputs( "\n", pCfg->output);

  return 0;
  }


/*
 * Generate a report
 */
int Report( FILE* output, const SStats* pStats, unsigned uCount)
  {
  unsigned long ulIndex;

  if ( output != stdout)
    fputs( kszBanner "\n", output);

  fprintf( output, "%s statistics from %s %s to %s %s\n",
    pStats->szLogType,
    pStats->szStartDate, pStats->szStartTime,
    pStats->szEndDate, pStats->szEndTime
  );

  fprintf( output, "Total packets %lu of which TCP %lu, UDP %lu, ICMP %lu\n",
    pStats->ulPackets,
    pStats->ulTcpPackets,
    pStats->ulUdpPackets,
    pStats->ulIcmpPackets
  );

  if ( pStats->uIps)
    {
    SIpAddress* pSrc;

    qsort(
      pStats->ipAddressArray,
      pStats->uIps,
      sizeof(pStats->ipAddressArray[0]),
      CmpIpCount
    );

    /* Top IP addreses */
    fprintf( output,
      "\nIP address      : Total   TCP   UDP  ICMP  TTL  Time of last packet\n" );

    pSrc = &pStats->ipAddressArray[ pStats->uIps];
    for ( ulIndex = 0;
      ulIndex < uCount && ulIndex < pStats->uIps;
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

      fprintf( output, "%-15s : %5lu %5lu %5lu %5lu %4lu  %s %s\n",
        szAddr,
        pSrc->ulCount, pSrc->ulTcpCount, pSrc->ulUdpCount, pSrc->ulIcmpCount,
        pSrc->ulTtl / pSrc->ulCount,
        pSrc->szDate, pSrc->szTime
      );
      }
    }

  if ( pStats->uDestPorts)
    {
    SDestPort* pDst;

    qsort(
      pStats->portArray,
      pStats->uDestPorts,
      sizeof(pStats->portArray[0]),
      CmpPortCount
    );

    /* Top ports */
    fprintf( output, "\nPort  : Total   TCP   UDP\n");

    pDst = &pStats->portArray[ pStats->uDestPorts];
    for ( ulIndex = 0;
      ulIndex < uCount && ulIndex < pStats->uDestPorts;
      ++ulIndex
    ) {
      --pDst;
      fprintf( output, "%-5u : %5lu %5lu %5lu\n",
        pDst->uPort,
        pDst->ulCount,
        pDst->ulTcpCount, pDst->ulCount - pDst->ulTcpCount
      );
      }
    }

  if ( 0 && pStats->uTtls)
    {
    STtl* pTtl;

    qsort(
      pStats->ttlArray,
      pStats->uTtls,
      sizeof pStats->ttlArray[0],
      CmpTtlCount
    );

    /* Top TTL's */
    fprintf( output, "\nTTL : Total\n");

    pTtl = &pStats->ttlArray[ pStats->uTtls];
    for ( ulIndex = 0;
      ulIndex < uCount && ulIndex < pStats->uTtls;
      ++ulIndex
    ) {
      --pTtl;
      fprintf( output, "%-3u : %5lu\n",
        pTtl->uTtl,
        pTtl->ulCount
      );
      }
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
  EProtocol eProtocol,
  unsigned uTtl,
  const char* pszDate,
  const char* pszTime
) {
  unsigned long ulCount;
  SIpAddress* pSrc;

  /* Find matching IP */
  for ( ulCount = pStats->uIps, pSrc = pStats->ipAddressArray;
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
    ulCount = pStats->uIps;
    if ( ulCount * sizeof(pStats->ipAddressArray[0]) >= pStats->sizeIps)
      {
      /* Extend the array */
      size_t size = pStats->sizeIps ? pStats->sizeIps * 2
        : INITIAL_IPS * sizeof(pStats->ipAddressArray[0]);
      void* pv = realloc( pStats->ipAddressArray, size);
      if ( NULL == pv)
        return 1;

      pStats->ipAddressArray = (SIpAddress*)pv;
      pStats->sizeIps = size;
      }

    pSrc = &pStats->ipAddressArray[ ulCount];
    pSrc->ulAddr = ulAddr;
    pSrc->ulCount = 1;
    pSrc->ulIcmpCount = 0;
    pSrc->ulUdpCount = 0;
    pSrc->ulTcpCount = 0;
    pSrc->ulTtl = 0;
    strcpy( pSrc->szDate, pszDate);
    strcpy( pSrc->szTime, pszTime);

    pStats->uIps = 1 + ulCount;
    }
  else
    {
    /* Update date & time if later */
    int i = strcmp( pSrc->szDate, pszDate);
    if ( i < 0)
      {
      strcpy( pSrc->szDate, pszDate);
      strcpy( pSrc->szTime, pszTime);
      }
    else if ( 0 == i && strcmp( pSrc->szTime, pszTime) < 0)
      {
      strcpy( pSrc->szTime, pszTime);
      }
    }

  switch ( eProtocol)
    {
  default:
    break;
  case kIcmp:
    ++(pSrc->ulIcmpCount);
    break;
  case kUdp:
    ++(pSrc->ulUdpCount);
    break;
  case kTcp:
    ++(pSrc->ulTcpCount);
    break;
    }

  pSrc->ulTtl += uTtl;
  return 0;
  }


/*
 * Add a target port
 */
int AddPort( SStats* pStats, unsigned uPort, EProtocol eProtocol)
  {
  unsigned uCount;
  SDestPort* pDst;

  for ( uCount = pStats->uDestPorts, pDst = pStats->portArray;
    uCount > 0;
    --uCount, ++pDst
  ) {
    if ( pDst->uPort == uPort)
      {
      ++(pDst->ulCount);
      break;
      }
    }

  if ( 0 == uCount)
    {
    uCount = pStats->uDestPorts;
    if ( uCount * sizeof(pStats->portArray[0]) >= pStats->sizePorts)
      {
      /* Extend the array */
      size_t size = pStats->sizePorts ? pStats->sizePorts * 2
        : INITIAL_PORTS * sizeof(pStats->portArray[0]);
      void* pv = realloc( pStats->portArray, size);
      if ( NULL == pv)
        return 1;

      pStats->portArray = (SDestPort*)pv;
      pStats->sizePorts = size;
      }

    pDst = &pStats->portArray[ uCount];
    pDst->uPort = uPort;
    pDst->ulCount = 1;
    pDst->ulTcpCount = 0;
    pStats->uDestPorts = 1 + uCount;
    }

  switch ( eProtocol)
    {
  default:
    break;
  case kTcp:
    ++(pDst->ulTcpCount);
    break;
    }
  return 0;
  }


/*
 * Add a TTL
 */
int AddTtl( SStats* pStats, unsigned uTtl)
  {
  unsigned uCount;
  STtl* pTtl;

  for ( uCount = pStats->uTtls, pTtl = pStats->ttlArray;
    uCount > 0;
    --uCount, ++pTtl
  ) {
    if ( pTtl->uTtl == uTtl)
      {
      ++(pTtl->ulCount);
      break;
      }
    }

  if ( 0 == uCount)
    {
    uCount = pStats->uTtls;
    if ( uCount * sizeof pStats->ttlArray[0] >= pStats->sizeTtls)
      {
      /* Extend the array */
      size_t size = pStats->sizeTtls ? pStats->sizeTtls * 2
        : INITIAL_TTLS * sizeof pStats->ttlArray[0];
      void* pv = realloc( pStats->ttlArray, size);
      if ( NULL == pv)
        return 1;

      pStats->ttlArray = (STtl*)pv;
      pStats->sizeTtls = size;
      }

    pTtl = &pStats->ttlArray[ uCount];
    pTtl->uTtl = uTtl;
    pTtl->ulCount = 1;
    pStats->uTtls = 1 + uCount;
    }

  return 0;
  }


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


int CmpPortCount( const void* pv1, const void* pv2)
  {
  if ( ((const SDestPort*)pv1)->ulCount < ((const SDestPort*)pv2)->ulCount)
    return -1;

  if ( ((const SDestPort*)pv1)->ulCount > ((const SDestPort*)pv2)->ulCount)
    return 1;

  /* Lowest port number sorts last */
  return (int)((const SDestPort*)pv2)->uPort - (int)((const SDestPort*)pv1)->uPort;
  }


int CmpTtlCount( const void* pv1, const void* pv2)
  {
  if ( ((const STtl*)pv1)->ulCount < ((const STtl*)pv2)->ulCount)
    return -1;

  if ( ((const STtl*)pv1)->ulCount > ((const STtl*)pv2)->ulCount)
    return 1;

  /* Lowest TTL sorts last */
  return (int)((const STtl*)pv2)->uTtl - (int)((const STtl*)pv1)->uTtl;
  }


/*
 * Convert month name to a number (1..12)
 */
unsigned MonthNum( const char* psz)
  {
  int i;
  static char const months[12][4] = {
    "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
  };

  for ( i = 0; i < 12; ++i)
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
      do
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

        case 'l':
          /* Log file format */
          pszArg = GetArg();
          if ( !strcmp( pszArg, "syslog"))
            pCfg->eLogFormat = kSyslog;
          else if ( !strcmp( pszArg, "pktdump"))
            pCfg->eLogFormat = kPktdump;
          else if ( !strcmp( pszArg, "netcen"))
            pCfg->eLogFormat = kNetcenturion;
          else
            {
            fprintf( stderr, "Invalid log format %s\n", pszArg);
            return EXIT_FAILURE;
            }
          pszArg = NULL;
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

        case 'p':
          /* Dst port */
          pszArg = GetArg();
          ip[1] = 0;
          if ( 1 > sscanf( pszArg, " %u-%u", &ip[0], &ip[1])
            || ip[0] > 65535U
            || (ip[1] > 0 && ip[0] > ip[1])
          ) {
            fprintf( stderr, "Invalid port %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          pCfg->uDstPort = ip[0];
          pCfg->uDstPortMax = ip[1] ? ip[1] : ip[0];
          break;

        case 'P':
          /* Src port */
          pszArg = GetArg();
          ip[1] = 0;
          if ( 1 > sscanf( pszArg, " %u-%u", &ip[0], &ip[1])
            || ip[0] > 65535U
            || (ip[1] > 0 && ip[0] > ip[1])
          ) {
            fprintf( stderr, "Invalid port %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          pCfg->uSrcPort = ip[0];
          pCfg->uSrcPortMax = ip[1] ? ip[1] : ip[0];
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

        case 'I':
          /* Dst IP address */
          pszArg = GetArg();
          uMask = 32;
          if ( 4 > sscanf( pszArg, "%u.%u.%u.%u/%u",
              &ip[0], &ip[1], &ip[2], &ip[3], &uMask)
          ) {
            fprintf( stderr, "Invalid IP address %s\n", pszArg);
            return EXIT_FAILURE;
            }
          pszArg = NULL;

          pCfg->ulDstAddr = ((unsigned long)ip[0] << 24)
            + ((unsigned long)ip[1] << 16)
            + (ip[2] << 8)
            + ip[3];
          pCfg->ulDstSubnetMask = 0xffffffffL << (32 - uMask);
          pCfg->ulDstAddr &= pCfg->ulDstSubnetMask;
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

        case 'X':
          /* Exclude dst IP address range */
          pszArg = GetArg();
          uMask = 32;
          if ( 4 > sscanf( pszArg, "%u.%u.%u.%u/%u",
              &ip[0], &ip[1], &ip[2], &ip[3], &uMask)
          ) {
            fprintf( stderr, "Invalid IP address %s\n", pszArg);
            return EXIT_FAILURE;
            }
          pszArg = NULL;

          pCfg->ulExDstAddr = ((unsigned long)ip[0] << 24)
            + ((unsigned long)ip[1] << 16)
            + (ip[2] << 8)
            + ip[3];
          pCfg->ulExDstSubnetMask = 0xffffffffL << (32 - uMask);
          pCfg->ulExDstAddr &= pCfg->ulExDstSubnetMask;
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
          if ( !strcmp( pszArg, "udp"))
            pCfg->eProtocol = kUdp;
          else if ( !strcmp( pszArg, "tcp"))
            pCfg->eProtocol = kTcp;
          else if ( !strcmp( pszArg, "icmp"))
            pCfg->eProtocol = kIcmp;
          else
            {
            fprintf( stderr, "Invalid protocol %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          break;

        case 'D':
          /* Sort on dest IP */
          pCfg->isSortByDstIp = 1;
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
