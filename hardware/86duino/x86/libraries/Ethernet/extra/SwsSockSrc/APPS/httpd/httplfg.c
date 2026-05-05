/*****************************************************************************
 *
 * httplfg.c : LFG webcam server
 *
 * History
 * -------
 *
 * 14-Jan-03, LVR, v1.00 - Created.
 *
 * Comments
 * --------
 *
 *****************************************************************************/

/* Compilation options */

/* Exports */
#include "httplfg.h"
#if OPT_HTTP_CAMERA

/* Imports */

/* ANSI C library */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "httpd.h"

/* LFG & TMG libraries */
#include "tmg_api.h"
#include "lfg_api.h"


/* VC6 remote debugging:
 *
 * To the custom build page of the project settings add:
 *
 * Commands:
 * %comspec% /c copy $(InputPath) \\Armstrong\DRIVEC\users\lvr
 *
 * Output:
 * \\Armstrong\DRIVEC\users\lvr\httpd.exe
 *
 * To the debug page (general category) add:
 *
 * Working directory:
 * \\Armstrong\DRIVEC\users\lvr
 *
 * Remote path:
 * \\Armstrong\DRIVEC\users\lvr\httpd.exe
 */

/***************************************************************************
 * Macros & constants
 ***************************************************************************/

/* Length of an array - sorely missing from stddef.h */
#ifndef lengthof
#define lengthof( _a) ((size_t)( sizeof(_a)/sizeof((_a)[0]) ))
#endif

#define CRLF "\x0d\x0a"


/***************************************************************************
 * Data types
 ***************************************************************************/

/* LFG instance */
typedef struct SInstance
  {
  struct tLFG lfg;
  int iImages;
  } SInstance;

typedef struct SContext
  {
  ui32 hImage1;
  ui32 uSize, uSent;
  } SContext;


/***************************************************************************
 * Function prototypes
 ***************************************************************************/

static FHttpMethod CamGet;
static FHttpMethod CamGetBody;
static FHttpMethod CamOptions;
static FHttpMethod Forbidden;
static void LfgError(ui32,const char*,ui32,const char*);
static void LfgEvent( ui32, ui32, ui32, void*);


/***************************************************************************
 * Module data
 ***************************************************************************/

/* Actions table */
static const SHttpAction actionArray[] = {
  { "/action/webcam",
    {
      {kszHttpGet,     &CamGet, NULL},
      {kszHttpHead,    &CamGet, NULL},
      {kszHttpPut,     &Forbidden, NULL},
      {kszHttpDelete,  &Forbidden, NULL},
      {kszHttpPost,    &Forbidden, NULL},
      {kszHttpOpts,    &CamOptions, NULL}
    }
  }
};

static SInstance s_inst; /* LFG instance */


/***************************************************************************
 * Functions
 ***************************************************************************/

/*
 * Init this module
 */
int HttpLfgInit( void)
  {
  ui32 err;
  ui32 hLfg;
  SInstance* const pInst = &s_inst;

#if defined WIN32 && defined _MSC_VER
  /* Catch DLL not found exception if TMG & LFG libraries are delay loaded */
  __try {
#endif

  /* Initialise TMG library (includes installing default error handler) */
  TMG_Init();

  /* Init the LFG instance */
  LFG_Init( &pInst->lfg);

#if defined WIN32 && defined _MSC_VER
  } __except( EXCEPTION_EXECUTE_HANDLER) {
    return 0; /* LFG not installed */
  }
#endif

  /* Open the LFG */
  pInst->lfg.dwDeviceAddr = 0;
  err = LFG_Open( &hLfg, &pInst->lfg, &LfgError);
  if ( LFG_OK != err )
    return 0; /* No LFG cards or driver */

  /* Install camera actions */
  HttpActions( actionArray, lengthof( actionArray) );

  return 1; /* Success */
  }


/*
 * Process a GET or HEAD request
 */
int CamGet(
  SHttpConnection* pConn,
  SHttpRequest* pRqst
) {
  SInstance* const pInst = &s_inst;
  SContext* const pContext = (SContext*)pRqst->reserve;
  int iRet;
  const char* pszStatus;
  const char* pszContentType = NULL;
  ui32 hJpegImage = 0;
  ui16 quality;

  if ( NULL == pConn)
    return HTTPD_BAD_REQUEST;

  if ( NULL != pRqst->pszQuery)
    return HttpReply( pConn, HTTPD_BAD_REQUEST, "Unexpected query", 0, NULL);

  if ( pRqst->lContent > 0)
    return HttpReply( pConn, HTTPD_BAD_REQUEST, "Unexpected content", 0, NULL);

  pContext->hImage1 = 0;
  pContext->uSize = 0;

  /* Set default LFG configuration */
  pInst->lfg.sLog.dwVideoMode = LFG_50_PAL_384x288_F1;
  pInst->lfg.sLog.dwPixelFormat = LFG_PIXEL_FORMAT_RGB24;
  quality = 8; /* Q factor, default:32 */

  /* Any parameters? */
  if ( NULL != pRqst->pszParam)
    {
    /* Yes then parse them */
    pszStatus = pRqst->pszParam;
    while ( '\0' != *pszStatus)
      {
      unsigned uVal;
      int count = 0;

      if ( 1 <= sscanf( pszStatus, " pal = %u%n", &uVal, &count))
        {
        /* 50Hz mode */
        pInst->lfg.sLog.dwVideoMode &= ~(VMODE_NTSC | VMODE_PAL | VMODE_MONO | VMODE_60HZ);
        pInst->lfg.sLog.dwVideoMode |= VMODE_50HZ;

        if ( uVal)
          {
          /* Colour */
          pInst->lfg.sLog.dwVideoMode |= VMODE_PAL;
          pInst->lfg.sLog.dwPixelFormat = LFG_PIXEL_FORMAT_RGB24;
          pInst->lfg.sLog.bColor = 128;
          }
        else
          {
          /* Mono */
          pInst->lfg.sLog.dwVideoMode |= VMODE_MONO;
          pInst->lfg.sLog.dwPixelFormat = LFG_PIXEL_FORMAT_Y8;
          pInst->lfg.sLog.bColor = 0;
          }
        }
      else if ( 1 <= sscanf( pszStatus, " ntsc = %u%n", &uVal, &count))
        {
        /* 60Hz mode */
        pInst->lfg.sLog.dwVideoMode &= ~(VMODE_NTSC | VMODE_PAL | VMODE_MONO | VMODE_50HZ);
        pInst->lfg.sLog.dwVideoMode |= VMODE_60HZ;

        if ( uVal)
          {
          /* Colour */
          pInst->lfg.sLog.dwVideoMode |= VMODE_NTSC;
          pInst->lfg.sLog.dwPixelFormat = LFG_PIXEL_FORMAT_RGB24;
          pInst->lfg.sLog.bColor = 128;
          }
        else
          {
          /* Mono */
          pInst->lfg.sLog.dwVideoMode |= VMODE_MONO;
          pInst->lfg.sLog.dwPixelFormat = LFG_PIXEL_FORMAT_Y8;
          pInst->lfg.sLog.bColor = 0;
          }
        }
      else if ( 1 <= sscanf( pszStatus, " size = %u%n", &uVal, &count))
        {
        /* Set size */
        pInst->lfg.sLog.dwVideoMode &= ~(VMODE_F12 | VMODE_REINT | VMODE_BITS_SIZE_MASK);
        switch ( uVal)
          {
        case 1:
          if ( pInst->lfg.sLog.dwVideoMode & VMODE_60HZ)
            pInst->lfg.sLog.dwVideoMode |= VMODE_640x480 | VMODE_F12 | VMODE_REINT;
          else
            pInst->lfg.sLog.dwVideoMode |= VMODE_768x576 | VMODE_F12 | VMODE_REINT;
          break;

        case 2:
        case 0:
          if ( pInst->lfg.sLog.dwVideoMode & VMODE_60HZ)
            pInst->lfg.sLog.dwVideoMode |= VMODE_320x240 | VMODE_F1;
          else
            pInst->lfg.sLog.dwVideoMode |= VMODE_384x288 | VMODE_F1;
          break;

        case 3:
        default:
          if ( pInst->lfg.sLog.dwVideoMode & VMODE_60HZ)
            pInst->lfg.sLog.dwVideoMode |= VMODE_160x120 | VMODE_F1;
          else
            pInst->lfg.sLog.dwVideoMode |= VMODE_192x144 | VMODE_F1;
          break;
          }
        }
      else if ( 1 <= sscanf( pszStatus, " quality = %u%n", &uVal, &count))
        {
        /* Set quality */
        quality = (ui16)uVal;
        }

      if ( 0 == count)
        return HttpReply( pConn, HTTPD_BAD_REQUEST, "Unknown parameter", 0, NULL);

      pszStatus += count;

      /* Skip trailing whitespace */
      while ( isspace( *pszStatus))
        ++pszStatus;

      /* Skip separator */
      while ( ';' == *pszStatus)
        ++pszStatus;
      }
    }

  /* Grab an image */
  do
    {
    ui32 err;
    void* pBuf;

    /* Set LFG configuration */
    pInst->lfg.sLog.nNumImages = 1;
    pInst->lfg.sLog.dwEvents = LFG_EVENT_NEW_IMAGE;
#ifdef _LFG_DOS32
    pInst->lfg.sLog.fPolledDrivenCallback = TRUE;
#endif
    err = LFG_SetAndGet( pInst->lfg.hCard, &pInst->lfg, LFG_WRITE);
    if ( err)
      {
      iRet = HTTPD_INTERN_ERROR;
      pszStatus = "Digitiser configuration failure";
      break;
      }

    LFG_TMG_ImageCreate( &pContext->hImage1 );
    err = LFG_TMG_ImageSet( pInst->lfg.hCard, &pInst->lfg, pContext->hImage1, 1);
    if ( err)
      {
      iRet = HTTPD_INTERN_ERROR;
      pszStatus = "Out of memory";
      break;
      }

    /* Start acquisition */
    pInst->iImages = 2; /* NB skip first image as it's often blank */
    LFG_EventHandlerInstall( pInst->lfg.hCard, &LfgEvent, pInst);
    err = LFG_AcquisitionStart( pInst->lfg.hCard);
    if ( err)
      {
      iRet = HTTPD_INTERN_ERROR;
      pszStatus = "Digitiser failure";
      break;
      }

    /* Wait for image */
    while ( pInst->iImages > 0)
      LFG_SetAndGet( pInst->lfg.hCard, &pInst->lfg, LFG_READ);

    /* Stop acquisition */
    LFG_AcquisitionStop( pInst->lfg.hCard);

    /* Compress image to JPEG */
    hJpegImage = TMG_JPEG_image_create();
    TMG_JPEG_set_Quality_factor( hJpegImage, quality); /* Q factor */
    err = TMG_JPEG_compress( pContext->hImage1, hJpegImage, TMG_RUN);
    if ( err)
      {
      iRet = HTTPD_INTERN_ERROR;
      pszStatus = "Image processing failure";
      break;
      }
#if 0 /* For testing image capture */
    TMG_image_set_outfilename( hJpegImage, "webcam.jpg");
    TMG_JPEG_file_write( hJpegImage, TMG_RUN);
#endif

    /* Get JPEG image stream */
    pBuf = TMG_image_get_ptr( pContext->hImage1, TMG_IMAGE_DATA);
    err = TMG_JPEG_buffer_write( hJpegImage, (ui8*)pBuf, &pContext->uSize, TMG_RUN);
    if ( err)
      {
      pContext->uSize = 0;
      iRet = HTTPD_INTERN_ERROR;
      pszStatus = "JPEG compress failed";
      break;
      }

    pszContentType = "image/jpeg";
    iRet = HTTPD_OK;
    pszStatus = "OK";
    }
#ifdef _MSC_VER
#pragma warning ( disable : 4127) /* conditional expression is constant */
#endif
  while (0);

  /* Prepare response header */
  if ( pContext->uSize > 0)
    {
    char* pszBuf = pRqst->buffer;
    if ( NULL != pszContentType)
      pszBuf += sprintf( pszBuf, kszHttpContentType ": %s" CRLF, pszContentType);
    pszBuf += sprintf( pszBuf, kszHttpPragma ": " kszHttpNoCache CRLF);
    pszBuf += sprintf( pszBuf, kszHttpCacheControl ": " kszHttpNoCache CRLF);
    }
  else
    {
    pRqst->buffer[ 0] = '\0';
    }

  if ( hJpegImage)
    TMG_image_destroy( hJpegImage);

  /* Destroy image if not sending data */
  if ( pContext->hImage1
    && (0 == pContext->uSize || 0 == strcmp( pRqst->pszMethod, kszHttpHead) )
  ) {
    LFG_TMG_ImageDestroy( pContext->hImage1);
    pContext->hImage1 = 0;
    }

  /* Send response header */
  HttpReply( pConn, iRet, pszStatus, (long)pContext->uSize, pRqst->buffer);

  if ( pContext->hImage1)
    {
    pContext->uSent = 0;
    CamGetBody( pConn, pRqst);
    }

  return iRet;
  }


int CamGetBody(
  SHttpConnection* pConn,
  SHttpRequest* pRqst
) {
  SContext* const pContext = (SContext*)pRqst->reserve;
  ui32 len;

  len = pContext->uSize - pContext->uSent;
  if ( len)
    {
    const char* pBuf;

#if UINT_MAX < 0xffffffffL
    if ( len > UINT_MAX)
      len = UINT_MAX;
#endif

    pBuf = (const char*)TMG_image_get_ptr( pContext->hImage1, TMG_IMAGE_DATA);
    pBuf += pContext->uSent;

    pContext->uSent += len;

    /* Send compressed image */
    return HttpSend( pConn, pBuf, (size_t)len, &CamGetBody);
    }

  /* Dispose of compressed image */
  LFG_TMG_ImageDestroy( pContext->hImage1);
  pContext->hImage1 = 0;

  /* Close the connection */
  return HttpSend( pConn, NULL, 0, 0);
  }


/*
 * Handle a forbidden request
 */
int Forbidden(
  SHttpConnection* pConn,
  SHttpRequest* pRqst
) {
  (void)pRqst;
  return HttpReply( pConn, HTTPD_FORBIDDEN, "Forbidden", 0, NULL);
  }


/*
 * OPTIONS request *
 */
int CamOptions(
  SHttpConnection* pConn,
  SHttpRequest* pRqst
) {
  pRqst->iConnClose = 1;
  return HttpReply( pConn, HTTPD_OK, "OK", 0,  kszHttpAllow
    ": " kszHttpGet
    ", " kszHttpHead
    ", " kszHttpOpts
    CRLF
  );
  }


/****************************************************************************
 * LFG support
 ****************************************************************************/

/*
 * LFG library error handler
 */
static void LfgError(
  ui32 hCard,
  const char* pszFn,
  ui32 err,
  const char* pszErr
) {
  (void)pszErr;
  (void)err;
  (void)pszFn;
  (void)hCard;

#ifndef NDEBUG
  LFG_ErrHandlerDefault( hCard, pszFn, err, pszErr);
#endif
  }

/*
 * LFG event handler
 */
static void LfgEvent(
  ui32 hCard,
  ui32 dwEvent,
  ui32 dwIntStatus,
  void* pv
) {
  SInstance* const pInst = (SInstance*)pv;
  (void)dwIntStatus;
  (void)hCard;

  if ( dwEvent & LFG_EVENT_NEW_IMAGE)
    {
    if ( 0 == --pInst->iImages)
      LFG_AcquisitionStop( pInst->lfg.hCard);
    }
  }
#endif /* OPT_HTTP_CAMERA */
/* End of file */
