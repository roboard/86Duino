/*****************************************************************************
 * Created by Lawrence Rust, Software Systems Consultants
 *
 * File name   : popmsg.c
 * Function    : POP3 message database
 * Authors     : Lawrence Rust
 * Systems     : ANSI C and Win32, DJGPP or POSIX
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 24-Mar-04, LVR, Split from popd.c
 * 27-Jul-09, LVR, Fix POSIX PlatformEnumFiles to return > 1 file
 *
 *
 * Comments
 * --------
 *
 *****************************************************************************/

/* Compilation options */


/* Exports */
#include "popmsg.h"

/* Imports */

/* ANSI C library */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*#include "sws_net.h"*/

/* App modules */
#include "sws_cfg.h"
#include "sws_str.h"


/***************************************************************************
 * Macros & constants
 ***************************************************************************/

/* Length of an array - sorely missing from stddef.h */
#ifndef lengthof
#define lengthof( _a) ((size_t)( sizeof(_a)/sizeof((_a)[0]) ))
#endif

/* Compile time assertion
 * Generates 'invalid array dimension' compiler error if condition false
 */
#undef STATIC_ASSERT
#ifdef __GNUC__
# define STATIC_ASSERT(e) extern char _dummy[(e) ? 1 : -1] __attribute__((unused))
#else
# define STATIC_ASSERT(e) extern char _dummy[(e) ? 1 : -1]
#endif

#define cNul       '\0'

#define kszRoot       "/"
#define kszLockFile   "lockfile"
#define kszMailFiles  "*.eml"

#ifndef MAX_PATH
#define MAX_PATH      256 /* Maximum pathname length */
#endif
#define MAX_ARG        41 /* Max arg length inc nul */

/* ini file values */
#define kszPopUsersSect   "pop_users"


/***************************************************************************
 * Data types
 ***************************************************************************/

/* Mail message info */
typedef struct SMsgInfo
  {
  char szPath[ MAX_PATH];
  unsigned long ulSize;
  int isDeleted;
  size_t nPath;
  } SMsgInfo;

/* User record */
struct SPopUser
  {
  char szName[ MAX_ARG];
  char szPassword[ MAX_ARG];
  char szInbox[ MAX_PATH];

  unsigned long ulBytes;
  unsigned uMessages;                   /* Message count */
  SMsgInfo msgArray[1];                 /* VLA of message info */
  };


/***************************************************************************
 * Function prototypes
 ***************************************************************************/

static int FindUser( SPopUser*, const char*);
static SWS_FCfgCallback EnumPopCallback;

typedef int FPlatformEnumMessages( void*, const char*, unsigned long);
static FPlatformEnumMessages EnumMessagesCallback;

static int PlatformEnumFiles( const char*, FPlatformEnumMessages*, void*);
static int PlatformCreateFile( const char*);
static int PlatformIsDir( const char*);
static int PlatformMkdir( const char*);


/***************************************************************************
 * Module data
 ***************************************************************************/
static const char* s_pszError = "";


/***************************************************************************
 * Functions
 ***************************************************************************/

/*
 * Verify user credentials
 */
extern int PopMsgCreateUser(
  SPopUser** ppUser,                    /* OUT: user identifier */
  const char* pszUser,                  /* IN: User name */
  const char* pszPassword,              /* IN: Password */
  int iVerbosity,                       /* IN: Logfile verbosity */
  FILE* file                            /* IN: Logfile or NULL */
) {
  SPopUser* pUser;
  size_t len;
  (void)iVerbosity;
  (void)file;

  *ppUser = NULL;

  /* Alloc user data */
  pUser = (SPopUser*)calloc( 1, sizeof(*pUser) );
  if ( NULL == pUser)
    {
    s_pszError = "Out of memory";
    return kPopMsgErrNoMem;
    }

  if ( !FindUser( pUser, pszUser)
    || (NULL != pszPassword && strcmp( pUser->szPassword, pszPassword))
    || (NULL == pszPassword && cNul != pUser->szPassword[ 0])
  ) {
    free( pUser);
    s_pszError = "Invalid login";
    return kPopMsgErrBadUser;
    }

  /* Create the inbox if necessary */
  if ( !PlatformIsDir( pUser->szInbox)
    && PlatformMkdir( pUser->szInbox)
  ) {
    free( pUser);
    s_pszError = "Mailbox unavailable";
    return kPopMsgErrFailed;
    }

  len = strlen( strcat( pUser->szInbox, kszRoot));

  /* Lock the inbox */
  strcpy( &pUser->szInbox[ len], kszLockFile);
  if ( !PlatformCreateFile( pUser->szInbox))
    {
    free( pUser);
    s_pszError = "Mailbox locked";
    return kPopMsgErrMbxLocked;
    }

  /* Enumerate messages */
  strcpy( &pUser->szInbox[ len], kszMailFiles);
  PlatformEnumFiles( pUser->szInbox, &EnumMessagesCallback, &pUser);
  pUser->szInbox[ len] = cNul;

  /* Return user handle */
  *ppUser = pUser;

  s_pszError = "";
  return kPopMsgErrNone;
  }

int EnumMessagesCallback(
  void* pvContext,
  const char* pszName,
  unsigned long ulSize
) {
  SPopUser** const ppUser = (SPopUser**)pvContext;
  SPopUser* pUser = *ppUser;
  SMsgInfo* pMsg;
  char* psz;

  /* Append to the list of messages */
  pUser = (SPopUser*) realloc( pUser, sizeof( *pUser) + sizeof( *pMsg) * pUser->uMessages);
  if ( NULL == pUser)
    return 1;

  pMsg = &pUser->msgArray[ pUser->uMessages++];

  psz = strrchr( pUser->szInbox, kszRoot[0]);
  if ( NULL != psz)
    {
    strncpy( pMsg->szPath, pUser->szInbox, psz - pUser->szInbox);
    pMsg->szPath[ psz - pUser->szInbox] = cNul;
    }
  else
    {
    strcpy( pMsg->szPath, pUser->szInbox);
    }
  strcat( pMsg->szPath, kszRoot);
  pMsg->nPath = strlen( pMsg->szPath);
  strcpy( pMsg->szPath + pMsg->nPath, pszName);

  pMsg->ulSize = ulSize;
  pMsg->isDeleted = 0;

  pUser->ulBytes += ulSize;

  *ppUser = pUser;

  return 0;
  }


/*
 * Destroy user
 */
extern void PopMsgDestroyUser( SPopUser* pUser)
  {
  if ( NULL != pUser)
    {
    /* Delete the lock file */
    strcat( pUser->szInbox, kszLockFile);
    remove( pUser->szInbox);

    free( pUser);
    }
  }


extern const char* PopMsgLastError( void)
  {
  return s_pszError;
  }


/*
 * Commit all operations
 */
extern int PopMsgCommit(
  SPopUser* pUser                       /* IN: User ID */
) {
  if ( NULL == pUser)
    return kPopMsgErrParam;

  return kPopMsgErrNone;
  }


/*
 * Get user name
 */
extern const char* PopMsgUserName(
  SPopUser* pUser                       /* IN: User ID */
) {
  if ( NULL == pUser)
    return NULL;

  return pUser->szName;
  }


/*
 * Get message count
 */
extern unsigned PopMsgCount(
  SPopUser* pUser                       /* IN: User ID */
) {
  if ( NULL == pUser)
    return 0;

  return pUser->uMessages;
  }


/*
 * Get total size of messages
 */
extern unsigned long PopTotalSize(
  SPopUser* pUser                       /* IN: User ID */
) {
  if ( NULL == pUser)
    return 0;

  return pUser->ulBytes;
  }


/*
 * Get message size
 */
extern unsigned long PopMsgSize(
  SPopUser* pUser,                      /* IN: User ID */
  unsigned uMsg                         /* IN: Message identifier, 1.. */
) {
  const SMsgInfo* pMsg;

  if ( NULL == pUser)
    return 0;

  if ( uMsg < 1 || uMsg > PopMsgCount( pUser))
    return 0;

  pMsg = &pUser->msgArray[ uMsg - 1];
  return pMsg->ulSize;
  }


/*
 * Get message unique ID
 */
extern const char* PopMsgUidl(          /* Returns 0= success */
  SPopUser* pUser,                      /* IN: User ID */
  unsigned uMsg                         /* IN: Message identifier, 1.. */
) {
  const SMsgInfo* pMsg;

  if ( NULL == pUser)
    return NULL;
  if ( uMsg < 1 || uMsg > pUser->uMessages)
    return NULL;

  pMsg = &pUser->msgArray[ uMsg - 1];
  return pMsg->szPath + pMsg->nPath;
  }


/*
 * Open a message for reading = fopen
 */
extern SPopMsg* PopMsgOpen(             /* Returns NULL on error */
  SPopUser* pUser,                      /* IN: User ID */
  unsigned uMsg                         /* IN: Message identifier, 1.. */
) {
  const SMsgInfo* pMsg;

  if ( NULL == pUser)
    return NULL;
  if ( uMsg < 1 || uMsg > pUser->uMessages)
    return NULL;

  pMsg = &pUser->msgArray[ uMsg - 1];
  return (SPopMsg*)fopen( pMsg->szPath, "r");
  }


/*
 * Read from an open message = fgets
 */
extern char* PopMsgRead(
  char* pBuf,                           /* OUT: data */
  int count,                            /* IN: buffer size */
  SPopMsg* pMsg                         /* IN */
) {
  return fgets( pBuf, count, (FILE*)pMsg);
  }


/*
 * Close an open message = fclose
 */
extern int PopMsgClose(
  SPopUser* pUser,                      /* IN: User ID */
  SPopMsg* pMsg                         /* IN */
) {
  (void)pUser;
  return 0 == fclose( (FILE*)pMsg) ? kPopMsgErrNone : kPopMsgErrParam;
  }


/*
 * Delete a message = remove
 */
extern int PopMsgDel(
  SPopUser* pUser,                      /* IN: User ID */
  unsigned uMsg                         /* IN: Message identifier, 1.. */
) {
  SMsgInfo* pMsg;

  if ( NULL == pUser)
    return -1;
  if ( uMsg < 1 || uMsg > pUser->uMessages)
    return -1;

  pMsg = &pUser->msgArray[ uMsg - 1];
  pMsg->isDeleted = 1;

  return 0 == remove( pMsg->szPath) ? kPopMsgErrNone : kPopMsgErrParam;
  }


/*
 * Undelete a message
 */
extern int PopMsgUndelete(
  SPopUser* pUser,                      /* IN: User ID */
  unsigned uMsg                         /* IN: Message identifier, 1.. */
) {
  SMsgInfo* pMsg;

  if ( NULL == pUser)
    return kPopMsgErrParam;

  if ( uMsg < 1 || uMsg > PopMsgCount( pUser))
    return kPopMsgErrParam;

  pMsg = &pUser->msgArray[ uMsg - 1];
  if ( pMsg->isDeleted)
    return kPopMsgErrFailed;

  return kPopMsgErrNone;
  }



extern int PopMsgIsDeleted(             /* Returns !0 if deleted */
  SPopUser* pUser,                      /* IN: User ID */
  unsigned uMsg                         /* IN: Message identifier, 1.. */
) {
  SMsgInfo* pMsg;

  if ( NULL == pUser)
    return 0;

  if ( uMsg < 1 || uMsg > PopMsgCount( pUser))
    return 0;

  pMsg = &pUser->msgArray[ uMsg - 1];
  return pMsg->isDeleted;
  }


/*
 * Get message header
 */
extern const char* PopMsgGetHdr(        /* Returns NULL on error */
  SPopUser* pUser,                      /* IN: User ID */
  unsigned uMsg                         /* IN: Message identifier, 1..PopMsgCount */
) {
  if ( NULL == pUser)
    return NULL;

  if ( uMsg < 1 || uMsg > PopMsgCount( pUser))
    return NULL;

  return NULL;
  }


/***************************************************************************
 * Support
 ***************************************************************************/

typedef struct SPopInfo
  {
  SPopUser* pUser;
  const char* pszUser;
  } SPopInfo;

/*
 * Find a POP3 mail record given a username
 */
int FindUser(                         /* !0 if successful */
  SPopUser* pUser,                    /* OUT: User record */
  const char* pszUser                 /* IN: Username */
) {
  SPopInfo info;

  info.pUser = pUser;
  info.pszUser = pszUser;
  if ( SWS_CfgEnum( kszPopUsersSect, &EnumPopCallback, &info) <= 0)
    return 0;

  if ( 0 == strcmp( "-", pUser->szPassword))
    *pUser->szPassword = cNul;

  return 1;
  }

SWS_CB( int, EnumPopCallback( void* pv, const char* pszVal))
  {
  SPopInfo* const pInfo = (SPopInfo*)pv;
  SPopUser* const pUser = pInfo->pUser;

  /* Parse mailbox entry */
  { STATIC_ASSERT( sizeof pUser->szName > 40);
    STATIC_ASSERT( sizeof pUser->szPassword > 40);
    STATIC_ASSERT( sizeof pUser->szInbox > 255); }
  if ( 3 == sscanf( pszVal, " %40s %40s %255s",
      pUser->szName, pUser->szPassword, pUser->szInbox)
    && 0 == SWS_stricmp( pInfo->pszUser, pUser->szName)
  ) {
    return 1;
    }

  return 0;
  }


/****************************************************************************
 * Platform specific code
 ****************************************************************************/

#if defined WIN32 || defined _WIN32 || defined __WIN32__ || defined WIN16 || defined _MSDOS || defined _DOS || defined _DOS16
/****************************************************************************
 * Win32/16 & MSDOS
 ****************************************************************************/

#include <io.h> /* _findfirst */
#include <fcntl.h>
#include <sys/stat.h>
#include <direct.h> /* _mkdir _rmdir */

/*
 * Enumerate files
 */
int PlatformEnumFiles(
  const char* pszPath,
  FPlatformEnumMessages* pfn,
  void* pv
) {
  int i = 0;
  struct _finddata_t info;
  long const hFind = _findfirst( pszPath, &info);

  if ( -1 ==  hFind)
    return 0;

  do
    {
    if ( 0 != strcmp( ".", info.name)
      && 0 != strcmp( "..", info.name)
      && NULL == strchr( info.name, '?') /* BUGBUG: Win2k SP4 Unicode filenames have ?'s */
    ) {
      ++i;
      if ( (*pfn)( pv, info.name, (unsigned long)info.size))
        break;
      }
    }
  while ( 0 == _findnext( hFind, &info) );

  _findclose( hFind);

  return i;
  }


/*
 * Create a file, fails if already exists
 */
int PlatformCreateFile( const char* pszPath)
  {
  int handle = _open( pszPath, _O_CREAT | _O_EXCL, _S_IREAD | _S_IWRITE);
  if ( -1 == handle)
    return 0;

  _close( handle);
  return 1;
  }

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
 * Make a directory
 */
int PlatformMkdir( const char* pszPath)
  {
  return _mkdir( pszPath);
  }

#elif defined _POSIX_SOURCE || defined _POSIX_C_SOURCE
/****************************************************************************
 * POSIX
 ****************************************************************************/

#undef fd_set
#undef socklen_t
#undef gethostname

#include <dirent.h>  /* opendir */
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef __DJGPP__
/* Disable expensive unneeded fields returned by stat */
unsigned short _djstat_flags
  = _STAT_INODE
  | _STAT_EXEC_EXT
  | _STAT_EXEC_MAGIC
  | _STAT_DIRSIZE
  | _STAT_ROOT_TIME
  ;
#endif

#define kszMatchAny "*.*"


/*
 * Enumerate files
 */
int PlatformEnumFiles(
  const char* pszPath,
  FPlatformEnumMessages* pfn,
  void* pv
) {
  int iRet = 0;
  DIR* dir;
  struct dirent* de;
  const char* pszName;
  char szPath[ MAX_PATH];

  /* Is the path a directory? */
  dir = opendir( pszPath);
  if ( NULL != dir)
    {
    /* Yes, all files */
    pszName = kszMatchAny;
    }
  else
    {
    /* No, split into pathname and filespec */
    char* psz = (char*)strrchr( pszPath, '/');
    if ( NULL == psz)
      return 0; /* No such directory */

    strncpy( szPath, pszPath, psz - pszPath);
    szPath[ psz - pszPath] = cNul;
    pszPath = szPath;
    dir = opendir( szPath);
    if ( NULL == dir)
      return 0; /* No such directory */

    pszName = ++psz;
    if ( cNul == *pszName)
      pszName = kszMatchAny;
    }

  while ( NULL != (de = readdir( dir)) )
    {
    if ( 0 != strcmp( ".", de->d_name)
      && 0 != strcmp( "..", de->d_name)
      && (0 == strcmp( pszName, kszMatchAny) || SWS_StrMatch( pszName, de->d_name, 0))
    ) {
      struct stat sbuf;
      char szName[ MAX_PATH];

      strcat( strcat( strcpy( szName, pszPath), kszRoot), de->d_name);
      if ( !stat( szName, &sbuf) )
        {
        ++iRet;
        if ( (*pfn)( pv, de->d_name, (unsigned long)sbuf.st_size))
          break;
        }
      }
    }

  closedir( dir);

  return iRet;
  }


/*
 * Create a file, fails if already exists
 */
int PlatformCreateFile( const char* pszPath)
  {
  int handle = open( pszPath, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if ( -1 == handle)
    return 0;

  close( handle);
  return 1;
  }


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
 * Make a directory
 */
int PlatformMkdir( const char* pszPath)
  {
  return mkdir( pszPath,
    S_IWUSR | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
  }

#elif defined __DJGPP__
/****************************************************************************
 * DJGPP
 ****************************************************************************/

#undef fd_set
#undef socklen_t
#undef gethostname

#include <unistd.h>
#include <dir.h> /* findfirst */
#include <fcntl.h>
#include <sys/stat.h>

/*
 * Enumerate files
 */
int PlatformEnumFiles(
  const char* pszPath,
  FPlatformEnumMessages* pfn,
  void* pv
) {
  struct ffblk info;
  int i = 0, done;

  done = findfirst(
    pszPath,
    &info,
    (0*FA_HIDDEN) | (0*FA_SYSTEM) | FA_RDONLY | FA_DIREC | FA_ARCH
  );

  while ( !done)
    {
    if ( 0 != strcmp( ".", info.ff_name)
      && 0 != strcmp( "..", info.ff_name)
    ) {
      ++i;
      if ( (*pfn)( pv, info.ff_name, (unsigned long)info.ff_fsize))
        break;
      }

    done = findnext( &info);
    }

  return i;
  }


/*
 * Create a file, fails if already exists
 */
int PlatformCreateFile( const char* pszPath)
  {
  int handle = open( pszPath, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if ( -1 == handle)
    return 0;

  close( handle);
  return 1;
  }


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
 * Make a directory
 */
int PlatformMkdir( const char* pszPath)
  {
  return mkdir( pszPath,
    S_IWUSR | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
  }

#else

/* Enumerate files */
int PlatformEnumFiles(
  const char* pszPath,
  FPlatformEnumMessages* pfn,
  void* pv
) {
  (void)pszPath;
  (void)pfn;
  (void)pv;
  {int WARN_PlatformEnumFiles_STUB;}
  return 0;
  }


/* Create a file, fails if already exists */
int PlatformCreateFile( const char* pszPath)
  {
  (void)pszPath;
  return 0;
  }


/* Test if directory exists */
int PlatformIsDir( const char* pszPath)
  {
  (void)pszPath;
  return 0;
  }


/* Make a directory */
int PlatformMkdir( const char* pszPath)
  {
  (void)pszPath;
  return -1;
  }

#endif

/* End of file */
