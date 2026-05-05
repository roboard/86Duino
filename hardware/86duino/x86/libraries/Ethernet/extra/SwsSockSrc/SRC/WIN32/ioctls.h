/****************************************************************************
 * Name:        ioctls.h                                                    *
 * Application: SwsVpkt                                                     *
 * Purpose:     Private IOCTL codes implemented by the driver               *
 *                                                                          *
 * Created by Lawrence Rust, Software Systems Consultants               .   *
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63                          *
 *__________________________________________________________________________*
 *                                                                          *
 * Revision History:                                                        *
 *                                                                          *
 * No.   Date     By   Reason                                               *
 *--------------------------------------------------------------------------*
 * 100  19 Mar 03  lvr  Creation                                            *
 * 101  29 Jun 05  lvr  Added IOCTL_GETINFO                                 *
 * 102  16 Oct 05  lvr  Added IOCTL_GETMULTICASTLIST                        *
 * 103  18 May 06  lvr  Added IOCTL_GETDESC                                 *
 *__________________________________________________________________________*/

#ifndef IOCTL_H
#define IOCTL_H 0x103

/* Dependencies */
#if SWS_DRVR
#include <devioctl.h>
#else
#include <winioctl.h>
#endif

#if SWS_DRVR
/* Definitions for Win32 SDK compatibility */
#ifndef BYTE
typedef UCHAR BYTE;
#endif
#ifndef WORD
typedef USHORT WORD;
#endif
#ifndef DWORD
typedef ULONG DWORD;
#endif
#ifndef BOOL
typedef BOOLEAN BOOL;
#endif
#endif


/* Win32 accessible device.  Adapters have 0.. appended */
#define IOCTL_DEVICE "SwsVpkt"

/* Device type */
#define IOCTL_TYPE     0xA50C  /* Microsoft uses 0 - 0x7FFF, OEMs use 0x8000 - 0xFFFF */
#define IOCTL_FCMIN    0x900   /* Microsoft uses function codes 0-0x7FF, OEM's use 0x800 - 0xFFF */

#define IOCTL_GETMACADDR \
   CTL_CODE( IOCTL_TYPE, (IOCTL_FCMIN + 0), METHOD_BUFFERED, FILE_READ_ACCESS)
/* lpvOutBuffer -> 6 BYTE MAC address */

#define IOCTL_SETMULTICASTLIST \
   CTL_CODE( IOCTL_TYPE, (IOCTL_FCMIN + 1), METHOD_BUFFERED, FILE_WRITE_ACCESS)
/* lpvInBuffer -> Multicast list */

#define IOCTL_SETPROMISCUITY \
   CTL_CODE( IOCTL_TYPE, (IOCTL_FCMIN + 2), METHOD_BUFFERED, FILE_WRITE_ACCESS)
/* lpvInBuffer -> BYTE, !0= enable promiscuous mode */

#define IOCTL_WRITE_SCATTER \
   CTL_CODE( IOCTL_TYPE, (IOCTL_FCMIN + 3), METHOD_OUT_DIRECT, FILE_WRITE_ACCESS)
/* lpvInBuffer -> array of IOCTL_BUFFER's */
typedef struct IOCTL_BUFFER
  {
  void* pv;
  DWORD dwLen;
  } IOCTL_BUFFER;

#define IOCTL_GETINFO \
   CTL_CODE( IOCTL_TYPE, (IOCTL_FCMIN + 4), METHOD_BUFFERED, FILE_READ_ACCESS)
/* lpvOutBuffer -> IOCTL_INFO */
typedef struct IOCTL_INFO
  {
  BOOLEAN bPowerOn;
  BOOLEAN bMediaDisconnected;
  BOOLEAN bWan;                          /* Adapter is a WAN */
  BOOLEAN bWanDown;                      /* WAN is not connected */
  } IOCTL_INFO;

#define IOCTL_GETMULTICASTLIST \
   CTL_CODE( IOCTL_TYPE, (IOCTL_FCMIN + 5), METHOD_BUFFERED, FILE_READ_ACCESS)
/* lpvOutBuffer -> Multicast list */

#define IOCTL_GETDESC \
   CTL_CODE( IOCTL_TYPE, (IOCTL_FCMIN + 6), METHOD_BUFFERED, FILE_READ_ACCESS)
/* lpvOutBuffer -> ASCIIZ string */

#endif /* ndef IOCTL_H */
/* End of file */
