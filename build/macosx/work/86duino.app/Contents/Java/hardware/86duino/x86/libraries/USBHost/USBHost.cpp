/*
  USBHost.cpp - DM&P Vortex86 USB Host library
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

#include "USBHost.h"

USBHost::USBHost() {
	
}

void USBHost::RegisterKeyboardClass(USBDeviceConfig *pusb) {
    DevConfig[0] = pusb;
}

void USBHost::RegisterMouseClass(USBDeviceConfig *pusb) {
    DevConfig[1] = pusb;
}

static bool kbInit = false;
static bool msInit = false;
static unsigned char preMouseButton = 0;
static int predX = 0, predY = 0, dX = 0, dY = 0; 
void USBHost::Task(void) {
	for(int i=0; i<2; i++)
		if(DevConfig[i])
			DevConfig[i]->handleEvent();	
}
