#include "SPIreg.h"
#include "io.h"
unsigned long Hybrid_Control_BASEADDR = 0xD001; 
unsigned long SPI_Control_BASEADDR[2] = {0xD083, 0xD0A3}; 
unsigned long SPI_DMA_BASEADDR[2] = {0, 0}; 
// unsigned long SPI1_Control_BASEADDR = 0xD0A0;
unsigned char int_routing_table[16] = {0xff, 0x08, 0xff, 0x02, 0x04, 0x05, 0x07, 0x06, 
									   0xff, 0x01, 0x03, 0x09, 0x0b, 0xff, 0x0d, 0x0f};


/******************************Hybrid Function**************************/

void Set_Hybrid_Control_BaseAddr(unsigned long addr, unsigned char mode) {
	void *pciDev = NULL;
	// Get Hybrid device base address
	pciDev = pci_Alloc(0x00, 0x06, 0x00); // PCI Hybrid configuration space
	if(pciDev == NULL) {printf("Hybrid device don't exist\n"); return;}
	Hybrid_Control_BASEADDR = (addr & 0xFFFFFFC0L) + mode;
	pci_Out32(pciDev, 0x48, Hybrid_Control_BASEADDR);
	pci_Free(pciDev);
}

unsigned long Get_Hybrid_Control_BaseAddr(void) {
	void *pciDev = NULL;
	// Get Hybrid device base address
	pciDev = pci_Alloc(0x00, 0x06, 0x00); // PCI Hybrid configuration space
	if(pciDev == NULL) {printf("Hybrid device don't exist\n"); return 0xFFFFFFFF;}
	Hybrid_Control_BASEADDR = (unsigned long)(pci_In32(pciDev, 0x48)); // get Hybrid base address
	pci_Free(pciDev);
	return Hybrid_Control_BASEADDR;
}

void Set_SPI0_Control_BaseAddr(unsigned long addr, unsigned char mode) {
	void *SpiDev = NULL;
	SpiDev = io_Alloc((Hybrid_Control_BASEADDR & 0x01) ^ 0x01, (Hybrid_Control_BASEADDR & 0xFFFFFFC0L), 0x10);
	if(SpiDev == NULL) {printf("SPI BaseAddr memory Alloc fail\n"); return;}
	SPI_Control_BASEADDR[0] = (addr & 0xFFFFFFE0L) + 0x02 + mode;
	io_Out32(SpiDev, 0x00, SPI_Control_BASEADDR[0]);
	io_Free(SpiDev);
}

void Set_SPI1_Control_BaseAddr(unsigned long addr, unsigned char mode) {
	void *SpiDev = NULL;
	SpiDev = io_Alloc((Hybrid_Control_BASEADDR & 0x01) ^ 0x01, (Hybrid_Control_BASEADDR & 0xFFFFFFC0L), 0x10);
	if(SpiDev == NULL) {printf("SPI BaseAddr memory Alloc fail\n"); return;}
	SPI_Control_BASEADDR[1] = (addr & 0xFFFFFFE0L) + 0x02 + mode;
	io_Out32(SpiDev, 0x08, SPI_Control_BASEADDR[1]);
	io_Free(SpiDev);
}

unsigned long Get_SPI0_Control_BaseAddr(void) {
	void *SpiDev = NULL;
	SpiDev = io_Alloc((Hybrid_Control_BASEADDR & 0x01) ^ 0x01, (Hybrid_Control_BASEADDR & 0xFFFFFFC0L), 0x10);
	if(SpiDev == NULL) {printf("SPI BaseAddr memory Alloc fail\n"); return 0xFFFFFFFF;}
	SPI_Control_BASEADDR[0] = io_In32(SpiDev, 0x00);
	io_Free(SpiDev);
	if(SPI_Control_BASEADDR[0] & 0x02 != 0x02) return 0xFFFFFFFF; //spi baseaddr decode disable
	else return SPI_Control_BASEADDR[0];
}

unsigned long Get_SPI1_Control_BaseAddr(void) {
	void *SpiDev = NULL;
	SpiDev = io_Alloc((Hybrid_Control_BASEADDR & 0x01) ^ 0x01, (Hybrid_Control_BASEADDR & 0xFFFFFFC0L), 0x10);
	if(SpiDev == NULL) {printf("SPI BaseAddr memory Alloc fail\n"); return 0xFFFFFFFF;}
	SPI_Control_BASEADDR[1] = io_In32(SpiDev, 0x08);
	io_Free(SpiDev);
	if(SPI_Control_BASEADDR[1] & 0x02 != 0x02) return 0xFFFFFFFF; //spi baseaddr decode disable
	else return SPI_Control_BASEADDR[1];
}

void Set_SPI0_Control_Register(unsigned long IRQ) {
	void *SpiDev = NULL;
	SpiDev = io_Alloc((Hybrid_Control_BASEADDR & 0x01) ^ 0x01, (Hybrid_Control_BASEADDR & 0xFFFFFFC0L), 0x10);
	if(SpiDev == NULL) {printf("SPI BaseAddr memory Alloc fail\n"); return;}
	io_Out32(SpiDev, 0x04, (unsigned long)int_routing_table[IRQ]);
	io_Free(SpiDev);
}

void Set_SPI1_Control_Register(unsigned long IRQ) {
	void *SpiDev = NULL;
	SpiDev = io_Alloc((Hybrid_Control_BASEADDR & 0x01) ^ 0x01, (Hybrid_Control_BASEADDR & 0xFFFFFFC0L), 0x10);
	if(SpiDev == NULL) {printf("SPI BaseAddr memory Alloc fail\n"); return;}
	io_Out32(SpiDev, 0x0C, (unsigned long)int_routing_table[IRQ]);
	io_Free(SpiDev);
}

void Set_SPI0_DMA_Configure(unsigned long addr, unsigned long IRQ) {
	void *SpiDev = NULL;
	SpiDev = io_Alloc((Hybrid_Control_BASEADDR & 0x01) ^ 0x01, (Hybrid_Control_BASEADDR & 0xFFFFFFC0L), 0x10);
	if(SpiDev == NULL) {printf("SPI BaseAddr memory Alloc fail\n"); return;}
	SPI_DMA_BASEADDR[0] = (addr & 0xFFFFFFE0L) | (((unsigned long)int_routing_table[IRQ] & 0x0F) << 1) | 0x01;
	io_Out32(SpiDev, 0x28, SPI_DMA_BASEADDR[0]);
	io_Free(SpiDev);
}

void Set_SPI1_DMA_Configure(unsigned long addr, unsigned long IRQ) {
	void *SpiDev = NULL;
	SpiDev = io_Alloc((Hybrid_Control_BASEADDR & 0x01) ^ 0x01, (Hybrid_Control_BASEADDR & 0xFFFFFFC0L), 0x10);
	if(SpiDev == NULL) {printf("SPI BaseAddr memory Alloc fail\n"); return;}
	SPI_DMA_BASEADDR[1] = (addr & 0xFFFFFFE0L) | ((unsigned long)int_routing_table[IRQ] << 1) | 0x01;
	io_Out32(SpiDev, 0x2C, SPI_DMA_BASEADDR[1]);
	io_Free(SpiDev);
}

unsigned long Get_SPI0_DMA_Baseaddr(void) {
	void *SpiDev = NULL;
	SpiDev = io_Alloc((Hybrid_Control_BASEADDR & 0x01) ^ 0x01, (Hybrid_Control_BASEADDR & 0xFFFFFFC0L), 0x10);
	if(SpiDev == NULL) {printf("SPI BaseAddr memory Alloc fail\n"); return 0xFFFFFFFF;}
	SPI_DMA_BASEADDR[0] = io_In32(SpiDev, 0x28);
	io_Free(SpiDev);
	if(SPI_DMA_BASEADDR[0] & 0x01 != 0x01) return 0xFFFFFFFF; //spi dma configure baseaddr decode disable
	else return SPI_DMA_BASEADDR[0] & 0xFFFFFFE0L;
}

unsigned long Get_SPI1_DMA_Baseaddr(void) {
	void *SpiDev = NULL;
	SpiDev = io_Alloc((Hybrid_Control_BASEADDR & 0x01) ^ 0x01, (Hybrid_Control_BASEADDR & 0xFFFFFFC0L), 0x10);
	if(SpiDev == NULL) {printf("SPI BaseAddr memory Alloc fail\n"); return 0xFFFFFFFF;}
	SPI_DMA_BASEADDR[1] = io_In32(SpiDev, 0x2C);
	io_Free(SpiDev);
	if(SPI_DMA_BASEADDR[1] & 0x01 != 0x01) return 0xFFFFFFFF; //spi dma configure baseaddr decode disable
	else return SPI_DMA_BASEADDR[1] & 0xFFFFFFE0L;
}

unsigned long Get_SPI0_DMA_IRQ(void) {
	unsigned long IRQ;
	void *SpiDev = NULL;
	SpiDev = io_Alloc((Hybrid_Control_BASEADDR & 0x01) ^ 0x01, (Hybrid_Control_BASEADDR & 0xFFFFFFC0L), 0x10);
	if(SpiDev == NULL) {printf("SPI BaseAddr memory Alloc fail\n"); return 0xFFFFFFFF;}
	IRQ = (io_In32(SpiDev, 0x28) & 0x0000001EL) >> 1;
	io_Free(SpiDev);
	return IRQ;
}

unsigned long Get_SPI1_DMA_IRQ(void) {
	unsigned long IRQ;
	void *SpiDev = NULL;
	SpiDev = io_Alloc((Hybrid_Control_BASEADDR & 0x01) ^ 0x01, (Hybrid_Control_BASEADDR & 0xFFFFFFC0L), 0x10);
	if(SpiDev == NULL) {printf("SPI BaseAddr memory Alloc fail\n"); return 0xFFFFFFFF;}
	IRQ = (io_In32(SpiDev, 0x2C) & 0x0000001EL) >> 1;
	io_Free(SpiDev);
	return IRQ;
}
// unsigned long Get_SPI0_Control_Register(void) {
	// void *SpiDev = NULL;
	// SpiDev = io_Alloc((Hybrid_Control_BASEADDR & 0x01) ^ 0x01, (Hybrid_Control_BASEADDR & 0xFFFFFFC0L), 0x10);
	// if(SpiDev == NULL) {printf("SPI BaseAddr memory Alloc fail\n"); return 0xFFFFFFFF;}
	// SPI_Control_BASEADDR[0] = io_In32(SpiDev, 0x00);
	// io_Free(SpiDev);
	// if(SPI_Control_BASEADDR[0] & 0x02 != 0x02) return 0xFFFFFFFF; //spi baseaddr decode disable
	// else return SPI_Control_BASEADDR[0];
// }

/******************************************************************/

static void *SpiDev[2] = {NULL, NULL}; 
static void *SpiDMADev[2] = {NULL, NULL};
// static void *Spi1Dev = NULL; 

int Spi_Init(unsigned char dev) {
	SpiDev[dev] = io_Alloc((SPI_Control_BASEADDR[dev] & 0x01) ^ 0x01, (SPI_Control_BASEADDR[dev] & 0xFFFFFFE0L), 0x20);	
	if(SpiDev[dev] == NULL) {
		printf("SPI%d io space memory Alloc fail\n", dev); 
		return -1;
	}
	return 0;
}

void Spi_Close(unsigned char dev) {
	if(SpiDev[dev] != NULL) io_Free(SpiDev[dev]);
}

int Spi_DMA_Init(unsigned char dev) {
	SpiDMADev[dev] = io_Alloc(IO_USE_MMIO, (SPI_DMA_BASEADDR[dev] & 0xFFFFFFE0L), 0x20);	
	if(SpiDMADev[dev] == NULL) {
		printf("SPI%d DMA Configure space memory Alloc fail\n", dev); 
		return -1;
	}
	return 0;
}

void Spi_DMA_Close(unsigned char dev) {
	if(SpiDMADev[dev] != NULL) io_Free(SpiDMADev[dev]);
}

void Spi_Write(unsigned char dev, unsigned short data) {
	if(SpiDev[dev] == NULL) return;
	io_Out16(SpiDev[dev], 0x00, data);
}

unsigned short Spi_Read(unsigned char dev) {
	if(SpiDev[dev] == NULL) return 0xFFFF;
	return io_In16(SpiDev[dev], 0x04);	
}

#define SPI_BUSY				(0x80)
#define SPI_OUTPUT_FIFO_FULL	(0x40)
#define SPI_INPUT_READY			(0x20)
#define SPI_TRANS_COMPLETE		(0x10)
#define SPI_OUTPUT_EMPTY		(0x08)
#define SPI_INPUT_FULL			(0x04)

unsigned char Spi_Status(unsigned char dev) {
	if(SpiDev[dev] == NULL) return 0xFF;
	return io_In8(SpiDev[dev], 0x08);	
}

bool Is_Spi_Busy(unsigned char dev) {
	if(SpiDev[dev] == NULL) return false;
	if((Spi_Status(dev) & SPI_BUSY) != 0) return true;
	else return false;
}

bool Is_Spi_Output_FIFO_Full(unsigned char dev) {
	if(SpiDev[dev] == NULL) return false;
	if((Spi_Status(dev) & SPI_OUTPUT_FIFO_FULL) != 0) return true;
	else return false;
}


bool Is_Spi_Input_Ready(unsigned char dev) {
	if(SpiDev[dev] == NULL) return false;
	if((Spi_Status(dev) & SPI_INPUT_READY) != 0) return true;
	else return false;
}

bool Is_Spi_Transmission_Complete(unsigned char dev) {
	if(SpiDev[dev] == NULL) return false;
	if((Spi_Status(dev) & SPI_TRANS_COMPLETE) != 0) return true;
	else return false;
}

bool Is_Spi_Output_Empty(unsigned char dev) {
	if(SpiDev[dev] == NULL) return false;
	if((Spi_Status(dev) & SPI_OUTPUT_EMPTY) != 0) return true;
	else return false;
}

bool Is_Spi_Input_Full(unsigned char dev) {
	if(SpiDev[dev] == NULL) return false;
	if((Spi_Status(dev) & SPI_INPUT_FULL) != 0) return true;
	else return false;
}

unsigned char Spi_CS_Reg(unsigned char dev) {
	if(SpiDev[dev] == NULL) return 0xFF;
	return io_In8(SpiDev[dev], 0x0A);	
}

void Disable_Spi_CS(unsigned char dev, unsigned char CS) {
	if(SpiDev[dev] == NULL) return;
	if(CS == 0) io_Out8(SpiDev[dev], 0x0A, Spi_CS_Reg(dev) | 0x01);	
	else io_Out8(SpiDev[dev], 0x0A, Spi_CS_Reg(dev) | 0x02);	
}

void Enable_Spi_CS(unsigned char dev, unsigned char CS) {
	if(SpiDev[dev] == NULL) return;
	if(CS == 0) io_Out8(SpiDev[dev], 0x0A, Spi_CS_Reg(dev) & 0x02);	
	else io_Out8(SpiDev[dev], 0x0A, Spi_CS_Reg(dev) & 0x01);	
}

void Disable_Spi_CS0(unsigned char dev) {
	if(SpiDev[dev] == NULL) return;
	io_Out8(SpiDev[dev], 0x0A, Spi_CS_Reg(dev) | 0x01);	
}

void Enable_Spi_CS0(unsigned char dev) {
	if(SpiDev[dev] == NULL) return;
	io_Out8(SpiDev[dev], 0x0A, Spi_CS_Reg(dev) & 0x02);	
}

void Disable_Spi_CS1(unsigned char dev) {
	if(SpiDev[dev] == NULL) return;
	io_Out8(SpiDev[dev], 0x0A, Spi_CS_Reg(dev) | 0x02);	
}

void Enable_Spi_CS1(unsigned char dev) {
	if(SpiDev[dev] == NULL) return;
	io_Out8(SpiDev[dev], 0x0A, Spi_CS_Reg(dev) & 0x01);	
}


unsigned char Spi_Error_Status(unsigned char dev) {
	if(SpiDev[dev] == NULL) return 0xFF;
	return io_In8(SpiDev[dev], 0x0B);	
}

#define SPI_WRITE_CONFIG_TIMING_ERROR	(0x10)
#define SPI_INPUT_FIFO_OVERRUN_ERROR	(0x08)
#define SPI_INPUT_FIFO_UNDERRUN_ERROR	(0x04)
#define SPI_OUTPUT_FIFO_OVERRUN_ERROR	(0x02)
bool IS_Spi_Write_Config_Timing_Error(unsigned char dev) {
	if(SpiDev[dev] == NULL) return false;
	if((Spi_Error_Status(dev) & SPI_WRITE_CONFIG_TIMING_ERROR) != 0) return true;
	else return false;	
}

void Clear_Spi_Write_Config_Timing_Error(unsigned char dev) {
	if(SpiDev[dev] == NULL) return;
	io_Out8(SpiDev[dev], 0x0B, SPI_WRITE_CONFIG_TIMING_ERROR);
}

bool IS_Spi_Input_FIFO_OverRun_Error(unsigned char dev) {
	if(SpiDev[dev] == NULL) return false;
	if((Spi_Error_Status(dev) & SPI_INPUT_FIFO_OVERRUN_ERROR) != 0) return true;
	else return false;	
}

void Clear_Spi_Input_FIFO_OverRun_Error(unsigned char dev) {
	if(SpiDev[dev] == NULL) return;
	io_Out8(SpiDev[dev], 0x0B, SPI_INPUT_FIFO_OVERRUN_ERROR);
}

bool IS_Spi_Input_FIFO_UnderRun_Error(unsigned char dev) {
	if(SpiDev[dev] == NULL) return false;
	if((Spi_Error_Status(dev) & SPI_INPUT_FIFO_UNDERRUN_ERROR) != 0) return true;
	else return false;	
}

void Clear_Spi_Input_FIFO_UnderRun_Error(unsigned char dev) {
	if(SpiDev[dev] == NULL) return;
	io_Out8(SpiDev[dev], 0x0B, SPI_INPUT_FIFO_UNDERRUN_ERROR);
}

bool IS_Spi_Output_FIFO_OverRun_Error(unsigned char dev) {
	if(SpiDev[dev] == NULL) return false;
	if((Spi_Error_Status(dev) & SPI_OUTPUT_FIFO_OVERRUN_ERROR) != 0) return true;
	else return false;	
}

void Clear_Spi_Output_FIFO_OverRun_Error(unsigned char dev) {
	if(SpiDev[dev] == NULL) return;
	io_Out8(SpiDev[dev], 0x0B, SPI_OUTPUT_FIFO_OVERRUN_ERROR);
}

//100M/(2*spi clock divided)
void Set_Spi_Clock_Divided(unsigned char dev, unsigned short Div) {
	if(SpiDev[dev] == NULL) return;
	io_Out16(SpiDev[dev], 0x10, Div);		
}

unsigned short Get_Spi_Clock_Divided(unsigned char dev) {
	if(SpiDev[dev] == NULL) return 0;
	return io_In16(SpiDev[dev], 0x10);		
}

#define HW_CS	(0x00)
#define SW_CS	(0x80)
void Set_Spi_CS_Mode(unsigned char dev, unsigned char mode) {
	if(SpiDev[dev] == NULL) return;
	io_Out8(SpiDev[dev], 0x12, (io_In8(SpiDev[dev], 0x12) & 0x7F) | mode);
}

#define MSBFIRST	(0x00)
#define LSBFIRST	(0x20)
void Set_Spi_BitOrder(unsigned char dev, unsigned char bitOrder) {
	if(SpiDev[dev] == NULL) return;
	io_Out8(SpiDev[dev], 0x12, (io_In8(SpiDev[dev], 0x12) & 0xDF) | bitOrder);	
}

#define SPI_MODE0 0x00 // CPOL = 0, CPHA = 0
#define SPI_MODE1 0x02 // CPOL = 0, CPHA = 1
#define SPI_MODE2 0x04 // CPOL = 1, CPHA = 0
#define SPI_MODE3 0x06 // CPOL = 1, CPHA = 1

void Set_Spi_DataMode(unsigned char dev, unsigned char mode) {
	if(SpiDev[dev] == NULL) return;
	io_Out8(SpiDev[dev], 0x12, (io_In8(SpiDev[dev], 0x12) & 0xF9) | mode);	
}

unsigned char Spi_Mode_Configure(unsigned char dev) {
	if(SpiDev[dev] == NULL) return 0xFF;
	return io_In8(SpiDev[dev], 0x12);
}

void Set_Spi_Mode_Configure(unsigned char dev, unsigned char data) {
	if(SpiDev[dev] == NULL) return;	
	io_Out8(SpiDev[dev], 0x12, data);	
}

void Reset_Spi_Output_FIFO(unsigned char dev) {
	if(SpiDev[dev] == NULL) return;
	io_Out8(SpiDev[dev], 0x13, 0x04);		
}

void Reset_Spi_Input_FIFO(unsigned char dev) {
	if(SpiDev[dev] == NULL) return;
	io_Out8(SpiDev[dev], 0x13, 0x02);		
}

void Reset_Spi_Module(unsigned char dev) {
	if(SpiDev[dev] == NULL) return;
	io_Out8(SpiDev[dev], 0x13, 0x01);		
}

unsigned char Spi_Reset_Status(unsigned char dev) {
	if(SpiDev[dev] == NULL) return 0xFF;
	return io_In8(SpiDev[dev], 0x13);	
}

#define Char_Length_8	(0x00)
#define Char_Length_16	(0x08)
void Set_Spi_Char_Length(unsigned char dev, unsigned char length) {
	if(SpiDev[dev] == NULL) return;
	io_Out8(SpiDev[dev], 0x14, length);		
}

unsigned char Get_Spi_Char_Length(unsigned char dev) {
	if(SpiDev[dev] == NULL) return 0xFF;
	return (io_In8(SpiDev[dev], 0x14) & 0x08);	
}

//in HW CS mode : (DTC+1)*SPI clock
//in SW CS mode : DTC*SPI clock
void Set_Spi_Transfar_delay(unsigned char dev, unsigned char DTC) {
	if(SpiDev[dev] == NULL) return;
	io_Out8(SpiDev[dev], 0x15, DTC);			
}

unsigned char Get_Spi_Transfar_delay(unsigned char dev) {
	if(SpiDev[dev] == NULL) return 0x00;
	return io_In8(SpiDev[dev], 0x15);	
}

/***********interrupt***********/

#define INPUT_FIFO_FULL_INT		(0x20)
#define OUTPUT_FIFO_EMPTY_INT	(0x08)
#define TRANSFER_COMPLETE_INT	(0x01)

unsigned char Spi_INT_Enable(unsigned char dev) {
	if(SpiDev[dev] == NULL) return 0xFF;
	return io_In8(SpiDev[dev], 0x18);
}

void Enable_Spi_INT(unsigned char dev, unsigned char even) {
	if(SpiDev[dev] == NULL) return;
	io_Out8(SpiDev[dev], 0x18, io_In8(SpiDev[dev], 0x18) | even);		
}

void Disable_Spi_INT(unsigned char dev, unsigned char even) {
	if(SpiDev[dev] == NULL) return;
	io_Out8(SpiDev[dev], 0x18, io_In8(SpiDev[dev], 0x18) & (unsigned char)(~even));		
}

unsigned char Spi_Int_Status(unsigned char dev) {
	if(SpiDev[dev] == NULL) return 0xFF;
	return io_In8(SpiDev[dev], 0x19);		
}

/***********DMA***********/

unsigned long Get_Spi_DMA_Control_Reg(unsigned char dev) {
	if(SpiDMADev[dev] == NULL) return 0xFFFFFFFFL;
	return io_In32(SpiDMADev[dev], 0x00);			
}

void Enable_Spi_DMA(unsigned char dev) {
	if(SpiDMADev[dev] == NULL) return;
	io_Out32(SpiDMADev[dev], 0x00, io_In32(SpiDMADev[dev], 0x00) | 0x80000000L);		
}

void Disable_Spi_DMA(unsigned char dev) {
	if(SpiDMADev[dev] == NULL) return;
	io_Out32(SpiDMADev[dev], 0x00, io_In32(SpiDMADev[dev], 0x00) & 0x7FFFFFFFL);		
}

void Enable_Spi_DMA_CSP(unsigned char dev) {
	if(SpiDMADev[dev] == NULL) return;
	io_Out32(SpiDMADev[dev], 0x00, io_In32(SpiDMADev[dev], 0x00) | 0x00000002L);			
}

void Disable_Spi_DMA_CSP(unsigned char dev) {
	if(SpiDMADev[dev] == NULL) return;
	io_Out32(SpiDMADev[dev], 0x00, io_In32(SpiDMADev[dev], 0x00) & 0xFFFFFFFDL);		
}

void Enable_Spi_DMA_INT(unsigned char dev) {
	if(SpiDMADev[dev] == NULL) return;
	io_Out32(SpiDMADev[dev], 0x00, io_In32(SpiDMADev[dev], 0x00) | 0x00000001L);		
}

void Disable_Spi_DMA_INT(unsigned char dev) {
	if(SpiDMADev[dev] == NULL) return;
	io_Out32(SpiDMADev[dev], 0x00, io_In32(SpiDMADev[dev], 0x00) & 0xFFFFFFFEL);		
}

unsigned long Get_Spi_DMA_INT_Status(unsigned char dev) {
	if(SpiDMADev[dev] == NULL) return 0xFFFFFFFFL;
	return (io_In32(SpiDMADev[dev], 0x04) & 0x01L);			
}

void Clear_Spi_DMA_INT_Status(unsigned char dev) {
	if(SpiDMADev[dev] == NULL) return ;
	io_Out32(SpiDMADev[dev], 0x04, 0x01L);	
}

unsigned long Get_Spi_DMA_OVF_Status(unsigned char dev) {
	if(SpiDMADev[dev] == NULL) return 0xFFFFFFFFL;
	return (io_In32(SpiDMADev[dev], 0x04) & 0x02L);			
}

void Clear_Spi_DMA_OVF_Status(unsigned char dev) {
	if(SpiDMADev[dev] == NULL) return ;
	io_Out32(SpiDMADev[dev], 0x04, 0x02L);	
}

unsigned long Get_Spi_DMA_Conter_Reg(unsigned char dev) {
	if(SpiDMADev[dev] == NULL) return 0xFFFFFFFFL;
	return io_In32(SpiDMADev[dev], 0x08);	
}

void Set_Spi_DMA_Conter_Reg(unsigned char dev, unsigned long data) {
	if(SpiDMADev[dev] == NULL) return ;
	io_Out32(SpiDMADev[dev], 0x08, data);	
}

#define ENTRY_4		(0x00L << 30)
#define ENTRY_8		(0x01L << 30)
#define ENTRY_12	(0x02L << 30)
#define ENTRY_16	(0x03L << 30)
void Set_Spi_DMA_FIFO_Threshold(unsigned char dev, unsigned long threshold) {
	if(SpiDMADev[dev] == NULL) return ;
	io_Out32(SpiDMADev[dev], 0x08, (io_In32(SpiDMADev[dev], 0x08) & 0x3FFFFFFFL) | threshold);		
}

unsigned long Get_Spi_DMA_FIFO_Threshold(unsigned char dev) {
	if(SpiDMADev[dev] == NULL) return 0xFFFFFFFFL;
	return io_In32(SpiDMADev[dev], 0x08) & 0xC0000000L;		
}

void Set_Spi_DMA_FIFO_Length(unsigned char dev, unsigned long length) {
	if(SpiDMADev[dev] == NULL) return ;
	io_Out32(SpiDMADev[dev], 0x08, (io_In32(SpiDMADev[dev], 0x08) & 0xFF000000L) | length);		
}

unsigned long Get_Spi_DMA_FIFO_Length(unsigned char dev) {
	if(SpiDMADev[dev] == NULL) return 0xFFFFFFFFL;
	return io_In32(SpiDMADev[dev], 0x08) & 0x00FFFFFFL;		
}

void Set_Spi_DMA_Output_Buffer_BA(unsigned char dev, unsigned long Baseaddr) {
	if(SpiDMADev[dev] == NULL) return ;
	io_Out32(SpiDMADev[dev], 0x0C, Baseaddr);		
}

unsigned long Get_Spi_DMA_Output_Buffer_BA(unsigned char dev) {
	if(SpiDMADev[dev] == NULL) return 0xFFFFFFFFL;
	return io_In32(SpiDMADev[dev], 0x0C) & 0xFFFFFFFC;		
}

void Set_Spi_DMA_Input_Buffer_BA(unsigned char dev, unsigned long Baseaddr) {
	if(SpiDMADev[dev] == NULL) return ;
	io_Out32(SpiDMADev[dev], 0x10, Baseaddr);		
}

unsigned long Get_Spi_DMA_Input_Buffer_BA(unsigned char dev) {
	if(SpiDMADev[dev] == NULL) return 0xFFFFFFFFL;
	return io_In32(SpiDMADev[dev], 0x10) & 0xFFFFFFFC;	 	
}

unsigned long Get_Spi_DMA_CSP_SELECT(unsigned char dev) {
	if(SpiDMADev[dev] == NULL) return 0xFFFFFFFFL;
	return (io_In32(SpiDMADev[dev], 0x14) & 0x03L);	 	
}

void Set_Spi_DMA_CSP_SELECT(unsigned char dev, unsigned long data) {
	if(SpiDMADev[dev] == NULL) return ;
	io_Out32(SpiDMADev[dev], 0x14, data);	
}

void Enable_Spi_DMA_CSP_CS(unsigned char dev, unsigned long CS) {
	if(SpiDMADev[dev] == NULL) return;
	if(CS != DMA_CSP_CS0 && CS != DMA_CSP_CS1) return;
	io_Out32(SpiDMADev[dev], 0x14, (io_In32(SpiDMADev[dev], 0x14) & 0x03L) | CS);	
}

void Disable_Spi_DMA_CSP_CS(unsigned char dev, unsigned long CS) {
	if(SpiDMADev[dev] == NULL) return;
	if(CS != DMA_CSP_CS0 && CS != DMA_CSP_CS1) return;
	io_Out32(SpiDMADev[dev], 0x14, (io_In32(SpiDMADev[dev], 0x14) & 0x03L) & (~CS));	
}



