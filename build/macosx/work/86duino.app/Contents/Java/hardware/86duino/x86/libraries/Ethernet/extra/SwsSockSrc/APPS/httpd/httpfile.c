/*****************************************************************************
 *
 * File name   : httpfile.c
 * Function    : web server file ops
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 13-Jan-03, LVR, v1.00 - Created.
 * 22-Jan-10, LVR, Added independent paths to public and authenticated files.
 *
 *****************************************************************************/

/* Compilation options */
#ifndef OPT_HTTP_TINY
#define OPT_HTTP_TINY 0 /* !0 for min function */
#endif

/* Exports */
#include "httpfile.h"

/* Imports */

/* ANSI C library */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "httpd.h"
#include "sws_cfg.h"
#include "sws_str.h"
#if !OPT_HTTP_TINY
#include "sws_time.h"
#endif


/***************************************************************************
 * Macros & constants
 ***************************************************************************/

/* Length of an array - sorely missing from stddef.h */
#ifndef lengthof
#define lengthof( _a) ((size_t)( sizeof(_a)/sizeof((_a)[0]) ))
#endif

/* Expand and stringize a macro name */
#define STR(s) #s
#define STRING(s) STR(s)

#define INVALID_TIME ((time_t)-1)
#define CRLF "\x0d\x0a"
#define kszUpDir ".."

/* ini file values */
#define kszHtmlRoot     "html_root"
#define kszAuthorize    "authorize"
#define kszSectTypes    "http_files"


/***************************************************************************
 * Data types
 ***************************************************************************/

/* Connection context */
typedef struct SContext
  {
  FILE* file;
  const char* pszContentType;
  } SContext;

#if !OPT_HTTP_TINY
/* File MIME tyoe */
struct SMineInfo
  {
  const char* pszName;
#define SIZE_MIME_TYPE 31
  char szMimeType[ SIZE_MIME_TYPE + 1];
  };
#endif


/***************************************************************************
 * Function prototypes
 ***************************************************************************/

static FHttpMethod HttpFileGet;
static FHttpMethod FileGetSendBody;
#if !OPT_HTTP_TINY
static FHttpMethod HttpFilePut;
static FHttpMethod HttpFileDel;
static FHttpMethod FilePutBody;
#endif
static FHttpMethod Forbidden;
static FHttpMethod Options;

static void UriToPathName( char*, const char*, const char*);
static const char* GetContentType( const char*);

static int PlatformIsDir( const char*);
static long PlatformFileLength( FILE*);
#if !OPT_HTTP_TINY
static time_t PlatformFileTime( FILE*);
static SWS_CB( int, EnumRestricted( void* pv, const char* pszVal));
static SWS_FCfgCallback EnumTypes;
#endif


/***************************************************************************
 * Module data
 ***************************************************************************/

static char s_szRoot[ 256]; /* File system root */
static char s_szPublic[ 256]; /* Default localpath prefix */

/* Table of known URI actions */
static const SHttpAction actionArray[] = {
  { "/*",
    {
      {kszHttpGet,     &HttpFileGet, NULL},
      {kszHttpHead,    &HttpFileGet, NULL},
#if OPT_HTTP_TINY
      {kszHttpPut,     &Forbidden, NULL},
      {kszHttpDelete,  &Forbidden, NULL},
#else
      {kszHttpPut,     &HttpFilePut, kszHttpAdmin},
      {kszHttpDelete,  &HttpFileDel, kszHttpAdmin},
#endif
      {kszHttpPost,    &Forbidden, NULL},
      {kszHttpOpts,    &Options, NULL}
    }
  }
};


/***************************************************************************
 * Functions
 ***************************************************************************/

/**
 * Init http file system root
 */
extern int HttpFileInit( void)
  {
  char szBuf[ sizeof s_szRoot];

  SWS_CfgGetString( kszHttpSect, kszHttpRoot, ".", s_szRoot, sizeof s_szRoot);
  SWS_CfgGetString( kszHttpSect, kszHtmlRoot, "", s_szPublic, sizeof s_szPublic);

#if !OPT_HTTP_TINY
  /* Enum stems that require authorization */
  SWS_CfgEnum( kszHttpSect, &EnumRestricted, NULL);
#endif

  HttpActions( actionArray, lengthof( actionArray) );

  strcat( strcpy( szBuf, s_szRoot), s_szPublic);
  return PlatformIsDir( szBuf);
  }


#if !OPT_HTTP_TINY
SWS_CB( int, EnumRestricted( void* pv, const char* pszVal))
  {
  char szStem[ 256], szAuth[ 32], szAdmin[ 32];
  int n;
  (void)pv;

  /* Parse entry */
  if ( 2 <= (n = sscanf( pszVal, " "kszAuthorize" = %255s %31[^ ;] %31[^ ;]",
      szStem, szAuth, szAdmin))
  ) {
    SHttpAction action = { NULL,
      {
        {kszHttpGet,     &HttpFileGet, NULL},
        {kszHttpHead,    &HttpFileGet, NULL},
        {kszHttpPut,     &Forbidden, NULL},
        {kszHttpDelete,  &Forbidden, NULL},
        {kszHttpOpts,    &Options, NULL}
      }
    };
    size_t size = strlen( szStem) + 1;
    size_t size2 = strlen( szAuth) + 1;
    size_t size3 = n >= 3 ? strlen( szAuth) + 1 : 0;
    /* NB this memory is never freed */
    char* psz = (char*) malloc( size + size2 + size3);
    if ( NULL == psz)
      {
      HttpLog( kHttpError, NULL, "ERROR %d: EnumRestricted malloc: %s\n",
        errno, strerror( errno));
      return -1;
      }

    action.pszStem = strcpy( psz, szStem);
    psz = (char*)strcpy( psz + size, szAuth);
    action.methods[0].pszRealm = psz; /* Get */
    action.methods[1].pszRealm = psz; /* Head */
    if ( size3)
      {
      psz = (char*)strcpy( psz + size2, szAdmin);
      action.methods[2].pszRealm = psz; /* Put */
      action.methods[2].pfnExecute = &HttpFilePut;
      action.methods[3].pszRealm = psz; /* Delete */
      action.methods[3].pfnExecute = &HttpFileDel;
      }

    HttpActions( &action, 1);
    }

  return 0;
  }


/**
 * Convert http local path to full file system path
 */
extern size_t HttpFilePath(
  char szOut[],                       /* OUT: full path to file */
  const char* pszIn,                  /* IN: local path */
  size_t size                         /* IN: output buffer size */
) {
  size_t len1, len2, len3;

  /* TODO Check if pszIn is a Realm and use prefix from that */
  len1 = strlen( s_szRoot);
  len2 = strlen( s_szPublic);
  len3 = 1 + len1 + len2 + strlen( pszIn);
  if ( len3 >= size)
    return len3;
  strcpy( szOut, s_szRoot);
  strcpy( &szOut[ len1], s_szPublic);
  strcpy( &szOut[ len1 + len2], pszIn);
  return 0;
  }
#endif


/*
 * Process a GET or HEAD file request
 */
int HttpFileGet(
  SHttpConnection* pConn,
  SHttpRequest* pRqst
) {
  char* pszBuf;
  const char* pszContentType;
  const char* pszStatus;
  int iRet;
  long len;
  SContext* const pContext = (SContext*)pRqst->reserve;
#if !OPT_HTTP_TINY
  time_t tModified;
#endif

  if ( NULL == pConn)
    return HTTPD_BAD_REQUEST;

  if ( NULL != pRqst->pszParam)
    return HttpReply( pConn, HTTPD_BAD_REQUEST, "Unexpected parameters", 0, NULL);

  if ( NULL != pRqst->pszQuery)
    return HttpReply( pConn, HTTPD_BAD_REQUEST, "Unexpected query", 0, NULL);

  if ( pRqst->lContent > 0)
    return HttpReply( pConn, HTTPD_BAD_REQUEST, "Unexpected content", 0, NULL);

  UriToPathName( pRqst->buffer, pRqst->pszPath, HttpRealmPath( pConn));

  /* Disallow .. in pathname */
  if ( strstr( pRqst->buffer, kszUpDir))
    return HttpReply( pConn, HTTPD_FORBIDDEN, "Forbidden .. in URI", 0, NULL);

#if !OPT_HTTP_TINY
  if ( PlatformIsDir( pRqst->buffer))
    {
    if ( NULL != (pszStatus = strrchr( pRqst->buffer, '/'))
      && '\0' == pszStatus[1]
    ) {
      /* TODO directory listing */
      return HttpReply( pConn, HTTPD_FORBIDDEN, "Forbidden", 0, NULL);
      }
    else
      {
      strcat( pRqst->buffer, "/" HTTP_INDEX_FILE);
      pContext->file = fopen( pRqst->buffer, "rb");
      if ( NULL != pContext->file)
        {
        fclose( pContext->file);
        sprintf( pRqst->buffer, kszHttpLocation ": %s", pRqst->pszPath);
        strcat( pRqst->buffer, "/" HTTP_INDEX_FILE CRLF);
        return HttpReply( pConn, HTTPD_MOVED_PERM, "Moved", 0, pRqst->buffer);
        }
      return HttpReply( pConn, HTTPD_NOT_FOUND, "Is a directory", 0, NULL);
      }
    }
#endif

  /* Open the file */
  pContext->file = fopen( pRqst->buffer, "rb");
  if ( NULL == pContext->file)
    {
    HttpLog( kHttpWarn, pConn, "WARN %d: HttpFileGet fopen(%s): %s\n",
      errno, pRqst->buffer, strerror( errno));
    return HttpReply( pConn, HTTPD_NOT_FOUND, "File not found", 0, NULL);
    }

  /* Get MIME type (RFC 1521) for content */
  pszContentType = GetContentType( pRqst->buffer);
#if FORCE_TYPE
  if ( NULL == pszContentType)
    pszContentType = "application/octet-stream";
#endif

#if !OPT_HTTP_TINY
  tModified = PlatformFileTime( pContext->file);

  /* Compare modification dates */
  if ( strcmp( pRqst->pszMethod, kszHttpHead)
    && pRqst->tIfModifiedSince != INVALID_TIME
    && INVALID_TIME != tModified
    && tModified <= pRqst->tIfModifiedSince
  ) {
    len = 0; /* BUG: Firefox 3.5.5 won't accept -1 */
    tModified = INVALID_TIME;
    iRet = HTTPD_NOT_MODIFIED;
    pszStatus = "Not modified";
    }
  else
#endif
    {
    /* Get file size */
    len = PlatformFileLength( pContext->file);
    if ( len > 0)
      {
      iRet = HTTPD_OK;
      pszStatus = "OK";
      }
    else
      {
      len = 0; /* BUG: Firefox 3.5.5 won't accept -1 */
      iRet = HTTPD_NO_CONTENT;
      pszStatus = "No content";
      }
    }

  /* Prepare response header */
  pszBuf = pRqst->buffer;
#if !FORCE_TYPE
  if ( NULL == pszContentType)
    pRqst->buffer[0] = '\0';
  else
#endif
    pszBuf += sprintf( pszBuf, kszHttpContentType ": %s" CRLF, pszContentType);

#if !OPT_HTTP_TINY
  if ( tModified != INVALID_TIME)
    {
    char szTime[40];
    pszBuf += sprintf( pszBuf, kszHttpLastModified ": %s" CRLF,
      SWS_TimeToStr( szTime, sizeof szTime, tModified));
    }
#endif

  if ( NULL != pContext->file
    && (len <= 0 || 0 == strcmp( pRqst->pszMethod, kszHttpHead))
  ) {
    fclose( pContext->file);
    pContext->file = NULL;
    }

  /* Send response header */
  iRet = HttpReply( pConn, iRet, pszStatus, len,
    pszBuf > pRqst->buffer ? pRqst->buffer : NULL);

  if ( NULL != pContext->file)
    FileGetSendBody( pConn, pRqst);

  return iRet;
  }


int FileGetSendBody(
  SHttpConnection* pConn,
  SHttpRequest* pRqst
) {
  SContext* const pContext = (SContext*)pRqst->reserve;
  int iRet;

   /* Send the body */
  if ( NULL == pConn)
    {
    /* Abort */
    iRet = 0;
    }
  else if ( ferror( pContext->file))
    {
    /* Read error */
    iRet = HttpSend( pConn, NULL, 0, 0);
    }
  else if ( feof( pContext->file))
    {
    /* End of body - close connection if required */
    iRet = HttpSend( pConn, NULL, 0, 0);
    }
  else
    {
    size_t count = fread( pRqst->buffer, 1, sizeof pRqst->buffer, pContext->file);
    return HttpSend( pConn, pRqst->buffer, count, &FileGetSendBody);
    }

  fclose( pContext->file);
  pContext->file = NULL;
  return iRet;
  }


#if !OPT_HTTP_TINY
/*
 * Process a PUT file request
 */
int HttpFilePut(
  SHttpConnection* pConn,
  SHttpRequest* pRqst
) {
  SContext* const pContext = (SContext*)pRqst->reserve;

  if ( NULL != pRqst->pszParam)
    return HttpReply( pConn, HTTPD_BAD_REQUEST, "Unexpected parameters", 0, NULL);

  if ( NULL != pRqst->pszQuery)
    return HttpReply( pConn, HTTPD_BAD_REQUEST, "Unexpected query", 0, NULL);

  UriToPathName( pRqst->buffer, pRqst->pszPath, HttpRealmPath( pConn));

  /* Disallow .. in pathname */
  if ( strstr( pRqst->buffer, kszUpDir))
    return HttpReply( pConn, HTTPD_FORBIDDEN, "Forbidden .. in URI", 0, NULL);

  if ( PlatformIsDir( pRqst->buffer))
    return HttpReply( pConn, HTTPD_FORBIDDEN, "Can't put a directory", 0, NULL);

  /* Open the file */
  pContext->file = fopen( pRqst->buffer, "wb");
  if ( NULL == pContext->file)
    {
    HttpLog( kHttpWarn, pConn, "WARN %d: HttpFilePut fopen(%s): %s\n",
      errno, pRqst->buffer, strerror( errno));
    return HttpReply( pConn, HTTPD_NOT_FOUND, "File not found", 0, NULL);
    }

  pContext->pszContentType = GetContentType( pRqst->buffer);

  if ( pRqst->lContent > 0)
    /* Recv body */
    return HttpRecv( pConn, &FilePutBody);

  fclose( pContext->file);
  sprintf( pRqst->buffer, kszHttpContentType ": %s" CRLF, pContext->pszContentType);
  return HttpReply( pConn, HTTPD_CREATED, "OK", 0, pRqst->buffer);
  }


int FilePutBody(
  SHttpConnection* pConn,
  SHttpRequest* pRqst
) {
  SContext* const pContext = (SContext*)pRqst->reserve;
  int iRet;

  if ( NULL == pConn)
    {
    /* Abort */
    iRet = 0;
    }
  else if ( 0 == pRqst->sizeAvail)
    {
    /* Complete */
    if ( fclose( pContext->file))
      {
      HttpLog( kHttpError, pConn, "ERROR %d: FilePutBody fclose: %s\n", errno, strerror( errno));
      iRet = HttpReply( pConn, HTTPD_INTERN_ERROR, "No resources", 0, NULL);
      }
    else
      {
      if ( NULL == pContext->pszContentType)
        sprintf( pRqst->buffer, kszHttpContentType ": %s" CRLF, pContext->pszContentType);

      iRet = HttpReply( pConn, HTTPD_CREATED, "OK", 0,
        pContext->pszContentType ? pRqst->buffer : NULL );
      }
    return iRet;
    }
  /* Save the content */
  else if ( pRqst->sizeAvail != fwrite( pRqst->buffer, 1, pRqst->sizeAvail, pContext->file))
    {
    /* Write error */
    HttpLog( kHttpError, pConn, "ERROR %d: FilePutBody fwrite: %s\n", errno, strerror( errno));
    iRet = HttpReply( pConn, HTTPD_INTERN_ERROR, "No resources", 0, NULL);
    }
  else
    return HTTPD_OK;

  fclose( pContext->file);
  return iRet;
  }


/*
 * Process a DELETE file request
 */
int HttpFileDel(
  SHttpConnection* pConn,
  SHttpRequest* pRqst
) {
  FILE* file;
  int iRet;
  const char* pszStatus;
  long len;

  if ( NULL != pRqst->pszParam)
    return HttpReply( pConn, HTTPD_BAD_REQUEST, "Unexpected parameters", 0, NULL);

  if ( NULL != pRqst->pszQuery)
    return HttpReply( pConn, HTTPD_BAD_REQUEST, "Unexpected query", 0, NULL);

  UriToPathName( pRqst->buffer, pRqst->pszPath, HttpRealmPath( pConn));

  /* Disallow .. in pathname */
  if ( strstr( pRqst->buffer, kszUpDir))
    return HttpReply( pConn, HTTPD_FORBIDDEN, "Forbidden .. in URI", 0, NULL);

  if ( PlatformIsDir( pRqst->buffer))
    return HttpReply( pConn, HTTPD_FORBIDDEN, "Can't delete a directory", 0, NULL);

  file = fopen( pRqst->buffer, "rb");
  if ( NULL == file)
    {
    HttpLog( kHttpWarn, pConn, "WARN %d: HttpFileDel fopen(%s): %s\n",
      errno, pRqst->buffer, strerror( errno));
    return HttpReply( pConn, HTTPD_NOT_FOUND, "File not found", 0, NULL);
    }

  fclose( file);

  /* Delete the file */
  if ( !remove( pRqst->buffer))
    {
    pszStatus = "OK";
    iRet = HTTPD_NO_CONTENT;
    len = 0; /* BUG: Firefox 3.5.5 won't accept -1 */
    }
  else
    {
    HttpLog( kHttpError, pConn, "ERROR %d: HttpFileDel remove(%s): %s\n",
      errno, pRqst->buffer, strerror( errno));
    pszStatus = "Failed";
    iRet = HTTPD_INTERN_ERROR;
    len = 0;
    }

  /* Send response */
  return HttpReply( pConn, iRet, pszStatus, len, NULL);
  }

#endif /*!OPT_HTTP_TINY*/


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
int Options(
  SHttpConnection* pConn,
  SHttpRequest* pRqst
) {
  pRqst->iConnClose = 1;
  return HttpReply( pConn, HTTPD_OK, "OK", 0, kszHttpAllow
    ": " kszHttpGet
    ", " kszHttpHead
#if !OPT_HTTP_TINY
    ", " kszHttpPut
    ", " kszHttpDelete
#endif
    ", " kszHttpOpts
    CRLF
  );
  }


/*
 * Convert URI to local pathname
 */
void UriToPathName( char* pszBuf, const char* pszPath, const char* pszPrefix)
  {
  /* Form local pathname ending with a single slash */
  pszBuf += sprintf( pszBuf, "%s%s/", s_szRoot, NULL == pszPrefix ? s_szPublic : pszPrefix);

  /* Remove all leading slashes from the URI */
  while( '/' == *pszPath)
    ++pszPath;

  /* Append URI to local pathname */
#if !OPT_HTTP_TINY
  /* Decode escape sequences in the URI */
  HttpDecodeString( pszBuf, pszPath);
#else
  strcpy( pszBuf, pszPath);
#endif
  }


/*
 * Get MIME type (RFC 1521) for content
 */
const char* GetContentType( const char* pszName)
  {
  /* Table of known file types */
  static struct {
    const char* pszMatch;
    const char* pszMimeType;
  } const s_filetypes[] = {
    /* In order of precedence */
    { "*.htm",  "text/html" },
    { "*.html", "text/html" },
    { "*.txt",  "text/plain" },
    { "*.css",  "text/css" },
    { "*.gif",  "image/gif" },
    { "*.png",  "image/png" },
    { "*.ico",  "image/vnd.microsoft.icon" },
    { "*.jpg",  "image/jpeg" },
    { "*.jpeg", "image/jpeg" }
  };
  size_t count;
#if !OPT_HTTP_TINY
  static struct SMineInfo info;

  /* Check user defined types first */
  info.pszName = pszName;
  if ( SWS_CfgEnum( kszSectTypes, &EnumTypes, &info))
    return info.szMimeType;
#endif

  for ( count = 0; count < lengthof( s_filetypes); ++count)
    {
    if ( SWS_StrMatch( s_filetypes[ count].pszMatch, pszName, 1) )
      return s_filetypes[ count].pszMimeType;
    }
  return NULL;
  }

#if !OPT_HTTP_TINY
SWS_CB( int, EnumTypes( void* pvCtx, const char* pszVal))
  {
  char szMatch[ 32];
  struct SMineInfo* pInfo = (struct SMineInfo*)pvCtx;

  if ( 2 == sscanf( pszVal, " %31s = %"STRING(SIZE_MIME_TYPE)"[^;]",
      szMatch, pInfo->szMimeType)
  ) {
    if ( SWS_StrMatch( szMatch, pInfo->pszName, 1) )
      return 1;
    }
  return 0;
  }
#endif


/****************************************************************************
 * Platform specific code
 ****************************************************************************/

#if defined WIN32 || defined _WIN32 || defined __WIN32__ || defined WIN16 || defined _MSDOS || defined _DOS || defined _DOS16
/****************************************************************************
 * Win32/16 & MSDOS
 ****************************************************************************/

#include <io.h> /* _filelength */
#include <sys/stat.h> /* _fstat */

#if defined __BORLANDC__
#define _fileno fileno
#define _filelength filelength
#define _stat stat
#define _fstat fstat
#define _S_IFDIR S_IFDIR

#elif defined __WATCOMC__
#define _filelength filelength

#elif defined __MWERKS__
#define _S_IFDIR S_IFDIR

#endif


/*
 * Test if directory exists
 */
int PlatformIsDir( const char* pszPath)
  {
  struct _stat info;

  if ( _stat( pszPath, &info))
    return 0; /* Doesn't exist */

  return (info.st_mode & _S_IFDIR) ? 1 : 0;
  }


/*
 * Get file size
 */
long PlatformFileLength( FILE* file )
  {
  return _filelength( _fileno( file));;
  }


#if !OPT_HTTP_TINY
/*
 * Get the time when a file was last modified
 */
time_t PlatformFileTime( FILE* file )
  {
  struct _stat info;

  if ( _fstat( _fileno( file), &info))
    return INVALID_TIME;

  return info.st_mtime;
  }
#endif


#elif defined _POSIX_SOURCE || defined _POSIX_C_SOURCE
/****************************************************************************
 * POSIX
 ****************************************************************************/
#include <sys/stat.h> /* fstat */


/*
 * Test if directory exists
 */
int PlatformIsDir( const char* pszPath)
  {
  struct stat info;

  if ( stat( pszPath, &info))
    return 0; /* Doesn't exist */

  return S_ISDIR( info.st_mode) ? 1 : 0;
  }


/*
 * Get file size
 */
long PlatformFileLength( FILE* file )
  {
  struct stat info;

  if ( fstat( fileno( file), &info))
    return -1;

  return (long)info.st_size;
  }


#if !OPT_HTTP_TINY
/*
 * Get the time when a file was last modified
 */
time_t PlatformFileTime( FILE* file )
  {
  struct stat info;

  if ( fstat( fileno( file), &info))
    return INVALID_TIME;

  return info.st_mtime;
  }
#endif


#elif defined __DJGPP__
/****************************************************************************
 * DJGPP
 ****************************************************************************/
#include <io.h>    /* filelength */
#include <dos.h>   /* getftime */
#include <dir.h>   /* findfirst */


/*
 * Test if directory exists
 */
int PlatformIsDir( const char* pszPath)
  {
  struct ffblk info;
  return findfirst( pszPath, &info, FA_DIREC) ? 0 : 1;
  }


/*
 * Get file size
 */
long PlatformFileLength( FILE* file )
  {
  return filelength( fileno( file) );
  }


#if !OPT_HTTP_TINY
/*
 * Get the time when a file was last modified
 */
time_t PlatformFileTime( FILE* file )
  {
  struct ftime info;
  struct tm date;

  /* Get file timestamp (localtime), years since 1980 */
  if ( getftime( fileno( file), &info))
    return INVALID_TIME;

  date.tm_sec  = info.ft_tsec * 2;
  date.tm_min  = info.ft_min;
  date.tm_hour = info.ft_hour;
  date.tm_mday = info.ft_day;
  date.tm_mon  = info.ft_month - 1;
  date.tm_year = info.ft_year + 80; /* Years since 1900 */
  date.tm_wday = 0;
  date.tm_yday = 0;
  date.tm_isdst = -1;

  return mktime( &date); /* local to UTC */
  }
#endif


#else
/****************************************************************************
 * ANSI Standard C
 ****************************************************************************/

int PlatformIsDir( const char* pszPath)
  {
  {int WARN_PlatformIsDir_STUB;}
  return 0 == strncmp( s_szRoot, pszPath, sizeof s_szRoot);
  }


/*
 * Get file size
 */
long PlatformFileLength( FILE* file )
  {
  long len, pos;

  pos = ftell( file);
  if ( -1 == pos)
    return -1;

  len = 0 == fseek( file, 0, SEEK_END) ? ftell( file) : -1;
  fseek( file, pos, SEEK_SET);

  return len;
  }


#if !OPT_HTTP_TINY
/*
 * Get the time when a file was last modified
 */
time_t PlatformFileTime( FILE* file )
  {
  (void)file;
  return INVALID_TIME;
  }
#endif

#endif
/* End of file */
