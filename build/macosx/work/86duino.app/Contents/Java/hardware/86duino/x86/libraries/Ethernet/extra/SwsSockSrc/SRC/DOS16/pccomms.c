/****************************************************************************
 *
 * File name   : pccomms.c
 * Function    : serial comms driver for DOS
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C.
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 07-Jan-03, LVR, Created.
 * 25-May-05, LVR, PCCOMMS_RXFIFO changed to 8 from 0.
 * 03-Aug-05, LVR, Added P2P
 * 08-Mar-06, LVR, Added WatcomC 1.4
 *
 * Comments:
 * --------
 * This file implements an 8250 serial comms driver.
 *
 ****************************************************************************
 */

/* Compilation options */
#ifndef OPT_SERIAL_ADAPTERS
#define OPT_SERIAL_ADAPTERS 2 /* Max no. serial,ports */
#endif

#include "config.h"   /* Build options */

/* Exports */
#include "pccomms.h"

#if OPT_DATALINK_P2P
/* Imports */
#include <stddef.h>
#include <string.h>   /* memcpy, strcmp */

#ifdef __DJGPP__
#include <sys/segments.h>
#include <sys/movedata.h>
#include <go32.h>
#include <dpmi.h>
#include <dos.h>
#include <pc.h>

#else
#include <dos.h>
#include <conio.h>
#endif

/* SwsSock */
#include "sws_sock.h" /* SWS_ENOBUFS */
#include "sws_str.h"  /* SWS_stricmp */
#include "sws_buf.h"  /* Buffer operations */
#include "p2p.h"      /* P2P driver registration */
#include "platform.h"

#include "debug.h"    /* TRACE, ASSERT & VERIFY */


/*
 * Macros & Constants
 */

/* Configurable vales */
#define PKEY_PORT   "port"    /* Numeric  */
#ifndef PCCOMMS_PORT
#define PCCOMMS_PORT 0        /* >>> disabled by default <<< */
#endif

#define PKEY_IRQ   "irq"      /* Numeric  */

#define PKEY_BAUD   "baud"    /* Numeric */
#ifndef PCCOMMS_BAUD
#define PCCOMMS_BAUD 38400U
#endif

#define PKEY_BITS   "bits"    /* Numeric */
#ifndef PCCOMMS_BITS
#define PCCOMMS_BITS 8
#endif

#define PKEY_PARITY "parity"  /* ASCIIZ odd, even, none, mark, space */
#ifndef PCCOMMS_PARITY
#define PCCOMMS_PARITY "none"
#endif

#define PKEY_STOP   "stop"    /* Numeric */
#ifndef PCCOMMS_STOP
#define PCCOMMS_STOP 1
#endif

#define PKEY_DUPLEX "duplex"  /* Numeric, boolean */
#ifndef PCCOMMS_DUPLEX
#define PCCOMMS_DUPLEX 1
#endif

#define PKEY_CTS "cts"        /* Numeric, boolean */
#ifndef PCCOMMS_CTS
#define PCCOMMS_CTS 0
#endif

#define PKEY_DSR "dsr"        /* Numeric, boolean */
#ifndef PCCOMMS_DSR
#define PCCOMMS_DSR 0
#endif

#define PKEY_DTR "dtr"        /* ASCIIZ enable, disable, handshake */
#ifndef PCCOMMS_DTR
#define PCCOMMS_DTR "enable"
#endif

#define PKEY_RTS "rts"        /* ASCIIZ enable, disable, handshake */
#ifndef PCCOMMS_RTS
#define PCCOMMS_RTS "enable"
#endif

#define PKEY_RXFIFO "rxfifo"  /* Numeric, 0,1,4,8,14 */
#ifndef PCCOMMS_RXFIFO
#define PCCOMMS_RXFIFO 8
#endif

#define PKEY_TXFIFO "txfifo"  /* Numeric, 0..15 */
#ifndef PCCOMMS_TXFIFO
#define PCCOMMS_TXFIFO 8
#endif


/* PC hardware constants */
#define IRQBASE0   0x08       /* IRQ0 vector no. */
#define IRQBASE8   0x70       /* IRQ8 vector no. */

#define COM1       0x3F8      /* COM1 - IRQ4 */
#define COM2       0x2F8      /* COM2 - IRQ3 */
#define COM3       0x330      /* COM3 - IRQ4 */
#define COM4       0x338      /* COM4 - IRQ3 */
#define COM5       0x280
#define COM6       0x288
#define COM7       0x290
#define COM8       0x298

/* 8250/16450/16550 register offsets */
#define DVLATLO   0     /* Baud rate divider lo byte */
#define DVLATHI   1     /* Baud rate divider hi byte */
#define TX_REG    0     /* Tx data - write */
#define RX_REG    0     /* Rx data - read */
#define INTNREG   1     /* R/W Interrupt ENABLE register */
#define INTDREG   2     /* R/O: Interrupt status register */
#define FIFOREG   2     /* W/O: 16550 FIFO control register */
#define LINCREG   3     /* R/W Line control */
#define MDMCREG   4     /* R/W Modem control */
#define LINSREG   5     /* R/O Line status */
#define MDMSREG   6     /* R/O Modem status */
#define PADREG    7     /* Scratch PAD */

/* Line control register bits */
#define LCR_DLAB   0x80
#define LCR_BRK    0x40
#define LCR_SPAR   0x20
#define LCR_EPAR   0x10
#define LCR_PREN   0x08
#define LCR_2SBIT  0x04
#define LCR_1SBIT  0x00
#define LCR_8DBIT  0x03
#define LCR_7DBIT  0x02
#define LCR_6DBIT  0x01
#define LCR_5DBIT  0x00

#define LCR_NOPARITY    (0 * LCR_PREN)
#define LCR_EVENPARITY  (LCR_EPAR | LCR_PREN)
#define LCR_ODDPARITY   LCR_PREN
#define LCR_MARKPARITY  (LCR_SPAR | LCR_PREN)
#define LCR_SPACEPARITY (LCR_SPAR | LCR_EPAR | LCR_PREN)

/* Modem control register bits */
#define MCR_LOOP   0x10     /* Loop back */
#define MCR_OUT2   0x08
#define MCR_OUT1   0x04     /* Interrupt DISABLE */
#define MCR_RTS    0x02
#define MCR_DTR    0x01
#define MCR_MASK   0x0f

/* Line status register bits */
#define LSR_TIME   0x80     /* Rx FIFO error */
#define LSR_TEMT   0x40     /* Tx shift register empty */
#define LSR_THRE   0x20     /* Tx holding register empty */
#define LSR_BRKI   0x10
#define LSR_FERR   0x08
#define LSR_PERR   0x04
#define LSR_OVRN   0x02
#define LSR_DRDY   0x01

/* Modem status register bits */
#define MSR_DCD    0x80
#define MSR_RING   0x40
#define MSR_DSR    0x20
#define MSR_CTS    0x10
#define MSR_DDCD   0x08
#define MSR_TERI   0x04
#define MSR_DDSR   0x02
#define MSR_DCTS   0x01

/* Interrupt ENABLE register bits */
#define INTONRX    0x01
#define INTONTX    0x02
#define INTONERR   0x04
#define INTONMDM   0x08
#define INTMASK    0x0f

/* FIFO control register bits */
#define FIFO_ENA   0x01
#define FIFO_RXRST 0x02
#define FIFO_TXRST 0x04
#define FIFO_RXLS  0x40
#define FIFO_RXMS  0x80
#define FIFO_RX1   (0*FIFO_RXMS | 0*FIFO_RXLS)
#define FIFO_RX4   (0*FIFO_RXMS | 1*FIFO_RXLS)
#define FIFO_RX8   (1*FIFO_RXMS | 0*FIFO_RXLS)
#define FIFO_RX14  (1*FIFO_RXMS | 1*FIFO_RXLS)
#define FIFO_TXMAX 15


/* Baud rate dividers */
#define BAUD110   0x417
#define BAUD150   0x300
#define BAUD300   0x180
#define BAUD600   0x0C0
#define BAUD1200  0x060
#define BAUD2400  0x030
#define BAUD4800  0x018
#define BAUD9600  0x00C
#define BAUD19200 0x006
#define BAUD38400 0x003
#define BAUD57600 0x002
#define BAUD115200 0x001

/* 8259 PIC register addresses */
#define PIC_OCR     0x20    /* Operation control register */
#define PIC_IMR     0x21    /* Interrupt mask */
#define PICS_OCR    0xa0    /* Operation control register */
#define PICS_IMR    0xa1    /* Interrupt mask */

/* 8259 register contents */
#define EOI_NS      0x20    /* Non-specific end of interrupt */
#define EOI_IRQ3    0x63
#define EOI_IRQ4    0x64
#define ENABLE_IRQ3 0xF7
#define ENABLE_IRQ4 0xEF


/* I/O accessors */
#ifdef __DJGPP__

#define OUTPB( p, v) outportb( p, v)
#define INPB( p) ((ui8)inportb( p))
#define DISABLE() disable()
#define ENABLE() enable()
#define INTERRUPT( ret, fn) ret fn( void)

#elif defined __WATCOMC__ && defined _DOS && !defined _DOS16

extern int SWS_PcCommsIdisable( void);
#pragma aux SWS_PcCommsIdisable = \
  "mov eax, 0x900"\
  "int 0x31" \
  "and eax, 1" \
   value [eax]

extern void SWS_PcCommsIenable( void);
#pragma aux SWS_PcCommsIenable = \
  "mov eax, 0x901"\
  "int 0x31" \
   modify [eax]

#define OUTPB( p, v) outp( p, v)
#define INPB( p) ((ui8)inp( p))
#define DISABLE() SWS_PcCommsIdisable()
#define ENABLE() SWS_PcCommsIenable()
#define INTERRUPT( ret, fn) ret __interrupt __far fn()

#else

#if defined __BORLANDC__ || defined __WATCOMC__
#define OUTPB( p, v) outp( p, v)
#define INPB( p) ((ui8)inp( p))
#else
#define OUTPB( p, v) _outp( p, v)
#define INPB( p) ((ui8)_inp( p))
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4704)    /* in-line assembler precludes global optimizations */
#endif
static int idisable( void)
  {
  register int enabled = 0;

  __asm pushf
  __asm cli
  __asm pop enabled
  __asm and enabled, 0x200

  return enabled;
  }
#ifdef _MSC_VER
#pragma warning( default : 4704)
#endif

#define DISABLE() idisable()
#define ENABLE() _enable()
#define INTERRUPT( ret, fn) ret __interrupt __far fn()

#ifdef _WINDOWS
typedef INTERRUPT( void, FWinInterrupt);
static void __cdecl _dos_setvect( unsigned uIntr, FWinInterrupt __far * lpfn)
  {
  _asm  mov     ah, 0x25
  _asm  mov     al, BYTE PTR uIntr
  _asm  push    ds
  _asm  lds     dx, dword ptr lpfn
  _asm  int     0x21
  _asm  pop     ds
  }
#endif
#endif

typedef INTERRUPT( void, FInterrupt);


/*
 * Local data types
 */
typedef struct SPcComms
  {
  ui32 uMagic;
#define PCCOMMS_MAGIC SWS_MAKEFOURCC('p','c','c','m')

  int iPort;

  unsigned combase;                   /* I/O base address of UART */
  unsigned irqvec;                    /* IRQ vector for this UART */
  unsigned picimr;
  unsigned picocr;
  ui8 picmask;                        /* PIC IRQ ENABLE mask */
  ui8 fifo;
  ui8 uTxFifo;
  char bHalfDuplex;
  char bUseCts;
  char bUseDsr;
  char bRtsHandshake;
  char bDtrHandshake;
  ui32 ulBaud;

#ifdef __DJGPP__
  _go32_dpmi_seginfo old_handler;
  _go32_dpmi_seginfo new_handler;
#else
  FInterrupt* pfnIsr;
#endif

  SWS_P2pCallbacks upcalls;          /* Up call info */

  volatile int inInterrupt;

  int volatile isTxPending;
  SWS_PTR( const ui8) volatile pTxBuffer;
  size_t volatile sizeTx;

  SWS_PTR( ui8) volatile pRxBuffer;
  size_t volatile received;
  Atomic_t aRx;
  int iEnd;

  /* Stats */
  volatile ui32 uTxChars, uRxChars;
  volatile ui32 uFramingErrors, uParityErrors, uOverrunErrors;
#if defined __WATCOMC__ && defined _DOS
  char stack[ 16384];
#endif
  } SPcComms;


/*
 * Prototypes
 */
static int AddSerialPort( const int);
static void FreeSerialPort( SPcComms* const);

static SWS_FP2pSend PcCommsSend;
static SWS_FP2pRecv PcCommsRecv;
static SWS_FP2pShutdown PcCommsShutdown;
static SWS_FP2pIoctl PcCommsIoctl;
static int PcCommsOnline( SPcComms*, int);
static unsigned PcCommsStatus( SPcComms*);
static void RxDisable( SPcComms*);

static void PcCommsInterruptHandler( SPcComms* const);

#define IRQHANDLER( irq) PcCommsInterruptHandler##irq
#define DECL_IRQHANDLER( irq) INTERRUPT( void, IRQHANDLER( irq) )

static DECL_IRQHANDLER( 0);
#if OPT_SERIAL_ADAPTERS > 1
static DECL_IRQHANDLER( 1);
#endif
#if OPT_SERIAL_ADAPTERS > 2
static DECL_IRQHANDLER( 2);
#endif
#if OPT_SERIAL_ADAPTERS > 3
static DECL_IRQHANDLER( 3);
#endif
#if OPT_SERIAL_ADAPTERS > 4
static DECL_IRQHANDLER( 4);
#endif
#if OPT_SERIAL_ADAPTERS > 5
static DECL_IRQHANDLER( 5);
#endif
#if OPT_SERIAL_ADAPTERS > 6
static DECL_IRQHANDLER( 6);
#endif
#if OPT_SERIAL_ADAPTERS > 7
static DECL_IRQHANDLER( 7);
#endif
#if OPT_SERIAL_ADAPTERS > 8
#error OPT_SERIAL_ADAPTERS set too high
#endif


/*
 * Module data
 */

static SPcComms* s_pAdapters[ OPT_SERIAL_ADAPTERS]; /* Array of interfaces */

static FInterrupt* const s_pfnInterrupt[ OPT_SERIAL_ADAPTERS] = {
  & IRQHANDLER( 0)
#if OPT_SERIAL_ADAPTERS > 1
  , & IRQHANDLER( 1)
#endif
#if OPT_SERIAL_ADAPTERS > 2
  , & IRQHANDLER( 2)
#endif
#if OPT_SERIAL_ADAPTERS > 3
  , & IRQHANDLER( 3)
#endif
#if OPT_SERIAL_ADAPTERS > 4
  , & IRQHANDLER( 4)
#endif
#if OPT_SERIAL_ADAPTERS > 5
  , & IRQHANDLER( 5)
#endif
#if OPT_SERIAL_ADAPTERS > 6
  , & IRQHANDLER( 6)
#endif
#if OPT_SERIAL_ADAPTERS > 7
  , & IRQHANDLER( 7)
#endif
#if OPT_SERIAL_ADAPTERS > 8
#error OPT_SERIAL_ADAPTERS set too high
#endif
  };

/* P2P device descriptor */
static char s_szIdent[] = SWS_PCCOMMS_IDENT "01";
static const SWS_P2pDevice s_PcComms =
  {
  sizeof( SWS_P2pDevice),
  SWS_P2P_VERSION,
  s_szIdent,
  &PcCommsSend,
  &PcCommsRecv,
  &PcCommsShutdown,
  &PcCommsIoctl
  };


/*
 * Initialise the driver
 */
extern int SWS_PcCommsInstall( void)    /* Returns !0 on error */
  {
  int i, iAdapters;

  TRACE( 9, ("SWS_PcCommsInstall()\n"));
  ASSERT( PlatformIsCritical());

  /* Search for serial ports */
  iAdapters = 0;
  for ( i = 0; i < OPT_SERIAL_ADAPTERS && iAdapters < OPT_DATALINK_P2P; ++i)
    {
    int err = AddSerialPort( i);
    if ( !err)
      ++iAdapters;
    }

  if ( 0 == iAdapters)
    {
    TRACE( 1, ("*** SWS_PcCommsInstall() no adapters found\n"));
    return SWS_ENETDOWN;
    }
  return 0;
  }


/*
 * Finalise the driver
 * Single threaded
 */
extern void SWS_PcCommsCleanup( void)
  {
  int i;

  TRACE( 9, ("SWS_PcCommsCleanup()\n"));
  ASSERT( PlatformIsCritical());

  /* All the drivers should have been shutdown by now */
  for ( i = 0; i < OPT_SERIAL_ADAPTERS; ++i)
    {
    SPcComms* const pAdapter = s_pAdapters[ i];
    if ( NULL != pAdapter)
      {
      s_pAdapters[ i] = NULL;
      FreeSerialPort( pAdapter);
      }
    }
  }


/*
 * Create an SwsSock interface for a serial port
 */
static int AddSerialPort( const int iPort)
  {
  SPcComms* pAdapter;
  int iRet;
  ui32 val;

  TRACE( 9, ("AddSerialPort(%d)\n",iPort));

  sprintf( s_szIdent, SWS_PCCOMMS_IDENT "%d", 1 + iPort);

  val = PlatformPropertyGetUint( s_szIdent, PKEY_PORT, PCCOMMS_PORT );
  if ( 0 == val)
    {
    /* Disabled */
    TRACE( 2, ("AddSerialPort(%d) no port\n", iPort));
    return SWS_ENODEV;
    }

  /* Allocate instance data */
  pAdapter = (SPcComms*)calloc( 1, sizeof(*pAdapter) );
  if ( NULL == pAdapter)
    {
    TRACE( 0, ("!!! AddSerialPort() Out of memory for instance\n"));
    return SWS_ENOMEM;
    }

  pAdapter->uMagic = PCCOMMS_MAGIC;
  pAdapter->iPort = iPort;

  /* Add to list of drivers */
  s_pAdapters[ iPort] = pAdapter;

  /* Get configuration data */
  iRet = 0;
  for (;;)
    {
    char szBuf[ 64];
    unsigned divisor;
    ui8 linectrl;
    ui8 modemctrl = 0;
    int bInterruptsWereEnabled;

    /* Base address */
    switch ( val)
      {
    /* COMn */
    case 1: pAdapter->combase = COM1; val = 4; break;
    case 2: pAdapter->combase = COM2; val = 3; break;
    case 3: pAdapter->combase = COM3; val = 4; break;
    case 4: pAdapter->combase = COM4; val = 3; break;
    default:
      /* Assume i/o port number */
      pAdapter->combase = (unsigned)val;
      val = 4;
      break;
      }

    /* Interrupt */
    pAdapter->irqvec = (unsigned)PlatformPropertyGetUint(
      s_szIdent, PKEY_IRQ, val );
    pAdapter->picmask = (ui8)~(1 << (pAdapter->irqvec % 8));
    pAdapter->picimr = pAdapter->irqvec < 8 ? PIC_IMR : PICS_IMR;
    pAdapter->picocr = pAdapter->irqvec < 8 ? PIC_OCR : PICS_OCR;

    switch ( pAdapter->irqvec)
      {
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
      pAdapter->irqvec += IRQBASE0;
      break;

    case 9:
      /* Irq 9 is mapped to 2 */
      pAdapter->irqvec = IRQBASE0 + 2;
      break;

    case 8:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
      pAdapter->irqvec += IRQBASE8 - 8;
      break;

    default:
      TRACE( 0, ("!!! AddSerialPort() Invalid irq:%u\n", pAdapter->irqvec ));
      iRet = SWS_EINVAL;
      goto error;
      }

    /* Baud rate */
    pAdapter->ulBaud = PlatformPropertyGetUint(
      s_szIdent, PKEY_BAUD, PCCOMMS_BAUD );
    switch ( pAdapter->ulBaud)
      {
    case 110:    divisor = BAUD110; break;
    case 150:    divisor = BAUD150; break;
    case 300:    divisor = BAUD300; break;
    case 600:    divisor = BAUD600; break;
    case 1200:   divisor = BAUD1200; break;
    case 2400:   divisor = BAUD2400; break;
    case 4800:   divisor = BAUD4800; break;
    case 9600:   divisor = BAUD9600; break;
    case 19200:  divisor = BAUD19200; break;
    case 38400U:  divisor = BAUD38400; break;
    case 57600U:  divisor = BAUD57600; break;
    case 115200UL: divisor = BAUD115200; break;
    default:
      TRACE( 0, ("!!! AddSerialPort() Unsupported baud rate '%lu'\n",
        pAdapter->ulBaud));
      pAdapter->ulBaud = 38400UL;
      divisor = BAUD38400;
      break;
      }

    /* Parity */
    PlatformPropertyGetString(
      s_szIdent,
      PKEY_PARITY,
      PCCOMMS_PARITY,
      szBuf,
      sizeof( szBuf)
    );
    if ( !SWS_stricmp( szBuf, "none"))
      linectrl = LCR_NOPARITY;
    else if ( !SWS_stricmp( szBuf, "odd"))
      linectrl = LCR_ODDPARITY;
    else if ( !SWS_stricmp( szBuf, "even"))
      linectrl = LCR_EVENPARITY;
    else
      {
      TRACE( 0, ("!!! AddSerialPort() Invalid parity '%s'\n", szBuf ));
      linectrl = LCR_NOPARITY;
      }

    /* Stop bits */
    val = PlatformPropertyGetUint( s_szIdent, PKEY_STOP, PCCOMMS_STOP );
    if ( 1 == val)
      linectrl |= LCR_1SBIT;
    else if ( 2 == val)
      linectrl |= LCR_2SBIT;
    else
      TRACE( 0, ("!!! AddSerialPort() Invalid stop bits %lu\n", val ));

    /* Word length */
    val = PlatformPropertyGetUint( s_szIdent, PKEY_BITS, PCCOMMS_BITS );
    if ( 8 == val)
      linectrl |= LCR_8DBIT;
    else if ( 7 == val)
      linectrl |= LCR_7DBIT;
    else
      TRACE( 0, ("!!! AddSerialPort() Invalid data bits %lu\n", val ));

    /* FIFO */
    val = PlatformPropertyGetUint(
      s_szIdent, PKEY_RXFIFO, PCCOMMS_RXFIFO );
    switch ( val)
      {
    case 0: break;
    case 1: pAdapter->fifo = FIFO_RX1 | FIFO_ENA; break;
    case 2:
    case 3:
    case 4: pAdapter->fifo = FIFO_RX4 | FIFO_ENA; break;
    case 5:
    case 6:
    case 7:
    case 8: pAdapter->fifo = FIFO_RX8 | FIFO_ENA; break;
    default:
    case 14: pAdapter->fifo = FIFO_RX14 | FIFO_ENA; break;
      }

    val = PlatformPropertyGetUint(
      s_szIdent, PKEY_TXFIFO, PCCOMMS_TXFIFO );
    if ( 0 == val)
      {
      pAdapter->uTxFifo = 1;
      }
    else if ( val <= FIFO_TXMAX)
      {
      pAdapter->uTxFifo = (ui8)val;
      pAdapter->fifo |= FIFO_ENA;
      }
    else
      {
      pAdapter->uTxFifo = FIFO_TXMAX;
      pAdapter->fifo |= FIFO_ENA;
      }

    /* CTS */
    pAdapter->bUseCts = (char)(0 != PlatformPropertyGetUint(
      s_szIdent, PKEY_CTS, PCCOMMS_CTS ));

    /* DSR */
    pAdapter->bUseDsr = (char)(0 != PlatformPropertyGetUint(
      s_szIdent, PKEY_DSR, PCCOMMS_DSR ));

    /* DTR */
    PlatformPropertyGetString(
      s_szIdent,
      PKEY_DTR,
      PCCOMMS_DTR,
      szBuf,
      sizeof( szBuf)
    );
    if ( !SWS_stricmp( szBuf, "enable"))
      modemctrl |= MCR_DTR;
    else if ( !SWS_stricmp( szBuf, "disable"))
      modemctrl &= ~MCR_DTR;
    else if ( !SWS_stricmp( szBuf, "handshake"))
      {
      modemctrl &= ~MCR_DTR;
      pAdapter->bDtrHandshake = 1;
      }
    else
      TRACE( 0, ("!!! AddSerialPort() Invalid DTR option '%s'\n", szBuf ));

    /* Full/half duplex */
    pAdapter->bHalfDuplex = (char)(0 == PlatformPropertyGetUint(
      s_szIdent, PKEY_DUPLEX, PCCOMMS_DUPLEX ));

    /* RTS */
    if ( pAdapter->bHalfDuplex)
      {
      modemctrl &= ~MCR_RTS;
      }
    else
      {
      PlatformPropertyGetString(
        s_szIdent,
        PKEY_RTS,
        PCCOMMS_RTS,
        szBuf,
        sizeof( szBuf)
      );
      if ( !SWS_stricmp( szBuf, "enable"))
        modemctrl |= MCR_RTS;
      else if ( !SWS_stricmp( szBuf, "disable"))
        modemctrl &= ~MCR_RTS;
      else if ( !SWS_stricmp( szBuf, "handshake"))
        {
        modemctrl &= ~MCR_RTS;
        pAdapter->bRtsHandshake = 1;
        }
      else
        TRACE( 0, ("!!! AddSerialPort() Invalid RTS option '%s'\n", szBuf ));
      }

    /* Is this a working device? */
    OUTPB( pAdapter->combase + PADREG, 0xa5 );
    OUTPB( pAdapter->combase + MDMCREG, 0 );
    if ( 0xa5 != INPB( pAdapter->combase + PADREG))
      {
      TRACE( 0, ("!!! AddSerialPort() UART not detected\n" ));
      iRet = SWS_ENETDOWN;
      break;
      }

    /* Disable device interrupts */
    bInterruptsWereEnabled = DISABLE();
    OUTPB( pAdapter->picimr, INPB( pAdapter->picimr ) | ~pAdapter->picmask );
    OUTPB( pAdapter->combase + INTNREG, 0 );
    OUTPB( pAdapter->combase + MDMCREG, 0 );    /* Tristate IRQ */
    if ( bInterruptsWereEnabled)
      ENABLE();

    /* Configure serial data format */
    OUTPB( pAdapter->combase + LINCREG, LCR_DLAB ); /* select divisor reg. */
    OUTPB( pAdapter->combase + DVLATLO, (ui8) divisor );    /* set low byte of baud rate */
    OUTPB( pAdapter->combase + DVLATHI, (ui8) (divisor >> 8) );
    OUTPB( pAdapter->combase + LINCREG, linectrl );

    if ( pAdapter->fifo)
      OUTPB( pAdapter->combase + FIFOREG, pAdapter->fifo | FIFO_TXRST | FIFO_RXRST );

    /* Clear any pending IRQ */
    while ( !(1 & INPB( pAdapter->combase + INTDREG)) )
      {
      INPB( pAdapter->combase + LINSREG);
      INPB( pAdapter->combase + MDMSREG);
      INPB( pAdapter->combase + RX_REG);
      }

    /* Hook CPU IRQ's */
#ifdef __DJGPP__
    pAdapter->new_handler.pm_selector = _go32_my_cs();
    pAdapter->new_handler.pm_offset = (unsigned long) s_pfnInterrupt[ iPort];
    if ( _go32_dpmi_allocate_iret_wrapper( &pAdapter->new_handler))
      {
      TRACE( 1, ("*** AddSerialPort() _go32_dpmi_allocate_iret_wrapper failed\n"));
      iRet = SWS_EINVAL;
      break;
      }
    _go32_dpmi_get_protected_mode_interrupt_vector( pAdapter->irqvec, &pAdapter->old_handler);
    if ( _go32_dpmi_set_protected_mode_interrupt_vector( pAdapter->irqvec, &pAdapter->new_handler))
      {
      TRACE( 1, ("*** AddSerialPort() _go32_dpmi_set_protected_mode_interrupt_vector failed\n"));
      iRet = SWS_EINVAL;
      break;
      }
#else
    pAdapter->pfnIsr = _dos_getvect( pAdapter->irqvec);
    _dos_setvect( pAdapter->irqvec, s_pfnInterrupt[ iPort]);
#endif

    /* Enable device interrupts */
    OUTPB( pAdapter->combase + MDMCREG, MCR_OUT2 | modemctrl);
    OUTPB( pAdapter->picimr, INPB( pAdapter->picimr) & pAdapter->picmask );
    OUTPB( pAdapter->combase + INTNREG, INTONMDM);

    /* Register the adapter with the SwsSock stack */
    iRet = SWS_P2pAddAdapter( &s_PcComms, pAdapter, &pAdapter->upcalls);
    if ( iRet)
      break;

    ASSERT( pAdapter->upcalls.pfnSendAck);
    ASSERT( pAdapter->upcalls.pfnRecvInd);
    ASSERT( pAdapter->upcalls.pfnStatInd);

    return 0;
    }

error:
  ASSERT( iRet);

  /* Release resources on error */
  s_pAdapters[ iPort] = NULL;
  FreeSerialPort( pAdapter);

  return iRet;
  }



/*
 * Callback from P2P module to shutdown an adapter
 * Single threaded
 */
static SWS_CB( void, PcCommsShutdown(
  SWS_PTR( void) pvContext            /* IN: Client context */
)){
  int i;

  TRACE( 9, ("W32CommsShutdown(%p)\n", pvContext));
  ASSERT( NULL != pvContext);

  /* Search for the driver */
  for ( i = 0; i < OPT_SERIAL_ADAPTERS; ++i)
    {
    SPcComms* const pAdapter = s_pAdapters[ i];
    if ( (SPcComms*)pvContext == pAdapter)
      {
      /* Shut it down */
      s_pAdapters[ i] = NULL;
      FreeSerialPort( pAdapter);
      return;
      }
    }

  TRACE( 1, ("*** W32CommsShutdown() Client %p not found)\n", pvContext));
  }


/*
 * Release serial driver resources
 */
static void FreeSerialPort( SPcComms* const pAdapter)
  {
  TRACE( 9, ("FreeSerialPort(%p)\n",pAdapter));
  ASSERT( NULL != pAdapter);

  if ( pAdapter->combase)
    {
    /* Disable device interrupts */
    int bInterruptsWereEnabled = DISABLE();
    OUTPB( pAdapter->picimr, INPB( pAdapter->picimr ) | ~pAdapter->picmask );
    OUTPB( pAdapter->combase + INTNREG, 0 );
    OUTPB( pAdapter->combase + MDMCREG, 0 );  /* Tristate IRQ */
    if ( bInterruptsWereEnabled)
      ENABLE();
    }

#ifdef __DJGPP__
  if ( pAdapter->old_handler.pm_offset)
    _go32_dpmi_set_protected_mode_interrupt_vector( pAdapter->irqvec, &pAdapter->old_handler );
  if ( pAdapter->new_handler.pm_offset)
    _go32_dpmi_free_iret_wrapper( &pAdapter->new_handler);
#else
  if ( pAdapter->pfnIsr)
    _dos_setvect( pAdapter->irqvec, pAdapter->pfnIsr);
#endif

  TRACE( 1, ("PcComms: TxChars:%lu RxChars:%lu"
    " RxOverflows:%lu FramingErrors:%lu ParityErrors:%lu\n",
    pAdapter->uTxChars, pAdapter->uRxChars,
    pAdapter->uOverrunErrors, pAdapter->uFramingErrors,
    pAdapter->uParityErrors
  ));

  /* Release resources */
  free( pAdapter);
  }


/*
 * Handle private ioctls
 */
static SWS_CB( int, PcCommsIoctl(     /* Return !0 on error */
  SWS_PTR( void) pvContext,           /* IN: Driver's context */
  unsigned uType,                     /* IN: Type */
  SWS_PTR( void) pvData,              /* I/O: -> data */
  SWS_PTR( size_t) pSize              /* I/O: data size */
)){
  SPcComms* const pAdapter = (SPcComms*)pvContext;

  TRACE( 9, ("PcCommsIoctl(%p,%u,%p,%p)\n",pvContext,uType,pvData,pSize));
  ASSERT( NULL != pAdapter);
  ASSERT( PCCOMMS_MAGIC == pAdapter->uMagic);

  switch ( uType)
    {
  default:
    TRACE( 3, ("PcCommsIoctl %u not implemented\n", uType));
    return SWS_EBADF;

  case SWS_kP2pIoctl_SetOnLine:
    /* En/dis/able ready/online state */
    if ( *pSize != sizeof(ui32))
      return SWS_EINVAL;

    return PcCommsOnline( pAdapter, *((SWS_PTR( ui32)) pvData) ? 1 : 0);

  case SWS_kP2pIoctl_GetBaud:
    if ( *pSize != sizeof(ui32))
      return SWS_EINVAL;

    *(SWS_PTR( ui32)) pvData = 0;
    return 0;

  case SWS_kP2pIoctl_GetModemStatus:
    if ( *pSize != sizeof(ui8))
      return SWS_EINVAL;

    *(SWS_PTR( ui8))pvData = (ui8)PcCommsStatus( pAdapter);
    return 0;
    }
  }


/*
 * En/dis/able online state
 */
static int PcCommsOnline( SPcComms* pAdapter, int iOnLine)
  {
  ui8 reg;
  int bInterruptsWereEnabled;

  TRACE( 9, ("PcCommsOnline(%p,%d)\n",pAdapter,iOnLine));
  ASSERT( NULL != pAdapter);
  ASSERT( PCCOMMS_MAGIC == pAdapter->uMagic);

  bInterruptsWereEnabled = pAdapter->inInterrupt ? 0 : DISABLE();

  reg = INPB( pAdapter->combase + MDMCREG);
  reg &= MCR_MASK;
  if ( iOnLine)
    reg |= MCR_DTR;
  else
    reg &= ~MCR_DTR;

  OUTPB( pAdapter->combase + MDMCREG, reg);

  if ( bInterruptsWereEnabled)
    ENABLE();

  return 0;
  }


/*
 * Get modem status
 */
static unsigned PcCommsStatus( SPcComms* pAdapter)
  {
  unsigned uStatus;
  ui8 uReg;

  TRACE( 9, ("PcCommsStatus(%p)\n",pAdapter));
  ASSERT( NULL != pAdapter);
  ASSERT( PCCOMMS_MAGIC == pAdapter->uMagic);

  uReg = INPB( pAdapter->combase + MDMSREG);

  uStatus = 0;
  if ( MSR_DCD & uReg)
    uStatus |= SWS_P2P_MODEM_DCD;
  if ( MSR_RING & uReg)
    uStatus |= SWS_P2P_MODEM_RNG;
  if ( MSR_DSR & uReg)
    uStatus |= SWS_P2P_MODEM_DSR;

  return uStatus;
  }


#ifdef __BORLANDC__
#pragma option -N- /* disable stack checking */
#endif
#ifdef _MSC_VER
#pragma check_stack( off)
#endif

/*
 * Transmit a buffer
 * CAUTION May be re-entered from PcCommsInterruptHandler upcall
 */
static SWS_CB( int, PcCommsSend(      /* Return !0 on error, -1 if pending */
  SWS_PTR( void) pvContext,           /* IN: Driver's context */
  SWS_PTR( const void) pvData,        /* IN: Data to send */
  size_t len                          /* IN: Byte count */
)){
  SPcComms* const pAdapter = (SPcComms*)pvContext;
  int bInterruptsWereEnabled;
  ui8 reg;
  int iTxEnable;

  TRACE( 10, ("PcCommsSend(%p,%p,%u)\n",pvContext,pvData,len));
  /*ASSERT( PlatformIsCritical()); Only valid if not in callback from IRQ */
  ASSERT( NULL != pAdapter);
  ASSERT( PCCOMMS_MAGIC == pAdapter->uMagic);
  ASSERT( 0 == pAdapter->sizeTx);

  if ( 0 == len)
    return 0;

  if ( NULL == pvData)
    {
    TRACE( 1, ("***PcCommsSend(%p) NULL buffer\n",pvContext));
    return SWS_EINVAL;
    }

  pAdapter->isTxPending = 1;
  pAdapter->pTxBuffer = (SWS_PTR(const ui8))pvData;
  pAdapter->sizeTx = len;

  /* Is UART tx IRQ enabled? */
  reg = INPB( pAdapter->combase + INTNREG);
  if ( !(reg & INTONTX))
    {
    bInterruptsWereEnabled = pAdapter->inInterrupt ? 0 : DISABLE();

    if ( pAdapter->bHalfDuplex)
      {
      /* Ensure RTS asserted  */
      reg = INPB( pAdapter->combase + MDMCREG);
      if ( !(reg & MCR_RTS))
        {
        reg &= MCR_MASK;
        reg |= MCR_RTS;
        OUTPB( pAdapter->combase + MDMCREG, reg);

        /* Allow settling time for half duplex control buffers */
        OUTPB( pAdapter->combase + MDMCREG, reg);
        OUTPB( pAdapter->combase + MDMCREG, reg);
        OUTPB( pAdapter->combase + MDMCREG, reg);
        OUTPB( pAdapter->combase + MDMCREG, reg);
        }
      }

    /* Test if handshake off */
    iTxEnable = 1;
    reg = INPB( pAdapter->combase + MDMSREG);
    if ( pAdapter->bUseCts && !(MSR_CTS & reg))
      iTxEnable = 0;
    if ( pAdapter->bUseDsr && !(MSR_DSR & reg))
      iTxEnable = 0;

    if ( iTxEnable)
      {
      /* Enable UART tx IRQ */
      reg = INPB( pAdapter->combase + INTNREG);
      reg &= INTMASK;
      reg |= INTONTX | INTONMDM;
      OUTPB( pAdapter->combase + INTNREG, reg);

      /* Prime Tx then wait for TxEmpty irq */
      pAdapter->sizeTx = len - 1;
      OUTPB( pAdapter->combase + TX_REG, *(pAdapter->pTxBuffer++) );
      ++(pAdapter->uTxChars);
      }
    else
      {
      /* Enable modem status IRQ */
      reg = INPB( pAdapter->combase + INTNREG);
      reg &= INTMASK;
      reg |= INTONMDM;
      OUTPB( pAdapter->combase + INTNREG, reg);
      }

    if ( bInterruptsWereEnabled)
      ENABLE();
    }

  return -1; /* Async completion */
  }


/*
 * Start receiving.
 * CAUTION May be re-entered from PcCommsInterruptHandler upcall
 */
static SWS_CB( int, PcCommsRecv(
  SWS_PTR( void) pvContext,           /* IN: Driver's context */
  SWS_PTR( void) pvData,              /* IN: Buffer */
  SWS_PTR( size_t) pLen,              /* I/O: Buffer size */
  int iEnd,                           /* IN: Delimiter, -ve if none */
  ui32 uTimeout                       /* IN: Timeout, 0= none, 1= immediate */
)){
  SPcComms* const pAdapter = (SPcComms*)pvContext;
  int bInterruptsWereEnabled;
  ui8 reg;

  TRACE( 10, ("PcCommsRecv(%p,%p,%p,%d,%lu)\n",pvContext,pvData,pLen,iEnd,uTimeout));
  /*ASSERT( PlatformIsCritical()); Only valid if not in callback from IRQ */
  ASSERT( NULL != pAdapter);
  ASSERT( PCCOMMS_MAGIC == pAdapter->uMagic);

  if ( NULL == pvData)
    {
    /* Abort pending input */
    if ( AtomicXchg( &pAdapter->aRx, 0) > 0)
      {
      bInterruptsWereEnabled = pAdapter->inInterrupt ? 0 : DISABLE();
      RxDisable( pAdapter);
      if ( bInterruptsWereEnabled)
        ENABLE();

      (*pAdapter->upcalls.pfnRecvInd)( pAdapter->upcalls.pvContext, 0, 0);
      }

    return 0;
    }

  ASSERT( NULL != pLen);
  if ( 0 == *pLen)
    return 0;

  ASSERT( 0 == AtomicRead( &pAdapter->aRx));

  switch ( uTimeout)
    {
  default:
    /* NB timeouts are not implemented */
    return SWS_EBADF;

  case WAIT_FOREVER:
    /* Infinite */
    break;

  case WAIT_NEVER:
    /* Immediate */
    *pLen = 0;
    return 0;
    }

  pAdapter->iEnd = iEnd;
  pAdapter->received = 0;
  pAdapter->pRxBuffer= (SWS_PTR(ui8))pvData;
  AtomicWrite( &pAdapter->aRx, *pLen);

  /* Enable UART rx interrupts */
  reg = INPB( pAdapter->combase + INTNREG);
  if ( !(reg & INTONRX))
    {
    bInterruptsWereEnabled = pAdapter->inInterrupt ? 0 : DISABLE();

    reg = INPB( pAdapter->combase + INTNREG);
    reg &= INTMASK;
    reg |= INTONRX | INTONERR | INTONMDM;
    OUTPB( pAdapter->combase + INTNREG, reg);

    /* Assert DTR and/or RTS */
    reg = INPB( pAdapter->combase + MDMCREG);
    reg &= MCR_MASK;
    reg |= pAdapter->bDtrHandshake ? MCR_DTR : 0;
    reg |= pAdapter->bRtsHandshake ? MCR_RTS : 0;
    OUTPB( pAdapter->combase + MDMCREG, reg);

    if ( bInterruptsWereEnabled)
      ENABLE();
    }

  return -1; /* Pending */
  }


/*
 * Disable Rx
 * CAUTION Called by PcCommsInterruptHandler
 */
void RxDisable( SPcComms* pAdapter)
  {
  ui8 reg;

  if ( pAdapter->bDtrHandshake
    || pAdapter->bRtsHandshake
  ) {
    reg = INPB( pAdapter->combase + MDMCREG);

    /* Negate DTR */
    if ( pAdapter->bDtrHandshake)
      reg &= ~MCR_DTR & MCR_MASK;

    /* Negate RTS */
    if ( pAdapter->bRtsHandshake)
      reg &= ~MCR_RTS & MCR_MASK;

    OUTPB( pAdapter->combase + MDMCREG, reg);
    }

  /* Disable Rx irq's */
  reg = INPB( pAdapter->combase + INTNREG);
  reg &= INTMASK & ~(INTONRX | INTONERR);
  OUTPB( pAdapter->combase + INTNREG, reg);
  }


/*
 * IRQ handler.
 * CAUTION
 */
static void PcCommsInterruptHandler( SPcComms* const pAdapter)
  {
  if ( NULL != pAdapter)
    {
    ui8 type;

    ASSERT( PCCOMMS_MAGIC == pAdapter->uMagic);
    pAdapter->inInterrupt = 1;

#if 0 /* Enable interrupts from higher priority devices */
    ENABLE();
#endif

    /* Until no irq pending */
    while ( !(1 & (type = INPB( pAdapter->combase + INTDREG))) )
      {
      ui8 reg;
      unsigned count;

      switch ( type & 0x07)
        {
      case 6: /* Line status interrupt (OE/PE/FE) */
      case 4: /* Rx full */
        ASSERT( AtomicRead( &pAdapter->aRx) > 0);

        /* Repeat until Rx FIFO empty */
        for ( ;;)
          {
          int iErr;

          iErr = -1; /* Rx incomplete */

          reg = INPB( pAdapter->combase + LINSREG );

          if ( reg & LSR_FERR)
            {
            ++(pAdapter->uFramingErrors);
            }
          if ( reg & LSR_PERR)
            {
            ++(pAdapter->uParityErrors);

            /* Abandon the receive operation */
            if ( AtomicXchg( &pAdapter->aRx, 0) > 0)
              iErr = SWS_EBADF;
            }
          if ( reg & LSR_OVRN)
            {
            ++(pAdapter->uOverrunErrors);
#if 0
            if ( pAdapter->fifo)
              OUTPB( pAdapter->combase + FIFOREG, pAdapter->fifo | FIFO_RXRST );
#endif
            /* Abandon the receive operation */
            if ( AtomicXchg( &pAdapter->aRx, 0) > 0)
              iErr = SWS_EBADF;
            }
          if ( reg & LSR_DRDY)
            {
            /* Read the character */
            ui8 rxchar = INPB( pAdapter->combase + RX_REG );
            ++(pAdapter->uRxChars);

            if ( -1 == iErr && AtomicRead( &pAdapter->aRx) > 0)
              {
              if ( 0 == AtomicDecrement( &pAdapter->aRx))
                {
                iErr = 0;
                }
              /* Is it the delimiter? */
              else if ( (int)rxchar == pAdapter->iEnd)
                {
                AtomicWrite( &pAdapter->aRx, 0);
                iErr = 0;
                }

              *(pAdapter->pRxBuffer++) = rxchar;
              ++(pAdapter->received);
              }
            }
          else if ( -1 == iErr)
            break; /* Rx FIFO empty */

          /* Is Rx complete? */
          if ( iErr >= 0)
            {
            /* Yes, indicate receive complete */
            ASSERT( 0 == AtomicRead( &pAdapter->aRx));
            (*pAdapter->upcalls.pfnRecvInd)(
              pAdapter->upcalls.pvContext, pAdapter->received, iErr);

            /* More buffer space? */
            if ( 0 == AtomicRead( &pAdapter->aRx))
              {
              /* No, disable Rx */
              RxDisable( pAdapter);
              }
            }
          }
        break;

      case 0: /* Change in modem status (CTS/DSR/RI/DCD) */
        reg = INPB( pAdapter->combase + MDMSREG );

        if ( pAdapter->upcalls.pfnStatInd)
          {
          if ( reg & MSR_DDCD)
            {
            (*pAdapter->upcalls.pfnStatInd)(
              pAdapter->upcalls.pvContext, SWS_kP2pStatusCD, (reg & MSR_DCD) ? 1 : 0);
            }

          if ( reg & MSR_TERI)
            {
            (*pAdapter->upcalls.pfnStatInd)(
              pAdapter->upcalls.pvContext, SWS_kP2pStatusRI, (reg & MSR_RING) ? 1 : 0);
            }

          if ( reg & MSR_DDSR)
            {
            (*pAdapter->upcalls.pfnStatInd)(
              pAdapter->upcalls.pvContext, SWS_kP2pStatusDSR, (reg & MSR_DSR) ? 1 : 0);
            }
          }

        if ( !pAdapter->bUseCts && !pAdapter->bUseDsr)
          break;

        if ( (pAdapter->bUseCts && !(reg & MSR_CTS))
          || (pAdapter->bUseDsr && !(reg & MSR_DSR))
        ) {
          /* Handshake negated, disable Tx irqs */
          reg = INPB( pAdapter->combase + INTNREG);
          reg &= INTMASK & ~INTONTX;
          OUTPB( pAdapter->combase + INTNREG, reg);
          break;
          }

        /* Ensure Tx irqs enabled */
        reg = INPB( pAdapter->combase + INTNREG);
        reg &= INTMASK;
        reg |= INTONTX;
        OUTPB( pAdapter->combase + INTNREG, reg);

        /* Ready for Tx data? */
        reg = INPB( pAdapter->combase + LINSREG );
        if ( !(reg & LSR_THRE))
          break; /* No */

        /* Fall thru to Tx empty */

      case 2: /* Tx empty interrupt */
        if ( 0 == pAdapter->sizeTx
          && pAdapter->isTxPending
        ) {
          /* Indicate transmit complete */
          pAdapter->isTxPending = 0;
          (*pAdapter->upcalls.pfnSendAck)( pAdapter->upcalls.pvContext, 0);
          }

        /* TxFIFO fill loop */
        for ( count = pAdapter->uTxFifo; count > 0; --count)
          {
          if ( pAdapter->sizeTx > 0)
            {
            /* Write a character */
            OUTPB( pAdapter->combase + TX_REG, *(pAdapter->pTxBuffer++) );
            ++(pAdapter->uTxChars);
            --(pAdapter->sizeTx);
            if ( 0 == pAdapter->sizeTx
              && pAdapter->isTxPending
            ) {
              /* Indicate transmit complete */
              pAdapter->isTxPending = 0;
              (*pAdapter->upcalls.pfnSendAck)( pAdapter->upcalls.pvContext, 0);
              }
            }
          else
            {
            /* Disable Tx irq's */
            reg = INPB( pAdapter->combase + INTNREG);
            reg &= INTMASK & ~INTONTX;
            OUTPB( pAdapter->combase + INTNREG, reg);

            if ( pAdapter->bHalfDuplex)
              {
              /* Wait for Tx shift register empty */
              do
                {
                reg = INPB( pAdapter->combase + LINSREG );
                }
              while ( !(reg & LSR_TEMT));

              /* Negate RTS */
              reg = INPB( pAdapter->combase + MDMCREG);
              reg &= ~MCR_RTS & MCR_MASK;
              OUTPB( pAdapter->combase + MDMCREG, reg);
              }

            break;
            }
          } /* TxFIFO fill loop */
        break;
        }
      }

    pAdapter->inInterrupt = 0;
    OUTPB( pAdapter->picocr, EOI_NS); /* Non-specific EOI */
    }

#if 0  /* Exit with IRQs enabled for DPMI 'cos iret may not restore iflag! */
  ENABLE();
#endif
  }


#if defined __WATCOMC__ && defined _DOS && !defined _DOS16
extern void __far * SWS_PcCommsSwitchStack( void __far *);
#pragma aux SWS_PcCommsSwitchStack = \
  "mov dx, ss" \
  "mov ss, bx" \
  "xchg eax, esp" \
  parm [bx eax] \
  value [dx eax]

extern void SWS_PcCommsLoades( void);
#pragma aux SWS_PcCommsLoades = "push ds" "pop es"

#define DEF_IRQHANDLER( irq) \
DECL_IRQHANDLER( irq) \
  { \
  static void __far * s_pvStack; \
  SWS_PcCommsLoades(); \
  if ( NULL == s_pvStack) { \
    s_pvStack = SWS_PcCommsSwitchStack( \
      s_pAdapters[ irq]->stack + sizeof( s_pAdapters[ irq]->stack) ); \
    PcCommsInterruptHandler( s_pAdapters[ irq]);\
    SWS_PcCommsSwitchStack( s_pvStack); \
    s_pvStack = NULL; \
  } \
  }

#else
#define DEF_IRQHANDLER( irq) \
DECL_IRQHANDLER( irq) \
  { \
  PcCommsInterruptHandler( s_pAdapters[ irq]);\
  }
#endif

static DEF_IRQHANDLER( 0)
#if OPT_SERIAL_ADAPTERS > 1
DEF_IRQHANDLER( 1)
#endif
#if OPT_SERIAL_ADAPTERS > 2
DEF_IRQHANDLER( 2)
#endif
#if OPT_SERIAL_ADAPTERS > 3
DEF_IRQHANDLER( 3)
#endif
#if OPT_SERIAL_ADAPTERS > 4
DEF_IRQHANDLER( 4)
#endif
#if OPT_SERIAL_ADAPTERS > 5
DEF_IRQHANDLER( 5)
#endif
#if OPT_SERIAL_ADAPTERS > 6
DEF_IRQHANDLER( 6)
#endif
#if OPT_SERIAL_ADAPTERS > 7
DEF_IRQHANDLER( 7)
#endif
#if OPT_SERIAL_ADAPTERS > 8
#error OPT_SERIAL_ADAPTERS set too high
#endif

#ifdef __BORLANDC__
#pragma option -N.
#endif
#ifdef _MSC_VER
#pragma check_stack()
#endif

#else
char SWS_Pccomms;
#endif /*OPT_DATALINK_P2P*/
/* End of file */
