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
#include <allegro.h>
#include "MouseController.h"
int __allegro_function_init = 0;
static bool msInit = false;
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
	/*
	if(install_mouse() != 0)
	{
		printf("Init mouse fail.\n");
		return;
	}*/
	
	buttons = dx = dy = 0;
}

static unsigned char preMouseButton = 0;
static int predX = 0, predY = 0, msdx = 0, msdy = 0;
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

	if (predX != msdx || predY != msdy)
		if(msdx != 0 && msdy != 0)
			OnMouseMove(msdx, msdy);

	preMouseButton = mouse_b;
	predX = msdx;
	predY = msdy;
}

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