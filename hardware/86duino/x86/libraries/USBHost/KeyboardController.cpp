/*
  kbctrl.cpp - Part of DM&P Vortex86 USB Host library
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
#include "KeyboardController.h"
#include "queue.h"

#define KBQUEUESIZE    (4096)

unsigned char scancodeMap[128] =
{
	0x00, // Reserved
	0x04, // Keyboard a and A
	0x05, // Keyboard b and B
	0x06, // Keyboard c and C
	0x07, // Keyboard d and D
	0x08, // Keyboard e and E
	0x09, // Keyboard f and F
	0x0A, // Keyboard g and G
	0x0B, // Keyboard h and H
	0x0C, // Keyboard i and I
	0x0D, // Keyboard j and J
	0x0E, // Keyboard k and K
	0x0F, // Keyboard l and L
	0x10, // Keyboard m and M
	0x11, // Keyboard n and N
	0x12, // Keyboard o and O
	0x13, // Keyboard p and P
	0x14, // Keyboard q and Q
	0x15, // Keyboard r and R
	0x16, // Keyboard s and S
	0x17, // Keyboard t and T
	0x18, // Keyboard u and U
	0x19, // Keyboard v and V
	0x1A, // Keyboard w and W
	0x1B, // Keyboard x and X
	0x1C, // Keyboard y and Y
	0x1D, // Keyboard z and Z
	
	0x27, // Keyboard 0 and )
	0x1E, // Keyboard 1 and !
	0x1F, // Keyboard 2 and @
	0x20, // Keyboard 3 and #
	0x21, // Keyboard 4 and $
	0x22, // Keyboard 5 and %
	0x23, // Keyboard 6 and ^
	0x24, // Keyboard 7 and &
	0x25, // Keyboard 8 and *
	0x26, // Keyboard 9 and (
	
	0x62, // Keypad 0 and Insert
	0x59, // Keypad 1 and End
	0x5A, // Keypad 2 and Down Arrow
	0x5B, // Keypad 3 and PageDn
	0x5C, // Keypad 4 and Left Arrow
	0x5D, // Keypad 5
	0x5E, // Keypad 6 and Right Arrow
	0x5F, // Keypad 7 and Home
	0x60, // Keypad 8 and Up Arrow
	0x61, // Keypad 9 and PageUp
	
	0x3A, // Keyboard F1
	0x3B, // Keyboard F2
	0x3C, // Keyboard F3
	0x3D, // Keyboard F4
	0x3E, // Keyboard F5
	0x3F, // Keyboard F6
	0x40, // Keyboard F7
	0x41, // Keyboard F8
	0x42, // Keyboard F9
	0x43, // Keyboard F10
	0x44, // Keyboard F11
	0x45, // Keyboard F12
	
	0x29, // Keyboard ESC
	0x35, // Keyboard Grave Accent and Tilde
	0x2D, // Keyboard - and (underscore)
	0x2E, // Keyboard = and +
	0x2A, // Keyboard DELETE (Backspace)
	0x2B, // Keyboard Tab
	0x2F, // Keyboard [ and {
	0x30, // Keyboard ] and }
	0x28, // Keyboard Return (ENTER)
	0x33, // Keyboard ; and :
	0x34, // Keyboard ' and "
	0x31, // Keyboard \ and |
	0x00, // No map 71
	0x36, // Keyboard , and <
	0x37, // Keyboard . and >
	0x38, // Keyboard / and ?
	0x2C, // Keyboard Spacebar
	
	0x49, // Keyboard Insert
	0x4C, // Keyboard Delete Forward
	0x4A, // Keyboard Home
	0x4D, // Keyboard End
	0x4B, // Keyboard PageUp
	0x4E, // Keyboard PageDown
	0x50, // Keyboard LeftArrow
	0x4F, // Keyboard RightArrow
	0x52, // Keyboard UpArrow
	0x51, // Keyboard DownArrow
	
	0x54, // Keypad /
	0x55, // Keypad *
	0x56, // Keypad -
	0x57, // Keypad +
	0x63, // Keypad . and Delete
	0x58, // Keypad ENTER
	
	0x46, // Keyboard PrintScreen
	0x48, // Keyboard Pause
	
	0x00, // No map 94
	0x00, // No map 95
	0x00, // No map 96
	0x00, // No map 97
	0x00, // No map 98
	0x00, // No map 99
	0x00, // No map 100
	0x00, // No map 101
	0x00, // No map 102
	0x00, // No map 103
	0x00, // No map 104
	0x00, // No map 105
	0x00, // No map 106
	0x00, // No map 107
	0x00, // No map 108
	0x00, // No map 109
	0x00, // No map 110
	0x00, // No map 111
	0x00, // No map 112
	0x00, // No map 113
	0x00, // No map 114
	
	0x00, // No map 115
	0x00, // No map 116
	0x00, // No map 117
	0x00, // No map 118
	0x00, // No map 119
	0x00, // No map 120
	0x00, // No map 121
	0x00, // No map 122
	0x65, // Keyboard Application (KEY_MENU)
	0x47, // Keyboard Scroll Lock
	0x53, // Keypad Num Lock and Clear
	0x39, // Keyboard Caps Lock
	0x00  // No map 127
};

static bool kbInit = false;
volatile Queue *kbQueue = NULL;
void keypress_watcher(int scancode) {
	PushQueue(kbQueue, scancode); // In Arduino, it has 10ms scan-time.
} END_OF_FUNCTION(keypress_watcher)

KeyboardController::KeyboardController(USBHost &pusb) {
	USBHost *p = &pusb;
	p->RegisterKeyboardClass(this);
	if(__allegro_function_init == 0)
	{
		allegro_init();
		__allegro_function_init = ALLEGRO_USE_KEYBOARD;
	}
	
	if(kbInit == false)
	{
		install_timer();
		install_keyboard();
		kbInit = true;
	}
	
	kbQueue = CreateQueue(KBQUEUESIZE);
	//if(kbQueue == NULL) return;
	LOCK_VARIABLE(kbQueue);
	keyboard_lowlevel_callback = keypress_watcher;
	set_keyboard_rate(0, 0); // close scan rate
}


unsigned char CapsLock = 0;
unsigned char NumLock = 0;
unsigned char ScrLock = 0;
void KeyboardController::handleLockingKeys(int scode) {
	// Scroll Lock keyLED
    if(scode == KEY_SCRLOCK)
    {
    	ScrLock = ScrLock ? 0 : 1;
    	if(ScrLock != 0)
			set_leds(KB_SCROLOCK_FLAG); // available when the BIOS support it.
		else
  			set_leds(-1);
    }
    // Number Lock keyLED
    else if(scode == KEY_NUMLOCK)
    {
		NumLock = NumLock ? 0 : 1;
    	if(NumLock != 0)
  			set_leds(KB_NUMLOCK_FLAG); // available when the BIOS support it.
  		else
  			set_leds(-1);
	}
	// Caps Lock keyLED
	else if(scode == KEY_CAPSLOCK)
	{
		CapsLock = CapsLock ? 0 : 1;
    	if(CapsLock != 0)
  			set_leds(KB_CAPSLOCK_FLAG); // available when the BIOS support it.
  		else
  			set_leds(-1);
	}
}

unsigned char numKeys[] = { '!', '@', '#', '$', '%', '^', '&', '*', '(', ')' };
unsigned char symKeysUp[] = { '_', '+', '{', '}', '|', '~', ':', '"', '~', '<', '>', '?' };
unsigned char symKeysLo[] = { '-', '=', '[', ']', '\\', ' ', ';', '\'', '`', ',', '.', '/' };
unsigned char padKeys[] = { '/', '*', '-', '+', 0x13 };
uint8_t KeyboardController::OemToAscii(uint8_t mod, uint8_t key)
{
	uint8_t shift = (mod & 0x22);

	// [a-z]
	if (key > 0x03 && key < 0x1e)
	{
		// Upper case letters
		if ( (CapsLock == 0 && (mod & 2)) ||
			 (CapsLock == 1 && (mod & 2) == 0) )
			return (key - 4 + 'A');

		// Lower case letters
		else
			return (key - 4 + 'a');
	}
	// Numbers
	else if (key > 0x1d && key < 0x27)
	{
		if (shift)
			return (numKeys[key - 0x1e]);
		else
			return (key - 0x1e + '1');
	}
	// Keypad Numbers
	else if (key > 0x58 && key < 0x62)
	{
		if (NumLock == 1)
			return (key - 0x59 + '1');
	}
	else if (key > 0x2c && key < 0x39)
		return ((shift) ? symKeysUp[key-0x2d] : symKeysLo[key-0x2d]);
	else if (key > 0x53 && key < 0x59)
		return padKeys[key - 0x54];
	else
	{
		switch (key)
		{
			case 0x2C:		return (0x20);                     // KEY_SPACE
			case 0x28:		return (0x13);                     // KEY_ENTER
			case 0x27:		return ((shift) ? ')' : '0');      // KEY_ZERO
			case 0x62:		return ((NumLock == 1) ? '0' : 0); // KEY_ZERO2
			case 0x63:		return ((NumLock == 1) ? '.' : 0); // KEY_PERIOD
		}
	}
	return 0;
}

void KeyboardController::KeyboardEvent(bool press, int scode) {
	uint8_t mod = 0, oemkey = 0;
	unsigned char asciicode;

	if (key[KEY_LCONTROL]) mod |= 0x01;
    if (key[KEY_LSHIFT])   mod |= 0x02;
    if (key[KEY_ALT])      mod |= 0x04;
    if (key[KEY_LWIN])     mod |= 0x08;
    if (key[KEY_RCONTROL]) mod |= 0x10;
    if (key[KEY_RSHIFT])   mod |= 0x20;
    if (key[KEY_ALTGR])    mod |= 0x40;
    if (key[KEY_RWIN])     mod |= 0x80;

    oemkey = scancodeMap[scode];
    
    if(oemkey != 0x00) // pause is a special key, it occur one interuupt on pressing but not on releasing.
    {
	    if(press)
	    {
	    	handleLockingKeys(scode);
	    	OnKeyDown(mod, oemkey);
	    }
	    else
	    {
	    	OnKeyUp(mod, oemkey);
	    }
	}
}

void KeyboardController::handleEvent(void) {
	int scode = 0, pressFlag = 0;
    
    // handle keyboard event:
	// 1. press key
	// 2. release key
	if(kbQueue != NULL && QueueEmpty(kbQueue) == false)
	{
		scode = PopQueue(kbQueue);
		pressFlag = scode & 0x80;
		scode &= 0x7f;
		if (pressFlag)
			KeyboardEvent(false, scode);
		else
			KeyboardEvent(true, scode);
	}
}


extern "C" {
	void __keyboardControllerEmptyCallback() { }
}

void keyPressed()  __attribute__ ((weak, alias("__keyboardControllerEmptyCallback")));
void keyReleased() __attribute__ ((weak, alias("__keyboardControllerEmptyCallback")));

void KeyboardController::OnKeyDown(uint8_t _mod, uint8_t _oemKey) {
	modifiers = _mod;
	keyOem = _oemKey;
	keyascii = OemToAscii(_mod, _oemKey);
	keyPressed();
}

void KeyboardController::OnKeyUp(uint8_t _mod, uint8_t _oemKey) {
	modifiers = _mod;
	keyOem = _oemKey;
	keyascii = OemToAscii(_mod, _oemKey);
	keyReleased();
}