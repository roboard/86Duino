// Adafruit Motor shield library
// copyright Adafruit Industries LLC, 2009
// Modified 01 November 2014 by Android Lin for 86Duino
// this code is public domain, enjoy!

/*
 * Usage Notes:
 * For PIC32, all features work properly with the following two exceptions:
 *
 * 1) Because the PIC32 only has 5 PWM outputs, and the AFMotor shield needs 6
 *    to completely operate (four for motor outputs and two for RC servos), the
 *    M1 motor output will not have PWM ability when used with a PIC32 board.
 *    However, there is a very simple workaround. If you need to drive a stepper
 *    or DC motor with PWM on motor output M1, you can use the PWM output on pin
 *    9 or pin 10 (normally use for RC servo outputs on Arduino, not needed for 
 *    RC servo outputs on PIC32) to drive the PWM input for M1 by simply putting
 *    a jumber from pin 9 to pin 11 or pin 10 to pin 11. Then uncomment one of the
 *    two #defines below to activate the PWM on either pin 9 or pin 10. You will
 *    then have a fully functional microstepping for 2 stepper motors, or four
 *    DC motor outputs with PWM.
 *
 * 2) There is a conflict between RC Servo outputs on pins 9 and pins 10 and 
 *    the operation of DC motors and stepper motors as of 9/2012. This issue
 *    will get fixed in future MPIDE releases, but at the present time it means
 *    that the Motor Party example will NOT work properly. Any time you attach
 *    an RC servo to pins 9 or pins 10, ALL PWM outputs on the whole board will
 *    stop working. Thus no steppers or DC motors.
 * 
 */
// <BPS> 09/15/2012 Modified for use with chipKIT boards


#ifndef _AFMotor_h_
#define _AFMotor_h_

#include <inttypes.h>

#include <avr/pgmspace.h>

#define MCM_MC      (0)
#define MCM_MD      (1)

#define MICROSTEPS 16   // 8 or 16

#define MOTOR12_64KHZ 1
#define MOTOR12_8KHZ  2
#define MOTOR12_2KHZ  3  
#define MOTOR12_1KHZ  4

#define MOTOR34_64KHZ 1
#define MOTOR34_8KHZ  2
#define MOTOR34_1KHZ  3

#define DC_MOTOR_PWM_RATE   MOTOR34_8KHZ    // PWM rate for DC motors
#define STEPPER1_PWM_RATE   MOTOR12_64KHZ   // PWM rate for stepper 1
#define STEPPER2_PWM_RATE   MOTOR34_64KHZ   // PWM rate for stepper 2


// Bit positions in the 74HCT595 shift register output
#define MOTOR1_A 2
#define MOTOR1_B 3
#define MOTOR2_A 1
#define MOTOR2_B 4
#define MOTOR4_A 0
#define MOTOR4_B 6
#define MOTOR3_A 5
#define MOTOR3_B 7

// Constants that the user passes in to the motor calls
#define FORWARD 1
#define BACKWARD 2
#define BRAKE 3
#define RELEASE 4

// Constants that the user passes in to the stepper calls
#define SINGLE 1
#define DOUBLE 2
#define INTERLEAVE 3
#define MICROSTEP 4

/*
#define LATCH 4
#define LATCH_DDR DDRB
#define LATCH_PORT PORTB

#define CLK_PORT PORTD
#define CLK_DDR DDRD
#define CLK 4

#define ENABLE_PORT PORTD
#define ENABLE_DDR DDRD
#define ENABLE 7

#define SER 0
#define SER_DDR DDRB
#define SER_PORT PORTB
*/

// Arduino pin names for interface to 74HCT595 latch
#define MOTORLATCH 12
#define MOTORCLK 4
#define MOTORENABLE 7
#define MOTORDATA 8

class AFMotorController
{
  public:
    AFMotorController(void);
    void enable(void);
    friend class AF_DCMotor;
    void latch_tx(void);
    uint8_t TimerInitalized;
};

class AF_DCMotor
{
 public:
  AF_DCMotor(uint8_t motornum, uint8_t freq = DC_MOTOR_PWM_RATE);
  void run(uint8_t);
  void setSpeed(uint8_t);

 private:
  uint8_t motornum, pwmfreq;
};

class AF_Stepper {
 public:
  AF_Stepper(uint16_t, uint8_t);
  void step(uint16_t steps, uint8_t dir,  uint8_t style = SINGLE);
  void setSpeed(uint16_t);
  uint8_t onestep(uint8_t dir, uint8_t style);
  void release(void);
  uint16_t revsteps; // # steps per revolution
  uint8_t steppernum;
  uint32_t usperstep, steppingcounter;
 private:
  uint8_t currentstep;

};

uint8_t getlatchstate(void);

#endif
