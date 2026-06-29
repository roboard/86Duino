/****************************************************************************
 *
 * File name   : sws_net.h
 * Function    : Bring in platform specific network headers.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, SwsSock, Win32, *nix
 *
 ****************************************************************************
 * File History
 * ------------
 * 24-May-05, LVR, v1.00 - Created.
 * 20-Jun-09, LVR, v1.01 - Add DECL_TIMEOUT
 *
 ****************************************************************************/

#ifndef SWS_NET_H
#define SWS_NET_H 0x0101 /* Version no. BCD */

/* Expand and stringize a macro name */
#define SWS_NET_STR(s) #s
#define SWS_NET_STRING(s) SWS_NET_STR(s)

/* Compiler */
#ifndef SWS_NET_CC
#if defined __WATCOMC__
# define SWS_NET_CC "watcomc" SWS_NET_STRING(__WATCOMC__)
#elif defined __BORLANDC__
# define SWS_NET_CC "borlandc" SWS_NET_STRING(__BORLANDC__)
#elif defined __TURBOC__
# define SWS_NET_CC "turboc" SWS_NET_STRING(__TURBOC__)
#elif defined __MWERKS__
# define SWS_NET_CC "mwerks" SWS_NET_STRING(__MWERKS__)
#elif defined __ARMCC_VERSION
# define SWS_NET_CC "armcc" SWS_NET_STRING(__ARMCC_VERSION)
# define SWS_NET_OS ""
#elif defined __GNUC__
# define SWS_NET_CC "gcc" SWS_NET_STRING(__GNUC__) "." \
   SWS_NET_STRING(__GNUC_MINOR__) "." SWS_NET_STRING(__GNUC_PATCHLEVEL__)
#elif defined _MSC_VER
# define SWS_NET_CC "msvc" SWS_NET_STRING(_MSC_VER)
#else
# define SWS_NET_CC "c"
#endif
#endif

/* Target */
#ifndef SWS_NET_OS
#if defined __DJGPP__
#define SWS_NET_OS "-djgpp" SWS_NET_STRING(__DJGPP__) ".0" SWS_NET_STRING(__DJGPP_MINOR__)
#elif defined __MINGW32__
#define SWS_NET_OS "-mingw32"
#elif defined __CYGWIN__
#define SWS_NET_OS "-cygwin"
#elif defined _DOS && (defined _M_I386 || defined M_I386 || defined __386__)
#define SWS_NET_OS "-dos32"
#elif defined _DOS16 || defined __MSDOS__ || defined _MSDOS || defined __DOS__
#define SWS_NET_OS "-dos"
#elif defined _WIN32 || defined WIN32 || defined __WIN32__ || defined __WINDOWS__
#define SWS_NET_OS "-win32"
#elif defined _WINDOWS
#define SWS_NET_OS "-win16"
#elif defined __APPLE__ && defined __MACH__
#define SWS_NET_OS "-macosX"
#elif defined macintosh || defined __powerc
#define SWS_NET_OS "-mac"
#elif defined LINUX || defined __LINUX__ || defined __linux__ || defined __linux
#define SWS_NET_OS "-linux"
#elif defined __unix__ || defined __unix
#define SWS_NET_OS "-unix"
#else
#define SWS_NET_OS ""
#endif
#endif


#include <stdio.h> /* fputs */
#include <stdlib.h> /* atexit */
#include <errno.h>

#if SWS_USE_SOCKLIB
#define SWS_SOCK_COMPAT /* For BSD/winsock compatible names */
#include "sws_sock.h"

#define GetLastError() SWS_GetLastError()
#define Sleep(_m) SWS_SockSleep(_m)
#define GetMillisecs(_v) do {\
  _v = SWS_SockTicks();\
} while(0)

#define setsockasync(s,b) do {\
  u_long _i = b; SWS_ioctl( s, SWS_FIONBIO, &_i);\
} while (0)

#ifndef EINVAL
#define EINVAL SWS_EINVAL
#endif
#ifndef ENOMEM
#define ENOMEM SWS_ENOMEM
#endif

#define SWS_NET_ID "SwsSock-" SWS_NET_CC SWS_NET_OS

#elif defined WIN32 || defined _WIN32 || defined __WIN32__ || defined _WINDOWS || defined __WINDOWS__
/* Win16/32 */
#if defined __MWERKS__
# pragma ANSI_strict off
#elif defined _MSC_VER
# pragma warning ( disable : 4201) /* nonstandard extension used : nameless struct/union */
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#if defined __MWERKS__
# pragma ANSI_strict reset
#elif defined _MSC_VER
# pragma warning ( default : 4201)
#endif

#if defined WIN32 || defined _WIN32 || defined __WIN32__
/* Win32 */
#define SWS_NET_ID "ws2_32-" SWS_NET_CC SWS_NET_OS
#include <winsock2.h>
#include <ws2tcpip.h>

#else
/* Win16 */
#define SWS_NET_ID "Winsock-" SWS_NET_CC SWS_NET_OS
#include <winsock.h>
#include <io.h> /* _wyield */
#define SD_SEND 1

#ifdef _WINDOWS /* QuickWin */
#define Win16Yield() _wyield()
#else
#define Win16Yield() Yield()
#endif

#define Sleep( _ms) do {\
  DWORD _t; GetMillisecs( _t);\
  _ms += _t;\
  do {\
    Win16Yield();\
    GetMillisecs( _t);\
    }\
  while ( _t <= _ms);\
  } while(0)
#endif /* Win16 */

#ifdef _MSC_VER
#pragma warning ( disable : 4127) /* conditional expression is constant */
#endif

typedef unsigned long in_addr_t;

/* Declare & init a timeout in millisecs for SO_SND/RCVTIMEO */
#define DECL_TIMEOUT( n,m) int n = (int)(m)

#define ESOCKTNOSUPPORT WSAESOCKTNOSUPPORT
#define EPROTONOSUPPORT WSAEPROTONOSUPPORT
#define ETIMEDOUT WSAETIMEDOUT
#define ECONNRESET WSAECONNRESET
#define ECONNREFUSED WSAECONNREFUSED
#define ECONNABORTED WSAECONNABORTED
#define ENOTCONN WSAENOTCONN
#define EHOSTUNREACH WSAEHOSTUNREACH
#define ENETUNREACH WSAENETUNREACH
#define EISCONN WSAEISCONN
#define EWOULDBLOCK WSAEWOULDBLOCK
#ifndef EAGAIN
#define EAGAIN EWOULDBLOCK
#endif
#define EINPROGRESS WSAEINPROGRESS
#define EALREADY WSAEALREADY
#define ENETDOWN WSAENETDOWN
#define EREMOTE WSAEREMOTE
#define EHOST_NOT_FOUND WSAHOST_NOT_FOUND
#define ETRY_AGAIN WSATRY_AGAIN
#define ENO_RECOVERY WSANO_RECOVERY
#define ENO_DATA WSANO_DATA
#define EADDRNOTAVAIL WSAEADDRNOTAVAIL
#define EMSGSIZE WSAEMSGSIZE
#define EADDRINUSE WSAEADDRINUSE

#define GetLastError() WSAGetLastError()
#define GetMillisecs(_v) do {\
  _v = GetTickCount();\
} while(0)
#ifndef SWS_PTR
#define SWS_PTR(p) p *
#endif

typedef int socklen_t;
#define setsockasync(s,b) do {\
  u_long _i = b; ioctlsocket( s, FIONBIO, &_i);\
} while (0)

#else
/* u*ix */
#define SWS_NET_ID SWS_NET_CC SWS_NET_OS

#include <unistd.h>       /* usleep, gethostname */
#include <fcntl.h>        /* fcntl */
#include <strings.h>      /* strcasecmp */

/* BSD sockets */
#include <sys/time.h>     /* select */
#include <sys/socket.h>   /* socket, accept... */
#include <sys/ioctl.h>    /* ioctl */
#include <netdb.h>        /* gethostbyname */
#include <netinet/in.h>   /* in_addr */
#include <netinet/tcp.h>  /* TCP_NODELAY */
#include <arpa/inet.h>    /* htonl etc and inet_addr */

#ifndef SWS_PTR
#define SWS_PTR(p) p *
#endif
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define SD_SEND 1

#define closesocket(s)          close(s)

#if defined F_SETFL && defined O_NONBLOCK
 /* POSIX */
# define setsockasync(s,b) (b)\
  ? fcntl( s, F_SETFL, fcntl( s, F_GETFL, 0) | O_NONBLOCK)\
  : fcntl( s, F_SETFL, fcntl( s, F_GETFL, 0) & ~O_NONBLOCK)
#elif defined __GNUC__
# define setsockasync(s,b)\
  ({ int _i = b; int _r = ioctl( s, FIONBIO, &_i); _r; })
#else
# define setsockasync(s,b) do {\
    int _i = b; ioctl( s, FIONBIO, &_i);\
  } while (0)
#endif

/* Declare & init a timeout in millisecs for SO_SND/RCVTIMEO */
#define DECL_TIMEOUT( name, m) \
  struct timeval name;\
  name.tv_sec = (m) / 1000;\
  name.tv_usec = ((m) % 1000) * 1000

#define EHOST_NOT_FOUND HOST_NOT_FOUND
#define ETRY_AGAIN TRY_AGAIN
#define ENO_DATA NO_DATA

#define GetLastError() errno
#define Sleep(_ms) usleep( 1000 * (_ms) )
#define GetMillisecs(_v) do {\
  struct timeval t;\
  gettimeofday( &t, NULL);\
  _v = (u_long)t.tv_sec * 1000 + t.tv_usec / 1000;\
} while(0)

#endif

/*
 * Cross platform SWS_SockUp/Down
 */
#if SWS_USE_SOCKLIB
# define SWS_SockUp( _gef, _v) do {\
  if ( SWS_SockStartup( _gef, _v))\
    {\
    fputs( "SWS_SockStartup FAILED\n", stderr);\
    exit( EXIT_FAILURE);\
    }\
  /* Register an exit handler to cleanup */ \
  atexit( &SWS_SockExit);\
  } while (0)
# ifdef _MSC_VER
#  pragma warning ( disable : 4127) /* conditional expression is constant */
# endif

# define SWS_PKTDUMP( _f, _v) \
  (SWS_DumpSetup(\
    (_v) >= 2 ? (_v) -1 : (_v) >= 1 ? -1 : 0,\
    NULL,\
    stdout == (_f) ? NULL : (_f)\
  ), &SWS_DumpPacket)

#elif defined WIN32 || defined _WIN32 || defined __WIN32__
# define SWS_SockUp( _gef, _v) do {\
  WSADATA wsaData;\
  if ( WSAStartup( 0x202, &wsaData))\
    {\
    fputs( "WSAStartup failed\n", stderr);\
    exit( EXIT_FAILURE);\
    }\
  /* Register an exit handler to cleanup the library */ \
  atexit( &SWS_SockExit);\
  } while (0)
 static __inline void SWS_SockExit( void) { WSACleanup(); }
# ifdef _MSC_VER
#  pragma warning ( disable : 4127) /* conditional expression is constant */
# endif
# define SWS_PKTDUMP( _f, _v) 0

#elif defined WIN16
# define SWS_SockUp( _gef, _v) do {\
  WSADATA wsaData;\
  if ( WSAStartup( 0x101, &wsaData))\
    {\
    fputs( "WSAStartup failed\n", stderr);\
    exit( EXIT_FAILURE);\
    }\
  /* Register an exit handler to cleanup the library */ \
  atexit( &SWS_SockExit);\
  } while (0)
 static __inline void SWS_SockExit( void) { WSACleanup(); }
# define SWS_PKTDUMP( _f, _v) 0

#else
# define SWS_SockUp( _gef, _v) ((void)0)
# define SWS_PKTDUMP( _f, _v) 0
#endif

#endif /* ndef SWS_NET_H */
/* End of file */
