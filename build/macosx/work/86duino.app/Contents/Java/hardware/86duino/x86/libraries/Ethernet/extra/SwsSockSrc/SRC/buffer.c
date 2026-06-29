/****************************************************************************
 *
 * File name   : buffer.c
 * Function    : Buffer manager
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C.
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 06-Apr-01, LVR, Created.
 * 03-Oct-01, LVR, Fix TRACE warnings for ARM SDT 2.51 .
 * 12-Oct-04, LVR, Fixed re-initialisation.by initing all static data
 *
 * Comments:
 * --------
 * This file implements the buffer manager
 *
 ****************************************************************************
 */

/* Compilation options */
#include "config.h"
#ifndef OPT_BUFFER_CHECKS /* !0 to include extra buffer integrity checks */
# if defined DEBUG || defined _DEBUG
#  define OPT_BUFFER_CHECKS 1
# else
#  define OPT_BUFFER_CHECKS 0
# endif
#endif

/* Exports */
#include "sws_buf.h"

/* Imports */
#include "platform.h"
#include <stddef.h>
#include <stdlib.h>   /* for malloc */
#include <string.h>   /* for memset */
#include <limits.h>

#include "sws_sock.h"
#include "platform.h"

#include "debug.h"


/*
 * Macros & Constants
 */


/*
 * Local data types
 */
/* A buffer descriptor */
typedef struct SBuffer
  {
  ui32 uMagic;
#define BUFFER_MAGIC SWS_MAKEFOURCC( 'b','u','f','r')
  struct SBuffer* pNext;              /* Linked list */

  SWS_PTR( void) pvShadow;            /* -> shadowed buffer */
  SWS_size_t uStart, uEnd, uSize;

#if OPT_BUFFER_CHECKS
  ui32 uPrefix;
#define BUFFER_PREFIX SWS_MAKEFOURCC( 'b','u','f','p')
#define BUFFER_SUFFIX SWS_MAKEFOURCC( 'b','u','f','s')
#endif
  SWS_VARRAY( ui8, data);             /* VARIABLE LENGTH */
  } SBuffer;

/* A handle descriptor */
typedef struct SWS_Handle
  {
  SBuffer* pBuffer;                   /* NULL if size 0 */
  int iRefs;
  SWS_size_t uSize;
  } SWS_Handle;

/* BufferSyncGet context */
typedef struct SBufferInfo
  {
  SWS_size_t uLen;                    /* IN */
  SWS_EBufferMode eMode;              /* IN */
  SWS_PTR( void) pvShadow;            /* IN */
  SBuffer* pBuffer;                   /* OUT */
  } SBufferInfo;

/*
 * Prototypes
 */
static SBuffer* BufferGet( SWS_size_t, SWS_EBufferMode, SWS_PTR( void) );
#if OPT_BUFFER_CHECKS >= 2
static SWS_FASTCALLDEF( SWS_size_t, BufferSize( const SBuffer*));
#endif
static SWS_FASTCALLDEF( SWS_size_t, BufferRemoveBack( SBuffer*, SWS_size_t));
static SWS_FASTCALLDEF( void, BufferRelease( SBuffer*));
static SWS_CB( int, BufferGetHandle( SWS_PTR( void) ));
static SWS_CB( int, BufferGetSync( SWS_PTR( void) ));
static SWS_CB( int, BufferSyncRelease( SWS_PTR( void) ));


/*
 * Module data
 */
/* Configuration */
static size_t s_uLargeSize;
static size_t s_uLargeCount;
static size_t s_uSmallSize;
static size_t s_uSmallCount;
static size_t s_uHandleCount;

/* Free buffer list */
static SBuffer* s_pBuffers;        /* Buffer heap */
static SBuffer* s_pSmallBuffers;   /* Small buffer free list */
static SBuffer* s_pLargeBuffers;   /* Large buffer free list */

static Atomic_t s_aUsed;
static size_t s_sizeTotal;

/* Buffer handle array */
static SWS_Handle* s_handleArray;


/*
 * Initialise the buffer manager
 * Single threaded
 */
int SWS_BufferInit( void)             /* Returns !0 on error */
  {
  size_t u, sizeSmall, sizeLarge;
  ui32 bufferSize;
  SBuffer* pBuffer;

  TRACE( 9, ("SWS_BufferInit()\n"));
  ASSERT( NULL == s_pBuffers);
  ASSERT( NULL == s_pLargeBuffers);
  ASSERT( NULL == s_pSmallBuffers);
  ASSERT( NULL == s_handleArray);
  ASSERT( PlatformIsCritical());

  /* Init static data */
  s_pBuffers = NULL;
  s_pSmallBuffers = NULL;
  s_pLargeBuffers = NULL;
  AtomicWrite( &s_aUsed, 0);
  s_sizeTotal = 0;
  s_handleArray = NULL;

  s_uHandleCount = (size_t)PlatformPropertyGetUint(
    PKEY_BUFFER_HANDLES, OPT_BUFFER_HANDLES);
  s_uLargeSize = (size_t)PlatformPropertyGetUint(
    PKEY_BUFFER_LARGE_SIZE, OPT_BUFFER_LARGE_SIZE);
  s_uLargeCount = (size_t)PlatformPropertyGetUint(
    PKEY_BUFFER_LARGE_COUNT, OPT_BUFFER_LARGE_COUNT);
  s_uSmallSize = (size_t)PlatformPropertyGetUint(
    PKEY_BUFFER_SMALL_SIZE, OPT_BUFFER_SMALL_SIZE);
  s_uSmallCount = (size_t)PlatformPropertyGetUint(
    PKEY_BUFFER_SMALL_COUNT, OPT_BUFFER_SMALL_COUNT);
  ASSERT( s_uSmallSize < s_uLargeSize);

  sizeSmall = sizeof(SBuffer) + s_uSmallSize;
#if OPT_BUFFER_CHECKS
  sizeSmall += sizeof(ui32);
#endif
  sizeSmall = ROUNDUP( sizeSmall, 2 * sizeof(ui32));

  sizeLarge = sizeof(SBuffer) + s_uLargeSize;
#if OPT_BUFFER_CHECKS
  sizeLarge += sizeof(ui32);
#endif
  sizeLarge = ROUNDUP( sizeLarge, 2 * sizeof(ui32));

  /* Allocate buffer pool */
  bufferSize = (ui32)s_uLargeCount * sizeLarge + (ui32)s_uSmallCount * sizeSmall;
#if SWS_SIZE_MAX < ULONG_MAX
  if ( bufferSize > SWS_SIZE_MAX)
    {
    TRACE( 0, ("!!! SWS_BufferInit() buffers too big\n"));
    return SWS_ENOMEM;
    }
#endif

  s_pBuffers = (SBuffer*) malloc( (size_t)bufferSize);
  if ( NULL == s_pBuffers)
    {
    TRACE( 0, ("!!! SWS_BufferInit() out of memory\n"));
    return SWS_ENOMEM;
    }

  /* Allocate handle pool */
  s_handleArray = (SWS_Handle*) calloc( s_uHandleCount, sizeof(SWS_Handle) );
  if ( NULL == s_pBuffers)
    {
    TRACE( 0, ("!!! SWS_BufferInit() out of memory\n"));
    free ( s_pBuffers);
    s_pBuffers = NULL;
    s_uHandleCount = 0;
    return SWS_ENOMEM;
    }

  pBuffer = s_pBuffers;

  /* Create large buffer free list */
  for ( u = 0; u < s_uLargeCount; ++u)
    {
    pBuffer->uMagic = BUFFER_MAGIC;
    pBuffer->uStart = 0;
    pBuffer->uEnd = 0;
    pBuffer->uSize = s_uLargeSize;
#if OPT_BUFFER_CHECKS
    pBuffer->uPrefix = BUFFER_PREFIX;
    *(SWS_PACKED(ui32)*)(pBuffer->data + pBuffer->uSize) = BUFFER_SUFFIX;
#endif

    pBuffer->pNext = s_pLargeBuffers;
    s_pLargeBuffers = pBuffer;

    pBuffer = (SBuffer*)((char*)pBuffer + sizeLarge);
    }

  /* Create small buffer free list */
  for ( u = 0; u < s_uSmallCount; ++u)
    {
    pBuffer->uMagic = BUFFER_MAGIC;
    pBuffer->uStart = 0;
    pBuffer->uEnd = 0;
    pBuffer->uSize = s_uSmallSize;
#if OPT_BUFFER_CHECKS
    pBuffer->uPrefix = BUFFER_PREFIX;
    *(SWS_PACKED(ui32)*)(pBuffer->data + pBuffer->uSize) = BUFFER_SUFFIX;
#endif

    pBuffer->pNext = s_pSmallBuffers;
    s_pSmallBuffers = pBuffer;

    pBuffer = (SBuffer*)((char*)pBuffer + sizeSmall);
    }

  /* Init all handles */
  for ( u = 0; u < s_uHandleCount; ++u)
    {
    s_handleArray[ u].pBuffer = NULL;
    s_handleArray[ u].iRefs = 0;
    }

  AtomicWrite( &s_aUsed, 0);
  s_sizeTotal = (size_t)bufferSize;

  return 0;
  }


/*
 * Finalise the buffer manager
 * Single threaded
 */
void SWS_BufferCleanup( void)
  {
  TRACE( 9, ("SWS_BufferCleanup()\n"));
  ASSERT( PlatformIsCritical());

  s_pLargeBuffers = NULL;
  s_pSmallBuffers = NULL;

#ifndef NDEBUG
  /* Check for leaked handles */
  {
  size_t u;
  SWS_BufferH h;
  for ( h = &s_handleArray[ u = 0]; u < s_uHandleCount; ++u, ++h)
    {
    ASSERT( h->iRefs >= 0);
    if ( h->iRefs > 0)
      {
      TRACE( 1, ("*** SWS_BufferCleanup() Handle %u (%p) leaked\n", u, h));
      h->iRefs = 0;
      h->pBuffer = NULL;
      }
    }
  }
#endif

#if OPT_BUFFER_CHECKS
  /* Check buffers for under/overrun writes */
  {
  size_t u;
  SWS_size_t sizeSmall, sizeLarge;
  SBuffer* pBuffer = s_pBuffers;

  sizeSmall = sizeof(SBuffer) + s_uSmallSize + sizeof(ui32);
  sizeSmall = ROUNDUP( sizeSmall, 2 * sizeof(ui32));

  sizeLarge = sizeof(SBuffer) + s_uLargeSize + sizeof(ui32);
  sizeLarge = ROUNDUP( sizeLarge, 2 * sizeof(ui32));

  /* Check large buffer list */
  for ( u = 0; u < s_uLargeCount; ++u)
    {
    ASSERT( pBuffer->uMagic == BUFFER_MAGIC);
    ASSERT( pBuffer->uPrefix == BUFFER_PREFIX);
    ASSERT( *(SWS_PACKED(ui32)*)(pBuffer->data + pBuffer->uSize) == BUFFER_SUFFIX);
    pBuffer = (SBuffer*)((char*)pBuffer + sizeLarge);
    }

  /* Check small buffer list */
  for ( u = 0; u < s_uSmallCount; ++u)
    {
    ASSERT( pBuffer->uMagic == BUFFER_MAGIC);
    ASSERT( pBuffer->uPrefix == BUFFER_PREFIX);
    ASSERT( *(SWS_PACKED(ui32)*)(pBuffer->data + pBuffer->uSize) == BUFFER_SUFFIX);
    pBuffer = (SBuffer*)((char*)pBuffer + sizeSmall);
    }
  }
#endif

  if ( NULL != s_handleArray)
    {
    free( s_handleArray);
    s_handleArray = NULL;
    s_uHandleCount = 0;
    }
  if ( NULL != s_pBuffers)
    {
    free( s_pBuffers);
    s_pBuffers = NULL;
    }
  }


/*
 * Get max contiguous buffer size
 */
SWS_EXPORT( SWS_size_t, SWS_BufferMaxFrag( void))
  {
  TRACE( 9, ("SWS_BufferMaxFrag()\n"));
  return s_uLargeSize;
  }


/*
 * Get available buffer space
 */
SWS_EXPORT( SWS_size_t, SWS_BufferAvailable( void))
  {
  TRACE( 9, ("SWS_BufferAvailable()\n"));
  return s_sizeTotal - (SWS_size_t)AtomicRead( &s_aUsed);
  }


/*
 * Get total buffer space
 */
SWS_EXPORT( SWS_size_t, SWS_BufferTotal( void))
  {
  TRACE( 9, ("SWS_BufferTotal()\n"));
  return s_sizeTotal;
  }


/*
 * Allocate a handle & buffer
 */
SWS_EXPORT( SWS_BufferH, SWS_BufferGetShadow(  /* Returns new buffer or NULL if none */
  SWS_size_t uLen,                    /* IN: Buffer size */
  SWS_EBufferMode eMode,              /* IN: Packing mode */
  SWS_PTR( void) pvShadow             /* IN: -> Data to shadow */
)){
  SWS_BufferH h;
  unsigned u;

  TRACE( 9, ("SWS_BufferGetShadow(%u,%d,%p)\n", uLen,eMode,pvShadow));

  /* Allocate a handle */
  u = (unsigned)PlatformSynchronise( &BufferGetHandle, NULL);
  if ( (unsigned)-1 == u)
    return NULL;

  h = &s_handleArray[ u];
  ASSERT( 1 == h->iRefs);
  ASSERT( NULL == h->pBuffer);

  if ( uLen)
    {
    /* Get a buffer */
    h->pBuffer = BufferGet( uLen, eMode, pvShadow);
    if ( NULL == h->pBuffer)
      {
      h->iRefs = 0;
      return NULL;
      }
    }

  h->uSize = uLen;
  return h;
  }


/*
 * Allocate a handle
 */
static SWS_CB( int, BufferGetHandle(
  SWS_PTR( void) pv
)){
  static unsigned s_uLast;
  unsigned u;
  SWS_BufferH h;

  TRACE( 9, ("BufferGetHandle(%p)\n", pv));
  ASSERT( PlatformIsCritical());
  (void)pv;

  if ( NULL == s_handleArray)
    return -1;

  /* Find a free handle */
  u = s_uLast;
  ASSERT( u < s_uHandleCount);
  while ( s_handleArray[ u].iRefs > 0)
    {
    if ( ++u >= s_uHandleCount)
      u = 0;

    if ( s_uLast == u)
      {
      TRACE( 1, ("*** BufferGetHandle()\n Out of handles"));
      return -1;
      }
    }
  s_uLast = u;

  h = &s_handleArray[ u];
  ASSERT( NULL == h->pBuffer);
  ASSERT( 0 == h->iRefs);
  h->iRefs = 1;
  h->uSize = 0;

  return (int)u;
  }


/*
 * Allocate a buffer
 */
static SBuffer* BufferGet(            /* Returns new buffer or NULL if none */
  SWS_size_t uLen,                    /* IN: Buffer size */
  SWS_EBufferMode eMode,              /* IN: Packing mode */
  SWS_PTR( void) pvShadow
) {
  SBufferInfo info;

  TRACE( 9, ("BufferGet(%u,%d,%p)\n", uLen,eMode,pvShadow));

  info.uLen = uLen;
  info.eMode = eMode;
  info.pvShadow = pvShadow;
  return !PlatformSynchronise( &BufferGetSync, &info) ? info.pBuffer : NULL;
  }

static SWS_CB( int, BufferGetSync(    /* Returns new buffer or NULL if none */
  SWS_PTR( void) pv
)){
  SWS_PTR( SBufferInfo) const pInfo = (SWS_PTR( SBufferInfo))pv;
  SBuffer* pBuffer;
  SWS_size_t uLen;

  TRACE( 9, ("BufferGetSync(%p)\n", pv));
  ASSERT( NULL != pInfo);
  ASSERT( PlatformIsCritical());

  uLen = pInfo->uLen;

  /* Try the small buffer free list first */
  if ( NULL != (pBuffer = s_pSmallBuffers)
    && uLen <= pBuffer->uSize
  ) {
    ASSERT( BUFFER_MAGIC == pBuffer->uMagic);
    ASSERT( s_uSmallSize == pBuffer->uSize);

    /* Unsplice */
    s_pSmallBuffers = pBuffer->pNext;
    pBuffer->pNext = NULL;
    AtomicAdd( &s_aUsed, (int)pBuffer->uSize );
    }
  /* Try large buffer free list and then small buffer free list again */
  else if ( NULL != (pBuffer = s_pLargeBuffers)
    || NULL != (pBuffer = s_pSmallBuffers)
  ) {
    ASSERT( BUFFER_MAGIC == pBuffer->uMagic);

    /* Unsplice */
    if ( s_pLargeBuffers == pBuffer)
      {
      ASSERT( s_uLargeSize == pBuffer->uSize);
      s_pLargeBuffers = pBuffer->pNext;
      }
    else
      {
      ASSERT( s_uSmallSize == pBuffer->uSize);
      s_pSmallBuffers = pBuffer->pNext;
      }
    pBuffer->pNext = NULL;
    AtomicAdd( &s_aUsed, (int)pBuffer->uSize );

    if ( uLen > pBuffer->uSize)
      {
      /* Recurse and append more buffers */
      SWS_PTR( void) pvShadow = pInfo->pvShadow;
      if ( NULL != pvShadow)
        pvShadow = (SWS_PTR( char))pvShadow + pBuffer->uSize;

      pBuffer->pNext = BufferGet( uLen - pBuffer->uSize, SWS_kBufferSuffix, pvShadow);
      if ( NULL == pBuffer->pNext)
        {
        BufferRelease( pBuffer);
        return SWS_ENOBUFS;
        }

      uLen = pBuffer->uSize;
      pInfo->eMode = SWS_kBufferPrefix;
      }
    }
  else
    {
    TRACE( 1, ("*** BufferGetSync() out of buffers\n"));
    return SWS_ENOBUFS;
    }
  ASSERT( pBuffer->uSize >= uLen);
  pBuffer->pvShadow = pInfo->pvShadow;

  switch ( pInfo->eMode)
    {
  default:
    ASSERT( !"Invalid buffer mode");
    /* Fall thru */
  case SWS_kBufferDefault:
    /* Pack in middle of buffer */
    pBuffer->uStart = (pBuffer->uSize - uLen) / 2;
    pBuffer->uEnd = pBuffer->uStart + uLen;
    break;

  case SWS_kBufferPrefix:
    /* Pack at end of buffer to allow for headers */
    pBuffer->uStart = pBuffer->uSize - uLen;
    pBuffer->uEnd = pBuffer->uSize;
    break;

  case SWS_kBufferSuffix:
    /* Pack at start of buffer to allow for trailers */
    pBuffer->uStart = 0;
    pBuffer->uEnd = uLen;
    break;
    }

#if OPT_BUFFER_CHECKS >= 2
  memset( pBuffer->data, 0xde, pBuffer->uSize);
#endif

#if OPT_BUFFER_CHECKS
  ASSERT( pBuffer->uPrefix == BUFFER_PREFIX);
  ASSERT( *(SWS_PACKED(ui32)*)(pBuffer->data + pBuffer->uSize) == BUFFER_SUFFIX);
#endif

  pInfo->pBuffer = pBuffer;
  return 0;
  }


/*
 * Free a buffer handle
 */
SWS_EXPORT( int, SWS_BufferRelease(
  SWS_BufferH hBuffer                 /* IN: Buffer to release */
)){
  int iRefs;

  TRACE( 9, ("SWS_BufferRelease(%p)\n", hBuffer));
  ASSERT( hBuffer >= s_handleArray);
  ASSERT( hBuffer < s_handleArray + s_uHandleCount);
  ASSERT( 0 < hBuffer->iRefs);

  if ( 1 == hBuffer->iRefs)
    {
    BufferRelease( hBuffer->pBuffer);
    hBuffer->pBuffer = NULL;
    hBuffer->iRefs = 0;
    hBuffer->uSize = 0;
    iRefs = 0;
    }
  else
    {
    ASSERT( hBuffer->iRefs > 1);
    iRefs = --(hBuffer->iRefs);
    }

  return iRefs;
  }


/*
 * Release a shadowed buffer - shadowed data is copied if ref count > 1
 */
SWS_EXPORT( int, SWS_BufferReleaseShadow(
  SWS_BufferH hBuffer                 /* IN: Buffer to release */
)){
  int iRefs;

  TRACE( 9, ("SWS_BufferReleaseShadow(%p)\n", hBuffer));
  ASSERT( hBuffer >= s_handleArray);
  ASSERT( hBuffer < s_handleArray + s_uHandleCount);
  ASSERT( 0 < hBuffer->iRefs);

  if ( hBuffer->iRefs > 1)
    {
    /* Copy shadowed data to buffer */
    SBuffer* pBuffer =  hBuffer->pBuffer;
    while ( NULL != pBuffer)
      {
      if ( NULL != pBuffer->pvShadow)
        {
        /* Copy shadowed data to buffer */
        memcpy( pBuffer->data + pBuffer->uStart, pBuffer->pvShadow, pBuffer->uEnd - pBuffer->uStart);
        pBuffer->pvShadow = NULL;
        }

      pBuffer = pBuffer->pNext;
      }

    iRefs = --(hBuffer->iRefs);
    }
  else
    {
    ASSERT( 1 == hBuffer->iRefs);
    BufferRelease( hBuffer->pBuffer);
    hBuffer->pBuffer = NULL;
    hBuffer->iRefs = 0;
    hBuffer->uSize = 0;
    iRefs = 0;
    }

  return iRefs;
  }


/*
 * Free a buffer descriptor
 */
static SWS_FASTCALL( void, BufferRelease(
  SBuffer* pBuffer                    /* IN: Buffer to release */
)){
  TRACE( 9, ("BufferRelease(%p)\n", pBuffer));

  while ( NULL != pBuffer)
    {
    SBuffer* pNext;

    ASSERT( BUFFER_MAGIC == pBuffer->uMagic);
    ASSERT( pBuffer->uEnd <= pBuffer->uSize);
    ASSERT( pBuffer->uEnd >= pBuffer->uStart);
#if OPT_BUFFER_CHECKS
    ASSERT( pBuffer->uPrefix == BUFFER_PREFIX);
    ASSERT( *(SWS_PACKED(ui32)*)(pBuffer->data + pBuffer->uSize) == BUFFER_SUFFIX);
#endif

    pNext = pBuffer->pNext;
    pBuffer->pvShadow = NULL;

    /* Return to free list */
    PlatformSynchronise( &BufferSyncRelease, pBuffer);

    pBuffer = pNext;
    }
  }

static SWS_CB( int, BufferSyncRelease(
  SWS_PTR( void) pv
)){
  SBuffer* const pBuffer = (SBuffer*)pv;

  TRACE( 9, ("BufferSyncRelease(%p)\n", pv));
  ASSERT( NULL != pBuffer);
  ASSERT( PlatformIsCritical());

#if OPT_BUFFER_CHECKS
  ASSERT( pBuffer->uPrefix == BUFFER_PREFIX);
  ASSERT( *(SWS_PACKED(ui32)*)(pBuffer->data + pBuffer->uSize) == BUFFER_SUFFIX);
#endif

  if ( pBuffer->uSize < s_uLargeSize)
    {
    ASSERT( s_uSmallSize == pBuffer->uSize);
    pBuffer->pNext = s_pSmallBuffers;
    s_pSmallBuffers = pBuffer;
    }
  else
    {
    ASSERT( s_uLargeSize == pBuffer->uSize);
    pBuffer->pNext = s_pLargeBuffers;
    s_pLargeBuffers = pBuffer;
    }

  ASSERT( (SWS_size_t)AtomicRead( &s_aUsed) >= pBuffer->uSize );
  AtomicAdd( &s_aUsed, -(int)pBuffer->uSize );

#if OPT_BUFFER_CHECKS >= 2
  memset( pBuffer->data, 0xad, pBuffer->uSize);
  ASSERT( pBuffer->uPrefix == BUFFER_PREFIX);
  ASSERT( *(SWS_PACKED(ui32)*)(pBuffer->data + pBuffer->uSize) == BUFFER_SUFFIX);
#endif

  return 0;
  }


/*
 * Add a reference to a buffer
 */
SWS_EXPORT( void, SWS_BufferAddRef(
  SWS_BufferH hBuffer                 /* IN: Buffer */
)){
  TRACE( 9, ("SWS_BufferAddRef(%p)\n", hBuffer));
  ASSERT( hBuffer >= s_handleArray);
  ASSERT( hBuffer < s_handleArray + s_uHandleCount);
  ASSERT( 0 < hBuffer->iRefs);

  if ( hBuffer->iRefs > 0)
    ++(hBuffer->iRefs);
  }


/*
 * Get allocated buffer size
 */
SWS_EXPORT( SWS_size_t, SWS_BufferSize(
  SWS_BufferH hBuffer                 /* IN: Buffer */
)){
  TRACE( 9, ("SWS_BufferSize(%p)\n", hBuffer));
  ASSERT( hBuffer >= s_handleArray);
  ASSERT( hBuffer < s_handleArray + s_uHandleCount);
  ASSERT( 0 < hBuffer->iRefs);

#if OPT_BUFFER_CHECKS >= 2
  ASSERT( hBuffer->uSize == BufferSize( hBuffer->pBuffer));
#endif
  return hBuffer->uSize;
  }


#if OPT_BUFFER_CHECKS >= 2
/*
 * Get buffer size
 */
static SWS_FASTCALL( SWS_size_t, BufferSize(
  const SBuffer* pBuffer              /* IN: Buffer */
)){
  SWS_size_t uLen = 0;

  TRACE( 9, ("BufferSize(%p)\n", pBuffer));

  while ( NULL != pBuffer)
    {
    ASSERT( BUFFER_MAGIC == pBuffer->uMagic);
    ASSERT( s_uLargeSize == pBuffer->uSize || s_uSmallSize == pBuffer->uSize);
    ASSERT( pBuffer->uEnd <= pBuffer->uSize);
    ASSERT( pBuffer->uEnd >= pBuffer->uStart);
#if OPT_BUFFER_CHECKS
    ASSERT( pBuffer->uPrefix == BUFFER_PREFIX);
    ASSERT( *(SWS_PACKED(ui32)*)(pBuffer->data + pBuffer->uSize) == BUFFER_SUFFIX);
#endif

    uLen += pBuffer->uEnd - pBuffer->uStart;
    pBuffer = pBuffer->pNext;
    }

  return uLen;
  }
#endif


/*
 * Get a pointer to data at a given offset in the buffer
 * >>>> NB This routine is critical to performance <<<<
 */
SWS_EXPORT( SWS_PTR( void), SWS_BufferGetFragment(  /* Returns -> fragment, NULL if empty */
  SWS_BufferH hBuffer,                /* IN: Buffer */
  SWS_size_t uOffset,                 /* IN: Starting offset */
  SWS_PTR( SWS_size_t) puLen          /* OUT: Fragment length, 0 if exhausted */
)){
  SBuffer* pBuffer;

  TRACE( 9, ("SWS_BufferGetFragment(%p,%u,%p)\n", hBuffer,uOffset,puLen));
#if OPT_BUFFER_CHECKS >= 2
  ASSERT( hBuffer >= s_handleArray);
  ASSERT( hBuffer < s_handleArray + s_uHandleCount);
  ASSERT( 0 < hBuffer->iRefs);
  ASSERT( NULL != puLen);
#endif

  /* Loop over the list of buffers until the required offset is found */
  for (
    pBuffer = hBuffer->pBuffer;
    NULL != pBuffer;
    pBuffer = pBuffer->pNext
  ) {
    SWS_size_t uLen;

#if OPT_BUFFER_CHECKS >= 2
    ASSERT( BUFFER_MAGIC == pBuffer->uMagic);
    ASSERT( s_uLargeSize == pBuffer->uSize || s_uSmallSize == pBuffer->uSize);
    ASSERT( pBuffer->uEnd <= pBuffer->uSize);
    ASSERT( pBuffer->uEnd >= pBuffer->uStart);
    ASSERT( pBuffer->uPrefix == BUFFER_PREFIX);
    ASSERT( *(SWS_PACKED(ui32)*)(pBuffer->data + pBuffer->uSize) == BUFFER_SUFFIX);
#endif

    uLen = pBuffer->uEnd - pBuffer->uStart;

    /* Is the offset within this buffer? */
    if ( uOffset < uLen)
      {
      /* Yes, then return length of fragment */
      *puLen = uLen - uOffset;

      /* Checked for shadowing */
      if ( NULL != pBuffer->pvShadow)
        return (SWS_PTR(char))pBuffer->pvShadow + uOffset;

      /* Return -> buffer */
      return &pBuffer->data[ pBuffer->uStart + uOffset];
      }
    uOffset -= uLen;
    }

  TRACE( 1, ("*** SWS_BufferGetFragment() %u beyond end of buffer\n", uOffset));

  *puLen = 0;
  return NULL;
  }


/*
 * Buffer re-sizing
 */
SWS_EXPORT( int, SWS_BufferAppendFront( /* Returns !0 on failure */
  SWS_BufferH hBuffer,                /* IN: Buffer */
  SWS_size_t uLen                     /* IN: Bytes to append */
)){
  SBuffer* pBuffer;

  TRACE( 9, ("SWS_BufferAppendFront(%p,%u)\n", hBuffer,uLen));
  ASSERT( hBuffer >= s_handleArray);
  ASSERT( hBuffer < s_handleArray + s_uHandleCount);
  ASSERT( 0 < hBuffer->iRefs);

  pBuffer = hBuffer->pBuffer;
  if ( NULL == pBuffer)
    {
    hBuffer->pBuffer = BufferGet( uLen, SWS_kBufferPrefix, NULL);
    if ( NULL == hBuffer->pBuffer)
      return SWS_ENOBUFS;

    hBuffer->uSize += uLen;
    return 0;
    }

  if ( NULL == pBuffer->pvShadow)
    {
    /* Use buffer prefix if big enough */
    if ( pBuffer->uStart >= uLen)
      {
      pBuffer->uStart -= uLen;
      hBuffer->uSize += uLen;
      return 0;
      }
    }

  /* Allocate another buffer */
  pBuffer = BufferGet( uLen, SWS_kBufferPrefix, NULL);
  if ( NULL == pBuffer)
    return SWS_ENOBUFS;

  /* Attach new buffer to front */
  pBuffer->pNext = hBuffer->pBuffer;
  hBuffer->pBuffer = pBuffer;
  hBuffer->uSize += uLen;

  return 0;
  }


SWS_EXPORT( int, SWS_BufferRemoveFront(  /* Returns !0 on failure */
  SWS_BufferH hBuffer,                /* IN: Buffer */
  SWS_size_t uLen                     /* IN: Bytes to remove */
)){
  SBuffer* pBuffer;

  TRACE( 9, ("SWS_BufferRemoveFront(%p,%u)\n", hBuffer,uLen));
  ASSERT( hBuffer >= s_handleArray);
  ASSERT( hBuffer < s_handleArray + s_uHandleCount);
  ASSERT( 0 < hBuffer->iRefs);

  pBuffer = hBuffer->pBuffer;

  if ( uLen > hBuffer->uSize)
    return SWS_ENOBUFS;

  while ( NULL != pBuffer)
    {
    SBuffer* pNext;
    SWS_size_t const u = pBuffer->uEnd - pBuffer->uStart;

    if ( uLen < u)
      {
      pBuffer->uStart += uLen;
      hBuffer->uSize -= uLen;

      /* Adjust shadow ptr */
      if ( NULL != pBuffer->pvShadow)
        pBuffer->pvShadow = (SWS_PTR(char))pBuffer->pvShadow + uLen;

      return 0;
      }
    uLen -= u;
    hBuffer->uSize -= u;

    /* Release the 1st buffer */
    hBuffer->pBuffer = pNext = pBuffer->pNext;
    pBuffer->pNext = NULL;
    BufferRelease( pBuffer);

    pBuffer = pNext;
    }

  ASSERT( 0 == uLen);
  hBuffer->uSize = uLen;
  return 0;
  }


SWS_EXPORT( int, SWS_BufferAppendBackShadow(  /* Returns !0 on failure */
  SWS_BufferH hBuffer,                /* IN: Buffer */
  SWS_size_t uLen,                    /* IN: Bytes to append */
  SWS_PTR( void) pvShadow
)){
  SBuffer* pBuffer;

  TRACE( 9, ("SWS_BufferAppendBackShadow(%p,%u,%p)\n", hBuffer,uLen,pvShadow));
  ASSERT( hBuffer >= s_handleArray);
  ASSERT( hBuffer < s_handleArray + s_uHandleCount);
  ASSERT( 0 < hBuffer->iRefs);

  pBuffer = hBuffer->pBuffer;
  if ( NULL == pBuffer)
    {
    hBuffer->pBuffer = BufferGet( uLen, SWS_kBufferSuffix, pvShadow);
    if ( NULL == hBuffer->pBuffer)
      return SWS_ENOBUFS;

    hBuffer->uSize += uLen;
    return 0;
    }

  /* Find last buffer in chain */
  while ( NULL != pBuffer->pNext)
    pBuffer = pBuffer->pNext;

  if ( NULL == pBuffer->pvShadow && NULL == pvShadow)
    {
    /* Use space at end of buffer */
    SWS_size_t uSpace = pBuffer->uSize - pBuffer->uEnd;
    if ( uLen < uSpace)
      {
      /* It's sufficient */
      pBuffer->uEnd += uLen;
      hBuffer->uSize += uLen;
      return 0;
      }

    if ( uSpace)
      {
      /* Use remaining */
      uLen -= uSpace;
      pBuffer->uEnd = pBuffer->uSize;
      hBuffer->uSize += uSpace;
      }
    }

  /* Attach a new buffer */
  pBuffer->pNext = BufferGet( uLen, SWS_kBufferSuffix, pvShadow);
  if ( NULL == pBuffer->pNext)
    return SWS_ENOBUFS;

  hBuffer->uSize += uLen;
  return 0;
  }


SWS_EXPORT( int, SWS_BufferRemoveBack(  /* Returns !0 on failure */
  SWS_BufferH hBuffer,                /* IN: Buffer */
  SWS_size_t uLen                     /* IN: Bytes to drop */
)){
  SBuffer* pBuffer;

  TRACE( 9, ("SWS_BufferRemoveBack(%p,%u)\n", hBuffer,uLen));
  ASSERT( hBuffer >= s_handleArray);
  ASSERT( hBuffer < s_handleArray + s_uHandleCount);
  ASSERT( 0 < hBuffer->iRefs);
  ASSERT( NULL != hBuffer->pBuffer);

  pBuffer = hBuffer->pBuffer;
  if ( uLen > hBuffer->uSize)
    {
    TRACE( 1, ("***SWS_BufferRemoveBack() too big\n"));
    return SWS_EINVAL;
    }

  VERIFY( uLen == BufferRemoveBack( pBuffer, uLen));
  hBuffer->uSize -= uLen;
  return 0;
  }


static SWS_FASTCALL( SWS_size_t, BufferRemoveBack(
  SBuffer* pBuffer,                   /* IN: Buffer */
  SWS_size_t uLen                     /* IN: Bytes to drop */
)){
  SBuffer* pNext;
  SWS_size_t uRemoved, uSize;

  TRACE( 9, ("BufferRemoveBack(%p,%u)\n", pBuffer,uLen));
  ASSERT( NULL != pBuffer);

  pNext = pBuffer->pNext;
  if ( NULL != pNext)
    {
    /* Recurse until reach end of chain */
    uRemoved = BufferRemoveBack( pNext, uLen);

    if ( pNext->uEnd == pNext->uStart)
      {
      BufferRelease( pNext);
      pBuffer->pNext = NULL;
      }

    ASSERT( uLen >= uRemoved);
    uLen -= uRemoved;
    }
  else
    {
    uRemoved = 0;
    }

  if ( uLen)
    {
    uSize = pBuffer->uEnd - pBuffer->uStart;
    if ( uLen > uSize)
      uLen = uSize;

    pBuffer->uEnd -= uLen;
    uRemoved += uLen;
    }

  return uRemoved;
  }


/*
 * Copy buffer to buffer
 */
SWS_EXPORT( int, SWS_BufferCopy(
  SWS_BufferH hDst,                   /* IN: Dst buffer */
  SWS_size_t uDst,                    /* IN: Dst offset */
  SWS_BufferH hSrc,                   /* IN: Src buffer */
  SWS_size_t uSrc,                    /* IN: Src offset */
  SWS_size_t uLen                     /* IN: Bytes to copy */
)){
  TRACE( 9, ("SWS_BufferCopy(%p,%u,%p,%u,%u)\n", hDst,uDst,hSrc,uSrc,uLen));
  ASSERT( hDst >= s_handleArray);
  ASSERT( hDst < s_handleArray + s_uHandleCount);
  ASSERT( 0 < hDst->iRefs);
  ASSERT( hSrc >= s_handleArray);
  ASSERT( hSrc < s_handleArray + s_uHandleCount);
  ASSERT( 0 < hSrc->iRefs);

  while ( uLen)
    {
    SWS_PTR( void) pvSrc;
    SWS_PTR( void) pvDst;
    SWS_size_t uLenSrc, uLenDst;

    pvSrc = SWS_BufferGetFragment( hSrc, uSrc, &uLenSrc);
    if ( NULL == pvSrc)
      return SWS_EFAULT;
    pvDst = SWS_BufferGetFragment( hDst, uDst, &uLenDst);
    if ( NULL == pvDst)
      return SWS_EFAULT;

    if ( uLenSrc > uLenDst)
      uLenSrc = uLenDst;
    if ( uLenSrc > uLen)
      uLenSrc = uLen;

    memcpy( pvDst, pvSrc, uLenSrc);

    uSrc += uLenSrc;
    uDst += uLenSrc;
    uLen -= uLenSrc;
    }

  return 0;
  }


/*
 * Duplicate a buffer
 */
SWS_EXPORT( SWS_BufferH, SWS_BufferDup( /* Returns new buffer or NULL if failed */
  SWS_BufferH hSrc,                   /* IN: Buffer */
  SWS_size_t uSize
)){
  SWS_BufferH hDst;
  SWS_size_t uSrc;

  TRACE( 9, ("SWS_BufferDup(%p,%u)\n", hSrc,uSize));
  ASSERT( hSrc >= s_handleArray);
  ASSERT( hSrc < s_handleArray + s_uHandleCount);
  ASSERT( 0 < hSrc->iRefs);

  uSrc = SWS_BufferSize( hSrc);
  if ( 0 == uSize)
    uSize = uSrc;

  hDst = SWS_BufferGet( uSize, SWS_kBufferDefault);
  if ( NULL == hDst)
    return NULL;

  if ( SWS_BufferCopy( hDst, 0, hSrc, 0, uSrc < uSize ? uSrc : uSize))
    {
    SWS_BufferRelease( hDst);
    return NULL;
    }

  return hDst;
  }


/*
 * Fill buffer from memory
 */
SWS_EXPORT( int, SWS_BufferCopyFromMemory(  /* Returns !0 on failure */
  SWS_BufferH hDst,                   /* IN: Dst buffer */
  SWS_size_t uDst,                    /* IN: Dst offset */
  SWS_PTR( const void) pvSrc,         /* IN: Src */
  SWS_size_t uLen                     /* IN: Bytes to copy */
)){
  TRACE( 9, ("SWS_BufferCopyFromMemory(%p,%u,%p,%u)\n", hDst,uDst,pvSrc,uLen));
  ASSERT( hDst >= s_handleArray);
  ASSERT( hDst < s_handleArray + s_uHandleCount);
  ASSERT( 0 < hDst->iRefs);

  while ( uLen)
    {
    SWS_PTR( void) pvDst;
    SWS_size_t uLenDst;

    pvDst = SWS_BufferGetFragment( hDst, uDst, &uLenDst);
    if ( NULL == pvDst)
      return SWS_EFAULT;

    if ( uLen < uLenDst)
      uLenDst = uLen;

    memcpy( pvDst, pvSrc, uLenDst);

    uDst += uLenDst;
    pvSrc = (SWS_PTR( const void))( (SWS_PTR(const char))pvSrc + uLenDst );
    uLen -= uLenDst;
    }

  return 0;
  }


/*
 * Fill memory from buffer
 */
SWS_EXPORT( int, SWS_BufferCopyToMemory(  /* Returns !0 on failure */
  SWS_PTR( void) pvDst,               /* IN: Dst */
  SWS_BufferH hSrc,                   /* IN: Src buffer */
  SWS_size_t uSrc,                    /* IN: Src offset */
  SWS_size_t uLen                     /* IN: Bytes to copy */
)){
  TRACE( 9, ("SWS_BufferCopyToMemory(%p,%p,%u,%u)\n", pvDst,hSrc,uSrc,uLen));
  ASSERT( hSrc >= s_handleArray);
  ASSERT( hSrc < s_handleArray + s_uHandleCount);
  ASSERT( 0 < hSrc->iRefs);

  while ( uLen)
    {
    SWS_PTR( void) pvSrc;
    SWS_size_t uLenSrc;

    pvSrc = SWS_BufferGetFragment( hSrc, uSrc, &uLenSrc);
    if ( NULL == pvSrc)
      return SWS_EFAULT;

    if ( uLen < uLenSrc)
      uLenSrc = uLen;

    memcpy( pvDst, pvSrc, uLenSrc);

    uSrc += uLenSrc;
    pvDst = (SWS_PTR( void))( (SWS_PTR(char))pvDst + uLenSrc );
    uLen -= uLenSrc;
    }

  return 0;
  }

/* End of file */
