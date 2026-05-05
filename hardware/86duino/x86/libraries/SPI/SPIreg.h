
#ifndef _SPIREG_H_INCLUDED
#define _SPIREG_H_INCLUDED

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "io.h"

void Set_Hybrid_Control_BaseAddr(unsigned long addr, unsigned char mode);
unsigned long Get_Hybrid_Control_BaseAddr(void);
void Set_SPI0_Control_BaseAddr(unsigned long addr, unsigned char mode);
void Set_SPI1_Control_BaseAddr(unsigned long addr, unsigned char mode);
unsigned long Get_SPI0_Control_BaseAddr(void);
unsigned long Get_SPI1_Control_BaseAddr(void);
void Set_SPI0_Control_Register(unsigned long IRQ);
void Set_SPI1_Control_Register(unsigned long IRQ);
void Set_SPI0_DMA_Configure(unsigned long addr, unsigned long IRQ);
void Set_SPI1_DMA_Configure(unsigned long addr, unsigned long IRQ);
unsigned long Get_SPI0_DMA_Baseaddr(void);
unsigned long Get_SPI1_DMA_Baseaddr(void);
unsigned long Get_SPI0_DMA_IRQ(void);
unsigned long Get_SPI1_DMA_IRQ(void);

int Spi_Init(unsigned char dev);
void Spi_Close(unsigned char dev);
void Spi_Write(unsigned char dev, unsigned short data);
unsigned short Spi_Read(unsigned char dev);

#define SPI_BUSY				(0x80)
#define SPI_OUTPUT_FIFO_FULL	(0x40)
#define SPI_INPUT_READY			(0x20)
#define SPI_TRANS_COMPLETE		(0x10)
#define SPI_OUTPUT_EMPTY		(0x08)
#define SPI_INPUT_FULL			(0x04)
unsigned char Spi_Status(unsigned char dev);
bool Is_Spi_Busy(unsigned char dev);
bool Is_Spi_Output_FIFO_Full(unsigned char dev);
bool Is_Spi_Input_Ready(unsigned char dev);
bool Is_Spi_Transmission_Complete(unsigned char dev);
bool Is_Spi_Output_Empty(unsigned char dev);
bool Is_Spi_Input_Full(unsigned char dev);

unsigned char Spi_CS_Reg(unsigned char dev);
void Enable_Spi_CS(unsigned char dev, unsigned char CS);
void Disable_Spi_CS(unsigned char dev, unsigned char CS);
void Enable_Spi_CS0(unsigned char dev);
void Disable_Spi_CS0(unsigned char dev);
void Enable_Spi_CS1(unsigned char dev);
void Disable_Spi_CS1(unsigned char dev);

unsigned char Spi_Error_Status(unsigned char dev);

#define SPI_WRITE_CONFIG_TIMING_ERROR	(0x10)
#define SPI_INPUT_FIFO_OVERRUN_ERROR	(0x08)//滿了但是沒去讀
#define SPI_INPUT_FIFO_UNDERRUN_ERROR	(0x04)//空的還去讀
#define SPI_OUTPUT_FIFO_OVERRUN_ERROR	(0x02)//滿了還送
bool IS_Spi_Write_Config_Timing_Error(unsigned char dev);
void Clear_Spi_Write_Config_Timing_Error(unsigned char dev);
bool IS_Spi_Input_FIFO_OverRun_Error(unsigned char dev);
void Clear_Spi_Input_FIFO_OverRun_Error(unsigned char dev);
bool IS_Spi_Input_FIFO_UnderRun_Error(unsigned char dev);
void Clear_Spi_Input_FIFO_UnderRun_Error(unsigned char dev);
bool IS_Spi_Output_FIFO_OverRun_Error(unsigned char dev);
void Clear_Spi_Output_FIFO_OverRun_Error(unsigned char dev);

void Set_Spi_Clock_Divided(unsigned char dev, unsigned short Div);
unsigned short Get_Spi_Clock_Divided(unsigned char dev);

#define HW_CS	(0x00)
#define SW_CS	(0x80)
void Set_Spi_CS_Mode(unsigned char dev, unsigned char mode);

#define MSBFIRST	(0x00)
#define LSBFIRST	(0x20)
void Set_Spi_BitOrder(unsigned char dev, unsigned char bitOrder);

#define SPI_MODE0 0x00 // CPOL = 0, CPHA = 0
#define SPI_MODE1 0x02 // CPOL = 0, CPHA = 1
#define SPI_MODE2 0x04 // CPOL = 1, CPHA = 0
#define SPI_MODE3 0x06 // CPOL = 1, CPHA = 1
void Set_Spi_DataMode(unsigned char dev, unsigned char mode);

unsigned char Spi_Mode_Configure(unsigned char dev);
void Set_Spi_Mode_Configure(unsigned char dev, unsigned char data);

void Reset_Spi_Output_FIFO(unsigned char dev);
void Reset_Spi_Input_FIFO(unsigned char dev);
void Reset_Spi_Module(unsigned char dev);
unsigned char Spi_Reset_Status(unsigned char dev);

#define Char_Length_8	(0x00)
#define Char_Length_16	(0x08)
void Set_Spi_Char_Length(unsigned char dev, unsigned char length);
unsigned char Get_Spi_Char_Length(unsigned char dev);

void Set_Spi_Transfar_delay(unsigned char dev, unsigned char DTC);
unsigned char Get_Spi_Transfar_delay(unsigned char dev);

#define INPUT_FIFO_FULL_INT		(0x20)
#define OUTPUT_FIFO_EMPTY_INT	(0x08)
#define TRANSFER_COMPLETE_INT	(0x01)
unsigned char Spi_INT_Enable(unsigned char dev);
void Enable_Spi_INT(unsigned char dev, unsigned char even);
void Disable_Spi_INT(unsigned char dev, unsigned char even);
unsigned char Spi_Int_Status(unsigned char dev);

int Spi_DMA_Init(unsigned char dev);
void Spi_DMA_Close(unsigned char dev);
unsigned long Get_Spi_DMA_Control_Reg(unsigned char dev);
void Enable_Spi_DMA(unsigned char dev);
void Disable_Spi_DMA(unsigned char dev);
void Enable_Spi_DMA_CSP(unsigned char dev);
void Disable_Spi_DMA_CSP(unsigned char dev);
void Enable_Spi_DMA_INT(unsigned char dev);
void Disable_Spi_DMA_INT(unsigned char dev);
unsigned long Get_Spi_DMA_INT_Status(unsigned char dev);
void Clear_Spi_DMA_INT_Status(unsigned char dev);
unsigned long Get_Spi_DMA_OVF_Status(unsigned char dev);
void Clear_Spi_DMA_OVF_Status(unsigned char dev);
unsigned long Get_Spi_DMA_Conter_Reg(unsigned char dev);
void Set_Spi_DMA_Conter_Reg(unsigned char dev, unsigned long data);
#define ENTRY_4		(0x00L << 30)
#define ENTRY_8		(0x01L << 30)
#define ENTRY_12	(0x02L << 30)
#define ENTRY_16	(0x03L << 30)
void Set_Spi_DMA_FIFO_Threshold(unsigned char dev, unsigned long threshold);
unsigned long Get_Spi_DMA_FIFO_Threshold(unsigned char dev);
void Set_Spi_DMA_FIFO_Length(unsigned char dev, unsigned long length);
unsigned long Get_Spi_DMA_FIFO_Length(unsigned char dev);
void Set_Spi_DMA_Output_Buffer_BA(unsigned char dev, unsigned long Baseaddr);
unsigned long Get_Spi_DMA_Output_Buffer_BA(unsigned char dev);
void Set_Spi_DMA_Input_Buffer_BA(unsigned char dev, unsigned long Baseaddr);
unsigned long Get_Spi_DMA_Input_Buffer_BA(unsigned char dev);
unsigned long Get_Spi_DMA_CSP_SELECT(unsigned char dev);
void Set_Spi_DMA_CSP_SELECT(unsigned char dev, unsigned long data);
#define DMA_CSP_CS0	(0x01L)
#define DMA_CSP_CS1	(0x02L)
void Enable_Spi_DMA_CSP_CS(unsigned char dev, unsigned long CS);
void Disable_Spi_DMA_CSP_CS(unsigned char dev, unsigned long CS);


#endif
