/*****************************************************************************
 *
 * File name   : ntp.c
 * Function    : NTP client (RFC 958,1059,1119 & 1305)
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, BSD sockets
 * Version     : 1.0
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 27-Feb-03, LVR, Created.
 *
 *
 * Comments
 * --------
 *
 *****************************************************************************/
/* Compilation options */
#ifndef SWS_USE_SOCKLIB
#define SWS_USE_SOCKLIB 0 /* 0= native sockets, !0 for sws_sock */
#endif

/* Exports */
#if !SWS_USE_SOCKLIB
#include "sws_ntp.h"
#endif

/* Imports */

/* ANSI C library */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "sws_net.h"
#include "sws_ntp.h"
#include "sws_cfg.h"


/***************************************************************************
 * Macros & constants
 ***************************************************************************/

/* Read/write in network (MSB first, or big-endian) byte order */
#define SWS_POKE8(_p,_o,_v) (*(SWS_PTR(ui8))((SWS_PTR(char))(_p) + (_o)) = (_v))
#define SWS_POKE16(_p,_o,_v) (*(SWS_PTR(SWS_PACKED(ui16)))((SWS_PTR(char))(_p) + (_o)) = htons(_v))
#define SWS_POKE32(_p,_o,_v) (*(SWS_PTR(SWS_PACKED(ui32)))((SWS_PTR(char))(_p) + (_o)) = htonl(_v))
#define SWS_PEEK8(_v,_p,_o) _v = (*(SWS_PTR(const ui8))((SWS_PTR(const char))(_p) + (_o)))
#define SWS_PEEK16(_v,_p,_o) _v = ntohs(*(SWS_PTR(SWS_PACKED(const ui16)))((SWS_PTR(const char))(_p) + (_o)))
#define SWS_PEEK32(_v,_p,_o) _v = ntohl(*(SWS_PTR(SWS_PACKED(const ui32)))((SWS_PTR(const char))(_p) + (_o)))

/* Default parameter values */
#define NTP_SEND_TIMEOUT 2000 /* Milliseconds */
#define NTP_RECV_TIMEOUT 1000 /* Milliseconds */
#define NTP_RETRIES 4

#define NTP_MINPOLL 6 /* 2^6 seconds */

#define SECS_1900_TO_1970 ((70UL * 365UL + 17UL) * (24UL * 3600UL))

#define HALF_SECOND (1UL<<31);


/* ini file values */
#define kszNtpSect   "ntp"
#define kszPort      "port"
#define kszServer    "server"


/***************************************************************************
 * Data types
 ***************************************************************************/


/***************************************************************************
 * Local function prototypes
 ***************************************************************************/

static void TimeSub( SWS_SNtpTime*, const SWS_SNtpTime*, const SWS_SNtpTime*);
static void TimeAdd( SWS_SNtpTime*, const SWS_SNtpTime*, const SWS_SNtpTime*);
static SWS_SNtpTime* PlatformGetTime( SWS_SNtpTime*);


/***************************************************************************
 * Module data
 ***************************************************************************/


/***************************************************************************
 * Functions
 ***************************************************************************/

/*
 * Read the time from an NTP server
 */
extern int SWS_NtpQuery(
  SWS_PTR( SWS_SNtpRecord) pRec,        /* OUT: */
  SWS_PTR( struct sockaddr) pSock,      /* IN: NTP server */
  size_t size
) {
  SOCKET s;
  int iRet;
  int iTry;
  unsigned timeout; /* Receive timeout in milliseconds */

  /* Select time server */
  if ( AF_UNSPEC == pSock->sa_family)
    {
    char szHost[ 256];
    struct hostent* pHost;
    u_long addr;

    if ( 0 == SWS_CfgGetString( kszNtpSect, kszServer, "", szHost, sizeof(szHost) ))
      return EADDRNOTAVAIL;

    /* Try an IP address */
    addr = inet_addr( szHost);
    if ( INADDR_ANY == addr)
      {
      /* Empty or 0 address */
      return EADDRNOTAVAIL;
      }
    else if ( INADDR_NONE != addr)
      {
      /* Valid IPv4 address */
      ((struct sockaddr_in*)pSock)->sin_addr.s_addr = addr;
      pSock->sa_family = AF_INET;
      }
    /* Try a hostname */
    else if ( NULL != (pHost = gethostbyname( szHost)) )
      {
      /* Valid hostname */
      pSock->sa_family = pHost->h_addrtype;
      memcpy( &(((struct sockaddr_in*)pSock)->sin_addr), pHost->h_addr, pHost->h_length);
      }
    else
      {
      return GetLastError();
      }
    }

  if ( AF_INET == pSock->sa_family
    && 0 == ((struct sockaddr_in*)pSock)->sin_port
  ) {
    ((struct sockaddr_in*)pSock)->sin_port = htons(
      (u_short)SWS_CfgGetValue( kszNtpSect, kszPort, SWS_NTP_SERVER)
    );
    }

  /* Open server socket */
  s = socket( pSock->sa_family, SOCK_DGRAM, 0);
  if ( INVALID_SOCKET == s)
    return GetLastError();

  /* Set the TOS */
  iRet = 0x10; /* Minimise delay */
  setsockopt(
    s,
    IPPROTO_IP, IP_TOS,
    (char*)&iRet, sizeof(iRet)
  );

#ifdef IP_DONTFRAGMENT
  /* Don't fragment */
  iRet = 1;
  setsockopt(
    s,
    IPPROTO_IP, IP_DONTFRAGMENT,
    (char*)&iRet, sizeof(iRet)
  );
#endif

  /* Set a send timeout */
  { DECL_TIMEOUT( tv, NTP_SEND_TIMEOUT);
  setsockopt(
    s,
    SOL_SOCKET, SO_SNDTIMEO,
    (char*)&tv, sizeof tv
  ); }
  timeout = NTP_RECV_TIMEOUT; /* initial receive timeout */

  iRet = ETIMEDOUT; /* Assume retries exceeded */
  for ( iTry = 0; iTry <= NTP_RETRIES; ++iTry )
    {
    unsigned char buf[ SWS_NTP_HDR_LEN];
    size_t count;
    SWS_SNtpTime tOrig;

    /* Format NTP request */
#if 0
    SWS_POKE8( buf, SWS_NTP_HDR_MODE, SWS_NTP_VN_3 | SWS_NTP_MODE_CLIENT);
#else
    SWS_POKE8( buf, SWS_NTP_HDR_MODE, SWS_NTP_VN_1);
#endif
    SWS_POKE8( buf, SWS_NTP_HDR_STRATUM, SWS_NTP_STRATUM_NOME);
    SWS_POKE8( buf, SWS_NTP_HDR_POLL, NTP_MINPOLL);
    SWS_POKE8( buf, SWS_NTP_HDR_PRECISION, 0);
    SWS_POKE32( buf, SWS_NTP_HDR_DISTANCE, 0);
    SWS_POKE32( buf, SWS_NTP_HDR_DRIFT, 0);
    SWS_POKE32( buf, SWS_NTP_HDR_REFID, 0);
    SWS_POKE32( buf, SWS_NTP_HDR_REFTIME, 0);
    SWS_POKE32( buf, SWS_NTP_HDR_REFTIME + 4, 0);
    SWS_POKE32( buf, SWS_NTP_HDR_RECVTIME, 0);
    SWS_POKE32( buf, SWS_NTP_HDR_RECVTIME + 4, 0);
    SWS_POKE32( buf, SWS_NTP_HDR_SENDTIME, 0);
    SWS_POKE32( buf, SWS_NTP_HDR_SENDTIME + 4, 0);

    PlatformGetTime( &tOrig);
    SWS_POKE32( buf, SWS_NTP_HDR_ORIGTIME, tOrig.secs);
    SWS_POKE32( buf, SWS_NTP_HDR_ORIGTIME + 4, tOrig.fract);

    /* Send the request */
    if ( SOCKET_ERROR == sendto( s, (const char*)buf, SWS_NTP_HDR_LEN, 0, pSock, size))
      {
      iRet = GetLastError();
      if ( ENETUNREACH == iRet
        || EHOSTUNREACH == iRet
      ) {
        /* ICMP net/host unreachable */
        break;
        }
      continue;
      }

    /* Set recv timeout */
    { DECL_TIMEOUT( tv, timeout);
    setsockopt(
      s,
      SOL_SOCKET, SO_RCVTIMEO,
      (char*)&tv, sizeof tv
    ); }

    /* Get the reply */
    count = recv( s, (char*)buf, sizeof( buf), 0);
    PlatformGetTime( &pRec->tSys);
    if ( SOCKET_ERROR != (int)count
      && count >= SWS_NTP_HDR_LEN
    ) {
      unsigned mode;
      SWS_SNtpTime tTrip, tSend, tDelta1, tDelta2;

      SWS_PEEK8( mode, buf, SWS_NTP_HDR_MODE);
      pRec->uVersion = (mode & SWS_NTP_VN_MASK) >> SWS_NTP_VN_SHIFT;
      pRec->uLeap = (mode & SWS_NTP_LI_MASK) >> SWS_NTP_LI_SHIFT;
      SWS_PEEK8( pRec->uStratum, buf, SWS_NTP_HDR_STRATUM);
      pRec->iPrecision = *(char*)(buf + SWS_NTP_HDR_PRECISION);
      SWS_PEEK32( pRec->uError, buf, SWS_NTP_HDR_DISTANCE);
      SWS_PEEK32( pRec->iDrift, buf, SWS_NTP_HDR_DRIFT);
      pRec->addrRef = *(ui32*)(buf + SWS_NTP_HDR_REFID);

      /* Calc round trip time (RTT) */
      SWS_PEEK32( tTrip.secs, buf, SWS_NTP_HDR_RECVTIME);
      SWS_PEEK32( tTrip.fract, buf, SWS_NTP_HDR_RECVTIME + 4);
      SWS_PEEK32( tSend.secs, buf, SWS_NTP_HDR_SENDTIME);
      SWS_PEEK32( tSend.fract, buf, SWS_NTP_HDR_SENDTIME + 4);
      TimeSub( &tDelta1, &tSend, &tTrip);
      TimeSub( &tDelta2, &pRec->tSys, &tOrig);
      TimeSub( &tTrip, &tDelta2, &tDelta1);
      pRec->tRtt = tTrip;

      /* Time of flight is approx RTT/2 */
      tTrip.fract /= 2;
      if ( tTrip.secs % 2)
        tTrip.fract += HALF_SECOND;
      tTrip.secs /= 2;

      /* Calc. current time from time sent plus time of flight */
      TimeAdd( &pRec->tNet, &tSend, &tTrip);

      SWS_PEEK32( pRec->tSet.secs, buf, SWS_NTP_HDR_REFTIME);
      SWS_PEEK32( pRec->tSet.fract, buf, SWS_NTP_HDR_REFTIME + 4);

      iRet = 0;
      break;
      }

    iRet = GetLastError();
    if ( ETIMEDOUT == iRet || EWOULDBLOCK == iRet || EAGAIN == iRet)
      {
      timeout *= 2; /* Exponential backoff */
      }
    else if ( ECONNRESET == iRet)
      {
      /* ICMP port unreachable */
      break;
      }
    }

  closesocket( s);

  return iRet;
  }


/*
 * Subtract two times
 */
void TimeSub(
  SWS_SNtpTime* pT1,
  const SWS_SNtpTime* pT2,
  const SWS_SNtpTime* pT3
) {
  if ( pT2->secs >= pT3->secs)
    {
    pT1->secs = pT2->secs - pT3->secs;

    if ( pT2->fract >= pT3->fract)
      {
      pT1->fract = pT2->fract - pT3->fract;
      }
    else if ( pT1->secs > 0)
      {
      --(pT1->secs);
      pT1->fract = (0xffffffffL - (pT3->fract - pT2->fract)) + 1;
      }
    else
      {
      /* Underflow */
      pT1->fract = 0;
      }
    }
  else
    {
    /* Underflow */
    pT1->secs = 0;
    pT1->fract = 0;
    }
  }


/*
 * Add two times
 */
void TimeAdd(
  SWS_SNtpTime* pT1,
  const SWS_SNtpTime* pT2,
  const SWS_SNtpTime* pT3
) {
  pT1->secs = pT2->secs + pT3->secs;
  pT1->fract = pT2->fract + pT3->fract;
  if ( pT1->fract < pT2->fract)
    ++(pT1->secs);
  }


/****************************************************************************
 * Platform specific code
 ****************************************************************************/

#if defined WIN32 || defined _WIN32 || defined __WIN32__
/****************************************************************************
 * Win32
 ****************************************************************************/

#undef GetLastError
#undef Sleep

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

/*
 * Get calendar time (GMT)
 */
SWS_SNtpTime* PlatformGetTime( SWS_SNtpTime* pTime)
  {
  register unsigned ms;
  register ui32 divisor;
  SYSTEMTIME syst;
  struct tm gmt;

  GetLocalTime( &syst);

  /* Convert SYSTEMTIME to time_t */
  gmt.tm_year = syst.wYear - 1900;
  gmt.tm_mon = syst.wMonth - 1;
  gmt.tm_mday = syst.wDay;
  gmt.tm_hour = syst.wHour;
  gmt.tm_min = syst.wMinute;
  gmt.tm_sec = syst.wSecond;
  gmt.tm_wday = 0;
  gmt.tm_yday = 0;
  gmt.tm_isdst = -1;
  /* Convert localtime to seconds UTC */
  pTime->secs = (ui32)mktime( &gmt);
  ms = syst.wMilliseconds;

  /* Shift reference from 1-Jan-1970 to 1-Jan-1900 */
  pTime->secs += SECS_1900_TO_1970;

  /* Convert millisecs to 32-bit fractional seconds */
  pTime->fract = 0;
  divisor = HALF_SECOND;
  do
    {
    ms <<= 1;
    if ( ms >= 1000)
      {
      ms -= 1000;
      pTime->fract += divisor;
      }

    divisor >>= 1;
    }
  while ( divisor && ms);

  return pTime;
  }

#elif defined _POSIX_SOURCE || defined _POSIX_C_SOURCE || defined __DJGPP__
/****************************************************************************
 * POSIX
 ****************************************************************************/
#undef timeval
#include <sys/time.h>

/*
 * Get calendar time (GMT)
 */
SWS_SNtpTime* PlatformGetTime( SWS_SNtpTime* pTime)
  {
  unsigned ms;
  ui32 divisor;
  struct timeval tv;

#ifdef __DJGPP__
  /* DJGPP2.03 sometimes returns invalid ftime (interrupts?)
   * Repeat until 2 requests compare equal
   */
  {
  struct timeval tv1;
  do
    {
    gettimeofday( &tv1, NULL);
    gettimeofday( &tv, NULL);
    }
  while ( tv.tv_sec != tv1.tv_sec);
  }
#else
  gettimeofday( &tv, NULL);
#endif

  /* Shift reference from midnight GMT 1-Jan-1970 to 1-Jan-1900 */
  pTime->secs = (ui32)tv.tv_sec + SECS_1900_TO_1970;
  pTime->fract = 0;

  /* Convert microsecs to 32-bit fractional seconds */
  ms = tv.tv_usec;
  divisor = HALF_SECOND;
  do
    {
    ms <<= 1;
    if ( ms >= 1000000UL)
      {
      ms -= 1000000UL;
      pTime->fract += divisor;
      }

    divisor >>= 1;
    }
  while ( divisor && ms);

  return pTime;
  }

#else
/****************************************************************************
 * ANSI C
 ****************************************************************************/

/*
 * Get calendar time (GMT)
 */
SWS_SNtpTime* PlatformGetTime( SWS_SNtpTime* pTime)
  {
  register unsigned ms;
  register ui32 divisor;
  time_t t;

  t = time( NULL); /* seconds since midnight (00:00:00), January 1, 1970 UTC */
  pTime->secs = (u_long)t;
  ms = (unsigned)(1000 * (t - pTime->secs));

  /* Shift reference from 1-Jan-1970 to 1-Jan-1900 */
  pTime->secs += SECS_1900_TO_1970;

  /* Convert millisecs to 32-bit fractional seconds */
  pTime->fract = 0;
  divisor = HALF_SECOND;
  do
    {
    ms <<= 1;
    if ( ms >= 1000)
      {
      ms -= 1000;
      pTime->fract += divisor;
      }

    divisor >>= 1;
    }
  while ( divisor && ms);

  return pTime;
  }

#endif

/* End of file */
