/****************************************************************************
 *
 * File name   : sws_str.c
 * Function    : String ops
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 12-Mar-10, LVR, v1.00 - Created.
 * 07-Apr-10, LVR, v1.00 - Added SWS_stristr
 *
 * Comments:
 * --------
 * This file implements general string ops
 *
 ****************************************************************************/

/* Compilation options */

/* Exports */
#include "sws_str.h"

/* Imports */
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "sws_host.h" /* Basic types */


/**
 * Wild-card name matching
 * Much smaller than POSIX regcomp
 */
SWS_EXTERN( int, SWS_StrMatch(         /* Returns 1 if matched */
  SWS_PTR( const char) pszPattern,     /* IN: Pattern may contain * and ? */
  SWS_PTR( const char) pszName,        /* IN: string to test */
  int ignoreCase                       /* IN: !0= case insensitive */
)){
  enum { kStar = '*', kQuery = '?', kLit = '\\', kPeriod = '.' };

  assert( NULL != pszPattern);
  assert( NULL != pszName);

  for ( ;;)
    {
    char c1, c2;

    while ( (c1 = *(pszPattern++)) == (c2 = *(pszName++)) )
      {
      if ( '\0' == c1)
        return 1; /* Matched */

      /* Don't match the wildcards */
      if ( kStar == c1 || kQuery == c1 || kLit == c1)
        break;
      }

    /* case insensitive */
    if ( ignoreCase && tolower( c1) == tolower( c2))
      continue;

    if ( kLit == c1)
      {
      /* Literalise next pattern */
      if ( c2 != *(pszPattern++))
        break; /* Mismatched */

      continue;
      }
    else if ( kQuery == c1)
      {
      if ( '\0' == c2)
        break; /* Mismatched */

#if 1 /* '?' doesn't match a period */
      if ( kPeriod == c2)
        break; /* Mismatched */
#endif
      continue;
      }
    else if ( kStar != c1)
      break; /* Mismatched */

    /* Match text following '*' */
    if ( SWS_StrMatch( pszPattern, pszName - 1, ignoreCase))
      return 1; /* Matched */

    if ( '\0' == c2)
      break; /* Mismatched */

#if 0 /* '*' doesn't match a period */
    if ( kPeriod == c2)
      break; /* Mismatched */
#endif
    --pszPattern;
    }

  return 0; /* Mismatched */
  }


/**
 * Case insensitive string compare
 */
SWS_EXTERN( int, SWS_stricmp(
  SWS_PTR( const char) psz1,
  SWS_PTR( const char) psz2
)){
  char c1, c2;

  assert( NULL != psz1);
  assert( NULL != psz2);

  do
    {
    c1 = *(psz1++);
    c2 = *(psz2++);
    }
  while ( '\0' != c1 && '\0' != c2 && tolower( c1) == tolower( c2) );

  return c1 - c2;
  }


/**
 * Case insensitive string compare
 */
SWS_EXTERN( int, SWS_strnicmp(
  SWS_PTR( const char) psz1,
  SWS_PTR( const char) psz2,
  SWS_size_t len
)){
  char c1, c2;

  assert( NULL != psz1);
  assert( NULL != psz2);

  do
    {
    if ( 0 == len--)
      return 0;
    c1 = *(psz1++);
    c2 = *(psz2++);
    }
  while ( '\0' != c1 && '\0' != c2 && tolower( c1) == tolower( c2) );

  return c1 - c2;
  }


/**
 * Case insensitive strstr
 * Could improve this with the Knuth–Morris–Pratt algorithm
 * http://en.wikipedia.org/wiki/Knuth-Morris-Pratt_algorithm
 */
SWS_EXTERN( SWS_PTR( const char), SWS_stristr(
  SWS_PTR( const char) pszHay,
  SWS_PTR( const char) pszNeedle
)){
  size_t nHay, nNeedle;

  assert( NULL != pszHay);
  assert( NULL != pszNeedle);

  nNeedle = strlen( pszNeedle);
  if ( 0 == nNeedle)
    return pszHay;

  for ( nHay = strlen( pszHay); nHay >= nNeedle; ++pszHay, --nHay)
    {
    if ( 0 == SWS_strnicmp( pszHay, pszNeedle, nNeedle))
      return pszHay;
    }

  return NULL;
  }

/* End of file */
