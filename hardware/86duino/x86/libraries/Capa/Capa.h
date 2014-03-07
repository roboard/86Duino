/**
 * Capacitive Sensing Library for 'duino / Wiring
 * Copyright (c) 2008 Paul Bagder  All rights reserved.
 * Version 04 by Paul Stoffregen - Arduino 1.0 compatibility, issue 146 fix
 * Version "Due-compatible" by Marco Lipparini
 * Version "86Duino" by Terry Wang  
 * 
 * This file is part of the MLX Hardware project.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * version 2 along with this code.
 * If not, see <http://www.gnu.org/licenses/gpl-2.0.html>.
 *
 * Project website: http://www.mlx-service.it
 */

#ifndef CapacitiveSensor_h
	#define CapacitiveSensor_h
	
	#include "Arduino.h"
	#include <limits.h>
	

	#define REGISTER_DATA_TYPE uint8_t*
	#define PORT_DATA_TYPE uint8_t

	#define SET_RECEIVE_INPUT() 	pinMode(this->_receivePin, INPUT)
	#define SET_RECEIVE_OUTPUT() 	pinMode(this->_receivePin, OUTPUT)
	#define SET_SEND_OUTPUT() 	pinMode(this->_sendPin, OUTPUT)

	#define RECEIVE_HIGH() 		digitalWrite(this->_receivePin,HIGH)
	#define RECEIVE_LOW()		digitalWrite(this->_receivePin,LOW)
	#define SEND_HIGH()		digitalWrite(this->_sendPin,HIGH)
	#define SEND_LOW()		digitalWrite(this->_sendPin,LOW)

	#define READ_RECEIVE()	digitalRead(this->_receivePin)


	class CapacitiveSensor
	{
		public:
			/**
			 * This value is the cost of the loop used by this object to
			 * calculate the absolute capacitance (in arbitrary unit).
			 */
			static const uint8_t LOOP_TIMING_FACTOR;
			
			/**
			 * [Constructor]
			 * Creates a new CapacitiveSensor object with the given pin
			 * numbers.
			 */
			CapacitiveSensor(uint8_t sendPin, uint8_t receivePin);
			
			/**
			 * [Set Timeout]
			 * This method allows you to change the timeout for any single
			 * capacitance reading sample.
			 */
			static void setTimeout(unsigned long timeout);
			
			/**
			 * [Read Capacitance]
			 * This method requires one parameter, samples, and returns a long
			 * integer containing the absolute capacitance, in arbitrary units.
			 * The samples parameter can be used to increase the returned
			 * resolution, at the expense of slower performance. The returned
			 * value is not averaged over the number of samples, and the total
			 * value is reported.
			 * 
			 * It will return -1 if the capacitance value exceeds the value of
			 * _timeout.
			 */
			long capacitiveSensor(uint8_t samples);
			long read(uint8_t samples);
			
			/**
			 * [Calibrate]
			 * This method resets the _leastReadTime to the highest value so
			 * that it will be overwritten on the next reads.
			 */
			void calibrate();
		private:
			/**
			 * Timeout. (default: 2000)
			 */
			static unsigned long _timeout;
			
			/**
			 * Send pin number.
			 */
			 
			uint8_t _sendPin;
			
			/**
			 * Receive pin number.
			 */
			uint8_t _receivePin;
			
			
			/**
			 * The least read time. This is used to keep the "untouched"
			 * value as close as possibile to zero.
			 */
			unsigned long _leastReadTime;
			
			/**
			 * [Sense Sample]
			 * This method calculates a single sample and adds it to the given
			 * total.
			 */
			int senseSample(long *total);
	};
#endif
