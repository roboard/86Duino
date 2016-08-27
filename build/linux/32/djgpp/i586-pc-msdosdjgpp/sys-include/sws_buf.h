/****************************************************************************
 *
 * File name   : sws_buf.h
 * Function    : Buffer manager public interface
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 * Release Date:
 *
 ****************************************************************************
 * File History
 * ------------
 * 06-Apr-01, LVR, v1.00 - Created.
 *
 *
 * Comments:
 * --------
 * This file contains the public interface to the buffer manager
 *
 ****************************************************************************/

#ifndef SWS_BUF_H
#define SWS_BUF_H 0x0100 /* Version no. BCD */

/* Dependencies */
#include "sws_host.h" /* Basic types */


/*
 * Constants
 */
/* Buffer allocation mode */
typedef enum SWS_EBufferMode
  {
  SWS_kBufferDefault = 0,
  SWS_kBufferPrefix,                  /* Buffer optimised for headers */
  SWS_kBufferSuffix                   /* Buffer optimised for trailers */
  } SWS_EBufferMode;


/*
 * Types
 */
/* Buffer handle */
typedef SWS_PTR( struct SWS_Handle) SWS_BufferH;


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Get max unfragmented buffer size */
SWS_EXPORT( SWS_size_t, SWS_BufferMaxFrag( void));

/* Get available buffer space */
SWS_EXPORT( SWS_size_t, SWS_BufferAvailable( void));

/* Get total buffer space */
SWS_EXPORT( SWS_size_t, SWS_BufferTotal( void));

/* Allocate a buffer from the pool.  Ref count is set to 1 */
#define SWS_BufferGet(_s,_m) SWS_BufferGetShadow( _s, _m, NULL)

/* Allocate a shadow buffer from the pool.  Ref count is set to 1 */
SWS_EXPORT( SWS_BufferH, SWS_BufferGetShadow( /* Returns new buffer or NULL if none */
  SWS_size_t,                         /* IN: Buffer size */
  SWS_EBufferMode,                    /* IN: Packing mode */
  SWS_PTR( void)                      /* IN: -> Data to shadow */
));

/* Release a buffer - buffer is returned to the pool when ref count falls to 0 */
SWS_EXPORT( int, SWS_BufferRelease(   /* Returns no. remaining refs */
  SWS_BufferH                         /* IN: Buffer to release */
));

/* Release a buffer - shadowed data is copied if ref count > 1 */
SWS_EXPORT( int, SWS_BufferReleaseShadow( /* Returns no. remaining refs */
  SWS_BufferH                         /* IN: Buffer to release */
));

/* Add a reference to a buffer */
SWS_EXPORT( void, SWS_BufferAddRef(
  SWS_BufferH                         /* IN: Buffer */
));

/* Get allocated buffer size */
SWS_EXPORT( SWS_size_t, SWS_BufferSize(
  SWS_BufferH                         /* IN: Buffer */
));

/* Get a pointer to data at a given offset in the buffer */
SWS_EXPORT( SWS_PTR( void), SWS_BufferGetFragment(  /* Returns -> fragment, NULL if empty */
  SWS_BufferH,                        /* IN: Buffer */
  SWS_size_t,                         /* IN: Starting offset */
  SWS_PTR( SWS_size_t)                /* OUT: Fragment length, 0 if exhausted */
));

/* Buffer re-sizing */
SWS_EXPORT( int, SWS_BufferAppendFront(  /* Returns !0 on failure */
  SWS_BufferH,                        /* IN: Buffer */
  SWS_size_t                          /* IN: Bytes to append */
));
#define SWS_BufferAppendBack(_h,_s) SWS_BufferAppendBackShadow(_h,_s,NULL)
SWS_EXPORT( int, SWS_BufferAppendBackShadow(  /* Returns !0 on failure */
  SWS_BufferH,                        /* IN: Buffer */
  SWS_size_t,                         /* IN: Bytes to append */
  SWS_PTR( void)                      /* IN: -> Data to shadow */
));
SWS_EXPORT( int, SWS_BufferRemoveFront(  /* Returns !0 on failure */
  SWS_BufferH,                        /* IN: Buffer */
  SWS_size_t                          /* IN: Bytes to drop */
));
SWS_EXPORT( int, SWS_BufferRemoveBack(  /* Returns !0 on failure */
  SWS_BufferH,                        /* IN: Buffer */
  SWS_size_t                          /* IN: Bytes to drop */
));

/* Copy buffer to buffer (non-overlapping) */
SWS_EXPORT( int, SWS_BufferCopy(
  SWS_BufferH,                        /* IN: Dst buffer */
  SWS_size_t,                         /* IN: Dst offset */
  SWS_BufferH,                        /* IN: Src buffer */
  SWS_size_t,                         /* IN: Src offset */
  SWS_size_t                          /* IN: Bytes to copy */
));

/* Duplicate a buffer */
SWS_EXPORT( SWS_BufferH, SWS_BufferDup( /* Returns new buffer or NULL if failed */
  SWS_BufferH,                        /* IN: Buffer */
  SWS_size_t                          /* IN: New size, 0= all */
));

/* Fill buffer from memory */
SWS_EXPORT( int, SWS_BufferCopyFromMemory(  /* Returns !0 on failure */
  SWS_BufferH,                        /* IN: Dst buffer */
  SWS_size_t,                         /* IN: Dst offset */
  SWS_PTR( const void),               /* IN: Src */
  SWS_size_t                          /* IN: Bytes to copy */
));

/* Fill memory from buffer */
SWS_EXPORT( int, SWS_BufferCopyToMemory(  /* Returns !0 on failure */
  SWS_PTR( void),                     /* IN: Dst */
  SWS_BufferH,                        /* IN: Src buffer */
  SWS_size_t,                         /* IN: Src offset */
  SWS_size_t                          /* IN: Bytes to copy */
));


/* Initialise the buffer manager */
extern int SWS_BufferInit( void);     /* Returns !0 on error */

/* Finalise the buffer manager */
extern void SWS_BufferCleanup( void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_BUF_H */
/* End of file */
