#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#include "io.h"
#include "irq.h"

static unsigned char RxBuffer[4096];
static unsigned char TxBuffer[4096];
static volatile int RxHead = 0;
static volatile int RxTail = 0;
static volatile int RxSize = 0;
static volatile int TxHead = 0;
static volatile int TxTail = 0;
static volatile int TxSize = 0;

static volatile long txcount = 0;
static volatile long rxcount = 0;

static int isr_handler(int irq, void *data)
{
	int i;
	unsigned char c, iir;
	
	int ISR_Status = ISR_NONE;
	
	while (!((iir = io_inpb(0x3f8 + 2)) & 0x01))
	{
		ISR_Status = ISR_HANDLED;
		
		switch (iir & 0x0e)
		{
			// timeout & receive data ready interrupt
			case 0x0c: case 0x04:
			{
				do {
					c = io_inpb(0x3f8);
					io_DisableINT();
					if (RxSize < 4096) {
						RxBuffer[RxTail] = c;
						RxTail = (RxTail + 1) % 4096;
						RxSize++;
						rxcount++;
					}
					io_RestoreINT();
					
				} while (io_inpb(0x3f8+5) & 0x01);
			}
			break;
			
			// transmitter holding register empty interrupt
			case 0x02:
			{
				io_DisableINT();
				if (TxSize > 0) {
					io_outpb(0x3f8, TxBuffer[TxHead]);
					TxHead = (TxHead + 1) % 4096;
					TxSize--;
					txcount++;
				} else {
					io_outpb(0x3f8+1, 0x01);
				}
				io_RestoreINT();
			}
			break;
			
			// received line status interrupt
			case 0x06:
			{
				io_inpb(0x3f8+5);
			}
			break;
			
			// modem status interrupt
			case 0x00:
			{
				io_inpb(0x3f8+6);
			}
			break;
			
			default: break;
			
		};
	}

	return ISR_Status;
}

int main(int argc, char *argv[])
{
	unsigned char lcr, ch;
	unsigned short uart_baseAddr;
	unsigned long nowtime, pretime;
	long txc, rxc;

	if (io_Init() == false) {
		printf("ERROR: IO init fail.\n");
		return 1;
	}
	if (irq_Init() == false) {
		printf("ERROR: IRQ init fail.\n");
		io_Close();
		return 1;
	}
	sb_Write(0xc0, sb_Read(0xc0) & 0x7fffffffL | ((unsigned long)1UL << 31));
	
	uart_baseAddr = sb_Read16(0x60) & 0xfffe;
	sb_Write16(0x60, sb_Read16(0x60) | 0x0001);
	io_outpdw(uart_baseAddr + 0*4, io_inpdw(uart_baseAddr + 0*4) & 0xffefffffL | ((unsigned long)1UL << 20));
	io_outpdw(uart_baseAddr + 0*4, io_inpdw(uart_baseAddr + 0*4) & 0xffbfffffL | ((unsigned long)1UL << 22));
	
	// set baudrate
	io_DisableINT();
	{
		lcr = io_inpb(0x3f8+3);  
		io_outpb(0x3f8+3, 0x80); 
		
		do {
			io_outpb(0x3f8+0, 6);
		} while (io_inpb(0x3f8+0) != 6);
		
		do {
			io_outpb(0x3f8+1, 0);
		} while (io_inpb(0x3f8+1) != 0);
		
		io_inpb(0x80); // do IO delay
		io_outpb(0x3f8+3, lcr);  
	}
	io_RestoreINT();
	
	// set format 8N1
	io_outpb(0x3f8+3, 0x03);
	// interrupt
	io_DisableINT();
	{
		irq_Setting(4, IRQ_LEVEL_TRIGGER);
		irq_InstallISR(4, isr_handler, NULL);
		
		io_outpb(0x3f8+1, 0x03);
		io_outpb(0x3f8+4, io_inpb(0x3f8+4) | 0x08);
	}
	io_RestoreINT();
	
	while (1) {
		if (kbhit() && getch() == 27)
			break;
		
		io_DisableINT();
		if (RxSize > 0) {
			ch = RxBuffer[RxHead];
			RxHead = (RxHead + 1) % 4096;
			RxSize--;
			TxBuffer[TxTail] = ch;
			TxTail = (TxTail + 1) % 4096;
			TxSize++;
			io_outpb(0x3f8+1, 0x03);
		}
		txc = txcount;
		rxc = rxcount;
		io_RestoreINT();
		
		if ((nowtime = timer_NowTime()) - pretime > 1000) {
			printf("rxcount = %ld, txcount = %ld\n", rxc, txc);
			pretime = nowtime;
		}
	}
	
	io_DisableINT();
	{
		io_outpb(0x3f8+4, io_inpb(0x3f8+4) & 0xf7); 
		io_outpb(0x3f8+1, 0x00);             
		
		irq_UninstallISR(4, NULL);
	}
	io_RestoreINT();
	irq_Close();
	io_Close();
	
	return 0;
}