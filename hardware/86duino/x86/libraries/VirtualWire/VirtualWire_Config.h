#ifndef VirtualWire_Config_h
#define VirtualWire_Config_h
//////////////////////////////////////////////////////////////////////////
//	The following configurations are intented only if VW_PLATFORM == VW_PLATFORM_GENERIC_AVR8

//	Uncomment this to select the platform as generic AVR8
//#define VW_PLATFORM VW_PLATFORM_GENERIC_AVR8

//	OPTIONAL: Define the IO pin used for PTT (push to talk)
//	If no PTT port will be defined, the PTT feature will be disabled
//#define VW_PTT_PORT PORT#
//#define VW_PTT_DDR  DDR#
//#define VW_PTT_PIN  PIN#?

// Define the IO pin used for transmitting data
//#define VW_TX_PORT PORT#
//#define VW_TX_DDR  DDR#
//#define VW_TX_PIN  PIN#?

// Define the IO pin used for receiving data
//#define VW_RX_PORT  PIN#
//#define VW_RX_DDR  DDR#
//#define VW_RX_PIN  PIN#?

//	Define the 16 bits timer index to be used by the library (e.g. 1)
//	The default timer configuration will use TIMSKn as interrupt mask register,
//     OCRnA as compare register and TIMERn_COMPA_vect as interrupt vector
//	If one of the above doesn't suite your current platform, please redefine
//     the timer setup routine, as indicated below
//#define VW_TIMER_INDEX 1

//
//	EXAMPLE: configuration suitable for At90USB162 or ATMega32U2
//

/*
	//	Select AVR8 platform
	#define VW_PLATFORM VW_PLATFORM_GENERIC_AVR8
	
	//	My radio doesn't have PTT feature => VW_PTT_PORT/DDR/PIN will be left undefined
	//#define VW_PTT_PORT
	//#define VW_PTT_DDR
	//#define VW_PTT_PIN

	//	VirtualWire TX
	#define VW_TX_PORT PORTD
	#define VW_TX_DDR  DDRD
	#define VW_TX_PIN  PIND7

	//	VirtualWire RX
	#define VW_RX_PORT PIND
	#define VW_RX_DDR  DDRD
	#define VW_RX_PIN  PIND6

	//	Reduce message length, saves up memory
	#define VW_MAX_MESSAGE_LEN 40

	//	Select Timer 1 as the 16 bits timer used by the library
	#define VW_TIMER_INDEX 1
*/

//
//	OPTIONAL: Alternative pin setup for advanced configurations
//	Instead of defining VW_PTT_PORT/DDR/PIN, VW_TX_PORT/DDR/PIN or VW_RX_PORT/DDR/PIN
//	   the user can use the following preprocessor directives to control the way
//	   VirtualWire library reads the RX and writes to the TX or PTT
//

//#define vw_pinSetup()
//#define vw_digitalWrite_ptt(value) 
//#define vw_digitalRead_rx()
//#define vw_digitalWrite_tx(value)

//
//	EXAMPLE: Advanced pin configuration that lights up a LED when PTT is high
//           RX=PORTD7, PTT=PORTD6, LED=PORTD5, TX=PORTD4
//

/*
	//	Select AVR8 platform
	#define VW_PLATFORM VW_PLATFORM_GENERIC_AVR8

	#define vw_pinSetup()\
			DDRD |=  (1<<PORTD6)|(1<<PORTD5)|(1<<PORTD4);\
			DDRD &= ~(1<<PORTD7);

	#define vw_digitalWrite_ptt(value)\
		((value)? PORTD |= (1<<PORTD6)|(1<<PORTD5) : PORTD &= ~((1<<PORTD6)|(1<<PORTD5))
	#define vw_digitalRead_rx() \
		(PORTD & (1<<PORTD7) ? 1 : 0)
	#define vw_digitalWrite_tx(value) \
		((value) ? PORTD |= (1<<PORTD4) : PORTD &= ~(1<<PORTD4))
*/

//
//	OPTIONAL: Alternative timer configuration
//	If the default timer setup doesn't suite the platform you are using, you can
//	   rewrite the timer setup routine and define the timer vector used by the ISR
//	   implemented within the library
//

//#define vw_timerSetup(speed)
//#define VW_TIMER_VECTOR

//
//	EXAMPLE: Setting up a user defined timer configuration for VirtualWire
//			 that uses OCR1B as compare register instead of OCR1A
//

/*
	#include <stdint.h>

	//	Select AVR8 platform
	#define VW_PLATFORM VW_PLATFORM_GENERIC_AVR8

	//	Declare my own timer setup function
	static inline void my_vw_timerSetup(uint8_t speed) __attribute__ ((always_inline));

	//	VirtualWire has a special routine for detecting prescaler and the number of ticks
	//	   automatically, but needs to be declared first in order to be used
	uint8_t vw_timer_calc(uint16_t speed, uint16_t max_ticks, uint16_t *nticks);

	//	Instruct VirtualWire to use my timer setup routine and my interrupt vector
	#define vw_timerSetup(speed) my_vw_timerSetup(speed);
	#define VW_TIMER_VECTOR TIMER1_COMPB_vect

	//	Define my setup timer routine, that uses OCR1B as compare register
	static inline void my_vw_timerSetup(uint8_t speed)
	{
		// Figure out prescaler value and counter match value
		prescaler = vw_timer_calc(speed, (uint16_t)-1, &nticks);
		if (!prescaler)
		{
			return; // fault
		}

		TCCR1A = 0; // Output Compare pins disconnected
		TCCR1B = _BV(WGM12); // Turn on CTC mode

		// Convert prescaler index to TCCR1B prescaler bits CS10, CS11, CS12
		TCCR1B |= prescaler;

		// Caution: special procedures for setting 16 bit regs
		// is handled by the compiler
		OCR1B = nticks;	
		
		//	Enable interrupt
		TIMSK1 |= _BV(OCIE1B);
	}
*/

#endif /* VirtualWire_Config_h */