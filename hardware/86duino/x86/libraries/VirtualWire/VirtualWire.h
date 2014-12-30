// VirtualWire.h
//
// Virtual Wire implementation for Arduino and other boards
// See the README file in this directory for documentation
// 
// Author: Mike McCauley (mikem@airspayce.com) DO NOT CONTACT THE AUTHOR DIRECTLY: USE THE LISTS
// Copyright (C) 2008 Mike McCauley
// Modified 01 November 2014 by Android Lin for 86Duino
// $Id: VirtualWire.h,v 1.14 2014/03/26 01:09:36 mikem Exp mikem $

/// \mainpage VirtualWire library for Arduino and other boards
///
/// This is the VirtualWire library.
///
/// \par END OF LIFE NOTICE
///
/// This VirtualWire library has now been superceded by the RadioHead 
/// library http://www.airspayce.com/mikem/arduino/RadioHead
/// RadioHead and its RH_ASK driver provides all the features supported by VirtualWire, and much more
/// besides, including Reliable Datagrams, Addressing, Routing and Meshes. All the platforms that
/// VirtualWire supported are also supported by RadioHead.
///
/// This library will no longer be maintained or updated, but we will continue to publish
/// it for the benefit of the the community. Nevertheless we recommend upgrading to RadioHead where
/// possible.
///
/// VirtualWire is a library for Arduino, Maple and others that provides features to send short
/// messages, without addressing, retransmit or acknowledgment, a bit like UDP
/// over wireless, using ASK (amplitude shift keying). Supports a number of
/// inexpensive radio transmitters and receivers. All that is required is
/// transmit data, receive data and (for transmitters, optionally) a PTT
/// transmitter enable. Can also be used over various analog connections (not just a data radio), 
/// such as the audio channel of an A/V sender
///
/// It is intended to be compatible with the RF Monolithics (www.rfm.com)
/// Virtual Wire protocol, but this has not been tested.
///
/// Does not use the Arduino UART. Messages are sent with a training preamble,
/// message length and checksum. Messages are sent with 4-to-6 bit encoding
/// for good DC balance, and a CRC checksum for message integrity.
///
/// But why not just use a UART connected directly to the
/// transmitter/receiver? As discussed in the RFM documentation, ASK receivers
/// require a burst of training pulses to synchronize the transmitter and
/// receiver, and also requires good balance between 0s and 1s in the message
/// stream in order to maintain the DC balance of the message. UARTs do not
/// provide these. They work a bit with ASK wireless, but not as well as this
/// code.
///
/// This library provides classes for 
/// - VirtualWire: unaddressed, unreliable messages
///
/// Example Arduino programs are included to show the main modes of use.
///
/// The version of the package that this documentation refers to can be downloaded 
/// from http://www.airspayce.com/mikem/arduino/VirtualWire/VirtualWire-1.27.zip
/// You can find the latest version at http://www.airspayce.com/mikem/arduino/VirtualWire
///
/// You can also find online help and disussion at 
/// http://groups.google.com/group/virtualwire
/// Please use that group for all questions and discussions on this topic. 
/// Do not contact the author directly, unless it is to discuss commercial licensing.
/// Before asking a question or reporting a bug, please read http://www.catb.org/esr/faqs/smart-questions.html
///
/// \par Theory of operation
///
/// See ASH Transceiver Software Designer's Guide of 2002.08.07
///   http://www.rfm.com/products/apnotes/tr_swg05.pdf
///
/// \par Supported Hardware
///
/// A range of communications
/// hardware is supported. The ones listed below are available in common retail
/// outlets in Australia and other countries for under $10 per unit. Many
/// other modules may also work with this software. 
///
/// Runs on a wide range of Arduino processors using Arduino IDE 1.0 or later.
/// Also runs on on Energia
/// with MSP430G2553 / G2452 and Arduino with ATMega328 (courtesy Yannick DEVOS - XV4Y), 
/// but untested by us. It also runs on Teensy 3.0 and 3.1 (courtesy of Paul
/// Stoffregen) using the Arduino IDE 1.0.5 and the Teensyduino addon 1.18. 
/// Also compiles and runs on ATtiny85 in
/// Arduino environment, courtesy r4z0r7o3. Also compiles on maple-ide-v0.0.12,
/// and runs on Maple, flymaple 1.1 etc. Runs on ATmega8/168 (Arduino Diecimila,
/// Uno etc), ATmega328 and can run on almost any other AVR8 platform,
/// without relying on the Arduino framework, by properly configuring the
/// library using 'VirtualWire_Config.h' header file for describing the access
/// to IO pins and for setting up the timer. From version 1.22 the library can
/// be compiled by a C compiler (by renaming VirtualWire.cpp into
/// VirtualWire.c) and can be easily integrated with other IDEs like 'Atmel
/// Studio' for example (courtesy of Alexandru Mircescu).
///
/// - Receivers
///  - RX-B1 (433.92MHz) (also known as ST-RX04-ASK)
/// - Transmitters: 
///  - TX-C1 (433.92MHz)
/// - Transceivers
///  - DR3100 (433.92MHz)
///
/// For testing purposes you can connect 2 VirtualWire instances directly, by
/// connecting pin 12 of one to 11 of the other and vice versa, like this for a duplex connection:
///
/// <code>
/// <pre>
/// Arduino 1         wires         Arduino 1
///  D11-----------------------------D12
///  D12-----------------------------D11
///  GND-----------------------------GND
/// </pre>
/// </code>
///
/// You can also connect 2 VirtualWire instances over a suitable analog
/// transmitter/receiver, such as the audio channel of an A/V transmitter/receiver. You may need
/// buffers at each end of the connection to convert the 0-5V digital output to a suitable analog voltage.
///
/// Caution: ATTiny85 has only 2 timers, one (timer 0) usually used for
/// millis() and one (timer 1) for PWM analog outputs. The VirtualWire
/// library, when built for ATTiny85, takes over timer 0, which prevents use
/// of millis() etc but does permit analog outputs.
///
/// \par Installation
///
/// To install, unzip the library into the libraries sub-directory of your
/// Arduino application directory. Then launch the Arduino environment; you
/// should see the library in the Sketch->Import Library menu, and example
/// code in
/// File->Sketchbook->Examples->VirtualWire menu.
///
/// \par Donations
///
/// This library is offered under a free GPL license for those who want to use it that way. 
/// We try hard to keep it up to date, fix bugs
/// and to provide free support. If this library has helped you save time or money, please consider donating at
/// http://www.airspayce.com or here:
///
/// \htmlonly <form action="https://www.paypal.com/cgi-bin/webscr" method="post"><input type="hidden" name="cmd" value="_donations" /> <input type="hidden" name="business" value="mikem@airspayce.com" /> <input type="hidden" name="lc" value="AU" /> <input type="hidden" name="item_name" value="Airspayce" /> <input type="hidden" name="item_number" value="VirtualWire" /> <input type="hidden" name="currency_code" value="USD" /> <input type="hidden" name="bn" value="PP-DonationsBF:btn_donateCC_LG.gif:NonHosted" /> <input type="image" alt="PayPal — The safer, easier way to pay online." name="submit" src="https://www.paypalobjects.com/en_AU/i/btn/btn_donateCC_LG.gif" /> <img alt="" src="https://www.paypalobjects.com/en_AU/i/scr/pixel.gif" width="1" height="1" border="0" /></form> \endhtmlonly
/// 
/// \par Trademarks
///
/// VirtualWire is a trademark of AirSpayce Pty Ltd. The VirtualWire mark was first used on April 20 2008 for
/// international trade, and is used only in relation to data communications hardware and software and related services.
/// It is not to be confused with any other similar marks covering other goods and services.
///
/// \par Copyright
///
/// This software is Copyright (C) 2011-2014 Mike McCauley. Use is subject to license
/// conditions. The main licensing options available are GPL V2 or Commercial:
/// 
/// \par Open Source Licensing GPL V2
///
/// This is the appropriate option if you want to share the source code of your
/// application with everyone you distribute it to, and you also want to give them
/// the right to share who uses it. If you wish to use this software under Open
/// Source Licensing, you must contribute all your source code to the open source
/// community in accordance with the GPL Version 2 when your application is
/// distributed. See http://www.gnu.org/copyleft/gpl.html
/// 
/// \par Commercial Licensing
///
/// This is the appropriate option if you are creating proprietary applications
/// and you are not prepared to distribute and share the source code of your
/// application. Contact info@airspayce.com for details.
///
/// \par Revision History
/// \version 1.0 Original release
/// 
/// \version 1.1 2008-06-24
/// 	Now can compile for atmega8
///	Reported by creatrope
/// \version 1.2 2009-03-30
///	Fixed a problem that prevented compiling with arduino-0015
///	Reported by Jaime Castro
/// \version 1.3 2009-04-01
///	Fixed a compatibility problem with ATMEGA328 of the new arduino
///	Now use SIGNAL(TIMER1_COMPA_vect) instead of ISR(SIG_OUTPUT_COMPARE1A)
///	as discussed in
///	http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1237714550/11
///	and reported by Jaime Castro.
/// \version 1.4 2010-01-29
///	Added vx_tx_active(), suggested by Alan Burlison.
/// \version 1.5 2011-09-09
///	Added vx_tx_active() function.
/// \version 1.6 2012-01-10
///	Fixed a problem where the receiver was always reenabled after
///	transmission. Reported by David Bath
/// \version 1.9 2012-02-07 Documentation updates
///	Documentation updates
/// \version 1.10 Updated CHANGES file with changes since 1.4.
/// \version 1.11 Converted documentation to Doxygen. Moved CHANGES log to this version history.
///     Ensure vw_rx_pin is not accessed unless receiver is enabled
/// \version 1.12 Compiles and runs on on Energia with MSP430G2553 / G2452 and Arduino with ATMega328. 
///     Patches contributed by Yannick DEVOS - XV4Y
/// \version 1.13 util/crc16.h needed for compiling on  Energia with MSP430G2553 / G2452 was accidentally
///     left out of the distribution
/// \version 1.14 Added support ATtiny85 on Arduino, patch provided by r4z0r7o3.
/// \version 1.15 Updated author and distribution location details to airspayce.com
/// \version 1.16 Added support for Teensy 3.0, contributed by Paul Stoffregen.
/// \version 1.17 Increase default MAX_MESSAGE_LENGTH to 80. Added vw_get_rx_good() and vw_get_rx_bad()
///               functions.
/// \version 1.18 Added support for Maple, Flymaple etc with STM32F103RE processor using timer 1.
///               Tested with Flymaple 1.1 and maple-ide-v0.0.12
/// \version 1.19 Added new function vw_rx_inverted(), to allow the incoming RX to be inverted (normally high).
///               Minor improvements to timer setup for Maple. Name vw_tx_active() changed from incorrect
///               vx_tx_active()
/// \version 1.20 Added support for ATtiny84, patched by Chuck Benedict.
/// \version 1.21 Added support for most AVR8 platforms with proper configuration, without depending
///               on Arduino environment, such as Atmega32u2, Atmega32U4, At90USB162 etc, 
///               contributed by Alexandru Daniel Mircescu. 
/// \version 1.22 Alexandru Daniel Mircescu fixed some problems with the recently added AtMega32U2 support.
/// \version 1.23 Fixed some errors and inaccuracies in documentation, with the kind assistance of Alexandru
///               Mircescu.
/// \version 1.24 Minor performance improvement to vw_symbol_6to4() courtesy Ralph Doncaster.
/// \version 1.25 Removed all use of floating point code, with assistance of a patch from Jim Remington.
///               Result is smaller code size.
/// \version 1.26 Removed util/crc16.h from distribution, since it is now included in arduino IDE
///               since version 1.0. Support for arduino IDE prior to 1.0 is now abandoned.
/// \version 1.27 Reinstated VWutil/crc16.h for the benefit of other platforms such as Teensy.
///               Testing on Teensy 3.1. Added End Of Life notice. This library will no longer be maintained 
///               and updated: use RadioHead instead.
///
/// \par Implementation Details
/// See: http://www.airspayce.com/mikem/arduino/VirtualWire.pdf
///
/// \par Performance
/// See: http://www.airspayce.com/mikem/arduino/VirtualWire.pdf
///
/// \par Connections
/// See: http://www.airspayce.com/mikem/arduino/VirtualWire.pdf
///
/// \file VirtualWire.h
/// \brief VirtualWire API
///
/// To use the VirtualWire library, you must have
/// \code
/// #include <VirtualWire.h>
/// \endcode
/// At the top of your sketch.
/// 

#ifndef VirtualWire_h
#define VirtualWire_h

#include <stdint.h>
#include "VirtualWire_Config.h"

#include "Arduino.h"

// These defs cause trouble on some versions of Arduino
#undef abs
#undef double
#undef round

#ifndef VW_MAX_MESSAGE_LEN 
/// Maximum number of bytes in a message, counting the byte count and FCS
	#define VW_MAX_MESSAGE_LEN 80
#endif //VW_MAX_MESSAGE_LEN 

#if !defined(VW_RX_SAMPLES_PER_BIT)
/// Number of samples per bit
	#define VW_RX_SAMPLES_PER_BIT 8
#endif //VW_RX_SAMPLES_PER_BIT  

/// The maximum payload length
#define VW_MAX_PAYLOAD VW_MAX_MESSAGE_LEN-3

/// The size of the receiver ramp. Ramp wraps modulo this number
#define VW_RX_RAMP_LEN 160

// Ramp adjustment parameters
// Standard is if a transition occurs before VW_RAMP_TRANSITION (80) in the ramp,
// the ramp is retarded by adding VW_RAMP_INC_RETARD (11)
// else by adding VW_RAMP_INC_ADVANCE (29)
// If there is no transition it is adjusted by VW_RAMP_INC (20)
/// Internal ramp adjustment parameter
#define VW_RAMP_INC (VW_RX_RAMP_LEN/VW_RX_SAMPLES_PER_BIT)
/// Internal ramp adjustment parameter
#define VW_RAMP_TRANSITION VW_RX_RAMP_LEN/2
/// Internal ramp adjustment parameter
#define VW_RAMP_ADJUST 9
/// Internal ramp adjustment parameter
#define VW_RAMP_INC_RETARD (VW_RAMP_INC-VW_RAMP_ADJUST)
/// Internal ramp adjustment parameter
#define VW_RAMP_INC_ADVANCE (VW_RAMP_INC+VW_RAMP_ADJUST)

/// Outgoing message bits grouped as 6-bit words
/// 36 alternating 1/0 bits, followed by 12 bits of start symbol
/// Followed immediately by the 4-6 bit encoded byte count, 
/// message buffer and 2 byte FCS
/// Each byte from the byte count on is translated into 2x6-bit words
/// Caution, each symbol is transmitted LSBit first, 
/// but each byte is transmitted high nybble first
#define VW_HEADER_LEN 8

// Cant really do this as a real C++ class, since we need to have 
// an ISR
#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus

    // Set the digital IO pin which will be used to enable the transmitter (press to talk, PTT)'
    /// This pin will only be accessed if
    /// the transmitter is enabled
    /// \param[in] pin The Arduino pin number to enable the transmitter. Defaults to 10.
    extern void vw_set_ptt_pin(uint8_t pin);
    
    /// Set the digital IO pin to be for transmit data. 
    /// This pin will only be accessed if
    /// the transmitter is enabled
    /// \param[in] pin The Arduino pin number for transmitting data. Defaults to 12.
    extern void vw_set_tx_pin(uint8_t pin);

    /// Set the digital IO pin to be for receive data.
    /// This pin will only be accessed if
    /// the receiver is enabled
    /// \param[in] pin The Arduino pin number for receiving data. Defaults to 11.
    extern void vw_set_rx_pin(uint8_t pin);

    /// By default the RX pin is expected to be low when idle, and to pulse high 
    /// for each data pulse.
    /// This flag forces it to be inverted. This may be necessary if your transport medium
    /// inverts the logic of your signal, such as happens with some types of A/V tramsmitter.
    /// \param[in] inverted True to invert sense of receiver input
    extern void vw_set_rx_inverted(uint8_t inverted);

    /// By default the PTT pin goes high when the transmitter is enabled.
    /// This flag forces it low when the transmitter is enabled.
    /// \param[in] inverted True to invert PTT
    extern void vw_set_ptt_inverted(uint8_t inverted);

    /// Initialise the VirtualWire software, to operate at speed bits per second
    /// Call this one in your setup() after any vw_set_* calls
    /// Must call vw_rx_start() before you will get any messages
    /// \param[in] speed Desired speed in bits per second
    extern void vw_setup(uint16_t speed);

    /// Start the Phase Locked Loop listening to the receiver
    /// Must do this before you can receive any messages
    /// When a message is available (good checksum or not), vw_have_message();
    /// will return true.
    extern void vw_rx_start();

    /// Stop the Phase Locked Loop listening to the receiver
    /// No messages will be received until vw_rx_start() is called again
    /// Saves interrupt processing cycles
    extern void vw_rx_stop();

    /// Returns the state of the
    /// transmitter
    /// \return true if the transmitter is active else false
    extern uint8_t vw_tx_active();

    /// Block until the transmitter is idle
    /// then returns
    extern void vw_wait_tx();

    /// Block until a message is available
    /// then returns
    extern void vw_wait_rx();

    /// Block until a message is available or for a max time
    /// \param[in] milliseconds Maximum time to wait in milliseconds.
    /// \return true if a message is available, false if the wait timed out.
    extern uint8_t vw_wait_rx_max(unsigned long milliseconds);

    /// Send a message with the given length. Returns almost immediately,
    /// and message will be sent at the right timing by interrupts
    /// \param[in] buf Pointer to the data to transmit
    /// \param[in] len Number of octetes to transmit
    /// \return true if the message was accepted for transmission, false if the message is too long (>VW_MAX_MESSAGE_LEN - 3)
    extern uint8_t vw_send(uint8_t* buf, uint8_t len);

    /// Returns true if an unread message is available
    /// \return true if a message is available to read
    extern uint8_t vw_have_message();

    /// If a message is available (good checksum or not), copies
    /// up to *len octets to buf.
    /// \param[in] buf Pointer to location to save the read data (must be at least *len bytes.
    /// \param[in,out] len Available space in buf. Will be set to the actual number of octets read
    /// \return true if there was a message and the checksum was good
    extern uint8_t vw_get_message(uint8_t* buf, uint8_t* len);

    /// Returns the count of good messages received
    /// Caution,: this is an 8 bit count and can easily overflow
    /// \return Count of good messages received
    extern uint8_t vw_get_rx_good();

    /// Returns the count of bad messages received, ie
    /// messages with bogus lengths, indicating corruption
    /// or lost octets.
    /// Caution,: this is an 8 bit count and can easily overflow
    /// \return Count of bad messages received
    extern uint8_t vw_get_rx_bad();

#ifdef __cplusplus
} //	extern "C"
#endif //__cplusplus

/// @example client.pde
/// Client side of simple client/server pair using VirtualWire

/// @example server.pde
/// Server side of simple client/server pair using VirtualWire

/// @example transmitter.pde
/// Transmitter side of simple one-way transmitter->receiver pair using VirtualWire

/// @example receiver.pde
/// Transmitter side of simple one-way transmitter->receiver pair using VirtualWire

#endif /* VirtualWire_h */
