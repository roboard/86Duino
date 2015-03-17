#ifndef __VORTEX86_TURBO_UART_H
#define __VORTEX86_TURBO_UART_H

#include <io.h>
#include <vortex86.h>

class Vortex86TurboUART
{
public:
	Vortex86TurboUART() : OldCS(0), OldHCS(0), OldTurbo(0), Stored(false) {}
	
	bool Vortex86TurboUARTInit();
	void Vortex86TurboUARTClose();
	
	void StoreVortex86TurboUART(int com);
	void RestoreVortex86TurboUART(int com);
	
	void EnableVortex86TurboUART(int com);
	void DisableVortex86TurboUART(int com);
	
	long ReCaculateBaudrate(int com, long baud);
	
private:
	int OldCS;
	int OldHCS;
	
	int OldTurbo;
	
	bool Stored;
};

bool Vortex86TurboUART::Vortex86TurboUARTInit()
{
	return io_Init();
}

void Vortex86TurboUART::Vortex86TurboUARTClose()
{
	io_Close();
}

void Vortex86TurboUART::StoreVortex86TurboUART(int com)
{
	if (Stored) return;
	
	switch (vx86_CpuID())
	{
	case CPU_VORTEX86EX:
	case CPU_VORTEX86DX2:
	case CPU_VORTEX86DX3:
		OldCS = vx86_uart_GetCS(com);
		OldHCS = vx86_uart_GetHCS(com);
		break;
	case CPU_VORTEX86DX_D:
	case CPU_VORTEX86MX:
	case CPU_VORTEX86MX_PLUS:
		OldTurbo = vx86dx_IsTurboMode(com);
		break;
	default:
		break;
	}
	
	Stored = true;
}

void Vortex86TurboUART::RestoreVortex86TurboUART(int com)
{
	if (!Stored) return;
	
	switch (vx86_CpuID())
	{
	case CPU_VORTEX86EX:
	case CPU_VORTEX86DX2:
	case CPU_VORTEX86DX3:
		vx86_uart_SetCS(com, OldCS);
		vx86_uart_SetHCS(com, OldHCS);
		break;
	case CPU_VORTEX86DX_D:
	case CPU_VORTEX86MX:
	case CPU_VORTEX86MX_PLUS:
		if (OldTurbo)
			vx86_EnableTurboMode(com);
		else
			vx86_DisableTurboMode(com);
		break;
	default:
		break;
	}
}

void Vortex86TurboUART::EnableVortex86TurboUART(int com)
{
	StoreVortex86TurboUART(com);
	
	switch (vx86_CpuID())
	{
	case CPU_VORTEX86EX:
	case CPU_VORTEX86DX2:
	case CPU_VORTEX86DX3:
		if (vx86_uart_GetSBCLK() == 0) {
			vx86_uart_SetCS(com, 1);
			vx86_uart_SetHCS(com, 0);
		} else {
			vx86_uart_SetCS(com, 1);
			vx86_uart_SetHCS(com, 1);
		}
		break;
	case CPU_VORTEX86DX_D:
	case CPU_VORTEX86MX:
	case CPU_VORTEX86MX_PLUS:
		vx86_EnableTurboMode(com);
		break;
	default:
		break;
	}
}

void Vortex86TurboUART::DisableVortex86TurboUART(int com)
{
	StoreVortex86TurboUART(com);
	
	switch (vx86_CpuID())
	{
	case CPU_VORTEX86EX:
	case CPU_VORTEX86DX2:
	case CPU_VORTEX86DX3:
		vx86_uart_SetCS(com, 0);
		vx86_uart_SetHCS(com, 0);
		break;
	case CPU_VORTEX86DX_D:
	case CPU_VORTEX86MX:
	case CPU_VORTEX86MX_PLUS:
		vx86_DisableTurboMode(com);
		break;
	default:
		break;
	}
}

long Vortex86TurboUART::ReCaculateBaudrate(int com, long baud)
{
	long div, MaxBps;
	
	DisableVortex86TurboUART(com);
	
	if (baud <= 115200L)
		return baud;
	
	EnableVortex86TurboUART(com);
	MaxBps = (long)vx86_uart_MaxBPS(com);
	div = MaxBps/baud;
	if (MaxBps != 1500000L || div != 1L) {
		if (div > 0) baud = 115200L/div;
	}
	
	return baud;
}

#endif