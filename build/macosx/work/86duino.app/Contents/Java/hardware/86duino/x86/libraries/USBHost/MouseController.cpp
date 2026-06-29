/*
  msctrl.cpp - Part of DM&P Vortex86 USB Host library
  Copyright (c) 2014 Android Lin <acen@dmp.com.tw>. All right reserved.

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

#include "Arduino.h"
#define ALLEGRO_HAVE_STDINT_H
#include <allegro.h>
#include "MouseController.h"
int __allegro_function_init = 0;
static bool msInit = false;

#if defined (USBHOST_REMOTE)
#include "ipcnet.h"
#define MAX_SOCKS   (4)
static int fd, rc;
static unsigned char rxbuf[4096];
static unsigned char txbuf[4096];
static bool init_OK = false;
static unsigned long val;
// static unsigned char mouse_b = 0;

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
        rc = ipcnet_send(fd, &txbuf[cur], i - cur);
        if (rc > 0) cur += rc;
    } while (cur != i);
    
}

// -1 : continue
//  0 : complite
static int recv_command_result(uint8_t* mybuf, int num) {
    int i, cur = 0;
    
    do {
        rc = ipcnet_recv(fd, &rxbuf[cur], num - cur);
        if (rc < 0 && ipcnet_errno(fd) == IPCNET_ECONNABORTED)
            return -1;
        cur += rc;
    } while (cur != num);
    
    for (i=0; i<num; i++)
        mybuf[i] = rxbuf[i];
    
    return 0;
}

static int fourBytes2Int(unsigned char a0, unsigned char a1, unsigned char a2, unsigned char a3) {
    int r = 0;
    r = a3;
    r = (r << 8) + a2;
    r = (r << 8) + a1;
    r = (r << 8) + a0;
    return r;
}


MouseController::MouseController(USBHost &pusb) {
    USBHost *p = &pusb;
	p->RegisterMouseClass(this);
    
	val = 1;
    
    fd = ipcnet_socket();
    ipcnet_ioctl(fd, IPCNET_FIONBIO, (unsigned long)&val);
    
    do {
        rc = ipcnet_connect(fd, 88); // 88 : USBHOST mouse port
        if (rc == 0)
            break;
        // if (rc < 0 && ipcnet_errno(fd) != ipcnet_EWOULDBLOCK) {
            // printf("begining...fail!\r");
            // printf("\n\n %d\n", ipcnet_errno(fd));
            // return;
        // }
        printf("begining...\r");
    } while (1);
    printf("\n begin() OK!\n");
	
	buttons = dx = dy = 0;
}
#else
MouseController::MouseController(USBHost &pusb) {
	USBHost *p = &pusb;
	p->RegisterMouseClass(this);
    
	if(__allegro_function_init == 0)
	{
		allegro_init();
		__allegro_function_init = ALLEGRO_USE_MOUSE;
	}
	
	if(msInit == false)
	{
		install_mouse();
		set_mouse_range(0, 0, 0x3FFF-1, 0x3FFF-1);
		msInit = true;
	}
	
	buttons = dx = dy = 0;
}
#endif

static unsigned char preMouseButton = 0;
static int predX = 0, predY = 0, msdx = 0, msdy = 0;
#if defined (USBHOST_REMOTE)
void MouseController::handleEvent(void) {
	int r;
    uint8_t _r[9];
    
    // handle mouse evnet:
	// 1. move mouse
	// 2. drag mouse
	// 3. press mouse button
	// 4. release mouse button
	// poll_mouse();
    
    send_command_para(1, NULL, 0); // 1 : "poll_mouse" command
    r = recv_command_result(_r, 9);
    if (r == 0) {
        //printf("%d %d %d %d %d %d %d %d %d\n", _r[0], _r[1], _r[2], _r[3], _r[4], _r[5], _r[6], _r[7], _r[8]);
        mouse_b = _r[0];
        msdx = fourBytes2Int(_r[1], _r[2], _r[3], _r[4]);
        msdy = fourBytes2Int(_r[5], _r[6], _r[7], _r[8]);
    }
	
	if ((preMouseButton & 0x01) == 0x00 && (mouse_b & 0x01) == 0x01)
		OnLeftButtonDown();

	if ((preMouseButton & 0x01) == 0x01 && (mouse_b & 0x01) == 0x00)
		OnLeftButtonUp();

	if ((preMouseButton & 0x02) == 0x00 && (mouse_b & 0x02) == 0x02)
		OnRightButtonDown();

	if ((preMouseButton & 0x02) == 0x02 && (mouse_b & 0x02) == 0x00)
		OnRightButtonUp();

	if ((preMouseButton & 0x04) == 0x00 && (mouse_b & 0x04) == 0x04)
		OnMiddleButtonDown();

	if ((preMouseButton & 0x04) == 0x04 && (mouse_b & 0x04) == 0x00)
		OnMiddleButtonUp();

    // get_mouse_mickeys(&msdx, &msdy);

	if (predX != msdx || predY != msdy)
		if(msdx != 0 && msdy != 0)
			OnMouseMove(msdx, msdy);

	preMouseButton = mouse_b;
	predX = msdx;
	predY = msdy;
}
#else
void MouseController::handleEvent(void) {
	// handle mouse evnet:
	// 1. move mouse
	// 2. drag mouse
	// 3. press mouse button
	// 4. release mouse button
	poll_mouse();
	
	if ((preMouseButton & 0x01) == 0x00 && (mouse_b & 0x01) == 0x01)
		OnLeftButtonDown();

	if ((preMouseButton & 0x01) == 0x01 && (mouse_b & 0x01) == 0x00)
		OnLeftButtonUp();

	if ((preMouseButton & 0x02) == 0x00 && (mouse_b & 0x02) == 0x02)
		OnRightButtonDown();

	if ((preMouseButton & 0x02) == 0x02 && (mouse_b & 0x02) == 0x00)
		OnRightButtonUp();

	if ((preMouseButton & 0x04) == 0x00 && (mouse_b & 0x04) == 0x04)
		OnMiddleButtonDown();

	if ((preMouseButton & 0x04) == 0x04 && (mouse_b & 0x04) == 0x00)
		OnMiddleButtonUp();

    get_mouse_mickeys(&msdx, &msdy);
    
    ex2SlaveData2 = msdx;
    ex2SlaveData3 = msdy;
    
	if (predX != msdx || predY != msdy)
		if(msdx != 0 && msdy != 0)
			OnMouseMove(msdx, msdy);

	ex2SlaveData1 = preMouseButton = mouse_b;
    //printf("%d %d %d\n", ex2SlaveData1, ex2SlaveData2, ex2SlaveData3);
	predX = msdx;
	predY = msdy;
}
#endif

extern "C" {
	void __mouseControllerEmptyCallback() { }
}

void mouseClicked()  __attribute__ ((weak, alias("__mouseControllerEmptyCallback")));
void mouseDragged()  __attribute__ ((weak, alias("__mouseControllerEmptyCallback")));
void mouseMoved()    __attribute__ ((weak, alias("__mouseControllerEmptyCallback")));
void mousePressed()  __attribute__ ((weak, alias("__mouseControllerEmptyCallback")));
void mouseReleased() __attribute__ ((weak, alias("__mouseControllerEmptyCallback")));

int MouseController::getXChange() {
	int r = dx;
	dx = 0;
	return r;
}

int MouseController::getYChange() {
	int r = dy;
	dy = 0;
	return r;
}

void MouseController::OnMouseMove(int msdx, int msdy) {
	dx += msdx;
	dy += msdy;
	if (buttons != 0)
		mouseDragged();
	else
		mouseMoved();
}

void MouseController::OnLeftButtonUp(void) {
	buttons &= ~LEFT_BUTTON;
	mouseReleased();
	mouseClicked();
}

void MouseController::OnLeftButtonDown(void) {
	buttons |= LEFT_BUTTON;
	mousePressed();
}

void MouseController::OnMiddleButtonUp(void) {
	buttons &= ~MIDDLE_BUTTON;
	mouseReleased();
	mouseClicked();
}

void MouseController::OnMiddleButtonDown(void) {
	buttons |= MIDDLE_BUTTON;
	mousePressed();
}

void MouseController::OnRightButtonUp(void) {
	buttons &= ~RIGHT_BUTTON;
	mouseReleased();
	mouseClicked();
}

void MouseController::OnRightButtonDown(void) {
	buttons |= RIGHT_BUTTON;
	mousePressed();
}