/*
  86DuinoHardWare.cpp - Part of DM&P Vortex86 Rosserial86 library
  Copyright (c) 2015 DY Hung <Dyhung@dmp.com.tw>. All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  (If you need a commercial license, please contact soc@dmp.com.tw 
   to get more information.)
*/

#ifndef ROS_86DUINO_HARDWARE_CPP
#define ROS_86DUINO_HARDWARE_CPP

#include <86DuinoConfig.h>
#include <86DuinoHardware.h>

#ifndef _86DUINO

	#include <vortex86/utilities/dmpcfg.h>
	
	/* Use Vortex86 Turbo Serial on Vortex86 chip. */
	#ifdef  ROS_VORTEX86_TURBO_SERIAL
	#define ROS_VORTEX86_TURBO_SERIAL__
	#include <vortex86/utilities/Vortex86TurboSerial.h>
	#endif
	
	/* Use SwsSock library on DOS. */
	#ifdef DMP_DOS_DJGPP
	#define ROS_USE_SWSSOCK_LIB__
	#endif
	
#endif

#if defined(_86DUINO)

	/* Works on 86Duino */
	#define DEFAULT_PORT        "/USB_COM"
	#define DEFAULT_PORTNUM     11411
	#include <sws_sock.h>
	#include <sws_cfg.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <HardwareSerial.h>
	#include <io.h>

#elif defined(DMP_DOS_DJGPP)

	/* Works on DOS*/
	#define DEFAULT_NET_OPTIONS "sws_sock.ini"
	#define CUSTOM_NET_OPTIONS  "sws_sock.tmp"
	#define DEFAULT_PORT        "/COM1"
	#define DEFAULT_PORTNUM     11411
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <vortex86/utilities/io.h>
	#include <vortex86/utilities/com.h>
	#include <vortex86/utilities/sws_sock.h>
	#include <vortex86/utilities/sws_cfg.h>
	static bool SwsInited1 = false;
	static bool SwsInited2 = false;
	static bool swssock_initialize(char *options)
	{
		if (SwsInited2) return true;
		
		if (SWS_CfgSetName(options, NULL) == 0)
			return false;

		SwsInited1 = true;
		if (SWS_SockStartup(0, NULL)) {
			SWS_SockExit();
			return false;
		}
		
		return true;
	}

#elif defined(DMP_LINUX)

	/* Works on Linux */
	#define DEFAULT_PORT        "/dev/ttyAM1"
	#define DEFAULT_PORTNUM     11411
	#include <stdio.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <errno.h>
	#include <termios.h>
	#include <string.h>
	#include <time.h>
	#include <stdint.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/times.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
	#include <netdb.h>
	#include <assert.h>


#elif defined(DMP_WINDOWS)

	/* Works on Windows */
	#define DEFAULT_PORT        "/COM1"
	#define DEFAULT_PORTNUM     11411
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <windows.h>
	#pragma comment(lib, "Ws2_32.lib")

#endif

class Vortex86Handle
{
public:
#if defined(_86DUINO)
	Vortex86Handle() : sock(SWS_INVALID_SOCKET), port(NULL), port_(NULL) {}
	SWS_SOCKET sock;
	HardwareSerial *port;
	Serial_ *port_;
#elif defined(DMP_DOS_DJGPP)
	Vortex86Handle() : sock(SWS_INVALID_SOCKET), port(NULL) {}
	SWS_SOCKET sock;
	COMPort *port;
#elif defined(DMP_LINUX)
	Vortex86Handle() : fd(-1) {}
	int fd;
#elif defined(DMP_WINDOWS)
	Vortex86Handle() : sock(INVALID_SOCKET), port(INVALID_HANDLE_VALUE) {}
	SOCKET sock;
	HANDLE port;
#endif
};

void dummy_write(Vortex86Handle *p, unsigned char *pBuf, int nByte)
{

}

int dummy_read(Vortex86Handle *p)
{
	return -1;
}

int serial_read(Vortex86Handle *p)
{
#if defined(_86DUINO)
	if (p->port)
		return p->port->read();
	if (p->port_)
		return p->port_->read();
#else
	unsigned char ch;
	
#if defined(DMP_DOS_DJGPP)
	if (com_Receive(p->port, &ch, 1) > 0)
#elif defined(DMP_LINUX)
	if (read(p->fd, &ch, 1) > 0)
#elif defined(DMP_WINDOWS)
	COMSTAT stat;
	DWORD dwBytesRead;
        
	stat.cbInQue = 0;
	if (ClearCommError(p->port, NULL, &stat) == FALSE)
		return -1;
	if (stat.cbInQue <= 0)
		return -1;
	
	if (ReadFile(p->port, &ch, 1, &dwBytesRead, NULL) == TRUE)
#endif
		return (int)ch;
	
	return -1;
#endif
}

void serial_write(Vortex86Handle *p, unsigned char *pBuf, int nByte)
{
	int res, ret;

	res = nByte;
	while (res > 0) {
#if defined(_86DUINO)
		if (p->port)
			ret = p->port->write(pBuf[0]);
		else
			ret = p->port_->write(pBuf[0]);
#elif defined(DMP_DOS_DJGPP)
		ret = com_Send(p->port, pBuf, res);
#elif defined(DMP_LINUX)
		ret = write(p->fd, pBuf, res);
#elif defined(DMP_WINDOWS)
		WriteFile(p->port, pBuf, res, (PDWORD)&ret, NULL);
#endif
		if (ret > 0) {
			res -= ret;
			pBuf += ret;
		}
	}
}

int tcp_read(Vortex86Handle *p)
{
	unsigned char ch;
	int ret;
	
#if defined(_86DUINO) || defined(DMP_DOS_DJGPP)
	if ((ret = SWS_recv(p->sock, &ch, 1, 0)) > 0)
#elif defined(DMP_LINUX)
	if ((ret = read(p->fd, &ch, 1)) > 0)
#elif defined(DMP_WINDOWS)
	if ((ret = recv(p->sock, (char *)&ch, 1, 0)) > 0)
#endif
		return (int)ch;
	
	return -1;
}

void tcp_write(Vortex86Handle *p, unsigned char *pBuf, int nByte)
{
	int res, ret;
	
	res = nByte;
	while (res > 0) {
#if defined(_86DUINO) || defined(DMP_DOS_DJGPP)
		ret = SWS_send(p->sock, pBuf, res, 0);
		if (ret < 0) {
			unsigned long error = SWS_GetLastError();
			if (error == SWS_EWOULDBLOCK) {
#elif defined(DMP_LINUX)
		ret = write(p->fd, pBuf, res);
		if (ret < 0) {
			unsigned long error = errno;
			if (error == EWOULDBLOCK) {
#elif defined(DMP_WINDOWS)
		ret = send(p->sock, (const char *)pBuf, res, 0);
		if (ret == SOCKET_ERROR) {
			unsigned long error = WSAGetLastError();
			if (error == WSAEWOULDBLOCK) {
#endif
				ret = 0;
			} else {
				printf("write(): send failed with error %ld\n", error);
				break;
			}
		}
		if (ret > 0) {
			res -= ret;
			pBuf += ret;
		}
	}
}

class Vortex86Hardware
{
public:
	Vortex86Hardware();
	void Vortex86HardwareExit();
	
	void init(char *pName, long baud);
	
	unsigned long time();
	
	int  (*read)   (Vortex86Handle *p);
	void (*write)  (Vortex86Handle *p, unsigned char *pBuf, int nByte);

	Vortex86Handle handle;
	
private:
	unsigned long initTime;
	int com;
	
#ifdef ROS_VORTEX86_TURBO_SERIAL__
	Vortex86TurboUART uart;
#endif
};

Vortex86Hardware::Vortex86Hardware()
{
	this->com = -1;
	this->initTime = 0UL;
	this->read = dummy_read;
	this->write = dummy_write;
	
	#ifdef ROS_VORTEX86_TURBO_SERIAL__
		
		#ifdef ROS_VORTEX86_CHIP_INITIALIZE
			uart.Vortex86TurboUARTInit();
		#endif
		
	#endif
}
	
void Vortex86Hardware::Vortex86HardwareExit()
{
	#ifdef ROS_VORTEX86_TURBO_SERIAL__

		uart.RestoreVortex86TurboUART(com);
		
		#ifdef ROS_VORTEX86_CHIP_INITIALIZE
			uart.Vortex86TurboUARTClose();
		#endif
		
	#endif
}

void Vortex86Hardware::init(char *p, long baud)
{
	int len, min;
	char pName[128];

	min = sizeof(pName) - 1;
	if (min > (len = strlen(p))) min = len;
	strncpy(pName, p, min);
	pName[min] = '\0';
	
	if (pName[0] == '/' || pName[0] == '\\') {
		
		/* Initial Serial on 86Duino. */
		#if defined(_86DUINO)
		
			if (strcmp(&pName[1], "Serial1") == 0) {
				Serial1.begin(baud);
				handle.port  = &Serial1;
				handle.port_ = NULL;
			} else if (strcmp(&pName[1], "Serial2") == 0) {
				Serial2.begin(baud);
				handle.port  = &Serial2;
				handle.port_ = NULL;
			} else if (strcmp(&pName[1], "Serial3") == 0) {
				Serial3.begin(baud);
				handle.port  = &Serial3;
				handle.port_ = NULL;
			} else if (strcmp(&pName[1], "Serial485") == 0) {
				Serial485.begin(baud);
				handle.port  = &Serial485;
				handle.port_ = NULL;
			}  else if (strcmp(&pName[1], "Serial232") == 0) {
				Serial232.begin(baud);
				handle.port  = &Serial232;
				handle.port_ = NULL;
			} else {
				Serial.begin(baud);
				handle.port  = NULL;
				handle.port_ = &Serial;
			}
			
		/* Initial Serial on DOS for Vortex86 chip. */
		#elif defined(DMP_DOS_DJGPP)
		
			unsigned long bps;
		
			printf("Opening serial port \"%s\".\n", &pName[1]);
			
				 if (strcmp(&pName[1], "COM1"   ) == 0) com = COM1;
			else if (strcmp(&pName[1], "COM2"   ) == 0) com = COM2;
			else if (strcmp(&pName[1], "COM3"   ) == 0) com = COM3;
			else if (strcmp(&pName[1], "COM4"   ) == 0) com = COM4;
			else if (strcmp(&pName[1], "COM5"   ) == 0) com = COM5;
			else if (strcmp(&pName[1], "COM6"   ) == 0) com = COM6;
			else if (strcmp(&pName[1], "COM7"   ) == 0) com = COM7;
			else if (strcmp(&pName[1], "COM8"   ) == 0) com = COM8;
			else if (strcmp(&pName[1], "COM9"   ) == 0) com = COM9;
			else if (strcmp(&pName[1], "COM10"  ) == 0) com = COM10;
			else if (strcmp(&pName[1], "USB_COM") == 0) com = USB_COM;
			else {
				printf("init(): Unable to open serial port - Exiting\n");
				exit(-1);
			}
			handle.port = com_Init(com);
			if (handle.port == NULL) {
				printf("init(): Unable to open serial port - Exiting\n");
				exit(-1);
			}
			
			switch (baud)
			{
				case 6000000L: bps = COM_UARTBAUD_6000000BPS; break;
				case 3000000L: bps = COM_UARTBAUD_3000000BPS; break;
				case 2000000L: bps = COM_UARTBAUD_2000000BPS; break;
				case 1500000L: bps = COM_UARTBAUD_1500000BPS; break;
				case 1000000L: bps = COM_UARTBAUD_1000000BPS; break;
				case 750000L:  bps = COM_UARTBAUD_750000BPS;  break;
				case 500000L:  bps = COM_UARTBAUD_500000BPS;  break;
				case 461538L:  bps = COM_UARTBAUD_461538BPS;  break;
				case 333333L:  bps = COM_UARTBAUD_333333BPS;  break;
				case 300000L:  bps = COM_UARTBAUD_300000BPS;  break;
				case 250000L:  bps = COM_UARTBAUD_250000BPS;  break;
				case 200000L:  bps = COM_UARTBAUD_200000BPS;  break;
				case 150000L:  bps = COM_UARTBAUD_150000BPS;  break;
				case 125000L:  bps = COM_UARTBAUD_125000BPS;  break;
				case 115200L:  bps = COM_UARTBAUD_115200BPS;  break;
				case 57600L:   bps = COM_UARTBAUD_57600BPS;   break;
				case 38400L:   bps = COM_UARTBAUD_38400BPS;   break;
				case 19200L:   bps = COM_UARTBAUD_19200BPS;   break;
				case 9600L:    bps = COM_UARTBAUD_9600BPS;    break;
				case 4800L:    bps = COM_UARTBAUD_4800BPS;    break;
				case 2400L:    bps = COM_UARTBAUD_2400BPS;    break;
				case 1200L:    bps = COM_UARTBAUD_1200BPS;    break;
				case 300L:     bps = COM_UARTBAUD_300BPS;     break;
				case 50L:      bps = COM_UARTBAUD_50BPS;      break;
				default:
					printf("init(): Unsupported Baudrate %ld, now using default baudrate 57600.\n", baud);
					bps = COM_UARTBAUD_57600BPS;
					baud = 57600;
					break;
			}
			if (com_SetBPS(handle.port, bps) == false) {
				printf("init(): Unsupported Baudrate %ld, now using default baudrate 57600.\n", baud);
				com_SetBPS(handle.port, COM_UARTBAUD_57600BPS);
			}
			com_SetFormat(handle.port, BYTESIZE8 + NOPARITY + STOPBIT1);
			com_SetTimeOut(handle.port, 0L, 500L);
			
			printf("Serial port \"%s\" is opened successfully.\n", &pName[1]);
		
		/* Initial Serial on Linux. */
		#elif defined(DMP_LINUX)
		
			struct termios options;
			speed_t bps;
			long temp_baud = baud;
			
			printf("Opening serial port \"%s\".\n", &pName[0]);

				 if (strcmp(&pName[0], "/dev/ttyS0") == 0) com = 0;
			else if (strcmp(&pName[0], "/dev/ttyS1") == 0) com = 1;
			else if (strcmp(&pName[0], "/dev/ttyS2") == 0) com = 2;
			else if (strcmp(&pName[0], "/dev/ttyS3") == 0) com = 3;
			else if (strcmp(&pName[0], "/dev/ttyS4") == 0) com = 4;
			else if (strcmp(&pName[0], "/dev/ttyS5") == 0) com = 5;
			else if (strcmp(&pName[0], "/dev/ttyS6") == 0) com = 6;
			else if (strcmp(&pName[0], "/dev/ttyS7") == 0) com = 7;
			else if (strcmp(&pName[0], "/dev/ttyS8") == 0) com = 8;
			else if (strcmp(&pName[0], "/dev/ttyS9") == 0) com = 9;

			handle.fd = open(pName, O_RDWR | O_NOCTTY | O_NDELAY);
			if (handle.fd == -1) {
				printf("init(): Unable to open serial port - Exiting\n");
				exit(-1);
			}
			fcntl(handle.fd, F_SETFL, FNDELAY);
			
			#ifdef ROS_VORTEX86_TURBO_SERIAL__
				temp_baud = uart.ReCaculateBaudrate(com, baud);
			#endif
			switch (temp_baud)
			{
				case  50L:     bps = B50;     break;
				case  75L:     bps = B75;     break;
				case  110L:    bps = B110;    break;
				case  134L:    bps = B134;    break;
				case  150L:    bps = B150;    break;
				case  200L:    bps = B200;    break;
				case  300L:    bps = B300;    break;
				case  600L:    bps = B600;    break;
				case  1200L:   bps = B1200;   break;
				case  1800L:   bps = B1800;   break;
				case  2400L:   bps = B2400;   break;
				case  4800L:   bps = B4800;   break;
				case  9600L:   bps = B9600;   break;
				case  19200L:  bps = B19200;  break;
				case  38400L:  bps = B38400;  break;
				case  57600L:  bps = B57600;  break;
				case  115200L: bps = B115200; break;
				default:
					printf("init(): Unsupported Baudrate %ld, now using default baudrate 57600.\n", baud);
					bps = B57600;
					baud = 57600;
					#ifdef ROS_VORTEX86_TURBO_SERIAL__
						uart.DisableVortex86TurboUART(com);
					#endif
					break;
			}
			
			tcgetattr(handle.fd, &options);
			cfsetispeed(&options, bps);
			cfsetospeed(&options, bps);
			options.c_cflag |= (CLOCAL | CREAD);
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			options.c_cflag &= ~CSIZE;
			options.c_cflag |= CS8;
			options.c_cflag &= ~CRTSCTS;
			options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
			options.c_iflag &= ~(IXON | IXOFF | IXANY);
			options.c_oflag &= ~OPOST;
			tcsetattr(handle.fd, TCSANOW, &options);
			
			printf("Serial port \"%s\" is opened successfully.\n", &pName[0]);
		
		/* Initial Serial on Windows. */		
		#elif defined(DMP_WINDOWS)
			DCB options;
			COMMTIMEOUTS timeouts;
			long temp_baud = baud;

			printf("Opening serial port \"%s\".\n", &pName[1]);
			
				 if (strcmp(&pName[1], "COM1" ) == 0 || strcmp(&pName[1], "\\\\.\\COM1" ) == 0) com = 0;
			else if (strcmp(&pName[1], "COM2" ) == 0 || strcmp(&pName[1], "\\\\.\\COM2" ) == 0) com = 1;
			else if (strcmp(&pName[1], "COM3" ) == 0 || strcmp(&pName[1], "\\\\.\\COM3" ) == 0) com = 2;
			else if (strcmp(&pName[1], "COM4" ) == 0 || strcmp(&pName[1], "\\\\.\\COM4" ) == 0) com = 3;
			else if (strcmp(&pName[1], "COM5" ) == 0 || strcmp(&pName[1], "\\\\.\\COM5" ) == 0) com = 4;
			else if (strcmp(&pName[1], "COM6" ) == 0 || strcmp(&pName[1], "\\\\.\\COM6" ) == 0) com = 5;
			else if (strcmp(&pName[1], "COM7" ) == 0 || strcmp(&pName[1], "\\\\.\\COM7" ) == 0) com = 6;
			else if (strcmp(&pName[1], "COM8" ) == 0 || strcmp(&pName[1], "\\\\.\\COM8" ) == 0) com = 7;
			else if (strcmp(&pName[1], "COM9" ) == 0 || strcmp(&pName[1], "\\\\.\\COM9" ) == 0) com = 8;
			else if (strcmp(&pName[1], "COM10") == 0 || strcmp(&pName[1], "\\\\.\\COM10") == 0) com = 9;
			
			handle.port = CreateFileA(&pName[1],                     // device name of COM port
									  GENERIC_READ | GENERIC_WRITE,  // access mode
									  0,                             // share mode
									  0,                             // security attributes
									  OPEN_EXISTING,                 // opens a device only if it exists
									  0,                             // non-overlapped
									  NULL);                         // NULL when opening an existing file
			if (handle.port == INVALID_HANDLE_VALUE) {
				printf("init(): Unable to open serial port - Exiting\n");
				exit(-1);
			}
			
			#ifdef ROS_VORTEX86_TURBO_SERIAL__
				temp_baud = uart.ReCaculateBaudrate(com, baud);
			#endif
			if (GetCommState(handle.port, &options) == FALSE) {
				printf("init(): Unable to GetCommState()\n");
				CloseHandle(handle.port);
				exit(-1);
			}
			options.BaudRate        = temp_baud;
			options.fBinary         = TRUE;                 // binary mode
			options.fOutxCtsFlow    = FALSE;                // no CTS output control
			options.fOutxDsrFlow    = FALSE;                // no DSR output control
			options.fDtrControl     = DTR_CONTROL_DISABLE;  // no DRT control
			options.fDsrSensitivity = FALSE;                // no sensitive to DSR
			options.fOutX           = FALSE;                // no S/W output flow control
			options.fInX            = FALSE;                // no S/W input flow control
			options.fErrorChar      = FALSE;                // no replace parity-error byte
			options.fNull           = FALSE;                // no discard NULL byte
			options.fRtsControl     = RTS_CONTROL_DISABLE;  // no S/W input flow control
			options.fAbortOnError   = FALSE;                // no terminate on errors
			options.ByteSize        = 8;
			options.StopBits        = ONESTOPBIT;
			options.Parity          = NOPARITY;
			if (SetCommState(handle.port, &options) == FALSE) {
				printf("init(): Unable to SetCommState()\n");
				CloseHandle(handle.port);
				exit(-1);
			}

			if (GetCommTimeouts(handle.port, &timeouts) == FALSE) {
				printf("init(): Unable to GetCommTimeouts()\n");
				CloseHandle(handle.port);
				exit(-1);
			}
			timeouts.ReadIntervalTimeout         = MAXDWORD;
			timeouts.ReadTotalTimeoutConstant    = 0;
			timeouts.ReadTotalTimeoutMultiplier  = 0;
			timeouts.WriteTotalTimeoutConstant   = 0;
			timeouts.WriteTotalTimeoutMultiplier = 0;
			if (SetCommTimeouts(handle.port, &timeouts) == FALSE) {
				printf("init(): Unable to SetCommTimeouts()\n");
				CloseHandle(handle.port);
				exit(-1);
			}
			ClearCommBreak(handle.port);
			ClearCommError(handle.port, NULL, NULL);               // clear all communication errors
			SetupComm(handle.port, 8192, 8192);                    // set read/write FIFO to 8KB
			PurgeComm(handle.port, PURGE_RXABORT | PURGE_RXCLEAR | // clear all communication buffers
								   PURGE_TXABORT | PURGE_TXCLEAR);
								   
			printf("Serial port \"%s\" is opened successfully.\n", &pName[1]);
		#endif
		
		this->read = serial_read;
		this->write = serial_write;
		
	} else {
	
		/* Initial Ethernet on 86Duino and DOS. */
		#if defined(_86DUINO) || defined(DMP_DOS_DJGPP)
			char *IP;
			char *PortNumStr;
			long PortNum;
			int ret;
			int flag = 1;
			SWS_u_long lArg = 1;
			struct SWS_sockaddr_in serv_addr;
			struct SWS_hostent *server;
			
			#ifdef ROS_USE_SWSSOCK_LIB__
				if (swssock_initialize(DEFAULT_NET_OPTIONS) == false) {
					printf("init(): SwsSock library startup fail.\n");
					exit(-1);
				}
				SwsInited2 = true;
			#endif
			
			IP = strtok(pName, ":");
			PortNumStr = strtok(NULL, ":");
			PortNum = 0;
			if (PortNumStr != NULL) {
				PortNum = strtol(PortNumStr, NULL, 10);
			}
			if (PortNum == 0) {
				PortNum = DEFAULT_PORTNUM;
			}

			handle.sock = SWS_socket(SWS_AF_INET, SWS_SOCK_STREAM, 0);
			if (handle.sock == SWS_INVALID_SOCKET) {
				printf("init(): open socket fail.\n");
				SWS_SockExit();
				#ifdef _86DUINO
					return;
				#else
					exit(-1);
				#endif
			}
			ret = SWS_setsockopt(handle.sock, SWS_IPPROTO_TCP, SWS_TCP_NODELAY, (char*)&flag, sizeof(flag));
			if (ret == -1) {
				printf("init(): Could not setsockopt(TCP_NODELAY)\n");
				SWS_close(handle.sock);
				SWS_SockExit();
				#if defined(_86DUINO)
					return;
				#else
					exit(-1);
				#endif
			}
			
			printf("Connecting to TCP server at %s:%ld....\n", IP, PortNum);
			bzero((char *)&serv_addr, sizeof(serv_addr));
			serv_addr.sin_family = SWS_AF_INET;
			serv_addr.sin_port = SWS_htons(PortNum);
			server = SWS_gethostbyname(IP);
			if (server == NULL) {
				serv_addr.sin_addr.SWS_s_addr = SWS_inet_addr(IP);
			} else {
				bcopy((char *)server->SWS_h_addr, (char *)&serv_addr.sin_addr.SWS_s_addr, server->h_length);
			}
			if (SWS_connect(handle.sock,(struct SWS_sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
				printf("init(): connect to server fail.\n");
				SWS_close(handle.sock);
				SWS_SockExit();
				#if defined(_86DUINO)
					return;
				#else
					exit(-1);
				#endif
			}
			SWS_ioctl(handle.sock, SWS_FIONBIO, &lArg);
			
			printf("Connet to %s:%ld successfully\n", IP, PortNum);
			
		/* Initial Ethernet on Linux. */
		#elif defined(DMP_LINUX)
			char *IP;
			char *PortNumStr;
			long int PortNum;
			int ret, lArg;
			int flag = 1;
			struct sockaddr_in serv_addr;
			struct hostent *server;
			
			IP = strtok(pName, ":");
			PortNumStr = strtok(NULL, ":");
			PortNum = 0;
			if (PortNumStr != NULL) {
				PortNum = strtol(PortNumStr, NULL, 10);
			}
			if (PortNum == 0) {
				PortNum = DEFAULT_PORTNUM;
			}

			handle.fd = socket(AF_INET, SOCK_STREAM, 0);
			if (handle.fd < 0) {
				printf("init(): open socket fail.\n");
				exit(-1);
			}
			ret = setsockopt(handle.fd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
			if (ret == -1) {
				printf("init(): Could not setsockopt(TCP_NODELAY)\n");
				close(handle.fd);
				exit(-1);
			}

			server = gethostbyname(IP);
			if (server == NULL) {
				printf("init(): gethostbyname() fail, no such host.\n");
				close(handle.fd);
				exit(0);
			}
			
			printf("Connecting to TCP server at %s:%ld....\n", IP, PortNum);
			bzero((char *) &serv_addr, sizeof(serv_addr));
			serv_addr.sin_family = AF_INET;
			bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
			serv_addr.sin_port = htons(PortNum);
			if (connect(handle.fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
				printf("init(): connect to server fail.\n");
				close(handle.fd);
				exit(-1);
			}
			lArg = fcntl(handle.fd, F_GETFL, 0);
			assert(lArg != -1);
			fcntl(handle.fd, F_SETFL, lArg | O_NONBLOCK);

			printf("Connet to %s:%ld successfully\n", IP, PortNum);
			
		/* Initial Ethernet on Windows. */		
		#elif defined(DMP_WINDOWS)
			char *IP;
			char *PortNumStr;
			long PortNum;
			int ret;
			int flag = 1;
			u_long lArg = 1;
			struct sockaddr_in serv_addr;
			struct hostent *server;
			WSADATA wsaData;
			
			if (ret = WSAStartup (MAKEWORD(2, 2), &wsaData)) {
				printf("init(): Could not initialize windows socket. (%d)\n", ret);
				exit(-1);
			}
			
			IP = strtok(pName, ":");
			PortNumStr = strtok(NULL, ":");
			PortNum = 0;
			if (PortNumStr != NULL) {
				PortNum = strtol(PortNumStr, NULL, 10);
			}
			if (PortNum == 0){
				PortNum = DEFAULT_PORTNUM;
			}
			
			handle.sock = socket(AF_INET, SOCK_STREAM, 0);
			if (handle.sock == INVALID_SOCKET) {
				printf("init(): open socket fail.\n");
				WSACleanup();
				exit(-1);
			}
			if (setsockopt(handle.sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&flag, sizeof (flag)) < 0) {
				printf("init(): Could not setsockopt(TCP_NODELAY)\n");
				closesocket (handle.sock);
				WSACleanup();
				exit(-1);
			}
			
			printf("Connecting to TCP server at %s:%ld....\n", IP, PortNum);
			ZeroMemory((char *)&serv_addr, sizeof(serv_addr));
			serv_addr.sin_family = AF_INET;
			serv_addr.sin_port = htons(PortNum);
			server = gethostbyname(IP);
			if (server == NULL) {
				serv_addr.sin_addr.s_addr = inet_addr(IP);
			} else {
				CopyMemory((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
			}
			if (connect(handle.sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
				printf("init(): connect to server fail.\n");
				closesocket (handle.sock);
				WSACleanup();
				exit(-1);
			}

			if (ret = ioctlsocket(handle.sock, FIONBIO, &lArg)) {
				printf("init(): Could not make socket nonblocking. (%d)\n", ret);
				closesocket (handle.sock);
				WSACleanup();
				exit(-1);
			}

			printf("Connet to %s:%ld successfully\n", IP, PortNum);
			
		#endif
		
		this->read = tcp_read;
		this->write = tcp_write;
	}
	initTime = this->time();
}

unsigned long Vortex86Hardware::time()
{
#if defined(_86DUINO) || defined(DMP_DOS_DJGPP)
	return timer_NowTime() - initTime;
#elif defined(DMP_LINUX)
	struct timespec cputime;
	clock_gettime(CLOCK_MONOTONIC, &cputime);
	return (unsigned long)((unsigned long long)(cputime.tv_sec)*1000L + (unsigned long long)(cputime.tv_nsec/1000000L)) - initTime;
#elif defined(DMP_WINDOWS)
	LARGE_INTEGER freq;
	LARGE_INTEGER tsc;

	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&tsc);
	return (unsigned long)((unsigned long long)tsc.QuadPart*1000L/freq.QuadPart) - initTime;
#endif
}

x86DuinoHardware::x86DuinoHardware()
{
	int len, min;
	char *pName;

	x86 = new Vortex86Hardware();

	pName = getenv("ROSSERIAL_PORT");
	if (pName == NULL) {
		pName = (char *)DEFAULT_PORT;
	}

	min = sizeof(portName) - 1;
	if (min > (len = strlen(pName))) min = len;
	strncpy(portName, pName, min);
	baud_ = 57600L;
}

x86DuinoHardware::x86DuinoHardware(char *pName, long baud/* = 57600L*/)
{
	int len, min;
	
	x86 = new Vortex86Hardware();
	
	min = sizeof(portName) - 1;
	if (min > (len = strlen(pName))) min = len;
	strncpy(portName, pName, min);
	baud_ = baud;
}

x86DuinoHardware::~x86DuinoHardware()
{
	x86->Vortex86HardwareExit();
	delete [] x86;
}

void x86DuinoHardware::setBaud(long baud)
{
	baud_ = baud;
}

long x86DuinoHardware::getBaud()
{
	return baud_;
}

void x86DuinoHardware::init()
{
	init(portName);
}

void x86DuinoHardware::init(char *pName)
{
	x86->init(pName, baud_);
}

int x86DuinoHardware::read()
{
	return x86->read(&x86->handle);
}

void x86DuinoHardware::write(unsigned char *pBuf, int nByte)
{
	x86->write(&x86->handle, pBuf, nByte);
}

unsigned long x86DuinoHardware::time()
{
	return x86->time();
}

void x86DuinoHardware::delay(unsigned long millisecond)
{
	millisecond += x86->time();
	while ((long)(millisecond - x86->time()) > 0L);
}

bool x86DuinoHardware::setDhcp()
{
#ifdef ROS_USE_SWSSOCK_LIB__
	char hostname[128];
	FILE *opt;
	
	opt = fopen(CUSTOM_NET_OPTIONS, "w");
	
	if (opt == NULL) {
		printf("setDhcp(): Open %s fail.\n", CUSTOM_NET_OPTIONS);
		return false;
	}
		
	fprintf(opt, "[PacketDriverInterface1]\n");
	fprintf(opt, "dhcp=1\n");
	fprintf(opt, "\n");
	fprintf(opt, "[dns]\n");
	fprintf(opt, "hostname=86Duino\n");
	fflush(opt);
	fclose(opt);
	
	if (swssock_initialize(CUSTOM_NET_OPTIONS)) {
		if (SWS_gethostname(hostname, sizeof(hostname)) != 0) {
			SWS_SockExit();
			printf("setDhcp(): Dhcp fail.\n");
			return false;
		}
		if (strcmp(hostname, "127.0.0.1") == 0) {
			SWS_SockExit();
			printf("setDhcp(): Dhcp fail.\n");
			return false;
		}
		SwsInited2 = true;
		return true;
	}
	printf("setDhcp(): SwsSock library starup fail.\n");
	
	return false;
#else
	return true;
#endif
}

bool x86DuinoHardware::setEthernet(char *ip)
{
#ifdef ROS_USE_SWSSOCK_LIB__
	char dns[32];
	unsigned long number = SWS_inet_addr(ip);
	sprintf(dns, "%d.%d.%d.1", 0xFFUL &  number
	                         , 0xFFUL & (number >>  8)
							 , 0xFFUL & (number >> 16));
	return setEthernet(ip, dns);
#else
	return true;
#endif
}

bool x86DuinoHardware::setEthernet(char *ip, char *dns)
{
#ifdef ROS_USE_SWSSOCK_LIB__
	char gateway[32];
	unsigned long number = SWS_inet_addr(ip);
	sprintf(gateway, "%d.%d.%d.1", 0xFFUL &  number
	                             , 0xFFUL & (number >>  8)
							     , 0xFFUL & (number >> 16));
	return setEthernet(ip, dns, gateway);
#else
	return true;
#endif
}

bool x86DuinoHardware::setEthernet(char *ip, char *dns, char *gateway)
{
#ifdef ROS_USE_SWSSOCK_LIB__
	char subnet[] = "255.255.255.0";
	return setEthernet(ip, dns, gateway, subnet);
#else
	return true;
#endif
}

bool x86DuinoHardware::setEthernet(char *ip, char *dns, char *gateway, char *subnet)
{
#ifdef ROS_USE_SWSSOCK_LIB__
	FILE *opt;
	unsigned long number;
	
	if (SWS_inet_addr(ip) == 0xFFFFFFFFUL) {
		printf("setEthernet(): Unrecognized IP address.\n");
		return false;
	}
	if (SWS_inet_addr(dns) == 0xFFFFFFFFUL) {
		printf("setEthernet(): Unrecognized DNS server address.\n");
		return false;
	}
	if (SWS_inet_addr(gateway) == 0xFFFFFFFFUL) {
		printf("setEthernet(): Unrecognized gateway address.\n");
		return false;
	}
	if (SWS_inet_addr(subnet) == 0xFFFFFFFFUL) {
		printf("setEthernet(): Unrecognized subnet mask.\n");
		return false;
	}
	
	
	opt = fopen(CUSTOM_NET_OPTIONS, "w");
	
	if (opt == NULL) {
		printf("setEthernet(): Open %s fail.\n", CUSTOM_NET_OPTIONS);
		return false;
	}
	
	fprintf(opt, (SwsInited1 == false) ? "[PacketDriverInterface1]\n" : "[PacketDriverInterface2]\n");
	fprintf(opt, "ipaddress=%s\n", ip);
	fprintf(opt, "ipsubnet=%s\n", subnet);
	fprintf(opt, "ipgateway=%s\n", gateway);
	fprintf(opt, "dhcp=0\n");
	fprintf(opt, "\n");
	fprintf(opt, "[dns]\n");
	fprintf(opt, "hostname=86Duino\n");
	fprintf(opt, "server=%s\n", dns);
	fflush(opt);
	fclose(opt);
	
	if (swssock_initialize(CUSTOM_NET_OPTIONS)) {
		SwsInited2 = true;
		return true;
	}
	printf("setEthernet(): SwsSock library starup fail.\n");
	
	return false;
#else
	return true;
#endif
}

#endif
