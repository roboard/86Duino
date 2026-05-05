/****************************************************************************
 *
 * File name   : sws_ba64.c
 * Function    : Encode/decode base64 strings
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 11-Mar-10, LVR, v1.00 - Created.
 *
 * Comments:
 * --------
 * This file implements base64 encoding & decoding (RFC 4648 obsolete 3548).
 *
 ****************************************************************************/

/* Compilation options */

/* Exports */
#include "sws_ba64.h"

/* Imports */
#include <stddef.h>


/**
 * Decode a base64 string
 */
extern size_t SWS_Base64Decode(
  unsigned char pDst[],                 /* OUT: Buffer for decoded string */
  size_t size,                          /* IN: Buffer size */
  const char* pszSrc                    /* IN: Encoded string */
) {
  int iEnd = 0;
  unsigned char * const pStart = pDst;

  while ( size > 1 && !iEnd)
    {
    int i = 0;
    unsigned long lSum = 0;

    /* Get the next 24 bits */
    do
      {
      int iCode;

      switch ( *pszSrc++)
        {
      default:
        return 0;

      case '\r':
      case '\n':
      case '\t':
      case ' ':
        continue;

      case '\0':
      case '=': /* Padding at end */
      case '?':
        iEnd = 1;
        switch ( i)
          {
        case 18:
          *pDst++ = (unsigned char)((lSum >> 10) & 0xff);
          if ( --size > 0)
            *pDst++ = (unsigned char)((lSum >> 2) & 0xff);
          break;

        case 12:
          *pDst++ = (unsigned char)((lSum >> 4) & 0xff);
          break;

        case 6:
        case 0:
          break;
          }
        goto done;

      case 'A': iCode = 0; break;
      case 'B': iCode = 1; break;
      case 'C': iCode = 2; break;
      case 'D': iCode = 3; break;
      case 'E': iCode = 4; break;
      case 'F': iCode = 5; break;
      case 'G': iCode = 6; break;
      case 'H': iCode = 7; break;
      case 'I': iCode = 8; break;
      case 'J': iCode = 9; break;
      case 'K': iCode = 10; break;
      case 'L': iCode = 11; break;
      case 'M': iCode = 12; break;
      case 'N': iCode = 13; break;
      case 'O': iCode = 14; break;
      case 'P': iCode = 15; break;
      case 'Q': iCode = 16; break;
      case 'R': iCode = 17; break;
      case 'S': iCode = 18; break;
      case 'T': iCode = 19; break;
      case 'U': iCode = 20; break;
      case 'V': iCode = 21; break;
      case 'W': iCode = 22; break;
      case 'X': iCode = 23; break;
      case 'Y': iCode = 24; break;
      case 'Z': iCode = 25; break;
      case 'a': iCode = 26; break;
      case 'b': iCode = 26 + 1; break;
      case 'c': iCode = 26 + 2; break;
      case 'd': iCode = 26 + 3; break;
      case 'e': iCode = 26 + 4; break;
      case 'f': iCode = 26 + 5; break;
      case 'g': iCode = 26 + 6; break;
      case 'h': iCode = 26 + 7; break;
      case 'i': iCode = 26 + 8; break;
      case 'j': iCode = 26 + 9; break;
      case 'k': iCode = 26 + 10; break;
      case 'l': iCode = 26 + 11; break;
      case 'm': iCode = 26 + 12; break;
      case 'n': iCode = 26 + 13; break;
      case 'o': iCode = 26 + 14; break;
      case 'p': iCode = 26 + 15; break;
      case 'q': iCode = 26 + 16; break;
      case 'r': iCode = 26 + 17; break;
      case 's': iCode = 26 + 18; break;
      case 't': iCode = 26 + 19; break;
      case 'u': iCode = 26 + 20; break;
      case 'v': iCode = 26 + 21; break;
      case 'w': iCode = 26 + 22; break;
      case 'x': iCode = 26 + 23; break;
      case 'y': iCode = 26 + 24; break;
      case 'z': iCode = 26 + 25; break;
      case '0': iCode = 52; break;
      case '1': iCode = 52 + 1; break;
      case '2': iCode = 52 + 2; break;
      case '3': iCode = 52 + 3; break;
      case '4': iCode = 52 + 4; break;
      case '5': iCode = 52 + 5; break;
      case '6': iCode = 52 + 6; break;
      case '7': iCode = 52 + 7; break;
      case '8': iCode = 52 + 8; break;
      case '9': iCode = 52 + 9; break;
      /* NB '-' & '_' are from RFC 4648 URL and Filename Safe Alphabet */
      case '-': case '+': iCode = 62; break;
      case '_': case '/': iCode = 63; break;
        }

      lSum = (lSum << 6) + iCode;
      i += 6;
      }
    while ( i < 24);

    *pDst++ = (unsigned char)((lSum >> 16) & 0xff);
    if ( --size > 0)
      {
      *pDst++ = (unsigned char)((lSum >> 8) & 0xff);
      if ( --size > 0)
        *pDst++ = (unsigned char)(lSum & 0xff);
      }
    }

done:
  *pDst = '\0';

  return (size_t)(pDst - pStart);
  }


/**
 * Convert a buffer to base64
 */
extern void SWS_Base64Encode(
  char* szBuf, size_t size,
  const unsigned char* src, size_t len
) {
  enum { kBits = 6, kBase = 1 << kBits };
  static const char lut[ kBase + 1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz" "0123456789+/";

  while ( len && size >= 4)
    {
    unsigned long ulSum;

    /* Pack 3 bytes into 4 chars */
    ulSum = *src++;
    if ( --len)
      {
      ulSum <<= 8;
      ulSum |= *src++;
      if ( --len)
        {
        ulSum <<= 8;
        ulSum |= *src++;
        --len;

        szBuf[3] = lut[ (unsigned)(ulSum % kBase)];
        ulSum /= kBase;
        }
      else
        {
        ulSum <<= 3 * kBits - 16; /* Align 16 to 18 */
        szBuf[3] = '=';
        }

      szBuf[2] = lut[ (unsigned)(ulSum % kBase)];
      ulSum /= kBase;
      szBuf[1] = lut[ (unsigned)(ulSum % kBase)];
      ulSum /= kBase;
      }
    else
      {
      ulSum <<= 2 * kBits - 8; /* Align 8 to 12 */
      szBuf[3] = szBuf[2] = '=';
      szBuf[1] = lut[ (unsigned)(ulSum % kBase)];
      ulSum /= kBase;
      }

    szBuf[0] = lut[ (unsigned)(ulSum % kBase)];

    szBuf += 4;
    size -= 4;
    }

  if ( size)
    szBuf[ 0] = '\0';
  }

/* End of file */
