/*****************************************************************************
 *
 * httpv4l.c : Video4Linux webcam server
 *
 * History
 * -------
 *
 * 19-May-09, LVR, v1.00 - Created.
 *
 * Comments
 * --------
 *
 *****************************************************************************/

/* Compilation options */

/* Exports */
#include "httpv4l.h"

/* Imports */

/* ANSI C library */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <setjmp.h>

/* POSIX */
#include <unistd.h> /* close */
#include <fcntl.h> /* open */
#include <sys/ioctl.h> /* ioctl */

/* Video4Linux */
#include <linux/videodev2.h>

/* libjpeg */
#include <jpeglib.h> /* BUG needs stdio.h */

/* App specific */
#include "httpd.h"
#include "sws_cfg.h"


/***************************************************************************
 * Macros & constants
 ***************************************************************************/

/* Length of an array - sorely missing from stddef.h */
#ifndef lengthof
#define lengthof( _a) ((size_t)( sizeof(_a)/sizeof((_a)[0]) ))
#endif

#define CRLF "\x0d\x0a"

/* ini file values */
#define ksz4vlDev   "v4l_dev"
#define kszDevice     "/dev/video0"
#define ksz4vlInp   "v4l_input"
#define ksz4vlStd   "v4l_std"
#define ksz4vlWidth "v4l_width"
#define ksz4vlHeight "v4l_height"
#define kszQuality  "j_quality"

/* v4l capture config */
#define STD_PAL V4L2_STD_PAL
#define STD_NTSC V4L2_STD_NTSC

#define PAL_WIDTH 680 /* BUG >690 causes tearing */
#define PAL_HEIGHT ((3*PAL_WIDTH +3)/4)

#define NTSC_WIDTH 640
#define NTSC_HEIGHT 480


/***************************************************************************
 * Data types
 ***************************************************************************/

/* Camera instance */
typedef struct SInstance
  {
  char szDevice[ 48]; /* v4l device name */
  int inp;            /* default video input# */
  v4l2_std_id std;    /* default video standard */
  unsigned width, height; /* default image size */
  unsigned quality;   /* default JPEG quality */
  } SInstance;

/* http send context */
typedef struct SContext
  {
  JSAMPROW pJpegImage;
  size_t uSize, uSent;
  } SContext;

/* image capture context */
typedef struct SCapture
  {
  /* Capture settings */
  int inp;            /* video input# */
  v4l2_std_id std;
  struct v4l2_format fmt;
  void* pvImage;      /* Captured image */

  /* Compression */
  unsigned quality;   /* JPEG quality */
  void* pvBuf;        /* Work buffer */
  size_t sizeBuf;     /* Work buffer size */
  jmp_buf jbuf;       /* Error exit */
  } SCapture;


/***************************************************************************
 * Function prototypes
 ***************************************************************************/

static FHttpMethod CamGet;
static FHttpMethod CamGetBody;
static FHttpMethod Forbidden;
static FHttpMethod CamOptions;

static int GrabImage( SInstance*, SCapture*, const char**);
static int ProcImage( SContext*, SCapture*, const char**);

static void init_destination( j_compress_ptr cinfo);
static boolean empty_output_buffer( j_compress_ptr cinfo);
static void term_destination( j_compress_ptr cinfo);
static void error_exit( j_common_ptr cinfo);


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

static SInstance s_inst; /* instance */


/***************************************************************************
 * Functions
 ***************************************************************************/

/**
 * Init this module
 */
extern int HttpV4lInit( void)
  {
  SInstance* const pInst = &s_inst;
  int fd; /* video device handle */

  /* Setup instance data */
  SWS_CfgGetString( kszHttpSect, ksz4vlDev, kszDevice, pInst->szDevice, sizeof pInst->szDevice);
  pInst->inp = SWS_CfgGetValue( kszHttpSect, ksz4vlInp, -1);
  pInst->std = SWS_CfgGetValue( kszHttpSect, ksz4vlStd, STD_PAL);
  pInst->width = SWS_CfgGetValue( kszHttpSect, ksz4vlWidth, PAL_WIDTH /2);
  pInst->height = SWS_CfgGetValue( kszHttpSect, ksz4vlHeight, PAL_HEIGHT /2);
  pInst->quality = SWS_CfgGetValue( kszHttpSect, kszQuality, 25); /* 0..100 */

  /* Open the video device */
  fd = open( pInst->szDevice, O_RDWR, 0);
  if ( fd < 0)
    return 0; /* No v4l device */

  do
    {
    struct v4l2_capability cap;

    /* Check caps */
    if ( -1 == ioctl( fd, VIDIOC_QUERYCAP, &cap))
      {
      fprintf( stderr, "Failed(%d) querying caps: %s\n",  errno, strerror (errno));
      break;
      }
    if ( !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
      {
      fputs( "Not a video capture device\n", stderr);
      break;
      }
    if ( !(cap.capabilities & V4L2_CAP_READWRITE))
      {
      fputs( "Device does not support read\n", stderr);
      break;
      }

    /* Prefer user specified video input# */
    if ( -1 == pInst->inp)
      {
      int i;

      /* Find the first composite input */
      pInst->inp = 0;
      for ( i = 0; ; ++i)
        {
        struct v4l2_input inp;
        inp.index = i;
        if ( -1 == ioctl( fd, VIDIOC_ENUMINPUT, &inp))
          break;
        if ( V4L2_INPUT_TYPE_CAMERA == inp.type
          && strstr( (const char*)inp.name, "omposit")
        ) {
          pInst->inp = i;
          break;
          }
        }
      }

    /* Install camera actions */
    HttpActions( actionArray, lengthof( actionArray) );

    close( fd);
    return 1; /* Success */
    }
  while ( 0);

  close( fd);
  return 0; /* No camera */
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
  SCapture cap;

  if ( NULL == pConn)
    return HTTPD_BAD_REQUEST;

  if ( NULL != pRqst->pszQuery)
    return HttpReply( pConn, HTTPD_BAD_REQUEST, "Unexpected query", 0, NULL);

  if ( pRqst->lContent > 0)
    return HttpReply( pConn, HTTPD_BAD_REQUEST, "Unexpected content", 0, NULL);

  pContext->pJpegImage = NULL;
  pContext->uSize = 0;

  /* Set default capture configuration */
  cap.inp = pInst->inp;
  cap.std = pInst->std;
  memset( &cap.fmt, 0, sizeof cap.fmt);
  cap.fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  cap.fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24; /* Preferred by libjpeg */
  cap.fmt.fmt.pix.width = pInst->width;
  cap.fmt.fmt.pix.height = pInst->height;
  cap.fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
  cap.pvImage = NULL;
  cap.quality = pInst->quality;

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
        cap.std = STD_PAL;

        if ( uVal)
          {
          /* Colour */
          cap.fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
          }
        else
          {
          /* Mono */
          cap.fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
          }
        }
      else if ( 1 <= sscanf( pszStatus, " ntsc = %u%n", &uVal, &count))
        {
        /* 60Hz mode */
        cap.std = STD_NTSC;

        if ( uVal)
          {
          /* Colour */
          cap.fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
          }
        else
          {
          /* Mono */
          cap.fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
          }
        }
      else if ( 1 <= sscanf( pszStatus, " size = %u%n", &uVal, &count))
        {
        /* Set size */
        switch ( uVal)
          {
        case 1:
          if ( STD_PAL == cap.std)
            cap.fmt.fmt.pix.width = PAL_WIDTH, cap.fmt.fmt.pix.height = PAL_HEIGHT;
          else
            cap.fmt.fmt.pix.width = NTSC_WIDTH, cap.fmt.fmt.pix.height = NTSC_HEIGHT;
          break;

        case 2:
        case 0:
          if ( STD_PAL == cap.std)
            cap.fmt.fmt.pix.width = PAL_WIDTH /2, cap.fmt.fmt.pix.height = PAL_HEIGHT /2;
          else
            cap.fmt.fmt.pix.width = NTSC_WIDTH /2, cap.fmt.fmt.pix.height = NTSC_HEIGHT /2;
          break;

        case 3:
        default:
          if ( STD_PAL == cap.std)
            cap.fmt.fmt.pix.width = PAL_WIDTH /4, cap.fmt.fmt.pix.height = PAL_HEIGHT /4;
          else
            cap.fmt.fmt.pix.width = NTSC_WIDTH /4, cap.fmt.fmt.pix.height = NTSC_HEIGHT /4;
          break;
          }
        }
      else if ( 1 <= sscanf( pszStatus, " quality = %u%n", &uVal, &count))
        {
        /* Set quality */
        cap.quality = uVal;
        }
      else if ( 1 <= sscanf( pszStatus, " input = %u%n", &uVal, &count))
        {
        /* Set input */
        cap.inp = uVal;
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
  pszStatus = "Camera error";
  iRet = HTTPD_INTERN_ERROR;
  if ( 0 == GrabImage( pInst, &cap, &pszStatus))
    {
    /* Compress JPEG */
    pszStatus = "JPEG error";
    if ( 0 == ProcImage( pContext, &cap, &pszStatus))
      {
      pszContentType = "image/jpeg";
      iRet = HTTPD_OK;
      pszStatus = "OK";
      }

    /* Dispose of raw image */
    free( cap.pvImage);
    }

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

  /* Destroy image if not sending data */
  if ( NULL != pContext->pJpegImage
    && (0 == pContext->uSize || 0 == strcmp( pRqst->pszMethod, kszHttpHead) )
  ) {
    free( pContext->pJpegImage);
    pContext->pJpegImage = NULL;
    }

  /* Send response header */
  HttpReply( pConn, iRet, pszStatus, (long)pContext->uSize, pRqst->buffer);

  if ( pContext->uSize && NULL != pContext->pJpegImage)
    {
    /* Send response body */
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
  unsigned len;

  len = pContext->uSize - pContext->uSent;
  if ( len)
    {
    JSAMPROW pBuf;

    pBuf = pContext->pJpegImage + pContext->uSent;
    pContext->uSent += len;

    /* Send compressed image */
    return HttpSend( pConn, pBuf, (size_t)len, &CamGetBody);
    }

  /* Dispose of compressed image */
  free( pContext->pJpegImage);
  pContext->pJpegImage = NULL;

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
 * support
 ****************************************************************************/

/*
 * Grab an image from the v4l device
 */
int GrabImage( SInstance* pInst, SCapture* pCap, const char** ppszStatus)
  {
  int iRet;
  int fd;

  fd = open( pInst->szDevice, O_RDWR, 0);
  if ( fd < 0)
    {
    *ppszStatus = strerror( errno);
    return -1; /* No v4l device */
    }

  iRet = -1;
  do
    {
    ssize_t n;

    /* Select input */
    ioctl( fd, VIDIOC_S_INPUT, &pCap->inp);

    /* Set the video standard */
    if ( -1 == ioctl( fd, VIDIOC_S_STD, &pCap->std))
      {
      *ppszStatus = "Unsupported video standard";
      break;
      }

    /* Set video format */
    if ( 0 != ioctl( fd, VIDIOC_S_FMT, &pCap->fmt))
      {
      pCap->fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
      if ( 0 != ioctl( fd, VIDIOC_S_FMT, &pCap->fmt))
        {
        *ppszStatus = "Unsupported video format";
        break;
        }
      }

    /* Allocate an image buffer */
    pCap->pvImage = malloc( pCap->fmt.fmt.pix.sizeimage);
    if ( NULL == pCap->pvImage)
      {
      *ppszStatus = "Out of memory";
      break;
      }

    /* Grab an image */
    n = read( fd, pCap->pvImage, pCap->fmt.fmt.pix.sizeimage);
    if ( -1 == n)
      {
      *ppszStatus = strerror( errno);
      free( pCap->pvImage);
      pCap->pvImage = NULL;
      break;
      }

    /* OK */
    iRet = 0;
    }
  while (0);

  close( fd);
  return iRet;
  }


/*
 * Jpeg compress an image
 */
int ProcImage( SContext* pContext, SCapture* pCap, const char** ppszStatus)
  {
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  struct jpeg_destination_mgr mgr;

  pCap->pvBuf = NULL;
  pCap->sizeBuf = 16 * 1024; /* Initial work buffer size */

  /* Setup JPEG compressor */
  cinfo.client_data = pCap;
  cinfo.err = jpeg_std_error( &jerr);
  cinfo.err->error_exit = &error_exit;
  if ( setjmp( pCap->jbuf))
    {
    free( pCap->pvBuf);
    pCap->pvBuf = NULL;
    jpeg_destroy_compress( &cinfo);
    *ppszStatus = "jpeglib error";
    return 1;
    }
  jpeg_create_compress( &cinfo);

  /* Setup destination */
  mgr.free_in_buffer = 0;
  mgr.next_output_byte = NULL;
  mgr.init_destination = &init_destination;
  mgr.empty_output_buffer = &empty_output_buffer;
  mgr.term_destination = &term_destination;
  cinfo.dest = &mgr;

  /* Set source image format */
  cinfo.image_width = pCap->fmt.fmt.pix.width;
  cinfo.image_height = pCap->fmt.fmt.pix.height;
  if ( V4L2_PIX_FMT_GREY == pCap->fmt.fmt.pix.pixelformat)
    {
    cinfo.input_components = 1;
    cinfo.in_color_space = JCS_GRAYSCALE;
    }
  else if ( V4L2_PIX_FMT_RGB24 == pCap->fmt.fmt.pix.pixelformat)
    {
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    }
  else if ( V4L2_PIX_FMT_BGR24 == pCap->fmt.fmt.pix.pixelformat)
    {
    /* swap R/B */
    JSAMPROW pRow = (JSAMPROW)pCap->pvImage;
    unsigned int i;
    for ( i = 0; i < pCap->fmt.fmt.pix.height; ++i, pRow += pCap->fmt.fmt.pix.bytesperline)
      {
      JSAMPROW p = pRow;
      unsigned int j;
      for ( j = 0; j < pCap->fmt.fmt.pix.width; ++j, p += 3)
        {
        const unsigned char u = p[0];
        p[0] = p[2];
        p[2] = u;
        }
      }
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    }
  else
    {
    *ppszStatus = "Incompatible image format";
    jpeg_destroy_compress( &cinfo);
    return 1;
    }
  jpeg_set_defaults( &cinfo);
  jpeg_set_quality( &cinfo, pCap->quality, TRUE);

  /* Compress image */
  jpeg_start_compress( &cinfo, TRUE);
  while ( cinfo.next_scanline < cinfo.image_height)
    {
    JSAMPROW row_pointer[1]; /* pointer to JSAMPLE row[s] */
    row_pointer[0] = (JSAMPROW)pCap->pvImage + cinfo.next_scanline * pCap->fmt.fmt.pix.bytesperline;
    jpeg_write_scanlines( &cinfo, row_pointer, 1);
    }
  jpeg_finish_compress( &cinfo);

  /* Get JPEG image stream */
  pContext->pJpegImage = (JOCTET*)pCap->pvBuf;
  pCap->pvBuf = NULL;
  pContext->uSize = mgr.next_output_byte - pContext->pJpegImage;
  jpeg_destroy_compress( &cinfo);
  return 0;
  }


/*
 * Setup JPEG output buffer
 */
void init_destination( j_compress_ptr cinfo)
  {
  SCapture* pCap = (SCapture*)cinfo->client_data;

  pCap->pvBuf = malloc( pCap->sizeBuf);
  if ( NULL == pCap->pvBuf)
    (*cinfo->err->error_exit)( (j_common_ptr)cinfo);
  else
    {
    cinfo->dest->next_output_byte = (JOCTET*)pCap->pvBuf;
    cinfo->dest->free_in_buffer = pCap->sizeBuf;
    }
  }

/*
 * JPEG output buffer full
 */
boolean empty_output_buffer( j_compress_ptr cinfo)
  {
  SCapture* pCap = (SCapture*)cinfo->client_data;
  void* pv;
  size_t size;

  size = 2 * pCap->sizeBuf;
  pv = realloc( pCap->pvBuf, size);
  if ( NULL == pv)
    (*cinfo->err->error_exit)( (j_common_ptr)cinfo);
  else
    {
    size_t used = cinfo->dest->next_output_byte - (JOCTET*)pCap->pvBuf;

    pCap->sizeBuf = size;
    pCap->pvBuf = pv;

    cinfo->dest->next_output_byte = (JOCTET*)pv + used;
    cinfo->dest->free_in_buffer = size - used;
    }
  return TRUE; /* FALSE for I/O suspension */
  }

/*
 * JPEG output complete
 */
void term_destination( j_compress_ptr cinfo)
  {
  (void) cinfo;
  }

/*
 * JPEG error handler
 */
void error_exit( j_common_ptr cinfo)
  {
  cinfo->err->output_message( cinfo);
  longjmp( ((SCapture*)cinfo->client_data)->jbuf, 1);
  }

/* End of file */
