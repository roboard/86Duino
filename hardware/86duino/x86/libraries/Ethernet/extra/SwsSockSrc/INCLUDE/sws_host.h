/****************************************************************************
 *
 * File name   : sws_host.h
 * Function    : Public host dependent definitions.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 * Release Date:
 *
 ****************************************************************************
 * File History
 * ------------
 * 02-Apr-01, LVR, v1.00 - Created.
 * 03-Oct-01, LVR, v1.01 - Add SWS_PACKED for ARM SDT 2.51 .
 * 16-Dec-04, LVR, v1.02 - SWS_EXPORT depends on SWS_DLL, remove SWS_SOCK_LIB
 * 04-Jun-09, LVR, v1.03 - Generic shared library defs
 *
 *
 * Comments:
 * --------
 * This file contains public host dependent definitions
 *
 ****************************************************************************/

#ifndef SWS_HOST_H
#define SWS_HOST_H 0x0103 /* Version no. BCD */

/* Dependencies */
#include <stddef.h>

/* Macros */
#ifndef SWS_PTR
# if defined WIN16
# define SWS_PTR( _r) _r __far *
# elif defined _DOS16
#  define SWS_PTR( _r) _r __far *
# else
#  define SWS_PTR( _r) _r *
# endif
#endif

#ifndef SWS_FNPTR
# if defined WIN16
#  define SWS_FNPTR( _r) _r __far __pascal *
# elif defined _DOS16
#  if defined __WATCOMC__
#   define SWS_FNPTR( _r) _r *
#  else
#   define SWS_FNPTR( _r) _r __far *
#  endif
# else
#  define SWS_FNPTR( _r) _r *
# endif
#endif

#ifndef SWS_CB
# if defined WIN16
# define SWS_CB( _ret, _fn) _ret __far __pascal _fn
# elif defined _DOS16
#  if defined __WATCOMC__
#   define SWS_CB( _ret, _fn) _ret __far __cdecl _fn
#  else
#   define SWS_CB( _ret, _fn) _ret __far _fn
#  endif
# elif defined WIN32 || defined _WIN32 || defined __WIN32__
#  define SWS_CB( _ret, _fn) _ret __stdcall _fn
# elif defined __WATCOMC__
#  define SWS_CB( _ret, _fn) _ret __cdecl _fn
# else
#  define SWS_CB( _ret, _fn) _ret _fn
# endif
#endif

#ifndef SWS_LIB
# ifndef SWS_DLL
#  define SWS_LIB
# endif
#endif

#ifndef SWS_EXPORT
# ifdef SWS_LIB
  /* Static library */
#  if defined WIN32 || defined _WIN32 || defined __WIN32__ /* Win32 LIB */
#   define SWS_EXPORT( _ret, _fn) extern _ret __stdcall _fn
#   define SWS_EXTERN( _ret, _fn) SWS_EXPORT( _ret, _fn)
#   define SWS_EXTERNC( _ret, _fn) extern _ret _fn
#  elif defined WIN16 || defined _WINDOWS /* Win16 LIB */
#   define SWS_EXPORT( _ret, _fn) extern _ret __far __pascal _fn
#   define SWS_EXTERN( _ret, _fn) SWS_EXPORT( _ret, _fn)
#   define SWS_EXTERNC( _ret, _fn) extern _ret _fn
#  elif defined _DOS16 /* DOS16 LIB */
#   if defined __WATCOMC__
#    define SWS_EXPORT( _ret, _fn) extern _ret __far __cdecl _fn
#   else
#    define SWS_EXPORT( _ret, _fn) extern _ret __far _fn
#   endif
#   define SWS_EXTERN( _ret, _fn) SWS_EXPORT( _ret, _fn)
#   define SWS_EXTERNC( _ret, _fn) extern _ret _fn
#  elif defined __WATCOMC__
#   define SWS_EXPORT( _ret, _fn) extern _ret __cdecl _fn
#   define SWS_EXTERN( _ret, _fn) SWS_EXPORT( _ret, _fn)
#   define SWS_EXTERNC( _ret, _fn) extern _ret _fn
#  else
#   define SWS_EXPORT( _ret, _fn) extern _ret _fn
#   define SWS_EXTERN( _ret, _fn) SWS_EXPORT( _ret, _fn)
#   define SWS_EXTERNC( _ret, _fn) SWS_EXPORT( _ret, _fn)
#  endif

#  define SWS_EXPORTDATA( _type, _data) extern _type _data
#  define SWS_EXPORTDATADEF( _type, _data) _type _data

 /* Dynamic library */
# elif defined WIN32 || defined _WIN32 || defined __WIN32__ /* Win32 DLL */
#  if defined _USRDLL
#   define SWS_EXPORT( _ret, _fn) extern __declspec( dllexport) _ret __stdcall _fn
#   define SWS_EXTERN( _ret, _fn) extern __declspec( dllexport) _ret __stdcall _fn
#   define SWS_EXTERNC( _ret, _fn) extern __declspec( dllexport) _ret _fn
#   define SWS_EXPORTDATA( _type, _data) extern __declspec( dllexport) _type _data
#   if defined __GNUC__ && !defined __cplusplus
#    define SWS_EXPORTDATADEF( _type, _data) __declspec( dllexport) _type _data
#   else
#    define SWS_EXPORTDATADEF( _type, _data) extern __declspec( dllexport) _type _data
#   endif
#  else
#   define SWS_EXPORT( _ret, _fn) extern __declspec( dllimport) _ret __stdcall _fn
#   define SWS_EXTERN( _ret, _fn) extern _ret __stdcall _fn
#   define SWS_EXTERNC( _ret, _fn) extern _ret _fn
#   define SWS_EXPORTDATA( _type, _data) extern __declspec( dllimport) _type _data
#  endif
# elif defined WIN16 /* Win16 DLL */
#  if defined _WINDLL
#   define SWS_EXPORT( _ret, _fn) extern _ret __export __far __pascal _fn
#   define SWS_EXTERN( _ret, _fn) extern _ret __export __far __pascal _fn
#   define SWS_EXTERNC( _ret, _fn) extern _ret __export __far _fn
#   define SWS_EXPORTDATA( _type, _data) extern __export _type _data
#   define SWS_EXPORTDATADEF( _type, _data) _type _data
#  else
#   define SWS_EXPORT( _ret, _fn) extern _ret __far __pascal _fn
#   define SWS_EXTERN( _ret, _fn) extern _ret __far __pascal _fn
#   define SWS_EXTERNC( _ret, _fn) extern _ret __far _fn
#   define SWS_EXPORTDATA( _type, _data) extern _type _data
#  endif
# else
  /* Generic shared library */
# define SWS_EXPORT( _ret, _fn) extern _ret _fn
# define SWS_EXTERN( _ret, _fn) SWS_EXPORT( _ret, _fn)
# define SWS_EXTERNC( _ret, _fn) SWS_EXPORT( _ret, _fn)
# define SWS_EXPORTDATA( _type, _data) extern _type _data
# define SWS_EXPORTDATADEF( _type, _data) _type _data
# endif
#endif

#ifndef SWS_PACKED
# if defined __ARMCC_VERSION
#  define SWS_PACKED( _type) __packed _type
/* #elif defined __WATCOMC__
#  define SWS_PACKED( _type) _Packed _type*/
# else
#  define SWS_PACKED( _type) _type
# endif
#endif

/* Platform dependent types */
#ifndef SWS_TYPEDEFS
# define SWS_TYPEDEFS
/* BUGBUG: these really should have an SWS_ prefix */
typedef char             i8;
typedef unsigned char   ui8;
typedef short           i16;
typedef unsigned short ui16;
typedef long            i32;
typedef unsigned long  ui32;
typedef size_t SWS_size_t;
# define SWS_SIZE_MAX UINT_MAX
#endif

/* Make a 32-bit code from 4 characters */
#ifndef SWS_MAKEFOURCC
# define SWS_MAKEFOURCC( ch0, ch1, ch2, ch3) (\
  (ui32)(ui8)(ch0) | ((ui32)(ui8)(ch1) << 8) | \
  ((ui32)(ui8)(ch2) << 16) | ((ui32)(ui8)(ch3) << 24) )
#endif

#endif /* ndef SWS_HOST_H */
/* End of file */
