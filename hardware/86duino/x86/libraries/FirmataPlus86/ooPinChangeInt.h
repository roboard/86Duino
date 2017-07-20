//	This is the ooPinChangeInt library for the Arduino.
//	See google code project for latest bugs and info http://code.google.com/p/arduino-oopinchangeint/

// We use 4-character tabstops, so IN VIM:  <esc>:set ts=4 sw=4
// ...that's: ESCAPE key, colon key, then "s-e-t SPACE key t-s = 4 SPACE key s-w = 4"
//
//-------- define these in your sketch, if applicable ----------------------------------------------------------
//-------- This must go ahead of the #include ooPinChangeInt.h statement in your sketch ------------------------
// You can reduce the memory footprint of this handler by declaring that there will be no pin change interrupts
// on any one or two of the three ports.  If only a single port remains, the handler will be declared inline
// reducing the size and latency of the handler.
// #define NO_PORTB_PINCHANGES // to indicate that port b will not be used for pin change interrupts
// #define NO_PORTC_PINCHANGES // to indicate that port c will not be used for pin change interrupts
// #define NO_PORTD_PINCHANGES // to indicate that port d will not be used for pin change interrupts
// You can reduce the code size by maybe 20 bytes, and you can speed up the interrupt routine
// slightly by declaring that you don't care if the static variable PCintPort::pinState
// is made available to your interrupt routine.
// #define NO_PIN_STATE        // to indicate that you don't need the pinState
//
// define DISABLE_PCINT_MULTI_SERVICE below to limit the handler to servicing a single interrupt per invocation.
// #define       DISABLE_PCINT_MULTI_SERVICE
// #define GET_OOPCIVERSION   // to enable the uint16_t getOOPCIintVersion () function.
//-------- define the above in your sketch, if applicable ------------------------------------------------------

/*
	ooPinChangeInt.h
	---- VERSIONS ----------------------------------------------------------------------------
	Library begins with the PinChangeInt v 1.3 code.  See http://code.google.com/p/arduino-pinchangeint/

	Version 1.03beta Wed Nov 21 18:20:46 CST 2012

	Version 1.00 Sat Dec  3 22:56:20 CST 2011
	Modified to use the new() operator and symbolic links instead of creating a pre-populated
	array of pointers to the pins.  This consumes more flash, but makes possible some
	additional C++ style functionality later.

	Version 1.01 Thu Dec  8 21:29:11 CST 2011
	Modified to use a C++ callback function.  The arduinoPin variable is no longer necessary,
	as this creates a new methodology for using the library.

	Version 1.02 Tue Mon Mar  5 18:37:28 CST 2012
	All code moved into this .h file so as to make it possible to recognize #define's in the
	user's sketch.

	Added #ifdef LIBCALL_OOPINCHANGEINT.  Programmers using this library in another library
	should define this macro, because this will allow you to #include it in your sketch AND
	#include it in the library.
    (As a matter of act, you must always #include this file in your sketch, even if it's only
	used to support another library.  See the Tigger library and example, for an example.)

	Code uses the cbiface library, which is a much simplified and renamed version of cb.h
	---- VERSIONS ----------------------------------------------------------------------------
	This is the ooPinChangeInt library for the Arduino.
	See google code project for latest, bugs and info http://code.google.com/p/arduino-oopinchangeint/

	This library provides an extension to the interrupt support for arduino by adding pin change
	interrupts, giving a way for users to have interrupts drive off of any pin (ATmega328-based
	Arduinos) and by the Port B, J, and K pins on the Arduino Mega and its ilk..

	This library was originally written by Chris J. Klick, Robot builder and all around geek, who said of it,
		"HI, Yeah, I wrote the original PCint library. It was a bit of a hack and the new one has better
		features.  I intended the code to be freely usable.  Didn't really think about a license.  Feel
		free to use it in your code: I hereby grant you permission."
	Thanks, Chris! A hack? I dare say not, if I have taken this any further it's merely by standing on the
	shoulders of giants. This library was the best "tutorial" I found on Arduino Pin Change Interrupts
	and because of that I decided to continue to maintain and (hopefully) improve it. We, the Arduino
	community of robot builders and geeks, owe you a great debt of gratitude for your hack- a hack in
	the finest sense.

	The library was then picked up by Lex Talionis, who created the Google Code website. We all owe a debt
	of thanks to Lex, too, for all his hard work! He is currently the other official maintainer of this
	code.

	Chris' original PCInt Arduino Playground example here: http://www.arduino.cc/playground/Main/PcInt

	Many thanks to all the contributors who have contributed bug fixes, code, and suggestions
	to this project: 
	John Boiles and Baziki (who added fixes to PcInt), Maurice Beelen, nms277, Akesson Karlpetter, and
	Orly Andico for various fixes to this code, Rob Tillaart for some excellent code reviews and nice
	optimizations, Andre' Franken for a good bug report that kept me thinking, cserveny.tamas a special
	shout out for providing the MEGA code to PinChangeInt- Thanks!

	Regarding the MEGA and friends, Cserveny says: "J is mostly useless, because of the hardware UART. I was
	not able to get pin change notifications from the TX pin (14), so only 15 left. All other pins are not
	connected on the arduino boards."
	
	LICENSE

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

	*/

#ifndef ooPinChangeInt_h
#define	ooPinChangeInt_h

#define OOPCIVERSION 1030

#include <Arduino.h>
#include <irq.h>
#include "cbiface.h"

class PCintPort {
public:
	PCintPort() {}
	// cbIface should be an object instantiated from a subclass of CallBackInterface
	static		int8_t attachInterrupt(uint8_t pin, CallBackInterface* cbIface, int mode);
	static		void detachInterrupt(uint8_t pin);

protected:
	class PCintPin {
	public:
		PCintPin() {}
		CallBackInterface* pinCallBack;
	};
	void		enable(uint8_t pin, CallBackInterface* cbIface, uint8_t mode);
	int8_t      addPin(uint8_t arduinoPin, CallBackInterface* cbIface, uint8_t mode);
};
#endif // End of ooPinChangeInt_h


// **********************************************************************************************************

#ifndef LIBCALL_OOPINCHANGEINT

#if defined (__86DUINO_ZERO) || defined (__86DUINO_ONE) || defined (__86DUINO_EDUCAKE) || defined (__86DUINO_AI)
	CallBackInterface* 	pinCallBackArea[12];
#elif defined (__86DUINO_PLC)
	CallBackInterface* 	pinCallBackArea[4];
#endif
PCintPort _port86int; // only one port, and this port support 12 interrupt pins


#define MCPFAU_CAP_LEVEL0    (0x08L << 16)
#define MCPFAU_CAP_LEVEL1    (0x09L << 16)

static int mc = 0, md = 1;
static int mcint_offset[2] = {0, 24};
static void clear_INTSTATUS(void) {
    mc_outp(mc, 0x04, 0xffL << mcint_offset[md]); //for EX
}

static void disable_MCINT(void) {
    mc_outp(mc, 0x00, mc_inp(mc, 0x00) & ~(0xffL << mcint_offset[md]));  // disable mc interrupt
    mc_outp(MC_GENERAL, 0x38, mc_inp(MC_GENERAL, 0x38) | (1L << mc));
}

static void enable_MCINT(unsigned long used_int) {
	mc_outp(MC_GENERAL, 0x38, mc_inp(MC_GENERAL, 0x38) & ~(1L << mc));
	mc_outp(mc, 0x00, (mc_inp(mc, 0x00) & ~(0xffL<<mcint_offset[md])) | (used_int << mcint_offset[md]));
}

static void (*sifIntMode[3])(int, int, unsigned long) = {mcpfau_SetCapMode1, mcpfau_SetCapMode2, mcpfau_SetCapMode3};
static void (*sifSetPol[3])(int, int, unsigned long) = {mcpfau_SetPolarity1, mcpfau_SetPolarity2, mcpfau_SetPolarity3};
static void (*sifSetRelease[3])(int, int, unsigned long) = {mcpfau_SetFAU1RELS, mcpfau_SetFAU2RELS, mcpfau_SetFAU3RELS};
static void (*sifClearStat[3])(int, int) = {mcpfau_ClearFAU1STAT, mcpfau_ClearFAU2STAT, mcpfau_ClearFAU3STAT};
static void (*sifSetMask[3])(int, int, unsigned long) = {mcpfau_SetMask1, mcpfau_SetMask2, mcpfau_SetMask3};
static unsigned long (*readCapStat[3])(int, int) = {mcpfau_ReadCAPSTAT1, mcpfau_ReadCAPSTAT2, mcpfau_ReadCAPSTAT3};
static unsigned long (*readCapFIFO[3])(int, int, unsigned long*) = {mcpfau_ReadCAPFIFO1, mcpfau_ReadCAPFIFO2, mcpfau_ReadCAPFIFO3};
static volatile bool mcm_init[4] = {false, false, false, false};
static unsigned long _usedMode[4][3];
static int user_int(int irq, void* data) {
	int i, m, n, evt = 0;
	unsigned long capdata;
	
	// detect interrupt pin
	for(i=0; i<EXTERNAL_NUM_INTERRUPTS; i++)
	{
		m = PIN86[INTPINSMAP[i]].ENCMC;
		n = i%3; // offset (capture pin number 1/2/3)
		if(mcm_init[m] == false) continue;
		if((mc_inp(m, 0x04) & ((0x20000000L)<<n)) != 0L) // detect input edge-trigger, !! only for MODULEB
		{
			mc_outp(m, 0x04, (0x20000000L)<<n);
			break;
		}

		if((mc_inp(m, 0x04) & ((0x04000000L)<<n)) != 0L) // detect input level-trigger, !! only for MODULEB
		{
			mc_outp(m, 0x04, (0x04000000L)<<n);
			break;
		}
	}

	// execute user function for the pin
	if(i < EXTERNAL_NUM_INTERRUPTS)
	{
		switch(_usedMode[m][n])
		{
		case MCPFAU_CAP_LEVEL0: case MCPFAU_CAP_LEVEL1:
			sifSetMask[n](m, MCSIF_MODULEB, MCPFAU_MASK_INACTIVE);
			sifClearStat[n](m, MCSIF_MODULEB);
			evt++;
			break;
		case MCPFAU_CAP_BOTH:
	    	while(readCapStat[n](m, MCSIF_MODULEB)!= MCENC_CAPFIFO_EMPTY)
          		if(readCapFIFO[n](m, MCSIF_MODULEB, &capdata) != MCPFAU_CAP_CAPCNT_OVERFLOW) evt++;
			break;
		case MCPFAU_CAP_1TO0:
			while(readCapStat[n](m, MCSIF_MODULEB)!= MCENC_CAPFIFO_EMPTY)
          		if(readCapFIFO[n](m, MCSIF_MODULEB, &capdata) == MCPFAU_CAP_1TO0EDGE) evt++;
          	break;
		case MCPFAU_CAP_0TO1:
			while(readCapStat[n](m, MCSIF_MODULEB)!= MCENC_CAPFIFO_EMPTY)
          		if(readCapFIFO[n](m, MCSIF_MODULEB, &capdata) == MCPFAU_CAP_0TO1EDGE) evt++;
          	break;
		}

		// do user's function
		for(; evt > 0; evt--)
			(*(pinCallBackArea[i])).cbmethod();

		// if select level-trigger, switch the MASK to "NONE" after user's function is complete.
		switch(_usedMode[m][n])
		{
		case MCPFAU_CAP_LEVEL0: case MCPFAU_CAP_LEVEL1:
			sifSetMask[n](m, MCSIF_MODULEB, MCPFAU_MASK_NONE);
			break;
		default: break;
		}
	}
	if(i == EXTERNAL_NUM_INTERRUPTS) return ISR_NONE;
	return ISR_HANDLED;
}

static uint8_t used_irq = 0xff;
static const char* name = "attachInt";
static bool interrupt_init(void) {
	if(used_irq != 0xff) return true;

    if(irq_InstallISR(GetMCIRQ(), user_int, (void*)name) == false)
	{
	    printf("irq_install fail\n"); return false;
	}

	used_irq = GetMCIRQ();
	return true;
}


static void mcmsif_init(void) {
    if(mcm_init[mc] == true) return;

    mcsif_SetInputFilter(mc, md, 20L);
    mcsif_SetSWDeadband(mc, md, 0L);
    mcsif_SetSWPOL(mc, md, MCSIF_SWPOL_REMAIN);
    mcsif_SetSamplWin(mc, md, MCSIF_SWSTART_DISABLE + MCSIF_SWEND_NOW);  // stop the current sampling window first
    mcsif_SetSamplWin(mc, md, MCSIF_SWSTART_NOW + MCSIF_SWEND_DISABLE);

	mcsif_SetMode(mc, md, MCSIF_PFAU);               // enable MC2 SIFA

    mcpfau_SetCapMode1(mc, md, MCPFAU_CAP_DISABLE);
    mcpfau_SetCapInterval1(mc, md, 1L);              // pin1 for FAUTRIG
    mcpfau_SetCap1INT(mc, md, 1L);
    mcpfau_SetPolarity1(mc, md, MCPFAU_POL_NORMAL);
    mcpfau_SetMask1(mc, md, MCPFAU_MASK_NONE);
    mcpfau_SetRLDTRIG1(mc, md, MCPFAU_RLDTRIG_DISABLE);
    mcpfau_SetFAU1TRIG(mc, md, MCPFAU_FAUTRIG_INPUT1);
    mcpfau_SetFAU1RELS(mc, md, MCPFAU_FAURELS_INPUT0);

    mcpfau_SetCapMode2(mc, md, MCPFAU_CAP_DISABLE);     // pin2 for RLDTRIG
    mcpfau_SetCapInterval2(mc, md, 1L);
    mcpfau_SetCap2INT(mc, md, 1L);
    mcpfau_SetPolarity2(mc, md, MCPFAU_POL_NORMAL);
    mcpfau_SetMask2(mc, md, MCPFAU_MASK_NONE);
    mcpfau_SetRLDTRIG2(mc, md, MCPFAU_RLDTRIG_DISABLE);
    mcpfau_SetFAU2TRIG(mc, md, MCPFAU_FAUTRIG_INPUT1);
    mcpfau_SetFAU2RELS(mc, md, MCPFAU_FAURELS_INPUT0);

    mcpfau_SetCapMode3(mc, md, MCPFAU_CAP_DISABLE);
    mcpfau_SetCapInterval3(mc, md, 1L);              // pin3 for FAUTRIG
    mcpfau_SetCap3INT(mc, md, 1L);
    mcpfau_SetPolarity3(mc, md, MCPFAU_POL_NORMAL);
    mcpfau_SetMask3(mc, md, MCPFAU_MASK_NONE);
    mcpfau_SetRLDTRIG3(mc, md, MCPFAU_RLDTRIG_DISABLE);
	mcpfau_SetFAU3TRIG(mc, md, MCPFAU_FAUTRIG_INPUT1);
	mcpfau_SetFAU3RELS(mc, md, MCPFAU_FAURELS_INPUT0);

	io_DisableINT();
	mcm_init[mc] = true;
	io_RestoreINT();
}

static void mcmsif_close(void) {
	mcsif_Disable(mc, md);
	io_DisableINT();
	mcm_init[mc] = false;
	io_RestoreINT();
}

void PCintPort::enable(uint8_t interruptNum, CallBackInterface* cbIface, uint8_t mode) {
    unsigned short crossbar_ioaddr;
	
	if(interruptNum >= EXTERNAL_NUM_INTERRUPTS)
	{
		printf("This interrupt%d has no one pin to use\n", interruptNum);
		return;
	}
	
	mc = PIN86[INTPINSMAP[interruptNum]].ENCMC;
    md = PIN86[INTPINSMAP[interruptNum]].ENCMD;

    if(pinCallBackArea[interruptNum] != NULL) return;
	if(interrupt_init() == false) return;
	mcmsif_init();

    clear_INTSTATUS();
	enable_MCINT(0xfc); // SIFB FAULT INT3/2/1 + STAT3/2/1 = 6 bits
	crossbar_ioaddr = sb_Read16(0x64)&0xfffe;
	
	mcsif_Disable(mc, md);

	io_DisableINT();
	pinCallBackArea[interruptNum] = cbIface;
	io_RestoreINT();

	switch (mode)
	{
	case LOW:
		sifSetPol[interruptNum%3](mc, md, MCPFAU_POL_INVERSE);
		sifSetMask[interruptNum%3](mc, md, MCPFAU_MASK_INACTIVE);
		sifSetRelease[interruptNum%3](mc, md, MCPFAU_FAURELS_FSTAT0);
		sifClearStat[interruptNum%3](mc, md);
		_usedMode[mc][interruptNum%3] = MCPFAU_CAP_LEVEL0;
		clear_INTSTATUS();
		break;
	case HIGH:
		sifSetMask[interruptNum%3](mc, md, MCPFAU_MASK_INACTIVE);
		sifSetRelease[interruptNum%3](mc, md, MCPFAU_FAURELS_FSTAT0);
		sifClearStat[interruptNum%3](mc, md);
		_usedMode[mc][interruptNum%3] = MCPFAU_CAP_LEVEL1;
		clear_INTSTATUS();
		break;
	case CHANGE:
		sifIntMode[interruptNum%3](mc, md, MCPFAU_CAP_BOTH);
		_usedMode[mc][interruptNum%3] = MCPFAU_CAP_BOTH;
		break;
	case FALLING:
		sifIntMode[interruptNum%3](mc, md, MCPFAU_CAP_1TO0);
		_usedMode[mc][interruptNum%3] = MCPFAU_CAP_1TO0;
		break;
	case RISING:
	    sifIntMode[interruptNum%3](mc, md, MCPFAU_CAP_0TO1);
	    _usedMode[mc][interruptNum%3] = MCPFAU_CAP_0TO1;
	    break;
	default:
		printf("No support this mode\n");
		return;
	}

	// switch crossbar to MCM_SIF_PIN
	io_outpb(crossbar_ioaddr + 0x90 + PIN86[INTPINSMAP[interruptNum]].gpN, 0x08);//RICH IO
	mcsif_Enable(mc, md);

	// If select level-trigger, switch the MASK to "NONE" after sif is enabled.
	switch (mode)
	{
	case LOW: case HIGH:
		sifSetMask[interruptNum%3](mc, md, MCPFAU_MASK_NONE);
		break;
	default: break;
	}
}

int8_t PCintPort::addPin(uint8_t arduinoPin, CallBackInterface* cbIface, uint8_t mode)
{
	enable(arduinoPin, cbIface, mode);
	return 0;
}

int8_t PCintPort::attachInterrupt(uint8_t arduinoPin, CallBackInterface* cbIface, int mode)
{
	return _port86int.addPin(arduinoPin, cbIface, mode);
}

void PCintPort::detachInterrupt(uint8_t interruptNum)
{
	int i;
	mc = PIN86[INTPINSMAP[interruptNum]].ENCMC;

	if(interruptNum >= EXTERNAL_NUM_INTERRUPTS) return;
	if(PIN86[INTPINSMAP[interruptNum]].userfunc == NULL) return;

	mcsif_Disable(mc, md);
	sifIntMode[interruptNum%3](mc, md, MCPFAU_CAP_DISABLE);

	io_DisableINT();
	PIN86[INTPINSMAP[interruptNum]].userfunc = NULL;
	io_RestoreINT();

	for(i=0; i<3; i++)
		if(PIN86[INTPINSMAP[mc*3+i]].userfunc != NULL) break;
	if(i == 3) mcmsif_close(); else mcsif_Enable(mc, md);

	for(i=0; i<EXTERNAL_NUM_INTERRUPTS; i++)
		if(PIN86[INTPINSMAP[i]].userfunc != NULL) break;
	if(i == EXTERNAL_NUM_INTERRUPTS)
	{
		if(irq_UninstallISR(used_irq, (void*)name) == false)
		    printf("irq_install fail\n");
		else
			used_irq = 0xff;
	}
}

#ifdef GET_OOPCIVERSION
uint16_t getOOPCIntVersion () {
	return ((uint16_t) OOPCIVERSION);
}
#endif // GET_OOPCIVERSION
#endif // LIBCALL_OOPINCHANGEINT

