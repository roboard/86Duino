/****************************************************************************
 *
 * File name   : packet.c
 * Function    : Packet driver
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C.
 * Version     : See sws_sock.h for overall library version information.
 * Release Date: See sws_sock.h for overall library release information.
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 27-Apr-01, LVR, Created.
 * 28-Oct-02, LVR, Add PKEY_RECV_MODE & PKEY_MAC_ADDRESS.
 * 01-Nov-02, LVR, PacketUpcall calls PacketRecvAck on error.
 * 06-Nov-02, LVR, Added PacketIoctl:SWS_kDatalinkProperty_Promiscuity.
 * 22-Dec-02, LVR, Added PacketSetMulticastList & SWS_kDatalinkProperty_MulticastList
 * 20-Sep-03, LVR, OPT_PKT_SEND_ASYNC defaults to 0 (was 1) so FA310 works OK
 * 03-Apr-05, LVR, OPT_PKT_SEND_ASYNC default on and add PKEY_SEND_ASYNC option
 *
 * Comments:
 * --------
 * This file interfaces a packet driver to the SwsSock stack
 *
 ****************************************************************************
 */

/* Compilation options */
#include "config.h"

#ifndef OPT_PKT_BUFFER_SIZE
#define OPT_PKT_BUFFER_SIZE (70 * 1024) /* Receive buffer size */
#endif
#ifndef OPT_PKT_BUFFER_RXQ
#define OPT_PKT_BUFFER_RXQ 160 /* Pending receive packets */
#endif

#ifndef OPT_PKT_SEND_ASYNC
#define OPT_PKT_SEND_ASYNC 1 /* !0 to use AS_SEND_PKT */
#endif

/* Exports */
#include "packet.h"

#if OPT_DATALINK_ETHERNET
/* Imports */
#include <stddef.h>
#include <string.h> /* memcpy */

/* DJGPP */
#include <sys/segments.h>
#include <sys/movedata.h>
#include <go32.h>
#include <dpmi.h>
#include <dos.h>

/* SwsR6040 */
#include "r6040.h"
#include "io.h"

/* SwsVN */
#include "vnet.h"

/* SwsSock */
#include "sws_enet.h"   /* Ethernet driver registration */
#include "sws_dlnk.h"   /* SWS_kDatalinkProperty_Promiscuity */
#include "sws_buf.h"    /* Buffer operations */
#include "sws_msgq.h"   /* Message queues */
#include "sws_sock.h"   /* Error codes */
#include "platform.h"   /* PlatformPropertyGetString/Int */

#include "debug.h"      /* TRACE, ASSERT & VERIFY */

/*
 * Macros & Constants
 */

/* Configurable vales */
#define PKEY_RECV_MODE      "rxmode"        /* Per adapter */
#define PKEY_MAC_ADDRESS    "mac_address"   /* Per adapter */
#define PKEY_SEND_ASYNC     "txasync"       /* Per adapter, OPT_PKT_SEND_ASYNC */
#define PKEY_BUFFER_SIZE    "buffer_size"   /* Global, OPT_PKT_BUFFER_SIZE */
#define PKEY_RXQ_LEN        "rxq"           /* Per adapter. OPT_PKT_BUFFER_RXQ */

/* Get a ptr to the start of a struct given a ptr to a field within it */
#ifndef SWS_CONTAINER
#define SWS_CONTAINER(ptr, type, field) \
    (type *)((char *)(ptr) - offsetof(type, field))
#endif

#define MAX_FRAME_SIZE  1514    /* Ethernet MTU=1500 (+14 byte MAC hdr), no FCS */

#define C_FLAG          1       /* dpmi flags register */

/* Packet driver interface classes */
#define CL_NONE         0
#define CL_ETHERNET     1
#define CL_PRONET_10    2
#define CL_IEEE8025     3
#define CL_OMNINET      4
#define CL_APPLETALK    5
#define CL_SERIAL_LINE  6
#define CL_STARLAN      7
#define CL_ARCNET       8
#define CL_AX25         9
#define CL_KISS         10
#define CL_IEEE8023     11
#define CL_FDDI         12
#define CL_INTERNET_X25 13
#define CL_LANSTAR      14
#define CL_SLFP         15
#define CL_NETROM       16
#define CL_PPP          17
#define CL_QTSO         18
#define NCLASS          19

/* Packet driver interface types (not a complete list) */
#define TC500   1
#define PC2000  10
#define WD8003  14
#define PC8250  15
#define ANYTYPE 0xffff

/* Packet driver function call numbers */
#define DRIVER_INFO         1
#define ACCESS_TYPE         2
#define RELEASE_TYPE        3
#define SEND_PKT            4
#define TERMINATE           5
#define GET_ADDRESS         6
#define RESET_INTERFACE     7
/* Optional high performance functions */
#define GET_PARAMETERS      10
#define AS_SEND_PKT         11
/* Optional extended functions */
#define SET_RCV_MODE        20
#define GET_RCV_MODE        21
#define SET_MULTICAST_LIST  22
#define GET_MULTICAST_LIST  23
#define GET_STATISTICS      24
#define SET_ADDRESS         25

/* Packet driver error return codes. From Appendix C. */
#define BAD_HANDLE      1   /* invalid handle number */
#define NO_CLASS        2   /* no interfaces of specified class found */
#define NO_TYPE         3   /* no interfaces of specified type found */
#define NO_NUMBER       4   /* no interfaces of specified number found */
#define BAD_TYPE        5   /* bad packet type specified */
#define NO_MULTICAST    6   /* this interface does not support multicast */
#define CANT_TERMINATE  7   /* this packet driver cannot terminate */
#define BAD_MODE        8   /* an invalid receiver mode was specified */
#define NO_SPACE        9   /* operation failed because of insufficient space */
#define TYPE_INUSE      10  /* the type had previously been accessed, and not released */
#define BAD_COMMAND     11  /* the command was out of range, or not implemented */
#define CANT_SEND       12  /* the packet couldn't be sent (usually hardware error) */
#define CANT_SET        13  /* hardware address couldn't be changed (> 1 handle open) */
#define BAD_ADDRESS     14  /* hardware address has bad length or format */
#define CANT_RESET      15  /* couldn't reset interface (> 1 handle open) */

/* SwsR6040 */
typedef struct SwsR6040Driver
{
    int isActived;
    R6040_HANDLE_t handle;

} SwsR6040Driver;

static void SwsR6040_PacketSent(void *user_data);
static void SwsR6040_PacketRecv(void *user_data);

/* SwsVN */
typedef struct SwsVNDriver
{
    int isActived;
    SWS_VN_t handle;

} SwsVNDriver;

static void SwsVN_PacketSent(void *user_data);
static void SwsVN_PacketRecv(void *user_data);

/*
 * Local data types
 */

/* Packet driver instance */
typedef struct SPacketDriver
{
    ui32 uMagic;
#define PACKET_MAGIC 0x3798e47fL

    struct SPacketDriver *pNext; /* List of drivers */
    int iInterrupt;              /* INT to call to access driver */
    ui16 handle;                 /* Driver handle */

    _go32_dpmi_seginfo rmcb_seginfo;     /* Real mode callback for Rx */
    _go32_dpmi_registers rmcb_registers; /* Real mode callback registers */

#if OPT_PKT_SEND_ASYNC
    int iSendAsync;
    _go32_dpmi_seginfo txRmcb_seginfo;     /* Real mode callback for Tx */
    _go32_dpmi_registers txRmcb_registers; /* Real mode callback registers */
#endif

    Atomic_t aUpCalls;
    _go32_dpmi_seginfo dosmem; /* Real mode receive buffer */
    unsigned long dosmemAddress;
    volatile unsigned read, write; /* Read/write cursors into buffer */
    volatile unsigned saveWrite;

    SWS_EnetMacAddress mac; /* Cached MAC address */
    int macAddrIsValid;     /* !0 if cache is valid */
    unsigned rxmode;

    SWS_EnetCallbacks upcalls; /* Up-call info */

    SWS_MsgqH hRxQ;              /* Queue of pending received data */
    char buffer[MAX_FRAME_SIZE]; /* Upcall buffer */

    SwsR6040Driver SwsR6040;
    SwsVNDriver SwsVN;
    SWS_DatalinkPriv SwsPriv;

} SPacketDriver;

/* Received data message */
typedef struct SPacketMsg
{
    unsigned long realAddress; /* Real mode -> data */
    unsigned len;              /* Length */
} SPacketMsg;

/*
 * Prototypes
 */
static SWS_FEnetSend PacketSend;
static SWS_FEnetRecvAck PacketRecvAck;
static SWS_FEnetGetMacAddr PacketGetMacAddr;
static SWS_FEnetShutdown PacketShutdown;
static SWS_FEnetIoctl PacketIoctl;

static void PacketRecv(__dpmi_regs *);
#if OPT_PKT_SEND_ASYNC
static void PacketSent(__dpmi_regs *);
#endif
static int AddPacketDriver(int, unsigned, unsigned);
static void FreePacketDriver(SPacketDriver *);
static int PacketSetRxMode(SPacketDriver *const, unsigned mode);
#if OPT_IP_MULTICAST
static int PacketSetMulticastList(SPacketDriver *const, SWS_PTR(void), size_t);
#endif

/*
 * SwsR6040 data.
 */
static int sws_IoInited = 0;

/*
 * Module data
 */
static size_t s_sizeBuffer;
static SPacketDriver *s_pPacketDrivers; /* List of interfaces */
static unsigned s_uInterfaces; /* No. packet driver interfaces */

/* Statistics */
static volatile unsigned s_packets, s_buffers, s_noHandle, s_badCmd;
static volatile unsigned s_errors, s_overflows;

/* Ethernet device descriptor */
static char s_szIdent[sizeof(SWS_PACKET_IDENT "01")];
static SWS_EnetDevice const s_packet = {
    sizeof(s_packet),
    SWS_ENET_VERSION,
    &s_szIdent[0],
    &PacketSend,
    &PacketRecvAck,
    &PacketGetMacAddr,
    &PacketShutdown,
    &PacketIoctl
};

/*
 * Initialise the driver
 * Single threaded
 */
int SWS_PacketInstall(void) /* Returns !0 on error */
{
    int i, iAdapters;

    TRACE(9, ("SWS_PacketInstall()\n"));
    ASSERT(PlatformIsCritical());

    s_sizeBuffer = (size_t)PlatformPropertyGetUint(
        SWS_PACKET_IDENT, PKEY_BUFFER_SIZE, OPT_PKT_BUFFER_SIZE);

    /* Search for packet drivers */
    iAdapters = 0;
    for (i = 0x60; i <= 0x80; ++i)
    {
        ui16 vec[2];
        ui32 addr;

        /* Get -> INT handler */
        dosmemget(i * 4, sizeof(vec), vec);
        addr = vec[1] * 16 + vec[0];
        if (addr)
        {
            static const char kszIdent[] = "PKT DRVR";
            char buf[80];

            dosmemget(addr + 3, sizeof(kszIdent), buf);
            if (!strcmp(buf, kszIdent))
            {
                /* Packet driver found */
                int err;
                __dpmi_regs regs;
                unsigned if_num;

                /* Get driver info */
                regs.h.ah = DRIVER_INFO;
                regs.h.al = 0xff;
                regs.x.bx = 0xffff;
                regs.x.cx = 0;
                regs.x.ss = 0;
                regs.x.sp = 0;
                regs.x.flags = 0;
                if (__dpmi_int(i, &regs))
                    continue;
                if ((C_FLAG & regs.x.flags) && BAD_HANDLE != regs.h.dh)
                {
                    TRACE(2, ("SWS_PacketInstall() %s error %u\n", kszIdent, regs.h.dh));
                    continue;
                }
                if (regs.h.ch != CL_ETHERNET)
                {
                    TRACE(2, ("SWS_PacketInstall() %s type %u\n", kszIdent, regs.h.ch));
                    continue;
                }

                /* Get driver string */
                dosmemget(regs.x.ds * 16 + regs.x.si, sizeof(buf), buf);
#if OPT_DEMO
                if (strcmp(buf, "SwsVpkt"))
                    continue;
#endif

                TRACE(2, ("SWS_PacketInstall() '%s' ver %u type %u func %u\n",
                          buf, regs.x.bx, regs.x.dx, regs.h.al));

                /* Add all interfaces (count-1 in CL reg) */
                for (if_num = 0; if_num <= regs.h.cl; ++if_num)
                {
                    err = AddPacketDriver(i, if_num, regs.h.al);
                    if (!err)
                        ++iAdapters;
                    else
                        return err;
                }
            }
        }
    }

    if (io_Init() == true)
    {
        #define MAX_R6040_NUM   (2)
        static int bus[MAX_R6040_NUM] = {0, 0};
        static int dev[MAX_R6040_NUM] = {8, 9};
        static int fun[MAX_R6040_NUM] = {0, 0};
        sws_IoInited = 1;
        for (int i = 0; i < MAX_R6040_NUM; i++)
        {
            void *pci = pci_Alloc(bus[i], dev[i], fun[i]);
            if (pci != NULL)
            {
                SPacketDriver *pPacket;
                SWS_EnetMacAddress mac;
                ui32 iobase = pci_In32(pci, 0x10) & 0xFFFFFFF0UL;
                ((ui16 *)mac.addr)[0] = io_inpw(iobase + 0x68);
                ((ui16 *)mac.addr)[1] = io_inpw(iobase + 0x6A);
                ((ui16 *)mac.addr)[2] = io_inpw(iobase + 0x6C);
                pci_Free(pci);
                pPacket = s_pPacketDrivers;
                while (pPacket != NULL)
                {
                    if (pPacket->mac.addr[0] == mac.addr[0] &&
                        pPacket->mac.addr[1] == mac.addr[1] &&
                        pPacket->mac.addr[2] == mac.addr[2] &&
                        pPacket->mac.addr[3] == mac.addr[3] &&
                        pPacket->mac.addr[4] == mac.addr[4] &&
                        pPacket->mac.addr[5] == mac.addr[5])
                        break;
                    pPacket = pPacket->pNext;
                }
                if (pPacket == NULL)
                {
                    int err = AddPacketDriver(-1, i, 5);
                    if (!err)
                        ++iAdapters;
                    else
                        return err;
                }
            }
        }
    }

    for (int i = 0; i < SwsVN_GetDriverCount(); i++)
    {
        int err = AddPacketDriver(-2, i, 5);
        if (!err)
            ++iAdapters;
        else
            return err;
    }

    if (0 == iAdapters)
    {
        TRACE(1, ("*** SWS_PacketInstall() no adapters found\n"));
        return SWS_ENETDOWN;
    }

    return 0;
}

/*
 * Finalise the driver
 * Single threaded
 */
void SWS_PacketCleanup(void)
{
    SPacketDriver *pPacket;

    TRACE(9, ("SWS_PacketCleanup()\n"));
    ASSERT(PlatformIsCritical());

    /* All the drivers should have been shutdown by now */
    while (NULL != (pPacket = s_pPacketDrivers))
    {
        s_pPacketDrivers = pPacket->pNext;
        FreePacketDriver(pPacket);
    }

    if (sws_IoInited)
    {
        sws_IoInited = 0;
        io_Close();
    }

    TRACE(1, ("PacketDrvr: RxPkts:%u BufReqs:%u BadInds:%u Qoflows:%u NoHandle:%u BadCmd:%u\n",
              s_packets, s_buffers, s_errors, s_overflows, s_noHandle, s_badCmd));
}

/*
 * Create an SwsSock interface for a packet driver
 */
int AddPacketDriver(int iInterrupt, unsigned if_num, unsigned if_func)
{
    SPacketDriver *pPacket;
    __dpmi_regs regs;
    int iRet;
#if !OPT_PKT_SEND_ASYNC
    (void)if_func;
#endif

    TRACE(9, ("AddPacketDriver(%d,%u,%u)\n", iInterrupt, if_num, if_func));

    sprintf(s_szIdent, SWS_PACKET_IDENT "%u", ++s_uInterfaces);

    /* Allocate instance data */
    pPacket = (SPacketDriver *)calloc(1, sizeof(SPacketDriver));
    if (NULL == pPacket)
    {
        TRACE(0, ("!!! AddPacketDriver() Out of memory for instance\n"));
        s_uInterfaces--;
        return SWS_ENOMEM;
    }

    pPacket->handle = 0xffff;
    pPacket->SwsR6040.isActived = (iInterrupt == -1);
    pPacket->SwsVN.isActived = (iInterrupt == -2);
    pPacket->SwsPriv.isPacketDriver = (iInterrupt >= 0);

    /* Create a receive buffer Q */
    pPacket->hRxQ = SWS_MsgqCreate(
        PlatformPropertyGetUint(s_szIdent, PKEY_RXQ_LEN, OPT_PKT_BUFFER_RXQ),
        sizeof(SPacketMsg));
    if (NULL == pPacket->hRxQ)
    {
        TRACE(0, ("!!! AddPacketDriver() Out of memory for RxQ\n"));
        FreePacketDriver(pPacket);
        return SWS_ENOMEM;
    }

    if (pPacket->SwsVN.isActived)
    {
        if ((pPacket->SwsVN.handle = SwsVN_GetVirtualNetwork(SwsVN_GetInterfaceNo(if_num))) == NULL)
        {
            TRACE(1, ("*** SwsVN_GetPrivateData() failed\n"));
            FreePacketDriver(pPacket);
            return SWS_ENOMEM;
        }
        SwsVN_RegisterReceivedCB(pPacket->SwsVN.handle, SwsVN_PacketRecv, pPacket);
    #if OPT_PKT_SEND_ASYNC
        SwsVN_RegisterSendCompletedCB(pPacket->SwsVN.handle, SwsVN_PacketSent, pPacket);
    #endif
    }
    else if (pPacket->SwsR6040.isActived)
    {
        if ((pPacket->SwsR6040.handle = r6040_Initialize(if_num)) == NULL)
        {
            TRACE(1, ("*** r6040_Initialize() failed\n"));
            FreePacketDriver(pPacket);
            return SWS_ENOMEM;
        }
        r6040_SETTINGS_PrivateData(pPacket->SwsR6040.handle, pPacket);
        r6040_RegisterReceivedCB(pPacket->SwsR6040.handle, SwsR6040_PacketRecv);
    #if OPT_PKT_SEND_ASYNC
        r6040_RegisterSendToDMACB(pPacket->SwsR6040.handle, SwsR6040_PacketSent);
    #endif
        if (r6040_EnableInterruptMode(pPacket->SwsR6040.handle) < 0) {
            TRACE(1, ("*** r6040_EnableInterruptMode() failed\n"));
            FreePacketDriver(pPacket);
            return SWS_ENOMEM;
        }
    }
    else
    {
        /* Allocate the real mode receive buffer */
        pPacket->dosmem.size = (s_sizeBuffer + 15) / 16; /* paragraphs */
        if (_go32_dpmi_allocate_dos_memory(&pPacket->dosmem))
        {
            TRACE(1, ("*** _dpmi_allocate_dos_memory(%u) failed\n", pPacket->dosmem.size));
            FreePacketDriver(pPacket);
            return SWS_ENOMEM;
        }
        pPacket->dosmemAddress = pPacket->dosmem.rm_segment * 16 + pPacket->dosmem.rm_offset;

        /* Allocate the Rx real mode callback */
        pPacket->rmcb_seginfo.pm_offset = (unsigned long)&PacketRecv;
        if (_go32_dpmi_allocate_real_mode_callback_retf(
                &pPacket->rmcb_seginfo,
                &pPacket->rmcb_registers))
        {
            TRACE(1, ("*** _go32_dpmi_allocate_real_mode_callback_retf failed\n"));
            FreePacketDriver(pPacket);
            return SWS_ENOMEM;
        }

    #if OPT_PKT_SEND_ASYNC
        /* Allocate the Tx real mode callback */
        pPacket->txRmcb_seginfo.pm_offset = (unsigned long)&PacketSent;
        if (_go32_dpmi_allocate_real_mode_callback_retf(
                &pPacket->txRmcb_seginfo,
                &pPacket->txRmcb_registers))
        {
            TRACE(1, ("*** _go32_dpmi_allocate_real_mode_callback_retf failed\n"));
            FreePacketDriver(pPacket);
            return SWS_ENOMEM;
        }
    #endif

        /* Attach to packet driver */
    #if OPT_DEMO
        regs.h.ah = 0x80;
        regs.h.al = 'S';
        regs.x.bx = 0x1234;
        regs.h.dl = if_num;
        regs.x.di = pPacket->rmcb_seginfo.rm_offset;
        regs.x.es = pPacket->rmcb_seginfo.rm_segment; /* es:di -> real mode callback */
        regs.x.cx = sprintf(pPacket->buffer, "SwsVpkt");
        dosmemput(pPacket->buffer, regs.x.cx, __tb);
        regs.x.si = __tb & 15;
        regs.x.ds = __tb >> 4; /* ds:si -> real mode transfer buffer */
    #else
        regs.h.ah = ACCESS_TYPE;
        regs.h.al = CL_ETHERNET;
        regs.x.bx = ANYTYPE;
        regs.h.dl = if_num;
        regs.x.di = pPacket->rmcb_seginfo.rm_offset;
        regs.x.es = pPacket->rmcb_seginfo.rm_segment; /* es:di -> real mode callback */
        regs.x.si = 0;
        regs.x.ds = 0; /* ds:si -> type */
        regs.x.cx = 0; /* typelen, 0= all types */
    #endif
        TRACE(2, ("SWS_PacketInstall() ACCESS_TYPE callback %04x:%04x\n",
                regs.x.es, regs.x.di));
        regs.x.ss = 0;
        regs.x.sp = 0;
        regs.x.flags = 0;
        __dpmi_int(iInterrupt, &regs);
        if (C_FLAG & regs.x.flags)
        {
            TRACE(1, ("*** SWS_PacketInstall() ACCESS_TYPE failed %d\n", regs.h.dh));
            FreePacketDriver(pPacket);
            return 1;
        }
        pPacket->handle = regs.x.ax;
    }
    pPacket->iInterrupt = iInterrupt;
    pPacket->uMagic = PACKET_MAGIC;

#if OPT_PKT_SEND_ASYNC
    if (if_func >= 5)
        pPacket->iSendAsync = (int)PlatformPropertyGetUint(s_szIdent, PKEY_SEND_ASYNC, 0);
    else
        pPacket->iSendAsync = 0;

    TRACE(2, ("SWS_PacketInstall() iSendAsync=%d\n", pPacket->iSendAsync));
    if (pPacket->iSendAsync && pPacket->SwsR6040.isActived == 0 && pPacket->SwsVN.isActived == 0)
    {
        /* Get receiver parameters, returned in es:di */
        regs.h.ah = GET_PARAMETERS;
        regs.x.ss = 0;
        regs.x.sp = 0;
        regs.x.flags = 0;
        __dpmi_int(iInterrupt, &regs);
        if (C_FLAG & regs.x.flags)
        {
            TRACE(1, ("*** SWS_PacketInstall() GET_PARAMETERS failed %d\n", regs.h.dh));
            pPacket->iSendAsync = 0;
        }
    }
#endif

#if 0 /* BUGBUG: Stops Netgear FA310TX packet driver from working */
  /* Reset the interface */
  regs.h.ah = RESET_INTERFACE;
  regs.x.bx = pPacket->handle;
  regs.x.ss = 0;
  regs.x.sp = 0;
  regs.x.flags = 0;
  if ( __dpmi_int( pPacket->iInterrupt, &regs))
    {
    TRACE( 1, ("*** SWS_PacketInstall dpmi_int error\n"));
    }
  else if (C_FLAG & regs.x.flags)
    {
    TRACE( 1, ("*** SWS_PacketInstall() RESET_INTERFACE failed %d\n", regs.h.dh));
    }
#endif

    {
        /* Read a user configured MAC address.  This overrides the value in EEPROM
         * BUGBUG: FA310TX packet driver stops all Rx data - must reload driver
         */
        char szBuf[32];
        unsigned addr[6];
        if (0 != PlatformPropertyGetString(s_szIdent, PKEY_MAC_ADDRESS, "", szBuf, sizeof(szBuf)) && 6 == sscanf(szBuf, "%x-%x-%x-%x-%x-%x", addr, addr + 1, addr + 2, addr + 3, addr + 4, addr + 5))
        {
            pPacket->mac.addr[0] = (ui8)addr[0];
            pPacket->mac.addr[1] = (ui8)addr[1];
            pPacket->mac.addr[2] = (ui8)addr[2];
            pPacket->mac.addr[3] = (ui8)addr[3];
            pPacket->mac.addr[4] = (ui8)addr[4];
            pPacket->mac.addr[5] = (ui8)addr[5];

            if (pPacket->SwsVN.isActived)
            {
                SwsVN_SetMacAddress(pPacket->SwsVN.handle, pPacket->mac.addr);
                pPacket->macAddrIsValid = 1;
            }
            else if (pPacket->SwsR6040.isActived)
            {
                r6040_SetMACAddress(pPacket->SwsR6040.handle, pPacket->mac.addr);
                pPacket->macAddrIsValid = 1;
            }
            else
            {
                /* Copy to real mode transfer buffer */
                dosmemput(pPacket->mac.addr, sizeof(pPacket->mac.addr), __tb);

                /* Set the MAC address */
                regs.h.ah = SET_ADDRESS; /* Extended function */
                regs.x.bx = pPacket->handle;
                regs.x.di = __tb & 15;
                regs.x.es = __tb >> 4; /* es:di -> real mode transfer buffer */
                regs.x.cx = sizeof(SWS_EnetMacAddress);
                regs.x.ss = 0;
                regs.x.sp = 0;
                regs.x.flags = 0;
                if (__dpmi_int(pPacket->iInterrupt, &regs))
                {
                    TRACE(1, ("*** SWS_PacketInstall dpmi_int error\n"));
                }
                else if (C_FLAG & regs.x.flags)
                {
                    TRACE(1, ("*** SWS_PacketInstall SET_ADDRESS error %d\n", regs.h.dh));
                }
                else
                {
                    /* Cache the address for later */
                    pPacket->macAddrIsValid = 1;
                }
            }
        }
    }

    /* Set receive mode */
    iRet = (int)PlatformPropertyGetUint(s_szIdent, PKEY_RECV_MODE, 3);
    if (PacketSetRxMode(pPacket, iRet))
        pPacket->rxmode = 3;

    /* Register with SwsSock as an Ethernet driver */
    pPacket->upcalls.uSize = sizeof(pPacket->upcalls);
    iRet = SWS_EnetAddAdapter(&s_packet, pPacket, &pPacket->upcalls);
    if (iRet)
    {
        FreePacketDriver(pPacket);
        return iRet;
    }
    ASSERT(pPacket->upcalls.pfnSendAck);
    ASSERT(pPacket->upcalls.pfnRecvInd);

    /* Add to list of drivers */
    pPacket->pNext = s_pPacketDrivers;
    s_pPacketDrivers = pPacket;

    PacketGetMacAddr(pPacket, &pPacket->mac);

    return 0;
}

/*
 * Callback from Ethernet module to shutdown an adapter
 */
static SWS_CB(void, PacketShutdown(
                        SWS_PTR(void) pvContext /* IN: Client context */
                        ))
{
    SPacketDriver *pPacket, *pPrev;
    ;

    TRACE(9, ("PacketShutdown(%p)\n", pvContext));
    ASSERT(NULL != pvContext);

    /* Search for the driver */
    pPrev = NULL;
    for (pPacket = s_pPacketDrivers; NULL != pPacket; pPacket = pPacket->pNext)
    {
        if ((SPacketDriver *)pvContext == pPacket)
        {
            /* Unsplice it */
            if (NULL != pPrev)
                pPrev->pNext = pPacket->pNext;
            else
                s_pPacketDrivers = pPacket->pNext;

            /* Shut it down */
            FreePacketDriver(pPacket);
            return;
        }

        pPrev = pPacket;
    }

    TRACE(1, ("*** PacketShutdown() Client %p not found)\n", pvContext));
}

/*
 * Release packet driver resources
 */
void FreePacketDriver(SPacketDriver *pPacket)
{
    TRACE(9, ("FreePacketDriver(%p)\n", pPacket));
    ASSERT(NULL != pPacket);

    s_uInterfaces--;
    pPacket->uMagic = 0;

    if (pPacket->SwsVN.isActived)
    {
        if (pPacket->SwsVN.handle)
        {
        #if OPT_PKT_SEND_ASYNC
            SwsVN_RegisterSendCompletedCB(pPacket->SwsVN.handle, NULL, NULL);
        #endif
            SwsVN_RegisterReceivedCB(pPacket->SwsVN.handle, NULL, NULL);
            pPacket->SwsVN.handle = NULL;
        }
    }
    else if (pPacket->SwsR6040.isActived)
    {
        if (pPacket->SwsR6040.handle)
        {
        #if OPT_PKT_SEND_ASYNC
            r6040_RegisterSendToDMACB(pPacket->SwsR6040.handle, NULL);
        #endif
            r6040_RegisterReceivedCB(pPacket->SwsR6040.handle, NULL);
            r6040_SETTINGS_PrivateData(pPacket->SwsR6040.handle, NULL);
            r6040_Shutdown(pPacket->SwsR6040.handle);
            pPacket->SwsR6040.handle = NULL;
        }
    }
    else
    {
        if (0xffff != pPacket->handle)
        {
            __dpmi_regs regs;

            if (3 != pPacket->rxmode)
                PacketSetRxMode(pPacket, 3);

            regs.h.ah = RELEASE_TYPE;
            regs.x.bx = pPacket->handle;
            regs.x.ss = 0;
            regs.x.sp = 0;
            regs.x.flags = 0;
            __dpmi_int(pPacket->iInterrupt, &regs);

            pPacket->handle = 0xffff;
        }

    #if OPT_PKT_SEND_ASYNC
        if (pPacket->txRmcb_seginfo.pm_offset)
        {
            _go32_dpmi_free_real_mode_callback(&pPacket->txRmcb_seginfo);
            pPacket->txRmcb_seginfo.pm_offset = 0;
        }
    #endif

        if (pPacket->rmcb_seginfo.pm_offset)
        {
            _go32_dpmi_free_real_mode_callback(&pPacket->rmcb_seginfo);
            pPacket->rmcb_seginfo.pm_offset = 0;
        }

        if (pPacket->dosmem.size)
        {
            _go32_dpmi_free_dos_memory(&pPacket->dosmem);
            pPacket->dosmem.size = 0;
        }
    }

    if (NULL != pPacket->hRxQ)
    {
        SWS_MsgqDestroy(pPacket->hRxQ);
        pPacket->hRxQ = NULL;
    }

    free(pPacket);
}

/*
 * Transmit a buffer, possibly fragmented
 * Single threaded
 */
SWS_CB(int, PacketSend(                         /* Return !0 on error, -1 if pending */
                       SWS_PTR(void) pvContext, /* IN: Driver's context */
                       SWS_BufferH hBuffer      /* IN: Buffer to send (owned by caller) */
                       ))
{
    SPacketDriver *const pPacket = (SPacketDriver *)pvContext;
    SWS_size_t len, uLen, uSrc;

    TRACE(9, ("PacketSend(%p,%p)\n", pvContext, hBuffer));
    ASSERT(NULL != pPacket);
    ASSERT(PACKET_MAGIC == pPacket->uMagic);

    len = SWS_BufferSize(hBuffer);
    ASSERT(len >= 60 /* Min Ethernet frame */);

    if (pPacket->SwsVN.isActived)
    {
        SWS_VN_FRAME_t packet = SwsVN_RequestPacket(pPacket->SwsVN.handle);

        if (packet == NULL)
        {
            TRACE(1, ("*** SwsVN_RequestPacket() failed\n"));
            return SWS_BASEERR + 1000;
        }

        /* Copy the packet data into a linear buffer */
        uSrc = 0;
        uLen = len;
        while (uLen)
        {
            SWS_PTR(const void)
            pvSrc;
            SWS_size_t uLenSrc;

            pvSrc = SWS_BufferGetFragment(hBuffer, uSrc, &uLenSrc);
            ASSERT(NULL != pvSrc);

            if (uLen < uLenSrc)
                uLenSrc = uLen;

            memcpy((char *)packet + uSrc, pvSrc, uLenSrc);
            uSrc += uLenSrc;
            uLen -= uLenSrc;
        }
        SwsVN_SendPacket(pPacket->SwsVN.handle, packet, len);
    }
    else if (pPacket->SwsR6040.isActived)
    {
        R6040_PACKET_t packet = r6040_RequestPacket(pPacket->SwsR6040.handle);

        if (packet == NULL)
        {
            TRACE(1, ("*** r6040_RequestPacket() failed\n"));
            return SWS_BASEERR + 1000;
        }

        /* Copy the packet data into a linear buffer */
        uSrc = 0;
        uLen = len;
        while (uLen)
        {
            SWS_PTR(const void)
            pvSrc;
            SWS_size_t uLenSrc;

            pvSrc = SWS_BufferGetFragment(hBuffer, uSrc, &uLenSrc);
            ASSERT(NULL != pvSrc);

            if (uLen < uLenSrc)
                uLenSrc = uLen;

            memcpy((char *)packet + uSrc, pvSrc, uLenSrc);
            uSrc += uLenSrc;
            uLen -= uLenSrc;
        }
        packet->size = len - 14;
        r6040_SendPacket(pPacket->SwsR6040.handle, packet);
        r6040_StartTransfer(pPacket->SwsR6040.handle);
    }
    else
    {
        __dpmi_regs regs;

        /* Copy the packet data into a linear buffer */
        uSrc = 0;
        uLen = len;
        while (uLen)
        {
            SWS_PTR(const void)
            pvSrc;
            SWS_size_t uLenSrc;

            pvSrc = SWS_BufferGetFragment(hBuffer, uSrc, &uLenSrc);
            ASSERT(NULL != pvSrc);

            if (uLen < uLenSrc)
                uLenSrc = uLen;

            /* Copy to real mode transfer buffer */
            dosmemput(pvSrc, uLenSrc, __tb + uSrc);
            uSrc += uLenSrc;
            uLen -= uLenSrc;
        }

        /* Request the packet driver to send the frame */
    #if OPT_PKT_SEND_ASYNC
        if (pPacket->iSendAsync)
        {
            /* Use high performance async send */
            regs.h.ah = AS_SEND_PKT;
            regs.h.al = 0;
            regs.x.si = __tb & 15;
            regs.x.ds = __tb >> 4; /* ds:si -> real mode transfer buffer */
            regs.x.cx = len;
            regs.x.di = pPacket->txRmcb_seginfo.rm_offset;
            regs.x.es = pPacket->txRmcb_seginfo.rm_segment; /* es:di -> real mode callback */
            regs.x.ss = 0;
            regs.x.sp = 0;
            regs.x.flags = 0;
            __dpmi_int(pPacket->iInterrupt, &regs);

            if (!(C_FLAG & regs.x.flags))
                return -1; /* Pending */

            /* BUGBUG: FA310TX packet driver (ngrpci) claims to be high performance
            * but fails AS_SEND_PKT.
            */
            if (BAD_COMMAND == regs.h.dh)
            {
                TRACE(1, ("*** PacketSend driver claims high performance but isn't\n"));
                pPacket->iSendAsync = 0;
                ; /* Reset async send flag */
                /* Continue with SEND_PKT */
            }
            else
            {
                TRACE(1, ("*** PacketSend AS_SEND_PKT driver error %d\n", regs.h.dh));
                return SWS_BASEERR + 1000 + regs.h.dh;
            }
        }
    #endif

        regs.h.ah = SEND_PKT;
        regs.x.si = __tb & 15;
        regs.x.ds = __tb >> 4; /* ds:si -> real mode transfer buffer */
        regs.x.cx = len;
        regs.x.ss = 0;
        regs.x.sp = 0;
        regs.x.flags = 0;
        __dpmi_int(pPacket->iInterrupt, &regs);

        if (C_FLAG & regs.x.flags)
        {
            TRACE(1, ("*** PacketSend SEND_PKT driver error %d\n", regs.h.dh));
            return SWS_BASEERR + 1000 + regs.h.dh;
        }
    }

    return 0;
}

#if OPT_PKT_SEND_ASYNC
/*
 * Packet driver transmit upcall.
 * Called by the packet driver when packet sent
 *
 * CAUTION IRQ TIME
 */
static void PacketSent(_go32_dpmi_registers *pRegs)
{
    const int iEnabled = enable(); /* Enable interrupts */
    SPacketDriver *const pPacket = SWS_CONTAINER(pRegs, SPacketDriver, txRmcb_registers);

    /* Verify driver context */
    if (PACKET_MAGIC == pPacket->uMagic)
    {
        (*pPacket->upcalls.pfnSendAck)(pPacket->upcalls.pvContext, pRegs->x.ax);
    }

    /* Restore interrupt state */
    if (!iEnabled)
        disable();
}

static void SwsR6040_PacketSent(void *user_data)
{
    SPacketDriver *pPacket = (SPacketDriver *)user_data;

    /* Verify driver context */
    if (PACKET_MAGIC == pPacket->uMagic)
    {
        (*pPacket->upcalls.pfnSendAck)(pPacket->upcalls.pvContext, 0);
    }
}

static void SwsVN_PacketSent(void *user_data)
{
    SPacketDriver *pPacket = (SPacketDriver *)user_data;

    /* Verify driver context */
    if (PACKET_MAGIC == pPacket->uMagic)
    {
        (*pPacket->upcalls.pfnSendAck)(pPacket->upcalls.pvContext, 0);
    }
}
#endif

/*
 * Make a receive indication upcall
 */
static int PacketUpcall(
    SPacketDriver *const pPacket,
    unsigned long realAddress,
    unsigned len)
{
    size_t read;

    ASSERT(realAddress >= pPacket->dosmemAddress);
    ASSERT(realAddress + len <= pPacket->dosmemAddress + s_sizeBuffer);

    /* Copy data from real mode buffer to common buffer */
    ASSERT(1 == AtomicRead(&pPacket->aUpCalls));
    ASSERT(len <= sizeof(pPacket->buffer));
    dosmemget(realAddress, len, pPacket->buffer);

    /* Update real mode buffer read cursor */
    read = (size_t)((realAddress + len) - pPacket->dosmemAddress);
    if (read < s_sizeBuffer)
        pPacket->read = read;
    else
        pPacket->read = 0;

    /* Indicate data received */
    return (*pPacket->upcalls.pfnRecvInd)(
        pPacket->upcalls.pvContext, pPacket->buffer, len);
}

static int SwsR6040_PacketUpcall(
    SPacketDriver *const pPacket,
    R6040_PACKET_t packet,
    unsigned len)
{
    ASSERT(1 == AtomicRead(&pPacket->aUpCalls));
    ASSERT(len <= sizeof(pPacket->buffer));
    memcpy(pPacket->buffer, packet, len);
    r6040_ReleasePacket(pPacket->SwsR6040.handle, packet);

    /* Indicate data received */
    return (*pPacket->upcalls.pfnRecvInd)(
        pPacket->upcalls.pvContext, pPacket->buffer, len);
}

static int SwsVN_PacketUpcall(
    SPacketDriver *const pPacket,
    SWS_VN_FRAME_t packet,
    unsigned len)
{
    ASSERT(1 == AtomicRead(&pPacket->aUpCalls));
    ASSERT(len <= sizeof(pPacket->buffer));
    memcpy(pPacket->buffer, packet, len);
    SwsVN_ReleasePacket(pPacket->SwsVN.handle, packet);

    /* Indicate data received */
    return (*pPacket->upcalls.pfnRecvInd)(
        pPacket->upcalls.pvContext, pPacket->buffer, len);
}

/*
 * Receive complete.  Buffer is returned to driver
 * Single threaded
 */
SWS_CB(void, PacketRecvAck(
                 SWS_PTR(void) pvContext, /* IN: Driver's context */
                 SWS_PTR(void) pvData,    /* IN: Buffer */
                 size_t len               /* IN: Buffer size */
                 ))
{
    SPacketDriver *const pPacket = (SPacketDriver *)pvContext;
    (void)pvData;
    (void)len;

    TRACE(10, ("PacketRecvAck(%p,%p,%u)\n", pvContext, pvData, (unsigned)len));
    ASSERT(NULL != pPacket);
    ASSERT(PACKET_MAGIC == pPacket->uMagic);
    ASSERT(pvData == pPacket->buffer);
    ASSERT(1 == AtomicRead(&pPacket->aUpCalls));

    do
    {
        SPacketMsg msg;

        /* Make a reservation on aUpCalls */
        AtomicWrite(&pPacket->aUpCalls, 2);

        /* Indicate the next queued packet */
        if (!SWS_MsgqGet(pPacket->hRxQ, sizeof(msg), &msg))
        {
            /* Release reservation */
            AtomicWrite(&pPacket->aUpCalls, 1);

            /* Make upcall */
            if (pPacket->SwsVN.isActived)
            {
                if (SwsVN_PacketUpcall(pPacket, (SWS_VN_FRAME_t)msg.realAddress, msg.len) <= 0)
                    return;
            }
            else if (pPacket->SwsR6040.isActived)
            {
                if (SwsR6040_PacketUpcall(pPacket, (R6040_PACKET_t)msg.realAddress, msg.len) <= 0)
                    return;
            }
            else if (PacketUpcall(pPacket, msg.realAddress, msg.len) <= 0)
            {
                return; /* Pending */
            }

            ++s_errors;
        }
    }
    /* Repeat if reservation lost */
    while (2 != AtomicCmpXchg(&pPacket->aUpCalls, 2, 0));
}

/*
 * Packet driver receive upcall.
 * Called by the packet driver twice for each incoming packet:
 * 1. with ax == 0 to allocate a buffer, returned in es:di
 * 2. with ax == 1 & ds:si -> buffer to signal data available.
 * bx= handle
 * cx= len.
 *
 * CAUTION IRQ TIME
 */
void PacketRecv(_go32_dpmi_registers *pRegs)
{
    const int iEnabled = enable(); /* Enable interrupts */
    SPacketDriver *const pPacket = SWS_CONTAINER(pRegs, SPacketDriver, rmcb_registers);

    /* Verify driver context */
    if (PACKET_MAGIC == pPacket->uMagic && pRegs->x.bx == pPacket->handle)
    {
        unsigned len, space;
        unsigned long realAddress;

        len = pRegs->x.cx;
        switch (pRegs->x.ax)
        {
        case 0: /* Request for buffer */
            ++s_buffers;

            pPacket->saveWrite = pPacket->write;

            /* Calc. space remaining in real mode buffer */
            if (pPacket->write >= pPacket->read)
            {
                space = s_sizeBuffer - pPacket->write;
                if (0 == pPacket->read)
                    --space;

                if (len > space)
                {
                    /* Wraparound and try from start of buffer */
                    space = pPacket->read;
                    if (space > 0)
                        --space;

                    if (len < space)
                        pPacket->write = 0;
                }
            }
            else
            {
                space = (pPacket->read - pPacket->write) - 1;
            }

            if (len <= space)
            {
                /* Calc address of write cursor in the real mode buffer */
                unsigned long ulReal;

                ulReal = pPacket->dosmem.rm_segment * 16 + pPacket->dosmem.rm_offset + pPacket->write;
                pRegs->x.es = ulReal / 16;
                pRegs->x.di = ulReal % 16;

                /* Update write cursor */
                pPacket->write += len;
                ASSERT(pPacket->write <= s_sizeBuffer);
                if (pPacket->write >= s_sizeBuffer)
                    pPacket->write = 0;
            }
            else
            {
                /* Out of buffer space */
                pRegs->x.es = pRegs->x.di = 0;
                (*pPacket->upcalls.pfnRecvInd)(pPacket->upcalls.pvContext, NULL, 0);
            }
            break;

        case 1: /* Packet ready */
            ++s_packets;

            realAddress = (unsigned long)pRegs->x.ds * 16 + pRegs->x.si;
            ASSERT(realAddress >= pPacket->dosmemAddress);
            ASSERT(realAddress + len <= pPacket->dosmemAddress + s_sizeBuffer);

            /* Attempt to claim the common buffer */
            if (0 == AtomicXchg(&pPacket->aUpCalls, 1))
            {
                /* Succeeded, so make upcall */
                if (PacketUpcall(pPacket, realAddress, len) > 0)
                {
                    /* Error, so release common buffer */
                    AtomicWrite(&pPacket->aUpCalls, 0);
                    ++s_errors;

                    /* Drop the packet and restore the write cursor */
                    len += (unsigned)(realAddress - pPacket->dosmemAddress);
                    if (len >= s_sizeBuffer)
                        len = 0;
                    if (len == pPacket->write)
                        pPacket->write = pPacket->saveWrite;
                }
            }
            else
            {
                SPacketMsg msg;

                /* Queue the data */
                msg.realAddress = realAddress;
                msg.len = len;
                if (SWS_MsgqPut(pPacket->hRxQ, sizeof(SPacketMsg), &msg))
                {
                    /* MsgQ full so drop the packet and restore the write cursor */
                    ++s_overflows;

                    len += (unsigned)(realAddress - pPacket->dosmemAddress);
                    if (len >= s_sizeBuffer)
                        len = 0;
                    if (len == pPacket->write)
                        pPacket->write = pPacket->saveWrite;

                    /* Indicate an error */
                    (*pPacket->upcalls.pfnRecvInd)(pPacket->upcalls.pvContext, NULL, 0);
                }
            }
            break;

        default:
            ++s_badCmd;
            break;
        }
    }
    else
    {
        ++s_noHandle;
        pRegs->x.es = pRegs->x.di = 0; /* No buffer */
    }

    /* Restore interrupt state */
    if (!iEnabled)
        disable();
}

static void SwsR6040_PacketRecv(void *user_data)
{
    SPacketDriver *pPacket = (SPacketDriver *)user_data;

    /* Verify driver context */
    if (PACKET_MAGIC == pPacket->uMagic)
    {
        R6040_PACKET_t packet = r6040_RecvPacket(pPacket->SwsR6040.handle);

        if (packet == NULL)
        {
            (*pPacket->upcalls.pfnRecvInd)(pPacket->upcalls.pvContext, NULL, 0);
        }
        else
        {
            ++s_buffers;
            ++s_packets;
            /* Attempt to claim the common buffer */
            if (0 == AtomicXchg(&pPacket->aUpCalls, 1))
            {
                /* Succeeded, so make upcall */
                if (SwsR6040_PacketUpcall(pPacket, packet, packet->size + 14) > 0)
                {
                    /* Error, so release common buffer */
                    AtomicWrite(&pPacket->aUpCalls, 0);
                    ++s_errors;
                }
            }
            else
            {
                SPacketMsg msg;

                msg.realAddress = (unsigned long)packet;
                msg.len = packet->size + 14;

                if (SWS_MsgqPut(pPacket->hRxQ, sizeof(SPacketMsg), &msg))
                {
                    ++s_overflows;
                    /* Indicate an error */
                    (*pPacket->upcalls.pfnRecvInd)(pPacket->upcalls.pvContext, NULL, 0);
                }
            }
        }
    }
    else
    {
        ++s_noHandle;
    }
}

static void SwsVN_PacketRecv(void *user_data)
{
    SPacketDriver *pPacket = (SPacketDriver *)user_data;

    /* Verify driver context */
    if (PACKET_MAGIC == pPacket->uMagic)
    {
        size_t len;
        SWS_VN_FRAME_t packet = SwsVN_RecvPacket(pPacket->SwsVN.handle, &len);

        if (packet == NULL)
        {
            (*pPacket->upcalls.pfnRecvInd)(pPacket->upcalls.pvContext, NULL, 0);
        }
        else
        {
            ++s_buffers;
            ++s_packets;
            /* Attempt to claim the common buffer */
            if (0 == AtomicXchg(&pPacket->aUpCalls, 1))
            {
                /* Succeeded, so make upcall */
                if (SwsVN_PacketUpcall(pPacket, packet, len) > 0)
                {
                    /* Error, so release common buffer */
                    AtomicWrite(&pPacket->aUpCalls, 0);
                    ++s_errors;
                }
            }
            else
            {
                SPacketMsg msg;

                msg.realAddress = (unsigned long)packet;
                msg.len = len;

                if (SWS_MsgqPut(pPacket->hRxQ, sizeof(SPacketMsg), &msg))
                {
                    ++s_overflows;
                    /* Indicate an error */
                    (*pPacket->upcalls.pfnRecvInd)(pPacket->upcalls.pvContext, NULL, 0);
                }
            }
        }
    }
    else
    {
        ++s_noHandle;
    }
}

/*
 * Get the adapter's MAC address
 */
SWS_CB(int, PacketGetMacAddr(                                 /* Return !0 on error */
                             SWS_PTR(void) pvContext,         /* IN: Driver's context */
                             SWS_PTR(SWS_EnetMacAddress) pMac /* OUT: Mac address */
                             ))
{
    SPacketDriver *const pPacket = (SPacketDriver *)pvContext;

    TRACE(9, ("PacketGetMacAddr(%p,%p)\n", pvContext, pMac));
    ASSERT(NULL != pPacket);
    ASSERT(PACKET_MAGIC == pPacket->uMagic);
    ASSERT(NULL != pMac);

    if (!pPacket->macAddrIsValid)
    {
        if (pPacket->SwsVN.isActived)
        {
            SwsVN_GetMacAddress(pPacket->SwsVN.handle, pPacket->mac.addr);
        }
        else if (pPacket->SwsR6040.isActived)
        {
            r6040_GetMACAddress(pPacket->SwsR6040.handle, pPacket->mac.addr);
        }
        else
        {
            __dpmi_regs regs;

            /* Get MAC address from the packet driver */
            regs.h.ah = GET_ADDRESS;
            regs.x.bx = pPacket->handle;
            regs.x.di = __tb & 15;
            regs.x.es = __tb >> 4; /* es:di -> real mode transfer buffer */
            regs.x.cx = sizeof(SWS_EnetMacAddress);
            regs.x.ss = 0;
            regs.x.sp = 0;
            regs.x.flags = 0;
            if (__dpmi_int(pPacket->iInterrupt, &regs))
            {
                TRACE(1, ("*** PacketGetMacAddr dpmi_int error\n"));
                return 1;
            }
            if (C_FLAG & regs.x.flags)
            {
                TRACE(1, ("*** PacketGetMacAddr driver error %d\n", regs.h.dh));
                return 1;
            }

            /* Cache the address for later */
            dosmemget(__tb, sizeof(pPacket->mac.addr), pPacket->mac.addr);
        }
        pPacket->macAddrIsValid = 1;
    }

    *pMac = pPacket->mac;

    return 0;
}

/*
 * Handle private ioctls
 */
SWS_CB(int, PacketIoctl(                         /* Return !0 on error */
                        SWS_PTR(void) pvContext, /* IN: Driver's context */
                        unsigned uType,          /* IN: Type */
                        SWS_PTR(void) pvData,    /* I/O: -> data */
                        SWS_PTR(size_t) pSize    /* I/O: data size */
                        ))
{
    SPacketDriver *const pPacket = (SPacketDriver *)pvContext;
    unsigned mode;

    TRACE(9, ("PacketIoctl(%p,%u,%p,%p)\n", pvContext, uType, pvData, pSize));
    ASSERT(NULL != pPacket);
    ASSERT(PACKET_MAGIC == pPacket->uMagic);

    switch (uType)
    {
    default:
        TRACE(3, ("PacketIoctl %u not implemented\n", uType));
        return SWS_EBADF;

    case SWS_kDatalinkProperty_Private:
        if (sizeof(pPacket->SwsPriv) > *pSize)
            return SWS_ENOBUFS;
        memcpy(pvData, &pPacket->SwsPriv, sizeof(pPacket->SwsPriv));
        *pSize = sizeof(pPacket->SwsPriv);
        break;

    case SWS_kEnetIoctl_Promiscuity:
        /* En/dis/able promiscuous mode */
        if (*pSize != sizeof(ui32))
            return SWS_EINVAL;

        mode = *((SWS_PTR(ui32))pvData) ? 6 : 3;
        if (6 == mode || 6 == pPacket->rxmode)
            return PacketSetRxMode(pPacket, mode);

        break;

#if OPT_IP_MULTICAST
    case SWS_kEnetIoctl_MulticastList:
        if (0 == *pSize)
        {
            /* Disable multicast */
            if (4 == pPacket->rxmode || 5 == pPacket->rxmode)
                PacketSetRxMode(pPacket, 3);
        }
        /* Set multicast list */
        else if (0 == PacketSetMulticastList(pPacket, pvData, *pSize))
        {
            /* The FA310TX packet driver (ngrpci v2.21 (980811)) has a bug in
             * enabling multicast reception. However, if mode 5 (all multicast)
             * is selected before mode 4 and a multicast packet is sent then all
             * subsequent multicast packets are received.
             */
            if (3 == pPacket->rxmode)
            {
                PacketSetRxMode(pPacket, 5);
                PacketSetRxMode(pPacket, 4);
            }
            else if (5 == pPacket->rxmode)
            {
                PacketSetRxMode(pPacket, 4);
            }
        }
        /* Unable to set multicast filter so enable rx all multicast */
        else if (3 == pPacket->rxmode || 4 == pPacket->rxmode)
        {
            PacketSetRxMode(pPacket, 5);
        }
        break;
#endif

#if OPT_DEMO
    case SWS_kDatalinkProperty_Private + 0x1234:
        if (*pSize != sizeof(ui32))
            return SWS_EINVAL;

        *(SWS_PTR(ui32))pvData = (*(SWS_PTR(ui32))pvData) ^ 0xDEC0DEA5L;
        return 0;
#endif
    }

    return 0;
}

/*
 * Set receive mode
 * 1= turn off receiver
 * 2= only packets sent to this interface
 * 3= mode 2 plus broadcast packets (default)
 * 4= mode 3 plus limited multicast packets
 * 5= mode 3 plus all multicast packets
 * 6= all packets
 *
 * BUGBUG: FA310TX packet driver
 *  mode 1 == 3,
 *  modes 2,4,5 stop all receive data - must reload driver
 *  mode 6 == 1, ie not promiscuous
 */
static int PacketSetRxMode(
    SPacketDriver *const pPacket,
    unsigned mode)
{

    TRACE(9, ("PacketSetRxMode(%p,%u)\n", pPacket, mode));
    ASSERT(NULL != pPacket);
    ASSERT(PACKET_MAGIC == pPacket->uMagic);

    if (pPacket->SwsVN.isActived)
    {
        if (SwsVN_SetRxMode(pPacket->SwsVN.handle, mode) < 0)
        {
            TRACE(1, ("*** PacketSetRxMode() SET_RCV_MODE %d failed\n", mode));
            return SWS_EINVAL;
        }
    }
    else if (pPacket->SwsR6040.isActived)
    {
        if (r6040_SetRxMode(pPacket->SwsR6040.handle, mode) < 0)
        {
            TRACE(1, ("*** PacketSetRxMode() SET_RCV_MODE %d failed\n", mode));
            return SWS_EINVAL;
        }
    }
    else
    {
        __dpmi_regs regs;

        /* set_rcv_mode - extended function */
        regs.h.ah = SET_RCV_MODE;
        regs.x.bx = pPacket->handle;
        regs.x.cx = mode;
        regs.x.ss = 0;
        regs.x.sp = 0;
        regs.x.flags = 0;
        if (__dpmi_int(pPacket->iInterrupt, &regs))
        {
            TRACE(1, ("*** PacketSetRxMode dpmi_int error\n"));
            return SWS_EBADF;
        }
        else if (C_FLAG & regs.x.flags)
        {
            TRACE(1, ("*** PacketSetRxMode() SET_RCV_MODE %d failed %d\n", regs.x.cx, regs.h.dh));
            return SWS_EINVAL;
        }
    }

    pPacket->rxmode = mode;
    return 0;
}

#if OPT_IP_MULTICAST
/*
 * Set multicast list
 */
static int PacketSetMulticastList(
    SPacketDriver *const pPacket,
    SWS_PTR(void) pv,
    size_t size)
{

    TRACE(9, ("PacketSetMulticastList(%p,%p,%u)\n", pPacket, pv, size));
    ASSERT(NULL != pPacket);
    ASSERT(PACKET_MAGIC == pPacket->uMagic);

    if (pPacket->SwsVN.isActived)
    {
        if (SwsVN_SetMulticastList(pPacket->SwsVN.handle, pv, size) < 0)
        {
            TRACE(1, ("*** PacketSetMulticastList() failed\n"));
            return SWS_EINVAL;
        }
        return 0;
    }
    else if (pPacket->SwsR6040.isActived)
    {
        if (r6040_SetMulticastList(pPacket->SwsR6040.handle, pv, size) < 0)
        {
            TRACE(1, ("*** PacketSetMulticastList() failed\n"));
            return SWS_EINVAL;
        }
        return 0;
    }
    else
    {
        __dpmi_regs regs;

        /* Copy list to real mode transfer buffer */
        if (NULL != pv)
            dosmemput(pv, size, __tb);

        regs.h.ah = SET_MULTICAST_LIST; /* extended function */
        regs.x.bx = pPacket->handle;
        regs.x.cx = size;
        regs.x.di = __tb & 15;
        regs.x.es = __tb >> 4; /* es:di -> real mode transfer buffer */
        regs.x.ss = 0;
        regs.x.sp = 0;
        regs.x.flags = 0;
        if (__dpmi_int(pPacket->iInterrupt, &regs))
        {
            TRACE(1, ("*** PacketSetMulticastList dpmi_int error\n"));
            return SWS_EBADF;
        }
        else if (C_FLAG & regs.x.flags)
        {
            TRACE(1, ("*** PacketSetMulticastList() SET_MULTICAST_LIST %d failed %d\n", regs.x.cx, regs.h.dh));
            return SWS_EINVAL;
        }        
    }

    return 0;
}
#endif
#endif /* OPT_DATALINK_ETHERNET */
/* End of file */
