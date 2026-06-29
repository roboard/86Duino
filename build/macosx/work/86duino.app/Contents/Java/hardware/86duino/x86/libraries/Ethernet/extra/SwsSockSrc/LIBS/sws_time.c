/****************************************************************************
 *
 * File name   : sws_time.c
 * Function    : Time/date ops
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 12-Mar-10, LVR, v1.00 - Created.
 *
 * Comments:
 * --------
 * This file implements general time/date ops
 *
 ****************************************************************************/

/* Compilation options */

/* Exports */
#include "sws_time.h"

/* Imports */
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>

#include "sws_host.h" /* Basic types */
#include "sws_str.h" /* SWS_stricmp */


/***************************************************************************
 * Macros & constants
 ***************************************************************************/

/* Length of an array - sorely missing from stddef.h */
#ifndef lengthof
#define lengthof( _a) ((size_t)( sizeof(_a)/sizeof((_a)[0]) ))
#endif

#define INVALID_TIME ((time_t)-1)


/**
 * Convert a MIME (RFC 822) format date to a UTC time_t
 */
SWS_EXTERN( time_t, SWS_TimeFromStr(
  SWS_PTR( const char) pszValue
)){
  struct tm date;
  int i;
  char szMonth[4];
  char szZone[6] = "";
  static const char* const s_months[] = {
    "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
  };
  static int s_iTz, s_isDst = -1;

  assert( NULL != pszValue);

  /* Try RFC 822/1123 and 850/1036 format */
  i = sscanf(
    pszValue,
    " %*s %d%*c%3s%*c%d %d:%d:%d %5s",
    &date.tm_mday,
    szMonth,
    &date.tm_year,
    &date.tm_hour,
    &date.tm_min,
    &date.tm_sec,
    szZone
  );
  if ( i < 6)
    {
    /* Maybe missing day of week */
    i = sscanf(
      pszValue,
      " %d%*c%3s%*c%d %d:%d:%d %5s",
      &date.tm_mday,
      szMonth,
      &date.tm_year,
      &date.tm_hour,
      &date.tm_min,
      &date.tm_sec,
      szZone
    );
    if ( i < 6)
      {
      /* ANSI C format */
      i = sscanf(
        pszValue,
        " %*s %3s %d %d:%d:%d %d",
        szMonth,
        &date.tm_mday,
        &date.tm_hour,
        &date.tm_min,
        &date.tm_sec,
        &date.tm_year
      );
      if ( i < 6)
        return INVALID_TIME;
      }
    }

  if ( date.tm_year < 50)
    date.tm_year += 100; /* 2000..2049 */
  else if ( date.tm_year >= 1900)
    date.tm_year -= 1900;
  else if ( date.tm_year >= 100) /* 50..99 => 1950..1999 */
    return INVALID_TIME;

  /* Convert month name to number */
  for (
    date.tm_mon = 0;
    date.tm_mon < (int)lengthof( s_months);
    ++date.tm_mon
  ) {
    if ( !SWS_stricmp( szMonth, s_months[ date.tm_mon]) )
      break;
    }
  if ( date.tm_mon >= (int)lengthof( s_months) )
    return INVALID_TIME;

  if ( -1 == s_isDst)
    {
    /* Calc current timezone */
    const time_t t = time( NULL);
    const struct tm gmt = *gmtime( &t);
    const struct tm lt = *localtime( &t);
    int iDiff = lt.tm_hour - gmt.tm_hour;
    if ( iDiff <= -12)
      iDiff += 24;
    else if ( iDiff > 12)
      iDiff -= 24;
    iDiff *= 100;
    iDiff += lt.tm_min - gmt.tm_min;
    s_iTz = iDiff; /* hhmm ahead of UTC */
    s_isDst = lt.tm_isdst;
    }
  date.tm_isdst = s_isDst;
  date.tm_wday = 0;
  date.tm_yday = 0;

  /* Calc hhmm ahead of localtime */
  if ( 1 == sscanf( szZone, "%d", &i))
    {
    i -= s_iTz;
    }
  else
    {
    /* Try obsolete timezone acronyms */
    static const struct
      {
      const char* pszTz;
      int iTz;
      }
    zoneArray[] = {
      { "GMT",    0},
      { "UT",     0},
      { "EDT", -400},
      { "EST", -500},
      { "CDT", -500},
      { "CST", -600},
      { "MDT", -600},
      { "MST", -700},
      { "PDT", -700},
      { "PST", -800}
    };
    size_t j;

    i = 0; /* Assume local time */
    szZone[ 3] = '\0';

    /* Lookup timezone */
    for ( j = 0; j < lengthof( zoneArray); ++j)
      {
      if ( 0 == SWS_stricmp( szZone, zoneArray[ j].pszTz) )
        {
        i = zoneArray[ j].iTz - s_iTz;
        break;
        }
      }
    }

  /* Convert time to local time */
  if ( i < 0)
    {
    i = -i;
    date.tm_hour += i / 100;
    date.tm_min += i % 100;
    }
  else if ( i)
    {
    date.tm_hour -= i / 100;
    date.tm_min -= i % 100;
    }

  /* Convert local time to UTC */
  return mktime( &date);
  }


/**
 * Convert a UTC time_t to MIME (RFC 822) format
 */
SWS_EXTERN( SWS_PTR( char), SWS_TimeToStr( /* Returns the string */
  SWS_PTR( char) pszBuf,              /* OUT date as string >= 30 bytes */
  SWS_size_t size,                    /* IN: buffer size */
  time_t t                            /* IN: UTC time */
)){
  const struct tm local = *localtime( &t);
#define kszHttpTime "%a, %d %b %Y %H:%M:%S"

#if __STDC_VERSION__ >= 199901L
  strftime( pszBuf, size, kszHttpTime " %z", &local);
#else
  const struct tm gmt = *gmtime( &t);
  size_t len = strftime( pszBuf, size - 6, kszHttpTime, &local);
  int i = local.tm_hour - gmt.tm_hour;
  if ( i <= -12)
    i += 24;
  else if ( i > 12)
    i -= 24;
  i *= 100;
  if ( local.tm_min >= gmt.tm_min)
    i += local.tm_min - gmt.tm_min;
  else
    i += (60 + local.tm_min) - gmt.tm_min - 100;
  sprintf( pszBuf + len, " %+05d", i);
#endif

  return pszBuf;
  }

/* End of file */
