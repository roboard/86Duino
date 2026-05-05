/*****************************************************************************
 *
 * File name   : main.c
 * Function    : DNS query utility
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, BSD sockts
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 18-Jul-09, LVR, 1.02 Added Netbios name lookup and minimise output
 * 02-Dec-02, LVR, Created.
 *
 *
 * Comments
 * --------
 *
 *****************************************************************************/
#define kszApp "dns"
#define kszVer "1.02"
#define kszBanner kszApp " " kszVer " (" SWS_NET_ID ")" ", " TIMESTAMP "."
#define kszAbout "Domain name <=> IP address translation."

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
#include <time.h>
#include <ctype.h>

#include "sws_net.h"
#if SWS_USE_SOCKLIB && PACKET_DUMP
#include "sws_dump.h"
#endif

#include "sws_dns.h"
#include "sws_cfg.h"


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


/***************************************************************************
 * Data types
 ***************************************************************************/

/* Parameters */
typedef struct SConfig
  {
  int iVerbosity;
  const char* pszHost;    /* Name to resolve */
  int mode;
  const char* pszServer;  /* Namer server */
  unsigned port;
  unsigned opcode;
  unsigned queryType;
  unsigned queryClass;
  } SConfig;


/***************************************************************************
 * Local function prototypes
 ***************************************************************************/

#ifdef SIGINT
static void OnSigInt( int);
#endif
static int ParseArgs( SConfig* const, int, const char* const [] );
static void Help( const char* const);

static int NameQuery( const char [], int, FILE*);
static int DnsQuery( const char [], u_long, unsigned, unsigned, unsigned, unsigned, int, FILE*);
static unsigned NetbiosName( void*, const char []);
static void Netbios2Asciiz( char []);


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
    "\nSyntax: %.*s [options] {hostname|IP}\n"
    "-d{n} : 0= host resolver, 1= DNS query (default), 2= Netbios query\n"
    "-n{s} : Name server (hostname|IP}\n"
    "-p{n} : Port. Default " STRING(SWS_DNS_SERVER) "\n"
    "-c{n} : Query opcode. Default %#x\n"
    "-q{n} : Query type: 1= hostname (default), 15= mx, 16=text, 255= all\n"
    "-s{n} : Class: 1= IN, 3= CHAOS\n"
    "-t    : Decrease verbosity of output\n"
    "-v    : Increase verbosity of output\n"
    "-V    : Show version and exit\n"
    "-h    : Help\n",
    (int)c, pszCmd
    , SWS_DNS_OP_QUERY | SWS_DNS_RD_MASK
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
  char szHost[ 256];
  u_long ulServer;

  /* Set default options */
  cfg.iVerbosity = 1;
  cfg.pszHost = NULL;
  cfg.mode = 1; /* DNS lookup */
  cfg.pszServer = NULL;
  cfg.port = SWS_DNS_SERVER;
  cfg.opcode = SWS_DNS_OP_QUERY | SWS_DNS_RD_MASK;
  cfg.queryType = SWS_DNS_TYPE_A;
  cfg.queryClass = SWS_DNS_CLASS_IN;

  /* Parse args */
  iRet = ParseArgs( &cfg, argc, argv );
  if ( EXIT_SUCCESS != iRet)
    return iRet;

  /* Check for required args */
  if ( NULL == cfg.pszHost || '\0' == cfg.pszHost[0])
    {
    fputs( "Host name or IP required\n", stderr);
    return EXIT_FAILURE;
    }

  /* Specify the configuration file */
  if ( !SWS_CfgSetName( NULL, argv[0] ) && cfg.mode && NULL == cfg.pszServer)
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
  SWS_SockUp( GEF( stdout, cfg.iVerbosity), NULL);

  /* Check network is up */
  if ( gethostname( szHost, sizeof(szHost))
    || NULL == (pHost = gethostbyname( szHost))
  ) {
    fprintf( stderr, "No network (driver not installed)\n");
    return EXIT_FAILURE;
    }

  /* Name server given? */
  if ( cfg.mode && NULL != cfg.pszServer)
    {
    /* Yes, try an IP address */
    ulServer = inet_addr( cfg.pszServer);
    if ( INADDR_ANY == ulServer)
      {
      fprintf( stderr, "Invalid host %s\n", cfg.pszServer );
      return EXIT_FAILURE;
      }
    else if ( INADDR_NONE != ulServer)
      {
      ulServer = ntohl( ulServer);
      }
    /* Try a hostname */
    else if ( NULL != (pHost = gethostbyname( cfg.pszServer ))
      && AF_INET == pHost->h_addrtype
    ) {
      ulServer = ntohl( ((const struct in_addr*)pHost->h_addr)->s_addr);
      }
    else
      {
      fprintf( stderr, "Unable to resolve %s\n", cfg.pszServer);
      return EXIT_FAILURE;
      }
    }
  else
    {
    ulServer = INADDR_ANY;
    }

  if ( cfg.iVerbosity > 1)
    {
    fprintf( stdout, "Resolving %s", cfg.pszHost);
    fflush( stdout);
    }

  switch ( cfg.mode)
    {
  case 2:
    /* Convert name to netbios form */
    NetbiosName( szHost, cfg.pszHost);

    iRet = DnsQuery(
      &szHost[ 1],
      ulServer, SWS_DNS_NB,
      cfg.opcode | (0*0x10/*BB*/),
      cfg.queryType, cfg.queryClass,
      cfg.iVerbosity > 1,
      stdout
    );
    break;

  case 1:
    iRet = DnsQuery(
      cfg.pszHost,
      ulServer, cfg.port,
      cfg.opcode, cfg.queryType, cfg.queryClass,
      cfg.iVerbosity > 1,
      stdout
    );
    break;

  case 0:
    /* Standard sockets calls */
    iRet = NameQuery( cfg.pszHost, cfg.iVerbosity > 1, stdout);
    break;

  default:
    fprintf( stderr, "Invalid resolver type %d\n", cfg.mode);
    return EXIT_FAILURE;
    }

  return iRet ? EXIT_FAILURE : EXIT_SUCCESS;
  }


/*
 * Lookup a name using standard sockets APIs
 */
int NameQuery( const char pszHost[], int more, FILE* logfile)
  {
  struct hostent* pHost;
  struct in_addr addr;
  int iRet, isName;

  if ( more )
    {
    fputs( "...", logfile);
    fflush( logfile);
    }

  /* Is it an IP address? */
  addr.s_addr = inet_addr( pszHost );
  if ( INADDR_ANY == addr.s_addr)
    {
    fprintf( stderr, "\nInvalid host address\n");
    return 1;
    }
  else if ( INADDR_NONE == addr.s_addr)
    {
    /* No, lookup host by name */
    isName = 1;
    pHost = gethostbyname( pszHost);
    }
  else
    {
    /* Yes, lookup host by address */
    isName = 0;
    pHost = gethostbyaddr( (const char*)&addr, sizeof( addr), AF_INET);
    }

  if ( more )
    fputs( "\n", logfile);

  if ( NULL != pHost)
    {
    int i;

    if ( isName)
      {
      /* Host name */
      if ( more)
        fprintf( logfile, "Name: %s = ", pHost->h_name);

      /* IP addresses */
      for ( i = 0; NULL != pHost->h_addr_list[i]; ++i)
        {
        fprintf( logfile, "%s%s",
          i > 0 ? " " : "",
          inet_ntoa( *(const struct in_addr*)pHost->h_addr_list[i] ) );
        if ( !more)
          break;
        }
      fputs( "\n", logfile);
      }
    else
      {
      fprintf( logfile, "%s\n", pHost->h_name);
      }

    if ( more)
      {
      /* Aliases */
      for ( i = 0; NULL != pHost->h_aliases[i]; ++i)
        fprintf( logfile, "Alias: %s for %s\n", pHost->h_name, pHost->h_aliases[i]);
      }

    iRet = 0;
    }
  else
    {
    /* Not found */
    const char* pszErr;

    iRet = GetLastError();
    switch ( iRet)
      {
#ifdef EHOST_NOT_FOUND
    case EHOST_NOT_FOUND:
      pszErr = "Name not found";
      break;
#endif
#ifdef ETRY_AGAIN
    case ETRY_AGAIN:
      pszErr = "No response";
      break;
#endif
#ifdef ENO_RECOVERY
    case ENO_RECOVERY:
      pszErr = "Unrecoverable error";
      break;
#endif
#ifdef ENO_DATA
    case ENO_DATA:
      pszErr = "No data";
      break;
#endif
    case ENETDOWN:
      pszErr = "Network failure";
      break;
    default:
      pszErr = strerror( iRet);
      break;
      }
    fprintf( stderr, "FAILED: %d (%s)\n", iRet, pszErr);
    }

  return iRet;
  }


/*
 * Lookup name using DNS queries
 */
int DnsQuery(
  const char pszHost[],
  u_long server,
  unsigned port,
  unsigned opcode,
  unsigned queryType,
  unsigned queryClass,
  int more,
  FILE* logfile
) {
  SWS_DnsQHandle hQuery;
  SWS_SDnsQRecord rec;
  struct in_addr addr;
  char szHost[ 256];
  int iRet;

  /* User specified name server? */
  if ( INADDR_ANY == server)
    {
    /* No, use default name server */
    server = SWS_DnsDefaultServer();

    /* Name server valid? */
    if ( INADDR_ANY == server || INADDR_NONE == server)
      {
      fprintf( stderr, " FAILED: No name server\n");
      return 1;
      }
    }

  /* Is it an IP address? */
  if ( SWS_DNS_TYPE_A == queryType
    && SWS_DNS_CLASS_IN == queryClass
    && INADDR_NONE != (addr.s_addr = inet_addr( pszHost ))
  ) {
    /* Yes */
    u_long a = ntohl( addr.s_addr);
    if ( INADDR_ANY == a)
      {
      fprintf( stderr, " Invalid host address\n");
      return 1;
      }

    /* Convert IP address to IN-ADDR.ARPA domain for query */
    sprintf( szHost, "%u.%u.%u.%u.IN-ADDR.ARPA",
      (unsigned)a & 255,
      (unsigned)(a >> 8) & 255,
      (unsigned)(a >> 16) & 255,
      (unsigned)(a >> 24)
    );

    if ( more )
      {
      addr.s_addr = htonl( server);
      fprintf( logfile, " using DNS server %s:%u ...", inet_ntoa( addr), port );
      fflush( logfile);
      }

    /* Lookup host by address */
    hQuery = SWS_DnsOpenQuery(
      opcode,
      szHost,
      SWS_DNS_TYPE_PTR,
      SWS_DNS_CLASS_IN,
      server,
      port
    );
    }
  /* Is it a Netbios name query */
  else if ( SWS_DNS_QTYPE_NB == queryType
    || SWS_DNS_QTYPE_NBSTAT == queryType
  ) {
    if ( more )
      {
      addr.s_addr = htonl( server);
      fprintf( logfile, " using NBNS server %s:%u ...", inet_ntoa( addr), port);
      fflush( logfile);
      }

    /* Lookup host */
    hQuery = SWS_DnsOpenQuery(
      opcode,
      pszHost,
      queryType,
      queryClass,
      server,
      port
    );
    }
  else
    {
    if ( more )
      {
      addr.s_addr = htonl( server);
      fprintf( logfile, " using DNS server %s:%u ...", inet_ntoa( addr), port );
      fflush( logfile);
      }

    /* Lookup host by name */
    hQuery = SWS_DnsOpenQuery(
      opcode,
      pszHost,
      queryType,
      queryClass,
      server,
      port
    );
    }

  if ( NULL != hQuery)
    {
    int rcode;

    /* Get result code */
    rcode = SWS_DnsQueryRcode( hQuery) & SWS_DNS_RCODE_MASK;
    if ( SWS_DNS_RC_GOOD == rcode)
      {
      if ( more )
        fputs( " OK\n", logfile);
      }
    else
      {
      if ( more )
        fputs( "\n", logfile);

      switch ( rcode)
        {
      case SWS_DNS_RC_FORMAT:
        fprintf( stderr, "FAILED: %d (Invalid query)\n", rcode);
        break;
      case SWS_DNS_RC_SFAIL:
        more = 1;
        fprintf( stderr, "FAILED: %d (Server failure)\n", rcode);
        break;
      case SWS_DNS_RC_NAMERR:
        more = 1;
        logfile = stderr;
        fprintf( stderr, "FAILED: %d (Name not found)\n", rcode);
        break;
      case SWS_DNS_RC_NOTIMP:
        fprintf( stderr, "FAILED: %d (Not implemented)\n", rcode);
        break;
      case SWS_DNS_RC_REFUSED:
        fprintf( stderr, "FAILED: %d (Refused)\n", rcode);
        break;

      case SWS_DNS_RC_NWERR:
        fprintf( stderr, "FAILED: Network error\n");
        break;
      case SWS_DNS_RC_NOREPLY:
        fprintf( stderr, "FAILED: server not responding\n");
        break;

      default:
        fprintf( stderr, "FAILED: %d\n", rcode);
        break;
        }
      }

    if ( rcode < SWS_DNS_RC_NWERR)
      {
      rcode = SWS_DnsQueryRcode( hQuery);
      if ( rcode & SWS_DNS_TC_MASK)
        fputs( "TRUNCATED, ", stderr);

      if ( more )
        {
        if ( rcode & SWS_DNS_AA_MASK)
          fprintf( logfile, "Authoritative answer\n");
        else
          fprintf( logfile, "Non-authoritative answer\n");
        }
      }

    /* Enumerate records */
    rcode = 0;
    while ( !SWS_DnsQueryRead( hQuery, &rec))
      {
      switch ( rec.eRecordType)
        {
      case SWS_kDnsQuestion:
        continue;

      case SWS_kDnsAnswer:
        break;

      case SWS_kDnsAuthority:
        fprintf( logfile, "Authority ");
        break;

      case SWS_kDnsAdditional:
        fprintf( logfile, "Extra ");
        break;

      default:
        fprintf( logfile, "Type %d: ", rec.eRecordType);
        break;
        }
      ++rcode;

      if ( SWS_DNS_CLASS_IN == rec.rclass
        || SWS_DNS_CLASS_CH == rec.rclass
      ) {
        unsigned offset;
        ui32 u;
        ui16 u1;
        ui8 u2;

        switch ( rec.rtype)
          {
        case SWS_DNS_TYPE_A:
          SWS_DnsQueryName( szHost, sizeof( szHost), hQuery, rec.nameIndex);
          SWS_DnsQueryData( &addr, sizeof( addr), hQuery, rec.dataIndex);
          if ( more || SWS_kDnsAdditional == rec.eRecordType || SWS_DNS_QTYPE_STAR == queryType)
            fprintf( logfile, "Name: %s = %s", szHost, inet_ntoa( addr) );
          else
            fputs( inet_ntoa( addr), logfile );
          break;
        case SWS_DNS_TYPE_NS:
          if ( more || SWS_kDnsAdditional == rec.eRecordType || SWS_DNS_QTYPE_STAR == queryType)
            {
            SWS_DnsQueryName( szHost, sizeof( szHost), hQuery, rec.nameIndex);
            fprintf( logfile, "Name server for %s is ", szHost);
            }
          SWS_DnsQueryName( szHost, sizeof( szHost), hQuery, rec.dataIndex);
          fputs( szHost, logfile);
          break;
        case SWS_DNS_TYPE_CNAME:
          if ( more || SWS_kDnsAdditional == rec.eRecordType || SWS_DNS_QTYPE_STAR == queryType)
            {
            SWS_DnsQueryName( szHost, sizeof( szHost), hQuery, rec.nameIndex);
            fprintf( logfile, "Alias: %s for ", szHost);
            SWS_DnsQueryName( szHost, sizeof( szHost), hQuery, rec.dataIndex);
            fputs( szHost, logfile);
            }
          else
            continue;
          break;
        case SWS_DNS_TYPE_PTR:
          if ( more || SWS_kDnsAdditional == rec.eRecordType || SWS_DNS_QTYPE_STAR == queryType)
            {
            SWS_DnsQueryName( szHost, sizeof( szHost), hQuery, rec.nameIndex);
            fprintf( logfile, "Ptr: %s = ", szHost);
            }
          SWS_DnsQueryName( szHost, sizeof( szHost), hQuery, rec.dataIndex);
          fputs( szHost, logfile);
          break;
        case SWS_DNS_TYPE_SOA:
          SWS_DnsQueryName( szHost, sizeof( szHost), hQuery, rec.nameIndex);
          fprintf( logfile, "SOA for %s", szHost);
          offset = SWS_DnsQueryName( szHost, sizeof( szHost), hQuery, rec.dataIndex);
          fprintf( logfile, " is %s", szHost);
          offset = SWS_DnsQueryName( szHost, sizeof( szHost), hQuery, offset);
          fprintf( logfile, " mbx %s", szHost);
          if ( more)
            {
            SWS_DnsQueryData( &u, sizeof( u), hQuery, offset);
            fprintf( logfile, " #%08lX", (unsigned long)ntohl( u));
            SWS_DnsQueryData( &u, sizeof( u), hQuery, offset + 4);
            fprintf( logfile, " refresh:%lu", (unsigned long)ntohl( u));
            SWS_DnsQueryData( &u, sizeof( u), hQuery, offset + 8);
            fprintf( logfile, " retry:%lu", (unsigned long)ntohl( u));
            SWS_DnsQueryData( &u, sizeof( u), hQuery, offset + 12);
            fprintf( logfile, " expire:%lu", (unsigned long)ntohl( u));
            SWS_DnsQueryData( &u, sizeof( u), hQuery, offset + 16);
            fprintf( logfile, " min:%lu", (unsigned long)ntohl( u) );
            }
          break;
        case SWS_DNS_TYPE_MX:
          if ( more || SWS_kDnsAdditional == rec.eRecordType || SWS_DNS_QTYPE_STAR == queryType)
            {
            SWS_DnsQueryName( szHost, sizeof( szHost), hQuery, rec.nameIndex);
            fprintf( logfile, "Mail eXchange for %s is ", szHost);
            }
          SWS_DnsQueryName( szHost, sizeof( szHost), hQuery, rec.dataIndex + 2);
          fputs( szHost, logfile);
          SWS_DnsQueryData( &u1, sizeof( u1), hQuery, rec.dataIndex);
          fprintf( logfile, " (pref %u)", ntohs( u1));
          break;
        case SWS_DNS_TYPE_TXT:
          if ( more || SWS_kDnsAdditional == rec.eRecordType|| SWS_DNS_QTYPE_STAR == queryType)
            {
            SWS_DnsQueryName( szHost, sizeof( szHost), hQuery, rec.nameIndex);
            fprintf( logfile, "Text: %s = ", szHost);
            }
          SWS_DnsQueryName( szHost, sizeof( szHost), hQuery, rec.dataIndex);
          fputs( szHost, logfile);
          break;
        case SWS_DNS_QTYPE_NBSTAT:
          if ( more || SWS_kDnsAdditional == rec.eRecordType)
            {
            SWS_DnsQueryName( szHost, sizeof( szHost), hQuery, rec.nameIndex);
            Netbios2Asciiz( szHost);
            fprintf( logfile, "NBSTAT: %s\n", szHost);
            }

          SWS_DnsQueryData( &u2, 1, hQuery, rec.dataIndex);
          offset = rec.dataIndex + 1;
          while ( u2-- > 0)
            {
            SWS_DnsQueryData( szHost, 16, hQuery, offset);
            fputs( "  [", logfile);
            for ( u = 0; u < 16; ++u)
              {
              if ( isprint( szHost[ u]) )
                fputc( szHost[ u], logfile);
              else
                fprintf( logfile, "<%02x>", *((ui8*)szHost + u) );
              }

            SWS_DnsQueryData( &u1, 2, hQuery, offset + 16);
            fprintf( logfile, "] %#x\n", u1);

            offset += 18;
            }

          /* Get unit ID (MAC address) */
          SWS_DnsQueryData( szHost, 6, hQuery, offset);
          fprintf( logfile, "  ID:");
          for ( offset = 0; offset < 6; ++offset)
            fprintf( logfile, offset > 0 ? "-%02X" : "%02X", *((ui8*)szHost + offset) );
          break;
        case SWS_DNS_QTYPE_NB:
          SWS_DnsQueryName( szHost, sizeof( szHost), hQuery, rec.nameIndex);
          Netbios2Asciiz( szHost);
          SWS_DnsQueryData( &addr, sizeof( addr), hQuery, rec.dataIndex +2);
          fprintf( logfile, "NB name: %s = %s", szHost, inet_ntoa( addr) );
          break;
        default:
          SWS_DnsQueryName( szHost, sizeof( szHost), hQuery, rec.nameIndex);
          fprintf( logfile, "%s type %u len %u", szHost, rec.rtype, rec.rdlen);
          break;
          }
        }
      else
        {
        fprintf( logfile, " Class %d type %d", rec.rclass, rec.rtype);
        }

      if ( more)
        {
        /* Show Time To Live */
        ui32 hr, mn, sc;
        mn = rec.ttl / 60;
        sc = rec.ttl - 60 * mn;
        hr = mn / 60;
        mn -= 60 * hr;
        fprintf( logfile, " (ttl %lu:%02lu:%02lu)\n", hr, mn, sc);
        }
      else
        {
        fputs( "\n", logfile);
        }
      }

    iRet = SWS_DNS_RC_GOOD != (SWS_DnsQueryRcode( hQuery) & SWS_DNS_RCODE_MASK);
    if ( 0 == iRet && 0 == rcode)
      {
      fprintf( logfile, "No data\n");
      iRet = 1;
      }

    SWS_DnsCloseQuery( hQuery);
    }
  else
    {
    fputs( " FAILED: Insufficient resources\n", stderr);
    iRet = 1;
    }

  return iRet;
  }


/*
 * Create a NetBIOS name from ASCIIZ
 */
unsigned NetbiosName( void* pv, const char pszSrc[])
  {
  unsigned char* pcDst = (unsigned char*)pv;
  unsigned char c;
  unsigned count = 0;
  const int isWild = '*' == pszSrc[0];

  *pcDst++ = 32; /* Length */

  while ( '\0' != (c = (unsigned char)*pszSrc++) )
    {
    if ( '.' == c)
      break;

    c = (unsigned char)toupper( c);
    *pcDst++ = (unsigned char)('A' + ((c / 16) & 0xf));
    *pcDst++ = (unsigned char)('A' + (c & 0xf));

    if ( ++count >= 16)
      break;
    }

  while ( count < 16)
    {
    ++count;
    if ( isWild)
      {
      /* Pad with nulls */
      *pcDst++ = (unsigned char)'A';
      *pcDst++ = (unsigned char)'A';
      }
    else
      {
      /* Pad with spaces */
      *pcDst++ = (unsigned char)'A' + 2;
      *pcDst++ = (unsigned char)'A';
      }
    }

  *pcDst++ = 0;
  return pcDst - (unsigned char*)pv;
  }


/*
 * Convert NetBIOS name to ASCIIZ
 */
void Netbios2Asciiz( char pszName[])
  {
  char* pszDst = pszName;
  unsigned count = 0;
  unsigned char c;

  while ( *pszName )
    {
    c = (unsigned char)((*(pszName++) - 'A') << 4);
    c |= *(pszName++) - 'A';
    if ( 0 == c)
      break;
    if ( isprint( c))
      *(pszDst++) = (char)c;
    else
      pszDst += sprintf( pszDst, "<%02X>", c);

    if ( ++count >= 16)
      break;
    }

  *pszDst++ = 0;
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
        long lVal;
        char* pszEnd;

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

        case 'd':
          /* Mode */
          pszArg = GetArg();
          lVal = strtol( pszArg, &pszEnd, 0);
          if ( *pszEnd || lVal < 0 || lVal > 2)
            {
            fprintf( stderr, "Invalid mode %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pCfg->mode = (int)lVal;
          if ( 2 == pCfg->mode)
            pCfg->queryType = SWS_DNS_QTYPE_NB;

          pszArg = NULL;
          break;

        case 'c':
          /* Query opcode */
          pszArg = GetArg();
          lVal = strtol( pszArg, &pszEnd, 0);
          if ( *pszEnd || lVal < 0 || lVal > 0xffff)
            {
            fprintf( stderr, "Invalid opcode %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pCfg->opcode = (unsigned)lVal;
          pszArg = NULL;
          break;

        case 'q':
          /* Query type */
          pszArg = GetArg();
          lVal = strtol( pszArg, &pszEnd, 0);
          if ( *pszEnd || lVal < 0 || lVal > 255)
            {
            fprintf( stderr, "Invalid query %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pCfg->queryType = (unsigned)lVal;
          pszArg = NULL;
          break;

        case 's':
          /* Query class */
          pszArg = GetArg();
          lVal = strtol( pszArg, &pszEnd, 0);
          if ( *pszEnd || lVal < 0 || lVal > 255)
            {
            fprintf( stderr, "Invalid query %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pCfg->queryClass = (unsigned)lVal;
          pszArg = NULL;
          break;

        case 'p':
          /* Port */
          pszArg = GetArg();
          lVal = strtol( pszArg, &pszEnd, 0);
          if ( *pszEnd || lVal <= 0 || lVal > 65535L)
            {
            fprintf( stderr, "Invalid port %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pCfg->port = (unsigned)lVal;
          pszArg = NULL;
          break;

        case 'n':
          /* Name server */
          pCfg->pszServer = GetArg();
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
        /* hostname */
        pCfg->pszHost = pszArgvArray[i];
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
