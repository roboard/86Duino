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

// Required libraries...
#include "CapacitiveSensor.h"


/*
 * Static fields...
 */
// Value determined empirically by the original author.
const uint8_t CapacitiveSensor::LOOP_TIMING_FACTOR = 310;

// Using floats to deal with large numbers.
#define TIMEOUT_FORMULA(TIMEOUT) (TIMEOUT * (float) CapacitiveSensor::LOOP_TIMING_FACTOR)

// Default timeout.
unsigned long CapacitiveSensor::_timeout = TIMEOUT_FORMULA(2000);

/*
 * Methods...
 */
// [Constructor]
CapacitiveSensor::CapacitiveSensor(uint8_t sendPin, uint8_t receivePin)
{
//	PORT_DATA_TYPE sendPort;
//	PORT_DATA_TYPE receivePort;
	
	this->_sendPin = sendPin;
	this->_receivePin = receivePin;
	this->calibrate();
	
	noInterrupts();
	SET_SEND_OUTPUT();
	interrupts();
}

// [Set Timeout]
void CapacitiveSensor::setTimeout(unsigned long timeout) {
	CapacitiveSensor::_timeout = TIMEOUT_FORMULA(timeout);
}

// [capacitiveSensor] for old version
long CapacitiveSensor::capacitiveSensor(uint8_t samples)
{
	return read(samples);
}

// [Read Capacitance]
long CapacitiveSensor::read(uint8_t samples)
{
	long total = 0;
	
	if (samples == 0) {
		return 0;
	}
	
	for (uint8_t i = 0; i < samples; i++) {
		if (senseSample(&total) < 0) {
			// Timeout...
			return -1;
		}
	}
	
	if (total < this->_leastReadTime) {
		this->_leastReadTime = total;
	}
	
	return total - this->_leastReadTime;
}

// [Calibrate]
void CapacitiveSensor::calibrate() {
	this->_leastReadTime = ULONG_MAX;
}

// [Sense Sample]
int CapacitiveSensor::senseSample(long *total)
{
	noInterrupts();
	
	SEND_LOW();
	//SET_RECEIVE_INPUT();
	//RECEIVE_LOW();
	SET_RECEIVE_OUTPUT();
	RECEIVE_LOW();
	SET_RECEIVE_INPUT();
	SEND_HIGH();
	
	interrupts();
	
	#define LOOP_CONDITION (READ_RECEIVE() && (*total < CapacitiveSensor::_timeout))
	
	// While receive pin is LOW and timeout has not occurred...
	while (digitalRead(this->_receivePin) == LOW) {
		(*total)++;
	}
	
	#define CHECK_TIMEOUT() \
		if (*total >= CapacitiveSensor::_timeout) { \
			return -1; \
		}
	CHECK_TIMEOUT();
	
	// Set receive pin HIGH briefly to charge up fully because the while loop above will
	// exit when pin is ~2.5V. 
	noInterrupts();

	//RECEIVE_HIGH();
	SET_RECEIVE_OUTPUT();
	RECEIVE_HIGH();
	//SET_RECEIVE_INPUT();
	pinMode(this->_receivePin, INPUT_PULLUP);
	//RECEIVE_LOW();//Turn off pull-up
	SET_RECEIVE_INPUT(); //Turn off pull-up
	SEND_LOW();
		
	interrupts();
	// While receive pin is HIGH and timeout has not occurred...
	while (LOOP_CONDITION) {
		(*total)++;
	}
	
	CHECK_TIMEOUT();
	
	return 1;
}
