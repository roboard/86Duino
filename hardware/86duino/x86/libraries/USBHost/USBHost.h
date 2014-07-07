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

#ifndef USBHOST_H_INCLUDED
#define USBHOST_H_INCLUDED

#include "Arduino.h"

#define ALLEGRO_USE_KEYBOARD    (0x01)
#define ALLEGRO_USE_MOUSE       (0x02)
extern int __allegro_function_init;

class USBDeviceConfig
{
	public:
		virtual void handleEvent() =0;
};

class USBHost
{
	USBDeviceConfig* DevConfig[2];
	
	public:
		USBHost();
		void RegisterKeyboardClass(USBDeviceConfig *pusb);
		void RegisterMouseClass(USBDeviceConfig *pusb);
		void Task();
};

// Keyboard
enum KeyboardModifiers {
	LeftCtrl = 1,
	LeftShift = 2,
	Alt = 4,
	LeftCmd = 8,
	RightCtrl = 16,
	RightShift = 32,
	AltGr = 64,
	RightCmd = 128
};

class KeyboardController : public USBHost, public USBDeviceConfig
{
	public:
		KeyboardController(USBHost &pusb);
		uint8_t getKey()       { return keyascii; };
		uint8_t getModifiers() { return modifiers; };
		uint8_t getOemKey()    { return keyOem; };
		void handleEvent(void);
		
	protected:
		void OnKeyDown(uint8_t mod, uint8_t key);
		void OnKeyUp(uint8_t mod, uint8_t key);
		
	private:
		uint8_t keyascii, keyOem, modifiers;
		void handleLockingKeys(int scode);
		void KeyboardEvent(bool press, int scode);
		uint8_t OemToAscii(uint8_t mod, uint8_t key);
};

// Mouse
enum MouseButton {
	LEFT_BUTTON   = 0x01,
	MIDDLE_BUTTON = 0x02,
	RIGHT_BUTTON  = 0x04
};

class MouseController : public USBHost, public USBDeviceConfig
{
	public:
		MouseController(USBHost &usb);
		bool getButton(MouseButton button) { return (buttons & button) == button; };
		int getXChange();
		int getYChange();
		void handleEvent(void);

	protected:
		void OnMouseMove(int msdx, int msdy);
		void OnLeftButtonUp(void);
		void OnLeftButtonDown(void);
		void OnMiddleButtonUp(void);
		void OnMiddleButtonDown(void);
		void OnRightButtonUp(void);
		void OnRightButtonDown(void);

	private:
		int dx, dy;
		int buttons;
};

#endif
