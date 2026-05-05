/****************************************************************************
 *
 * File name   : platform.h
 * Function    : Platform specific definitions.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 * Release Date:
 *
 ****************************************************************************
 * File History
 * ------------
 * 03-Apr-01, LVR, v1.00 - Created.
 * 03-Oct-01, LVR, v1.01 Fix TRACE warnings for ARM SDT 2.51 .
 * 12-Jan-03, LVR, v1.02 Added SWS_VARRAY.
 * 29-Jan-03, LVR, v1.03 Removed PlatformPropertySetString & Uint.
 * 19-Mar-05, LVR, v1.04 Revised.SWS_FASTCALLDEF for gcc 3.4.3
 * 30-Jul-05, LVR, v1.05 Added WAIT_FOREVER and WAIT_NEVER
 *
 ****************************************************************************/

#ifndef SWS_PLATFORM_H
#define SWS_PLATFORM_H 0x0105 /* Version no. BCD */

/* Dependencies */
#include <stdlib.h> /* malloc, free etc */
#if defined _MSC_VER && _MSC_VER >= 1000
#include <malloc.h> /* _alloca */
#include <crtdbg.h> /* _dgb_malloc etc */
#endif

#include "sws_host.h" /* Basic types */


/*
 * Macros
 */

/* Length of an array - sorely missing from stddef.h */
#ifndef lengthof
#define lengthof( _a) ((size_t)( sizeof(_a)/sizeof((_a)[0]) ))
#endif

/* Get a ptr to the start of a struct given a ptr to a field within it */
#define SWS_CONTAINER( ptr, type, field) \
  ((type*)( (char*)(ptr) - offsetof( type, field) ))

/* Variable length array */
#if __STDC_VERSION__ >= 199901L
# define SWS_VARRAY( _t, _n) _t _n[] /* C99 variable length array */
#elif defined _MSC_VER
# define SWS_VARRAY( _t, _n) _t _n[ 0]
# pragma warning ( disable : 4200)    /* nonstandard extension used : zero-sized array in struct/union */
#elif defined __GNUC__ && !defined  __STRICT_ANSI__
# define SWS_VARRAY( _t, _n) _t _n[ 0]
#else
# define SWS_VARRAY( _t, _n) _t _n[ 1]
#endif

/* Roundup a value */
#ifndef ROUNDUP
#define ROUNDUP( _x,_z) ((size_t)((_x) + ((-(ptrdiff_t)(_x)) & ((_z) -1)) ))
#endif

/* Swap endianess */
#define SWAP16( _x) (ui16)( ((((unsigned)(_x))>>8)&0xff) | ((((unsigned)(_x))<<8)&0xff00) )
#define SWAP32( _x) (ui32)( (((ui32)SWAP16((unsigned)_x))<<16) | SWAP16((unsigned)((_x)>>16)) )

/* Read/write in network (MSB first, or big-endian) byte order */
#define SWS_POKE8(_p,_o,_v) (*(SWS_PTR(ui8))((SWS_PTR(char))(_p) + (_o)) = (_v))
#define SWS_POKE16(_p,_o,_v) NetWrite16( (SWS_PTR(ui16))((SWS_PTR(char))(_p) + (_o)), _v)
#define SWS_POKE32(_p,_o,_v) NetWrite32( (SWS_PTR(ui32))((SWS_PTR(char))(_p) + (_o)), _v)
#define SWS_POKECSUM(_p,_o,_v) (*(SWS_PTR(ui16))((SWS_PTR(char))(_p) + (_o)) = (ui16)(_v))
#define SWS_PEEK8(_v,_p,_o) _v = (*(SWS_PTR(const ui8))((SWS_PTR(const char))(_p) + (_o)))
#define SWS_PEEK16(_v,_p,_o) NetRead16( _v, ((SWS_PTR(const ui16))((SWS_PTR(const char))(_p) + (_o))) )
#define SWS_PEEK32(_v,_p,_o) NetRead32( _v, ((SWS_PTR(const ui32))((SWS_PTR(const char))(_p) + (_o))) )


/* Function call optimisations */
#ifndef INLINE
#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
# define INLINE(_ret,_f) inline _ret _f
#elif defined _MSC_VER
# define INLINE(_ret,_f) __inline _ret _f
#elif defined __GNUC__
# define INLINE(_ret,_f) __inline__ _ret _f
#elif defined __ARMCC_VERSION
# define INLINE(_ret,_f) __inline _ret _f
#elif defined __MWERKS__ || defined __WATCOMC__
# define INLINE(_ret,_f) __inline _ret _f
#else
# define INLINE(_ret,_f) _ret _f
#endif
#endif

#undef FORCEINLINE
#if defined _MSC_VER && _MSC_VER >= 1100
# define FORCEINLINE(_ret,_f) __forceinline _ret _f
#else
# define FORCEINLINE(_ret,_f) INLINE(_ret,_f)
#endif

#ifndef SWS_FASTCALLDEF
#if defined _MSC_VER && _MSC_VER >= 1000 /* MSVC4 & later */
 /* BUGBUG MSVC 1.52c (_MSC_VER == 800) sometimes compiles __fastcall wrongly */
# define SWS_FASTCALLDEF(_ret,_f) _ret __fastcall _f
# define SWS_FASTCALL(_ret,_f) _ret __fastcall _f
#elif defined __DJGPP__ /* gcc 3.4.3 */
# define SWS_FASTCALLDEF(_ret,_f) _ret __attribute__ (( __fastcall__ )) _f
# define SWS_FASTCALL(_ret,_f) _ret __attribute__ (( __fastcall__ )) _f
#elif defined __GNUC__ &&  defined __i386__
# define SWS_FASTCALLDEF(_ret,_f) _ret __attribute__ (( __regparm__ (3) )) _f
# define SWS_FASTCALL(_ret,_f) _ret __attribute__ (( __regparm__ (3) )) _f
#elif (defined __MWERKS__ && defined WIN32) || defined __WATCOMC__
# define SWS_FASTCALLDEF(_ret,_f) _ret __fastcall _f
# define SWS_FASTCALL(_ret,_f) _ret __fastcall _f
#else
# define SWS_FASTCALLDEF(_ret,_f) _ret _f
# define SWS_FASTCALL(_ret,_f) _ret _f
#endif
#endif


/*
 * Functions
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Platform specific initialisation & finalisation */
extern int PlatformInit( void);
extern int PlatformPostInit( void);
extern void PlatformPreCleanup( void);
extern void PlatformCleanup( void);

/* Persistent properties */
extern ui32 PlatformPropertyGetUint( const char*, const char*, ui32);
extern int PlatformPropertyGetString( const char*, const char*, const char*, char*, size_t);

/* Monotonic millisecond timer/counter */
extern ui32 PlatformTime( void);
extern void PlatformIdle( void);
#define PlatformSleep( ms) \
  do {ui32 _u=(ms)+PlatformTime();while((i32)PlatformTime() < (i32)_u) PlatformIdle();} while(0)

/* Timer callbacks.  Only 1 timer is used */
typedef SWS_CB( void, FPlatformTimeout( SWS_PTR( void) ));
extern int PlatformTimerStart( ui32, SWS_FNPTR( FPlatformTimeout), SWS_PTR( void) );
extern void PlatformTimerStop( void );

/* Execute a function in 'critical section' */
typedef SWS_CB( int, FPlatformCallback( SWS_PTR( void)));
extern int PlatformSynchronise( SWS_FNPTR( FPlatformCallback), SWS_PTR( void) );
extern int PlatformIsCritical( void);

/* Events */
typedef SWS_PTR( struct SEvent) EventH;
extern EventH PlatformCreateEvent( void);
extern void PlatformDestroyEvent( EventH);
extern void PlatformSetEvent( EventH, i32);
extern i32 PlatformGetEvent( EventH);
extern i32 PlatformWaitEvent( EventH, ui32); /* Returns event value */
extern void PlatformWaitAnyEvent( ui32);
#define WAIT_FOREVER 0
#define WAIT_NEVER 1
#define WAIT_MIN 2

/* Deferred procedure calls */
typedef SWS_CB( void, FPlatformDpc( SWS_PTR( void), SWS_PTR( void), ui32 ));
extern int PlatformDpc( SWS_FNPTR( FPlatformDpc), SWS_PTR( void), SWS_PTR( void), ui32);

/* Read/write in network (MSB first, or big-endian) byte order */
#if defined _M_IX86 || defined _M_I86 || defined __I86__ || defined __i386__
 /* x86 Little endian */
# define SWS_BIG_ENDIAN 0
# define NetWrite32(_p,_v) do { register const ui32 _u=(_v); *(SWS_PTR(ui32))(_p)=SWAP32(_u); } while(0)
# define NetWrite16(_p,_v) do { register const ui16 _u=(_v); *(SWS_PTR(ui16))(_p)=SWAP16(_u); } while(0)
# define NetRead32(_v,_p) do { register const ui32 _u=*(SWS_PTR(const ui32))(_p); _v=SWAP32(_u); } while(0)
# define NetRead16(_v,_p) do { register const ui16 _u=*(SWS_PTR(const ui16))(_p); _v=SWAP16(_u); } while(0)
# ifdef _MSC_VER
#  pragma warning ( disable : 4127)    /* conditional expression is constant */
# endif
#elif  defined __ARMCC_VERSION && !defined __BIG_ENDIAN
 /* ARM Little endian.  NB doesn't support misaligned transfers */
# define SWS_BIG_ENDIAN 0
# if 1 /* Use unaligned accesses */
#  define NetWrite32(_p,_v) do { register const ui32 _u=(_v); *(SWS_PTR(__packed ui32))(_p)=SWAP32(_u); } while(0)
#  define NetWrite16(_p,_v) do { register const ui16 _u=(_v); *(SWS_PTR(__packed ui16))(_p)=SWAP16(_u); } while(0)
#  define NetRead32(_v,_p) do { register const ui32 _u=*(SWS_PTR(__packed const ui32))(_p); _v=SWAP32(_u); } while(0)
#  define NetRead16(_v,_p) do { register const ui16 _u=*(SWS_PTR(__packed const ui16))(_p); _v=SWAP16(_u); } while(0)
# else
#  define NetWrite32(_p,_v) do { const ui32 _u=(_v);_u=SWAP32(_u); memcpy(_p,&_u,sizeof(_u)); } while(0)
#  define NetWrite16(_p,_v) do { const ui16 _u=(_v);_u=SWAP16(_u); memcpy(_p,&_u,sizeof(_u)); } while(0)
#  define NetRead32(_v,_p) do { ui32 _u; memcpy(&_u,_p,sizeof(_u)); _v=SWAP32(_u); } while(0)
#  define NetRead16(_v,_p) do { ui16 _u; memcpy(&_u,_p,sizeof(_u)); _v=SWAP32(_u); } while(0)
# endif
#elif defined __powerc || defined macintosh || defined __powerpc__ || defined __powerpc
 /* Big endian */
# define SWS_BIG_ENDIAN 1
# define NetWrite32(_p,_v) (*(SWS_PTR(ui32))(_p) = (_v))
# define NetWrite16(_p,_v) (*(SWS_PTR(ui16))(_p) = (_v))
# define NetRead32(_v,_p) (_v = *(SWS_PTR(const ui32))(_p))
# define NetRead16(_v,_p) (_v = *(SWS_PTR(const ui16))(_p))
#else
# error Endian-ness undefined
#endif

/* Atomic operators */
typedef struct Atomic Atomic_t;

#if OPT_ATOMIC_INTRINSIC
/* Use potentially unsafe intrinsic C operators */
struct Atomic { volatile int v; };
#define AtomicRead( _p) (_p)->v
#define AtomicWrite( _p, _v) ((_p)->v = (_v))
#define AtomicDecrement( _p) --((_p)->v)
#define AtomicIncrement( _p) ++((_p)->v)
#define AtomicAdd( _p, _v) (_p)->v += _v
extern SWS_FASTCALLDEF( int, AtomicXchg( Atomic_t*, int));
extern SWS_FASTCALLDEF( int, AtomicCmpXchg( Atomic_t*, int cmp, int up));

#elif defined _DOS16 /* VC1.5 & BC3.1 */
/* Functions coded in inline assembler */
struct Atomic { volatile int v; };
#define AtomicWrite( _p, _v) ((_p)->v = (_v))
#define AtomicRead( _p) (_p)->v
extern int AtomicDecrement( Atomic_t*); /* = --*p */
extern int AtomicIncrement( Atomic_t*); /* = ++*p */
extern int AtomicAdd( Atomic_t*, int); /* = *p += v */
extern int AtomicXchg( Atomic_t*, int);
extern int AtomicCmpXchg( Atomic_t*, int cmp, int up);

#else
/* Platform specific operators */
struct Atomic { volatile int v; };
#define AtomicWrite( _p, _v) ((_p)->v = (_v))
#define AtomicRead( _p) (_p)->v
extern SWS_FASTCALLDEF( int, AtomicDecrement( Atomic_t*)); /* = --*p */
extern SWS_FASTCALLDEF( int, AtomicIncrement( Atomic_t*)); /* = ++*p */
extern SWS_FASTCALLDEF( int, AtomicAdd( Atomic_t*, int)); /* = *p += v */
extern SWS_FASTCALLDEF( int, AtomicXchg( Atomic_t*, int));
extern SWS_FASTCALLDEF( int, AtomicCmpXchg( Atomic_t*, int cmp, int up));

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_PLATFORM_H */
/* End of file */
