/*****************************************************************************
 * Created by Lawrence Rust, Software Systems Consultants
 *
 * File name   : sws_cfg.c
 * Function    : configuration data access
 * Project     : Various
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 29-Jan-03, LVR, Created.
 * 14-Sep-03, LVR, v1.00 - Moved SWS_CFG_MAX_KEY to sws_cfg.h
 * 19-Nov-04, LVR, v1.03 - Added SWS_CfgSetString
 * 30-Nov-04, LVR, v1.04 - Added SWS_CfgSetValue
 * 05-Dec-04, LVR, v1.04 - Made SWS_CfgEnum re-entrant
 * 04-Mar-05, LVR, v1.04 - Add GetIniFile
 * 12-May-05, LVR, v1.04 - Add isCreate param to GetIniFile
 * 18-Jul-09, LVR, v1.04 - Support SWS_CFG_SET on Linux
 * 27-Jan-10, LVR, v1.05 - SWS_CfgSetName parses file, SWS_CfgEnum uses cache
 * 19-Jul-10, LVR, v1.06 - Add SWS_CfgGetName
 *
 *
 * Comments
 * --------
 *
 *****************************************************************************/

/* Compilation options */
#ifndef SWS_CFG_SET
#define SWS_CFG_SET 0     /* !0 for SWS_CfgSetString & SWS_CfgSetValue */
#endif

#if defined _MSC_VER
# pragma warning ( disable : 4711) /* function selected for automatic inline expansion */
#endif
#if defined _MSC_VER && _MSC_VER <= 800 /* VC++ 1.5 & earlier */
# pragma warning( disable : 4705)   /* statement has no effect */
#endif

#if SWS_CFG_SET && defined __GNUC__
#undef __STRICT_ANSI__ /* for fileno in stdio.h */
#endif


/* Exports */
#include "sws_cfg.h"

/* Imports */

/* ANSI C library */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#if SWS_CFG_SET && defined _MSC_VER
#include <io.h> /* tell & chsize */
#endif
#if SWS_CFG_SET && defined __GNUC__
#include <unistd.h> /* fileno & ftruncate */
#endif


/***************************************************************************
 * Macros & constants
 ***************************************************************************/

/* Expand and stringize a macro name */
#define STRING_(s) #s
#define STRING(s) STRING_(s)

/* Compile time assertion
 * Generates 'invalid array dimension' compiler error if condition false
 */
#undef STATIC_ASSERT
#ifdef __GNUC__
# define STATIC_ASSERT(e) extern char _dummy[(e) ? 1 : -1] __attribute__((unused))
#else
# define STATIC_ASSERT(e) extern char _dummy[(e) ? 1 : -1]
#endif

#define ALPHANUM "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"\
  "!\"#$%&'()*+,-./:;<>?@\\^_`{|}~"

#define kszModeRO "r"  /* File open mode */
#if SWS_CFG_SET
#define kszModeRW "r+" /* File open mode */
#define kszModeWR "w+" /* File open mode */
#endif


/***************************************************************************
 * Data types
 ***************************************************************************/

/* EnumIniStringCallback context info */
typedef struct SGetIniString
  {
  const char* pszVal;
  char* pszBuf;
  size_t len;
  } SGetIniString;

/* SetStringCallback context info */
typedef struct SSetIniString
  {
  const char* pszKey;
  const char* pszName;
  const char* pszVal;
  char* buffer;
  size_t len;
  int isKey, ins;
  } SSetIniString;

/* A string value */
struct SValue
  {
  char* pszData;
  };

/* A section */
struct SSection
  {
  char szName[ SWS_CFG_MAX_KEY + 1];
  unsigned uCount;
  struct SValue* aVal; /* An array of values */
  size_t size;
  };

/* An array of sections */
struct SSectionList
  {
  unsigned uCount;
  struct SSection* aSect; /* Sorted array */
  size_t size;
  const struct SSection* pRecent; /* Most recently found section */
  };


/***************************************************************************
 * Function prototypes
 ***************************************************************************/
static FILE* GetIniFile( SWS_PTR( const char), int);

static SWS_FCfgCallback EnumIniStringCallback;
#if SWS_CFG_SET
static int EnumFile( FILE*, SWS_FNPTR( SWS_FCfgCallback), SWS_PTR( void) );
static SWS_FCfgCallback SetStringCallback;
#endif

static void BuildTable( struct SSectionList*, FILE*);
static void FreeTable( struct SSectionList*);
static struct SSection* FindSection( struct SSectionList*, const char*, int);
static struct SValue* AddValue( struct SSection*, const char*);
static int CmpSection( const void*, const void*);
static void CfgOnExit( void);

static int Stricmp( SWS_PTR( const char), SWS_PTR( const char) );


/***************************************************************************
 * Module data
 ***************************************************************************/

static struct SSectionList s_sections; /* The named section cache */
static char s_szFilename[ 256];


/***************************************************************************
 * Exported Functions
 ***************************************************************************/

/*
 * Set ini file
 */
SWS_EXTERN( int, SWS_CfgSetName(
  SWS_PTR( const char) pszName,       /* IN: config filename */
  SWS_PTR( const char) pszPath        /* IN: argv[0] */
)){
  FILE* file;
  char szBuf[ 256];
  char* pszSep;

  if ( NULL == pszName)
    pszName = SWS_CFGFILE;

  /* Attempt to open filename as supplied */
  file = GetIniFile( pszName, 0);

  /* If failed then prepend any supplied path */
  if ( NULL == file                   /* File not found? */
    && *pszName                       /* Non-empty name? else just close */
    && NULL != pszPath                /* Path provided? */
  ) {
    /* Extract path from argv[0] */
    if ( '\"' == *pszPath)
      sscanf( pszPath + 1, "%255[^\"]", szBuf);
    else
      strncpy( szBuf, pszPath, sizeof(szBuf) - 1);

    /* Append filename to path */
    pszSep = strrchr( szBuf, '\\');
    if ( NULL == pszSep)
      pszSep = strrchr( szBuf, '/');

    if ( NULL != pszSep)
      {
      strncpy( pszSep + 1, pszName, sizeof szBuf - (size_t)((pszSep + 1) - szBuf));
      }
    else
      {
      strcat( szBuf, "/");
      strcat( szBuf, pszName);
      }

    /* Open filename prefixed with supplied path */
    file = GetIniFile( szBuf, 0);
    }

  /* If failed then try prepending PATH EV */
  if ( NULL == file
    && *pszName                       /* Non-empty name? else just close */
    && NULL != pszPath                /* Path provided? */
    && NULL != (pszSep = getenv( "PATH"))
  ) {
    /* Search each element of PATH */
    for ( pszSep = strtok( pszSep, ";:");
      pszSep != NULL;
      pszSep = strtok( NULL, ";:")
    ) {
      sprintf( szBuf, "%.224s/%.31s", pszSep, pszName);
      file = GetIniFile( szBuf, 0);
      if ( NULL != file)
        break;
      }
    }

  return NULL != file;
  }


/*
 * Get ini file
 */
SWS_EXTERN( SWS_PTR( const char), SWS_CfgGetName( void))
  {
  return s_szFilename;
  }


/*
 * Enumerate configuration strings
 */
SWS_EXTERN( int, SWS_CfgEnum(         /* Returns !0 if found */
  SWS_PTR( const char) pszKey,        /* IN: Section name */
  SWS_FNPTR( SWS_FCfgCallback) pfn,   /* IN: Callback */
  SWS_PTR( void) pv                   /* IN: Callback context */
)){
  int iRet = 0;
  const struct SSectionList* const pList = &s_sections;
  const struct SSection* pSect;

  /* Does key match the most recently found section? */
  pSect = s_sections.pRecent;
  if ( NULL == pSect || strcmp( pSect->szName, pszKey))
    {
    /* No, search for the section by name */
    /* NB passing pszKey as the key to bsearch requires the following */
    STATIC_ASSERT( 0 == offsetof( struct SSection, szName));
    pSect = (const struct SSection*)bsearch(
      pszKey, pList->aSect, pList->uCount, sizeof pList->aSect[0], CmpSection);
    }

  if ( NULL != pSect)
    {
    const struct SValue* pVal;
    unsigned u;

    /* Save most recently found section */
    s_sections.pRecent = pSect;

    /* Iterate over all values in the section */
    for ( pVal = &pSect->aVal[ u = 0]; u < pSect->uCount; ++u, ++pVal)
      {
      /* Callback the client */
      iRet = (*pfn)( pv, pVal->pszData);
      if ( iRet)
        break;
      }
    }

  return iRet;
  }


/*
 * Get a configuration string
 */
SWS_EXTERN( int, SWS_CfgGetString(    /* Returns no. chars read */
  SWS_PTR( const char) pszKey,        /* IN: Section name */
  SWS_PTR( const char) pszVal,        /* IN: Value name */
  SWS_PTR( const char) pszDefault,    /* IN: Default value */
  SWS_PTR( char) pszBuf,              /* OUT: String buffer */
  size_t len                          /* IN: String buffer size */
)){
  SGetIniString info;

  info.pszVal = pszVal;
  info.pszBuf = pszBuf;
  info.len = len;

  if ( SWS_CfgEnum( pszKey, &EnumIniStringCallback, &info) <= 0)
    {
    /* Not found so use default */
    if ( NULL != pszDefault)
      {
      info.len = strlen( pszDefault);
      if ( len < info.len)
        info.len = len;
      strncpy( pszBuf, pszDefault, len);
      }
    else
      {
      info.len = 0;
      }
    }

  return (int)info.len;
  }


#if SWS_CFG_SET
/*
 * Set a configuration string
 */
SWS_EXTERN( int, SWS_CfgSetString(    /* 0= fail */
  SWS_PTR( const char) pszKey,        /* IN: Section name */
  SWS_PTR( const char) pszName,       /* IN: Value name (maybe NULL) */
  SWS_PTR( const char) pszVal         /* IN: Value */
)){
  int iRet = 0;
  FILE* const file = GetIniFile( NULL, 1);

  if ( NULL != file)
    {
    SSetIniString info;

    /* Reposition to start of file */
    rewind( file);

    info.pszKey = pszKey;
    info.pszName = pszName;
    info.pszVal = pszVal;
    info.buffer = NULL;
    info.len = 0;
    info.isKey = 0;
    info.ins = 0;

    if ( 0 == EnumFile( file, &SetStringCallback, &info))
      {
      /* Append section & value if not found */
      if ( !info.ins)
        {
        char szBuf[ SWS_CFG_MAX_KEY + 4];
        sprintf( szBuf, "\n[%." STRING( SWS_CFG_MAX_KEY) "s]", pszKey);
        iRet = SetStringCallback( &info, szBuf);
        if ( iRet)
          goto err;
        }

      if ( info.ins)
        {
        /* Update the file with the amended contents */
        rewind( file);
        iRet = (1 == fwrite( info.buffer, info.len, 1, file));

        /* BUGBUG: Want to truncate the file but ANSI C has no method */
#if defined _MSC_VER
        {
        int ifd = fileno( file);
        fflush( file);
        chsize( ifd, tell( ifd));
        }
#elif defined __GNUC__
        fflush( file);
        ftruncate( fileno( file), ftell( file));
#else
        fwrite( "\n[]\n", 4, 1, file);
        fflush( file);
#endif
        /* Dispose of cached data */
        FreeTable( &s_sections);
        }
      }

  err:
    free( info.buffer);
    }

  return iRet;
  }
#endif


 /*
 * Get a configuration value
 */
SWS_EXTERN( ui32, SWS_CfgGetValue(    /* Returns the value */
  SWS_PTR( const char) pszKey,        /* IN: Section name */
  SWS_PTR( const char) pszVal,        /* IN: Value name */
  ui32 uDefault                       /* IN: Default value */
)){
  char szBuf[ 32];
  char* pszEnd;
  unsigned long ulVal;

  if ( 0 == SWS_CfgGetString( pszKey, pszVal, "", szBuf, sizeof( szBuf) ))
    return uDefault;

  ulVal = strtoul( szBuf, &pszEnd, 0);
  if ( pszEnd == szBuf /* No conversion */
    || (';' != *pszEnd && isgraph( *pszEnd)) /* Non-numeric tail */
  ) {
    return uDefault;
    }

  return ulVal;
  }


#if SWS_CFG_SET
/*
 * Set a configuration value
 */
SWS_EXTERN( int, SWS_CfgSetValue(     /* Returns !0 on success */
  SWS_PTR( const char) pszKey,        /* IN: Section name */
  SWS_PTR( const char) pszVal,        /* IN: Value name */
  ui32 uVal                           /* IN: value */
)){
  char szBuf[ 32];

  sprintf( szBuf, "%lu", uVal);
  return SWS_CfgSetString( pszKey, pszVal, szBuf);
  }
#endif


/***************************************************************************
 * Local Functions
 ***************************************************************************/

/*
 * SWS_CfgEnum callback from SWS_CfgGetString
 */
SWS_CB( int, EnumIniStringCallback( SWS_PTR( void) pv, SWS_PTR( const char) pszVal))
  {
  SGetIniString* const pInfo = (SGetIniString*) pv;
  char szBuf[ SWS_CFG_MAX_KEY + 1];
  int count = 0;

  /* Find the value */
  if ( 1 <= sscanf(
      pszVal,
      " %" STRING( SWS_CFG_MAX_KEY) "[" ALPHANUM "] =%n %n",
      szBuf, &count, &count
    )
    && count > 0
    && 0 == Stricmp( szBuf, pInfo->pszVal)
  ) {
    /* Found the value */
    const char* psz = pszVal + count;

    /* Skip leading space */
    while ( isspace( *psz) )
      ++psz;

    strncpy( pInfo->pszBuf, psz, pInfo->len);

    count = (int)strlen( psz);
    if ( (size_t)count < pInfo->len)
      pInfo->len = (size_t)count;

    return 1;
    }

  return 0;
  }


#if SWS_CFG_SET
/*
 * Enumerate all lines in a file
 */
static int EnumFile(
  FILE* file,
  SWS_FNPTR( SWS_FCfgCallback) pfn,   /* IN: Callback */
  SWS_PTR( void) pv                   /* IN: Callback context */
) {
  char szBuf[ SWS_CFG_MAX_LINE + 1];
  int iRet = 0;

  /* Read a line at a time until EOF */
  while ( NULL != fgets( szBuf, sizeof( szBuf) - 1, file) )
    {
    char* psz;

    /* Zap the trailing newline */
    psz = strchr( szBuf, '\n');
    if ( NULL != psz)
      *psz = '\0';
    psz = strchr( szBuf, '\r');
    if ( NULL != psz)
      *psz = '\0';

    /* Callback the client */
    iRet = (*pfn)( pv, szBuf);
    if ( iRet)
      break;
    }

  return iRet;
  }


/*
 * EnumFile callback from SWS_CfgSetString
 */
SWS_CB( int, SetStringCallback( void* pv, const char* pszVal))
  {
  SSetIniString* const pInfo = (SSetIniString*) pv;
  size_t len;
  void* p;
  int i = 0;
  char szKey[ SWS_CFG_MAX_KEY + 1];

  /* Append line to buffer */
  p = realloc( pInfo->buffer, pInfo->len + SWS_CFG_MAX_LINE + 2);
  if ( NULL == p)
    return 1;

  pInfo->buffer = (char*)p;
  len = sprintf( pInfo->buffer + pInfo->len, "%." STRING( SWS_CFG_MAX_LINE) "s\n", pszVal);
  pInfo->len += len;

  /* Is this a section heading? */
  if ( 1 <= sscanf(
      pszVal,
      " [ %" STRING( SWS_CFG_MAX_KEY) "[" ALPHANUM "] ]%n",
      szKey, &i
    )
    && i > 0
  ) {
    /* Yes, is it the requested section? */
    pInfo->isKey = 0 == Stricmp( szKey, pInfo->pszKey);
    if ( pInfo->isKey)
      {
      /* Yes */
      len = 1 + pInfo->len; /* 1 for trailing '\0' */
      if ( NULL != pInfo->pszName)
        len += 1 + strlen( pInfo->pszName); /* 1 for '=' */
      if ( NULL != pInfo->pszVal)
        len += 1 + strlen( pInfo->pszVal); /* 1 for trailing '\n' */

      /* Expand the buffer */
      p = realloc( pInfo->buffer, len);
      if ( NULL == p)
        return 1;

      pInfo->buffer = (char*) p;

      if ( NULL != pInfo->pszName && NULL != pInfo->pszVal)
        {
        /* Insert the new name & value */
        pInfo->len += sprintf(
          pInfo->buffer + pInfo->len,
          "%." STRING( SWS_CFG_MAX_KEY) "s=%s\n",
          pInfo->pszName, pInfo->pszVal
        );
        }
      else if ( NULL != pInfo->pszVal)
        {
        /* Insert the new value */
        pInfo->len += sprintf( pInfo->buffer + pInfo->len, "%s\n", pInfo->pszVal);
        }

      pInfo->ins = 1;
      }
    }
  /* Was the section name matched? */
  else if ( pInfo->isKey)
    {
    /* Yes, is value name specified? */
    if ( NULL != pInfo->pszName)
      {
      /* Yes, get value name */
      if ( 1 <= sscanf(
          pszVal,
          " %" STRING( SWS_CFG_MAX_KEY) "[" ALPHANUM "] =%n %n",
          szKey, &i, &i
        )
        && i > 0
      ) {
        /* Matching value name? */
        if ( 0 == Stricmp( szKey, pInfo->pszName) )
          {
          /* Yes, delete the old value */
          pInfo->len -= len;
          }
        }
      }
    /* No name, matching value? */
    else if ( NULL != pInfo->pszVal
      && 0 == strcmp( pszVal, pInfo->pszVal)
    ) {
      /* Yes, delete the old value */
      pInfo->len -= len;
      }
    else if ( NULL == pInfo->pszVal /* Delete section? */
      && *pszVal
      && ';' != *pszVal
    ) {
      /* Delete this line */
      pInfo->len -= len;
      }
    }

  return 0;
  }
#endif


/*
 * Get/set ini file
 */
static FILE* GetIniFile( SWS_PTR( const char) pszName, int isCreate)
  {
  static FILE * s_pIniFile;
#if !SWS_CFG_SET
  (void)isCreate;
#endif

  if ( NULL != pszName)
    {
    if ( NULL != s_pIniFile)
      {
      fclose( s_pIniFile);
      s_pIniFile = NULL;
      }
    if ( s_sections.uCount)
      FreeTable( &s_sections);

    /* If filename is empty just close file */
    if ( !*pszName)
      return NULL;
    }

  if ( NULL == s_pIniFile)
    {
    if ( NULL == pszName)
      pszName = SWS_CFGFILE;

    if ( s_sections.uCount)
      FreeTable( &s_sections);

#if SWS_CFG_SET
    /* Open existing file r/w */
    s_pIniFile = fopen( pszName, kszModeRW);
    if ( NULL == s_pIniFile)
      {
      /* Failed so try creating */
      if ( isCreate)
        s_pIniFile = fopen( pszName, kszModeWR);

      if ( NULL == s_pIniFile)
        {
        /* Failed so try opening existing file r/o */
        s_pIniFile = fopen( pszName, kszModeRO);
        }
      }
#else
    /* Open existing file r/o */
    s_pIniFile = fopen( pszName, kszModeRO);
#endif

    if ( NULL != s_pIniFile)
      {
      static int s_once;

      strncpy( s_szFilename, pszName, sizeof s_szFilename);

      /* Parse the file and build a list of sections/values */
      BuildTable( &s_sections, s_pIniFile);

      if ( 0 == s_once)
        {
        s_once = 1;
        atexit( &CfgOnExit);
        }

#if !SWS_CFG_SET
      /* The open file is no longer required */
      fclose( s_pIniFile);
      s_pIniFile = NULL;
      return (FILE*)1;
#endif
      }
    }

  return s_pIniFile;
  }

static void CfgOnExit( void)
  {
  /* Close the ini file and dispose of section list */
  GetIniFile( "", 0);
  }


/*
 * Parse the ini file and build a sorted list of named sections
 */
static void BuildTable( struct SSectionList* pList, FILE* file)
  {
  char szBuf[ SWS_CFG_MAX_LINE + 1];
  struct SSection* pSection = NULL;

  rewind( file);

  /* Read a line at a time until EOF */
  while ( NULL != fgets( szBuf, sizeof szBuf - 1, file) )
    {
    char szKey[ SWS_CFG_MAX_KEY + 1];
    int i = 0;

    /* Check for a section (key) name */
    if ( 1 <= sscanf( szBuf, " [ %"STRING(SWS_CFG_MAX_KEY)"[" ALPHANUM "] ]%n", szKey, &i))
      {
      /* Add the section */
      pSection = i > 0 ? FindSection( pList, szKey, 1) : NULL;
      }
    else if ( NULL != pSection)
      {
      char* psz = szBuf;
      while ( isspace( *psz))
        ++psz;
      /* Add the string to the current section */
      if ( '\0' != *psz && ';' != *psz)
        AddValue( pSection, psz);
      }
    }

  qsort( pList->aSect, pList->uCount, sizeof pList->aSect[0], CmpSection);
  }


/*
 * Dispose of the section list
 */
static void FreeTable( struct SSectionList* pList)
  {
  struct SSection* pSect;
  unsigned u;

  for ( pSect = &pList->aSect[ u = 0]; u < pList->uCount; ++u, ++pSect)
    {
    struct SValue* pVal;
    unsigned v;

    for ( pVal = &pSect->aVal[ v = 0]; v < pSect->uCount; ++v, ++pVal)
      free( pVal->pszData), pVal->pszData = NULL;

    free( pSect->aVal), pSect->aVal = NULL;
    }

  pList->uCount = 0;
  pList->size = 0;
  pList->pRecent = NULL;
  free( pList->aSect), pList->aSect = NULL;
  }


/*
 * Find or create a named section
 */
static struct SSection* FindSection( struct SSectionList* pList, const char* pszSection, int isCreated)
  {
  struct SSection* pSect;
  unsigned u;

  for ( pSect = &pList->aSect[ u = 0]; u < pList->uCount; ++u, ++pSect)
    {
    if ( 0 == Stricmp( pszSection, pSect->szName))
      return pSect;
    }

  if ( !isCreated)
    return NULL;

  if ( pList->uCount * sizeof pList->aSect[0] >= pList->size)
    {
    /* Extend the array of sections */
    enum { kSections = 8 };
    size_t size = pList->size ? pList->size * 2  : kSections * sizeof pList->aSect[0];
    void* pv = realloc( pList->aSect, size);
    if ( NULL == pv)
      return NULL;

    pList->aSect = (struct SSection*)pv;
    pList->size = size;
    }
  pSect = &pList->aSect[ pList->uCount++];
  strncpy( pSect->szName, pszSection, sizeof pSect->szName);
  pSect->uCount = 0;
  pSect->aVal = NULL;
  pSect->size = 0;
  return pSect;
  }


/*
 * Append a value to a section
 */
static struct SValue* AddValue( struct SSection* pSect, const char* pszValue)
  {
  struct SValue* pVal;

  if ( pSect->uCount * sizeof pSect->aVal[0] >= pSect->size)
    {
    /* Extend the array of values */
    enum { kValues = 32 };
    size_t size = pSect->size ? pSect->size * 2  : kValues * sizeof pSect->aVal[0];
    void* pv = realloc( pSect->aVal, size);
    if ( NULL == pv)
      return NULL;

    pSect->aVal = (struct SValue*)pv;
    pSect->size = size;
    }

  pVal = &pSect->aVal[ pSect->uCount++];
  pVal->pszData = (char*)malloc( strlen( pszValue) + 1);
  if ( NULL == pVal->pszData)
    {
    pSect->uCount--;
    return NULL;
    }
  sscanf( pszValue, "%[^\r\n]", pVal->pszData);
  return pVal;
  }


/*
 * qsort/bsearch callback to compare section names
 */
static int CmpSection( const void* pvKey, const void* pvMmbr)
  {
  return strcmp(
    ((const struct SSection*)pvKey)->szName, ((const struct SSection*)pvMmbr)->szName);
  }


/*
 * Case insensitive string compare
 */
static int Stricmp( SWS_PTR( const char) psz1, SWS_PTR( const char) psz2)
  {
  int c1, c2;

  do
    {
    c1 = tolower( *psz1);
    ++psz1;
    c2 = tolower( *psz2);
    ++psz2;
    }
  while ( c1 == c2 && '\0' != c1 && '\0' != c2 );

  return c1 - c2;
  }

/* End of file */
