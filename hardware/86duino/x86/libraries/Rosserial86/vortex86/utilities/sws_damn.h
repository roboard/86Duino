/****************************************************************************
 *
 * File name   : sws_damn.h
 * Function    : Linux daemon.support
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, Linux
 *
 ****************************************************************************
 * File History
 * ------------
 * 21-Jul-09, LVR, v1.00 - Created
 * 05-Mar-10, LVR, v1.01 - Added SWS_DaemonSetGroup
 *
 *
 * Comments:
 * --------
 ****************************************************************************/
#ifndef SWS_DAEMON_H
#define SWS_DAEMON_H 0x0101

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <sys/stat.h> /* ustat */
#include <sys/wait.h> /* waitpid */
#include <unistd.h> /* chdir */
#ifdef DAEMON_SETID
#include <grp.h> /* initgroups, setgroups */
#include <pwd.h> /* getpwnam */
#endif

static const char* SWS_DaemonPidName;
static void SWS_DaemonExit( void);
#ifdef DAEMON_SETID
static int SWS_DaemonChown( FILE* file, const char* pszAcct);
#endif


/* Fork off the daemon and terminate the parent */
static int SWS_Daemon( void)
  {
  pid_t pid; /* Process ID */

  /* Fork off the parent process */
  pid = fork();
  switch ( pid )
    {
  case -1: /* Error */
    perror( "SWS_Daemon failed to fork");
    return -1;

  case 0: /* The child */
    break;

  default: /* The parent */
#if DEBUG
    printf( "Created daemon with PID %u\n", (unsigned)pid );
#endif
    exit( EXIT_SUCCESS);
    }

  /* Create a new session ID for the child process */
  if ( -1 == setsid() )
    {
    perror( "SWS_Daemon setsid failed");
    return -1;
    }

  /* Change the file mode mask */
#ifndef DAEMON_UMASK
#define DAEMON_UMASK 022 /* 022 results in -rw-r--r-- access rights */
#endif
  umask( DAEMON_UMASK );

#ifdef DAEMON_CLOSE_FILES
  /* Close the standard file descriptors */
  close( STDIN_FILENO ); /* Used by httpd/popen for CGI */
  close( STDOUT_FILENO ); /* Redirected for log output */
#endif

  return 0;
  }


/* Create a file (typically in /var/run) containing the PID */
static int SWS_DaemonPidFile( const char* pszPid, const char* pszAcct)
  {
  FILE* file;

  if ( NULL == pszPid || '\0' == *pszPid)
    return -1;

  file = fopen( pszPid, "w");
  if ( NULL == file)
    {
    perror( "SWS_DaemonPidFile failed to create file");
    return -1;
    }

  fprintf( file, "%u\n", (unsigned)getpid() );

#ifdef DAEMON_SETID
  /* Change file ownership */
  if ( pszAcct)
    SWS_DaemonChown( file, pszAcct);
#else
  (void)pszAcct;
#endif

  fclose( file);

  /* Setup an exit callback to delete the file */
  SWS_DaemonPidName = pszPid;
  atexit( &SWS_DaemonExit);

  return 0;
  }


/* atexit handler */
static void SWS_DaemonExit( void)
  {
  if ( SWS_DaemonPidName)
    remove( SWS_DaemonPidName);
  }


#ifdef DAEMON_SETID
/* Set the group id & additional group set for an acct */
static int SWS_DaemonSetGid( const char* pszAcct)
  {
  int iRet = -1;
  struct passwd* pwd;

  if ( NULL == pszAcct)
    return -1;
  if ( '\0' == *pszAcct)
    return 0;

  errno = 0;
  pwd = getpwnam( pszAcct);
  if ( NULL != pwd)
    {
    enum { kGroups = 32 };
    gid_t groups[ kGroups];
    int n;

    /* Set the group set */
    if ( initgroups( pwd->pw_name, pwd->pw_gid))
      perror( "SWS_DaemonSetGid: initgroups failed");
    else if ( 0 > (n = getgroups( kGroups, groups)))
      perror( "SWS_DaemonSetGid: getgroups failed");
    else if ( setgroups( (size_t)n, groups))
      perror( "SWS_DaemonSetGid: setgroups failed");

    /* Set the current group */
    if ( setgid( pwd->pw_gid))
      perror( "SWS_DaemonSetGid: setgid failed");
    else
      iRet = 0;
    }
  else if ( errno)
    perror( "SWS_DaemonSetGid: getpwnam failed");
  else
    fprintf( stderr, "SWS_DaemonSetGid: user '%s' not known\n", pszAcct);

  return iRet;
  }


/* Set the user id for an acct */
static int SWS_DaemonSetUid( const char* pszAcct)
  {
  struct passwd* pwd;
  int iRet = -1;

  if ( NULL == pszAcct)
    return -1;
  if ( '\0' == *pszAcct)
    return 0;

  errno = 0;
  pwd = getpwnam( pszAcct);
  if ( NULL != pwd)
    {
    /* Set the current group */
    if ( setgid( pwd->pw_gid))
      perror( "SWS_DaemonSetUid: setgid failed");

    /* Irrevocably drop root privileges */
    if ( setuid( pwd->pw_uid))
      perror( "SWS_DaemonSetUid failed");
    else
      iRet = 0; /* success */

    if ( NULL != pwd->pw_dir && pwd->pw_dir[0] && chdir( pwd->pw_dir ) )
      fprintf( stderr, "SWS_DaemonSetUid failed to chdir(%s): %s\n", pwd->pw_dir, strerror( errno) );
    }
  else if ( errno)
    perror( "SWS_DaemonSetUid: getpwnam failed");
  else
    fprintf( stderr, "SWS_DaemonSetUid: user '%s' not known\n", pszAcct);

  return iRet;
  }


/* Set the owner & group of a file */
static int SWS_DaemonChown( FILE* file, const char* pszAcct)
  {
  int iRet = -1;
  struct passwd* pwd;

  if ( NULL == pszAcct)
    return -1;
  if ( '\0' == *pszAcct)
    return 0;

  errno = 0;
  pwd = getpwnam( pszAcct);
  if ( NULL != pwd)
    {
    enum { kGroups = 32 };
    gid_t g=-1, groups[ kGroups];

    /* Get the group set */
    if ( 0 > getgroups( kGroups, groups))
      perror( "SWS_DaemonChown: getgroups failed");
    else
      g = groups[0];

    /* Set the new owner */
    if ( fchown( fileno( file), pwd->pw_uid, g))
      perror( "SWS_DaemonChown: fchown failed");
    else
      iRet = 0;
    }
  else if ( errno)
    perror( "SWS_DaemonChown: getpwnam failed");
  else
    fprintf( stderr, "SWS_DaemonChown: user '%s' not known\n", pszAcct);

  return iRet;
  }
#endif

#endif /* ndef SWS_DAEMON_H */
/* End of file */
