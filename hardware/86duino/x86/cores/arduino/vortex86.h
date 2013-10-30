#ifndef __DMP_VORTEX86_H
#define __DMP_VORTEX86_H

#include "dmpcfg.h"

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------
//            Vortex86 UART functions
// ---------------------------------------------------
//
// vx86_uart_Init(com, mode)
//     [com] : 0 ~ (Maximum size of UART - 1)
//     [mode]: Maximum baud mode
DMPAPI(bool) vx86_uart_Init(int com, int mode);
#define MAX_BAUD_6000000			(1)
#define MAX_BAUD_3000000			(2)
#define MAX_BAUD_750000 			(3)
#define MAX_BAUD_115200A 			(4)
#define MAX_BAUD_115200B			(5)
DMPAPI(void) vx86_uart_Close(int com);

DMPAPI(unsigned short) vx86_uart_GetBaseAddr(int com);
DMPAPI(void) vx86_uart_SetFIFO16(int com);
DMPAPI(void) vx86_uart_SetFIFO32(int com);

DMPAPI(int) vx86_uart_GetIRQ(int com);

DMPAPI(unsigned long) vx86_uart_MaxBPS(int com);
DMPAPI(void) vx86_uart_EnableHalfDuplex(int com);
DMPAPI(void) vx86_uart_EnableFullDuplex(int com);


// ---------------------------------------------------
//       Vortex86 USB Device functions
// ---------------------------------------------------
DMPAPI(unsigned short) vx86_GetUSBDevAddr(void);
DMPAPI(int) vx86_GetUSBDevIRQ(void);

// ---------------------------------------------------
//       Vortex86 CAN bus functions
// ---------------------------------------------------
DMPAPI(unsigned short) vx86_GetCANBusIOAddr(void);
DMPAPI(unsigned long) vx86_GetCANBusMMIOAddr(void);
DMPAPI(int) vx86_GetCANBusIRQ(void);

#ifdef __cplusplus
}
#endif

#endif
