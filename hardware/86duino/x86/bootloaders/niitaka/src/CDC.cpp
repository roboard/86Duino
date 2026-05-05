/* Copyright (c) 2011, Peter Barrett  
**  
** Permission to use, copy, modify, and/or distribute this software for  
** any purpose with or without fee is hereby granted, provided that the  
** above copyright notice and this permission notice appear in all copies.  
** 
** THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL  
** WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED  
** WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR  
** BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES  
** OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,  
** WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,  
** ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS  
** SOFTWARE.

  Modified 01 November 2013 by Android Lin  
*/

#include <stdio.h>
#include "com.h"
#include "USBAPI.h"

#include "fifolan.h"

extern COMPort *handle;

#define MAX_SOCKS   (4)
static int fd, rc, temp, fds[MAX_SOCKS] = {-1, -1, -1, -1};
static unsigned char rxbuf[4096];
static unsigned char txbuf[4096];
static unsigned long val, rx_timeout, tx_timeout, temp2, temp3;
static bool init_OK = false;

// port 86: serial port
// port 87: SD port

// serial command: 
// 1 : end
// 2 : available
// 3 : read
// 4 : availableForWrite
// 5 : flush
// 6 : write
// 7 : send
// 8 : ready
void Serial_::begin(unsigned long baud_count)
{
	peek_stored = false; peek_val = -1;
    
    if (init_OK) return;
    
    val = 1;
    rx_timeout = 100;
    tx_timeout = 100;
    
    fd = flan_socket();
    flan_ioctl(fd, FLAN_FIONBIO, (unsigned long)&val);
    flan_setsockopt(fd, FLAN_SO_RCVTIMEO, (const void*)&rx_timeout, sizeof(rx_timeout));
    flan_setsockopt(fd, FLAN_SO_SNDTIMEO, (const void*)&tx_timeout, sizeof(tx_timeout));
    flan_getsockopt(fd, FLAN_SO_RCVTIMEO, &temp2, &temp3);
    flan_getsockopt(fd, FLAN_SO_SNDTIMEO, &temp2, &temp3);
    
    do {
        rc = flan_connect(fd, 86); // 86 : serial port
        if (rc == 0)
            break;
        // if (rc < 0 && flan_errno(fd) != FLAN_EWOULDBLOCK) {
            // printf("begining...fail!\r");
            // printf("\n\n %d\n", flan_errno(fd));
            // return;
        // }
        printf("begining...\r");
    } while (1);
    printf("\n begin() OK!\n");
    init_OK = true;
}

void Serial_::begin(unsigned long baud_count, byte config)
{
	peek_stored = false; peek_val = -1;
}

static void send_command_para(uint8_t c, uint8_t* p, int p_size) {
    int i = 0, cur = 0, j;
    
    // command
    txbuf[i++] = c;
    
    // other parameters
    if (p != NULL) {
        for (j=0; j<p_size; j++)
            txbuf[i++] = *(p++);
    }
    
    do {
        rc = flan_send(fd, &txbuf[cur], i - cur);
        if (rc > 0) cur += rc;
    } while (cur != i);
    
}

// -1 : continue
//  0 : complite
static int recv_command_result(uint8_t* mybuf, int num) {
    int i, cur = 0;
    
    do {
        rc = flan_recv(fd, &rxbuf[cur], num - cur);
        if (rc < 0 && flan_errno(fd) == FLAN_ECONNABORTED)
            return -1;
        cur += rc;
    } while (cur != num);
    
    for (i=0; i<num; i++)
        mybuf[i] = rxbuf[i];
    
    return 0;
}

void Serial_::end(void)
{
    int r;
    uint8_t _r[1];
    
    if (!init_OK) return;
    send_command_para(1, NULL, 0); // 1 : "end" command
    r = recv_command_result(_r, 1);
    if (r == 0) {
        flan_close(fd);
    }
    init_OK = false;
}


void Serial_::accept(void) 
{
}


int Serial_::available(void)
{
    int i, r, t;
    uint8_t _r[4];
    
    if (!init_OK) return 0;
    
    send_command_para(2, NULL, 0); // 2 : "available" command
    r = recv_command_result(_r, 4);
    if (r == 0) {
        t = _r[3];
        for (i=2; i>=0; i--)
            t = (t << 8) + _r[i];
        return t;
    }
    return 0;
}


int Serial_::peek(void)
{
    int r;
    uint8_t _r[2];
    
    if (!init_OK) return -1;
    
	if(peek_stored == true)
		return peek_val;
  	else
  	{
        send_command_para(3, NULL, 0); // 3 : "read" command
        r = recv_command_result(_r, 2);
        if (r == 0) {
            if (_r[0] == 0xFF && _r[1] == 0xFF) {peek_val = -1; return -1;}
            peek_val = _r[0];
            peek_stored = true;
            return peek_val;
        }
	}
    return -1;
}


int Serial_::read(void)
{
	int r;
    uint8_t _r[2];
    
    if (!init_OK) return -1;
    
	if(peek_stored == true)
	{
		peek_stored = false;
		return peek_val;
	}
	else
	{
        send_command_para(3, NULL, 0); // 3 : "read" command
        r = recv_command_result(_r, 2);
        if (r == 0) {
            if (_r[0] == 0xFF && _r[1] == 0xFF) return -1;
            return _r[0];
        }
	}
    return -1;
}

int Serial_::availableForWrite(void)
{
    int i, r, t;
    uint8_t _r[4];
    
    if (!init_OK) return 0;
    
    send_command_para(4, NULL, 0); // 4 : "com_QueryTxQueue" command
    r = recv_command_result(_r, 4);
    if (r == 0) {
        t = _r[3];
        for (i=2; i>=0; i--)
            t = (t << 8) + _r[i];
        return t;
    }
    return 0;
}

void Serial_::flush(void)
{
    int r;
    uint8_t _r[1];
    
    if (!init_OK) return;
    
    send_command_para(5, NULL, 0); // 5 : "com_FlushTxQueue" command
    r = recv_command_result(_r, 1);
    if (r == 0) {
        printf("com_FlushTxQueue() OK!\n");
    }
}


size_t Serial_::write(uint8_t c)
{
	int r;
    uint8_t _r[1];
    
    if (!init_OK) return 0;
    
    send_command_para(6, &c, 1); // 6 : "write" command
    r = recv_command_result(_r, 1);
    if (r == 0) {
        return 1;
    }
		
	return 0;
}

size_t Serial_::write(const uint8_t *buffer, size_t size)
{
	int r;
    uint8_t _r[1];
    
    if (!init_OK) return 0;

    send_command_para(7, (uint8_t *)buffer, size); // 7 : "send" command
    r = recv_command_result(_r, 1);
    if (r == 0) {
        return _r[0];
    }
		
	return 0;
}

Serial_::operator bool() {
	bool result = false;
    int r;
    uint8_t _r[1];
    
    send_command_para(8, NULL, 0); // 8 : "Ready" command
    r = recv_command_result(_r, 1);
    if (r == 0) {
        if (_r[0] == 1)
            result = true;
        timer_Delay(10);
    }
	
    return result;
}
//*/
/*// Master
extern COMPort *handle;

void Serial_::begin(unsigned long baud_count)
{
	peek_stored = false; peek_val = -1;
}

void Serial_::begin(unsigned long baud_count, byte config)
{
	peek_stored = false; peek_val = -1;
}
 
void Serial_::end(void)
{
    if(handle == NULL) return;
    com_Close(handle);
    handle = NULL;
}


void Serial_::accept(void) 
{
}


int Serial_::available(void)
{
	if(handle == NULL) return 0;
	return com_QueryRxQueue(handle);
}


int Serial_::peek(void)
{
	if(handle == NULL) return -1;
	if(peek_stored == true)
		return peek_val;
  	else
  	{
		if((peek_val = com_Read(handle)) == 0xFFFF)
  			return -1; //peek_val = -1;
  		peek_stored = true;
		return peek_val;
	}
}


int Serial_::read(void)
{
	int c;
	if(handle == NULL) return -1;
	if(peek_stored == true)
	{
		peek_stored = false;
		return peek_val;
	}
	else
	{
		c = com_Read(handle);
		return (c == 0xFFFF) ? -1 : c;
	}	
}

int Serial_::availableForWrite(void)
{
	if(handle == NULL) return 0;
	return com_QueryTxQueue(handle);
}

void Serial_::flush(void)
{
	if(handle == NULL) return;
	com_FlushTxQueue(handle);
}


size_t Serial_::write(uint8_t c)
{
	if (handle != NULL && com_Ready(handle) != false && com_GetDTR(handle) != 0)
	{
		return (com_Write(handle, c) == true) ? 1 : 0;
	}
		
	return 0;
}

size_t Serial_::write(const uint8_t *buffer, size_t size)
{
	if (handle != NULL && com_Ready(handle) != false && com_GetDTR(handle) != 0)
	{
		int r = com_Send(handle, (uint8_t *)buffer, size);
		return (r > 0) ? r : 0;
	}
	return 0;
}

Serial_::operator bool() {
	bool result = false;
	if (com_Ready(handle) != false && com_GetDTR(handle) != 0) 
		result = true;
	timer_Delay(10);
	return result;
}
//*/
Serial_ Serial;

