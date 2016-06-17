/*
  Servo.cpp - Servo library for 86Duino
  Copyright (c) 2015 Android Lin <acen@dmp.com.tw>. All right reserved.

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

#include <Arduino.h> 

#include "Servo86.h"
#include "io.h"
#include "irq.h"
#include "pins_arduino.h"

/* RTC */   
#define RTCIRQ    (8)
unsigned char _freq = 15;
bool timerRTCEnable = false;
static char* isrname_rtc = "TimerRTC";
static bool timerRTCInit = false;
void RTC_initialize(long microseconds);
void RTC_timer_start(long microseconds);
void RTC_timer_close();
void RTC_setPeriod(long microseconds);

volatile struct _servo86 {
	unsigned long prevposition;
    unsigned long curposition;
	unsigned long targetposition;
	unsigned long starttime;
	unsigned long endtime;
	unsigned long pausetime;
	int           state;
	long          mixoffset;
} sv86[MAX_SERVOS];



/* SERVO86 */
//#define DEBUG_MSG

// The state of Servos
#define SERVO_NONE      (-1)
#define SERVO_IDLE      (0)
#define SERVO_MOVING    (1)
#define SERVO_PAUSE     (2)

Servo nullServo; // Used by multi-Servo functions

typedef struct {
  int pin;
  long value;
} irqservo_t;


#define usToTicks(_us)    ((_us)*SYSCLK)     // converts microseconds to tick
#define ticksToUs(_ticks) ((_ticks)/SYSCLK) // converts from ticks back to microseconds
#define ENDPIN    (999)

#define TRIM_DURATION       2                               // compensation ticks to trim adjust for digitalWrite delays // 12 August 2009

//#define NBR_TIMERS        (MAX_SERVOS / SERVOS_PER_TIMER)

static servo_t servos[MAX_SERVOS];                          // static array of servo structures
static servo_t sorted[MAX_SERVOS];

static unsigned char had_target_pos[MAX_SERVOS];
// In ISR, we need two array between
static volatile irqservo_t irqservosA[MAX_SERVOS+1];
static volatile irqservo_t irqservosB[MAX_SERVOS+1]; 
static volatile irqservo_t *servosA = irqservosA; // for user interface, it is real time
static volatile irqservo_t *servosB = irqservosB; // for interrupt ISR, it will be updated per 20ms period
static volatile bool uploading = false;
static volatile bool havemail = false;
//static volatile int8_t Channel[_Nbr_16timers ];             // counter for the servo being pulsed for each timer (or -1 if refresh interval)

uint8_t ServoCount = 0;                                     // the total number of attached servos
uint8_t ServoSortCount = 0;
static volatile uint8_t ServoIRQCountA = 0; // it is real time
static volatile uint8_t ServoIRQCountB = 0; // it will be updated per 20ms period
static volatile bool over_periodA = false;
static volatile bool over_periodB = false;
static volatile bool pwm_IsEnable = false;

#define SERVO_MIN() (this->min)  // minimum value in uS for this servo
#define SERVO_MAX() (this->max)  // maximum value in uS for this servo 
static int mc = 3;
static int md = 2;
#define EVTPOINT    (5000L)

/************************************ MCM ************************************/
static int mcint_offset[3] = {0, 8, 16};
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

/************ static functions common to all instances ***********************/

static volatile int channel;
static volatile int evtnumber; // It is a excess variable, but can read the code more clearly. 
static void pulse_init(bool mcm_init) {
	if(mcm_init == true)
	{  
		// Set_MCIRQ(GetMCIRQ()); // moved to wiring.cpp
		mcpwm_SetOutMask(mc, md, MCPWM_HMASK_NONE + MCPWM_LMASK_NONE);
		mcpwm_SetOutPolarity(mc, md, MCPWM_HPOL_NORMAL + MCPWM_LPOL_NORMAL);
		mcpwm_SetDeadband(mc, md, 0L);
		mcpwm_ReloadOUT_Unsafe(mc, md, MCPWM_RELOAD_NOW);
		mcpwm_SetWaveform(mc, md, MCPWM_EDGE_A0I1);
		mcpwm_SetSamplCycle(mc, md, 0x04L);   // sample cycle: 20ms
		mcpwm_SetOutPolarity(mc, md, MCPWM_HPOL_INVERSE + MCPWM_LPOL_INVERSE);
		mcpwm_SetDeadband(mc, md, 3L);
		mcpwm_ReloadOUT_Unsafe(mc, md, MCPWM_RELOAD_NOW);
    }  

	// start fist pulse
    mcpwm_SetEVTMode(mc, md, MCPWM_EVT_PERPWM2);  
    mcpwm_SetEVT(mc, md, EVTPOINT, 0, 0);
    mcpwm_ReloadEVT(mc,md,MCPWM_RELOAD_NOW);
	mcpwm_SetWidth(mc, md, (*(servosB + 0)).value, 1000L); // 1000L is duty, it's value is not important
    mcpwm_ReloadPWM(mc, md, MCPWM_RELOAD_NOW);
	clear_INTSTATUS();
	enable_MCINT(USER_EVT_INT); 
      
    mcpwm_Enable(mc, md);
    digitalWrite((*(servosB + 0)).pin, HIGH);
    mcpwm_SetWidth(mc, md, (*(servosB + 0)).value, 1000L); // 1000L is duty, it's value is not important
    mcpwm_ReloadPWM(mc, md, MCPWM_RELOAD_PEREND);
    evtnumber = 0;
    channel = 0;
}

static int isr_handler(int irq, void* data) {
	unsigned long val;
	volatile irqservo_t *tmp;
	unsigned long svintmask = USER_EVT_INT << mcint_offset[md];
    if((mc_inp(mc, 0x04) & svintmask) == 0L) return ISR_NONE;
	
	mc_outp(mc, 0x04, svintmask);
    
    do
    	val = mcpwm_ReadSTATREG2(mc, md);
    while(val>=80L && val<EVTPOINT);
	
	
	if(evtnumber > 0)
	{
		if(evtnumber == ServoIRQCountB) // have no next pulse, ready to complete
		{
			if((evtnumber & 0x03) == 0x03 || (evtnumber & 0x03) == 0x00) digitalWrite((*(servosB + channel - 1)).pin, HIGH); // delay 1 I/O time
			digitalWrite((*(servosB + channel - 1)).pin, LOW); // Set pin form high to low
			if(over_periodB == true) goto NEXT; else evtnumber = -2;
		}
		else if(evtnumber == (ServoIRQCountB - 1)) // It is the last pin, has no next pin to send pulse
		{
			if((evtnumber & 0x03) == 0x03 || (evtnumber & 0x03) == 0x00) digitalWrite((*(servosB + channel - 1)).pin, HIGH); // delay 1 I/O time
			digitalWrite((*(servosB + channel - 1)).pin, LOW); // Set pin form high to low
			mcpwm_SetWidth(mc, md, (*(servosB + ServoIRQCountB)).value, 1000L);
	  		mcpwm_ReloadPWM(mc, md, MCPWM_RELOAD_PEREND);
		}
		else // Have the next pin
		{
			if((evtnumber & 0x03) == 0x03 || (evtnumber & 0x03) == 0x00)
			{
				digitalWrite((*(servosB + channel + 1)).pin, HIGH);
				digitalWrite((*(servosB + channel - 1)).pin, LOW);
			}
			else
			{
				digitalWrite((*(servosB + channel - 1)).pin, LOW);
				digitalWrite((*(servosB + channel + 1)).pin, HIGH);
			}
			mcpwm_SetWidth(mc, md, (*(servosB + channel + 1)).value, 1000L);
	  		mcpwm_ReloadPWM(mc, md, MCPWM_RELOAD_PEREND);
		}
	}
	else if(evtnumber == 0) // First enter ISR 
	{
		if(evtnumber == (ServoIRQCountB - 1)) // It is the last pin, has no next pin to send pulse
		{
			mcpwm_SetWidth(mc, md, (*(servosB + ServoIRQCountB)).value, 1000L); 
	  		mcpwm_ReloadPWM(mc, md, MCPWM_RELOAD_PEREND);
		}
		else // Have the next pin
		{
			digitalWrite((*(servosB + 1)).pin, HIGH);
			mcpwm_SetWidth(mc, md, (*(servosB + 1)).value, 1000L);
	  		mcpwm_ReloadPWM(mc, md, MCPWM_RELOAD_PEREND);
		}
	}
	else if(evtnumber == -1)
	{
NEXT:
		mcpwm_Disable(mc, md);
		// exchange pointer of array servosA and servosB to get new data
		if(uploading == false && havemail == true)
		{
			tmp = servosA;
			servosA = servosB;
			servosB = tmp;
			ServoIRQCountB = ServoIRQCountA;
			over_periodB = over_periodA;
			havemail = false;
		}
		if(ServoIRQCountB <= 0) {pwm_IsEnable = false; return ISR_HANDLED;} // after this, interrupt will not be enabled again
		pulse_init(false); // Init again
		return ISR_HANDLED;
	}
	
	channel++;
    evtnumber++;
    return ISR_HANDLED;
}




static bool isISRActive(void) {
	for(uint8_t channel=0; channel < ServoCount; channel++)
		if(servos[channel].Pin.isActive == true)
			return true;
	return false;
}


/****************** end of static functions ******************************/

static void insert(servo_t s) {
	uint8_t i;
	
	for(i=ServoSortCount; i>=0; i--)
		if((i == 0) || (i != 0 && s.ticks >= sorted[i-1].ticks))
		{
			sorted[i].ticks = s.ticks;
			sorted[i].Pin.nbr = s.Pin.nbr;
			break;
		}
		else
		{
			sorted[i].ticks = sorted[i-1].ticks;
			sorted[i].Pin.nbr = sorted[i-1].Pin.nbr;
		}
	ServoSortCount++;
}

static void del(servo_t s) {
	int i;
	for(i=0; i<ServoSortCount; i++)
		if(sorted[i].Pin.nbr == s.Pin.nbr) break;
	for(; i<ServoSortCount; i++)
		if(i == (ServoSortCount-1))
		{
			sorted[i].ticks = s.ticks;
			sorted[i].Pin.nbr = ENDPIN;
		}
		else
		{
			sorted[i].ticks = sorted[i+1].ticks;
			sorted[i].Pin.nbr = sorted[i+1].Pin.nbr;
		}
	ServoSortCount--;
}

static void update(servo_t s) {
    int i;
	for(i=0; i<ServoSortCount; i++)
		if(sorted[i].Pin.nbr == s.Pin.nbr) break;
	
	if(i != (ServoSortCount-1) && s.ticks >= sorted[i+1].ticks)
	{
		for(; i<ServoSortCount; i++)
			if((i == (ServoSortCount-1)) || (i != (ServoSortCount-1) && s.ticks <= sorted[i+1].ticks))
			{
				sorted[i].ticks = s.ticks;
				sorted[i].Pin.nbr = s.Pin.nbr;
				break;
			}
			else
			{
				sorted[i].ticks = sorted[i+1].ticks;
				sorted[i].Pin.nbr = sorted[i+1].Pin.nbr;
			}
	}
	else
	{
		for(; i>=0; i--)
			if((i == 0) || (i != 0 && s.ticks >= sorted[i-1].ticks))
			{
				sorted[i].ticks = s.ticks;
				sorted[i].Pin.nbr = s.Pin.nbr;
				break;
			}
			else
			{
				sorted[i].ticks = sorted[i-1].ticks;
				sorted[i].Pin.nbr = sorted[i-1].Pin.nbr;
			}
	}
}

#define INIT_TIME    (1800L)
static void cp2irqservos(void) {
    int i;
    unsigned long total = 0L;
    
	#if defined DEBUG_MODE
    	for(i=0; i<ServoSortCount; i++)
			printf("%d. channel:%d value:%d\n", i, sorted[i].Pin.nbr, sorted[i].ticks);
	#endif
    
	io_DisableINT();
	uploading = true; // Start to upload, so let ISR to ignore exchagne process when the uploading = true
	io_RestoreINT();
	
	ServoIRQCountA = ServoSortCount;
	if(ServoSortCount == 0) goto END; // if user detach all channel, ServoSortCount will be 0
	
	for(i=0; i<ServoSortCount; i++)
    {   
		(*(servosA + i)).pin = sorted[i].Pin.nbr;
    	if(i == 0)
			(*(servosA + i)).value = sorted[i].ticks/2;
	    else 
	    	(*(servosA + i)).value = sorted[i].ticks - (*(servosA + i - 1)).value;
		total += (*(servosA + i)).value;
    }
    (*(servosA + i)).pin = sorted[i-1].Pin.nbr;
	(*(servosA + i)).value = 2000000L - (total + (*(servosA + 0)).value) - INIT_TIME;
	if((*(servosA + i)).value < EVTPOINT)
	{
		(*(servosA + i)).value = 0xFFFFFFFFL;
		over_periodA = true;
	}
	else
		over_periodA = false;
	
	#if defined DEBUG_MODE
		for(i=0; i<(ServoSortCount+1); i++)
			printf("%d. channel:%d value:%d\n", i, (*(servosA + i)).pin, (*(servosA + i)).value);
	#endif
END:		
	io_DisableINT();
	uploading = false; // End to upload
	havemail = true; // Tell the ISR to exhange data.
	io_RestoreINT();
}

static char* name = "Servo";
static bool initServoIRQ(void) {
	if(Global_irq_Init == false)
	{
		if(irq_Init() == false) 
	    {
	        printf("irq_init fail\n"); return false;
	    }
	    
	    if(irq_Setting(GetMCIRQ(), IRQ_LEVEL_TRIGGER + IRQ_DISABLE_INTR) == false)
	    {
	        printf("%s\n", __FUNCTION__); return false;
	    }
	    Global_irq_Init = true;
	}
    
    if(irq_InstallISR(GetMCIRQ(), isr_handler, (void*)name) == false)
    {
        printf("irq_install fail\n"); return false;
    }
    
    return true;
}

static bool closeServoIRQ(void) {
    irq_UninstallISR(GetMCIRQ(), (void*)name);
}

static bool isPWMPin(uint8_t pin) {
    int mcpwm = PIN86[pin].PWMMC;
	int mdpwm = PIN86[pin].PWMMD;
	if(mcpwm == NOPWM || mdpwm == NOPWM) return false;
	if(mcpwm == MC_MODULE3 && mdpwm == MCPWM_MODULEB) return false;// this pin is uesd by tone and IRrmote lib after version 103
    return true;
}



static char mc_md_inuse[PINS];
static Servo* Servoptr[45] = {NULL};
static double _min = 0.0;   // the value can be changed by angleMap()
static double _max = 180.0; // the value can be changed by angleMap()
Servo::Servo() {
	if(ServoCount < MAX_SERVOS)
	{
		if(ServoCount == 0) initServoIRQ(); 
		this->servoIndex = ServoCount++; // assign a servo index to this instance
		this->min = MIN_PULSE_WIDTH;
		this->max = MAX_PULSE_WIDTH;
		had_target_pos[this->servoIndex] = 0;
		sv86[this->servoIndex].state = SERVO_NONE;
		target_position = usToTicks(DEFAULT_PULSE_WIDTH);
		servos[this->servoIndex].ticks = target_position;
		servos[this->servoIndex].Pin.isActive = false;
		speed_us = FULL_SPEED;
		total_time = 0L; // max speed
		pos_offset = 0L;
		sv86[this->servoIndex].mixoffset = 0L;
		Servoptr[servoIndex] = NULL;
		this->angle_resolution = (MAX_PULSE_WIDTH - MIN_PULSE_WIDTH) / 180.0;
	}
	else
		this->servoIndex = INVALID_SERVO ;  // too many servos
}

uint8_t Servo::attach(int pin) {
	return this->attach(pin, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH);
}

uint8_t Servo::attach(int pin, unsigned long min, unsigned long max) {
	if(pin >= PINS || pin < 0) return 0;
	if(servos[this->servoIndex].Pin.isActive == true) return this->servoIndex; // if pin have been active, ingore sort process
	if(this->servoIndex < MAX_SERVOS)
	{
		servos[this->servoIndex].Pin.nbr = pin;
		this->min = min;
		this->max = max; 
		if(isPWMPin(pin) == false)
		{
			pinMode(pin, OUTPUT);
			digitalWrite(pin, LOW);
		}
		else
		{
			if(mcpwm_ReadReloadOUT(PIN86[pin].PWMMC, PIN86[pin].PWMMD) != 0L)
				mcpwm_ReloadOUT_Unsafe(PIN86[pin].PWMMC, PIN86[pin].PWMMD, MCPWM_RELOAD_CANCEL);
			mcpwm_SetOutMask(PIN86[pin].PWMMC, PIN86[pin].PWMMD, MCPWM_HMASK_INACTIVE);
			mcpwm_ReloadOUT_Unsafe(PIN86[pin].PWMMC, PIN86[pin].PWMMD, MCPWM_RELOAD_NOW);
		}
		servos[this->servoIndex].Pin.isActive = true;  // this must be set after the check for isISRActive
		Servoptr[servoIndex] = this;
	}

	return this->servoIndex;
}

void Servo::detach() {
	uint8_t pin;
	int mc_pwm, md_pwm;
	
	if(this->servoIndex >= MAX_SERVOS) return;
	if(servos[this->servoIndex].Pin.isActive == false) return; // if pin have been inactive, ingore sort process
	
	servos[this->servoIndex].Pin.isActive = false;
	pin = servos[this->servoIndex].Pin.nbr;
	
	if(isPWMPin(pin) == false)
	{
		del(servos[this->servoIndex]);
		cp2irqservos();
	}
	else
	{
		mc_pwm = PIN86[pin].PWMMC;
		md_pwm = PIN86[pin].PWMMD;
		mcpwm_SetOutMask(mc_pwm, md_pwm, MCPWM_HMASK_INACTIVE);
		mcpwm_ReloadOUT_Unsafe(mc_pwm, md_pwm, MCPWM_RELOAD_PEREND);
		mc_md_inuse[pin] = 0;
	}
	
	had_target_pos[this->servoIndex] = 0;
	Servoptr[this->servoIndex] = NULL;
}

static void sendPWM(uint8_t pin, unsigned int val) {
    unsigned short crossbar_ioaddr = 0;
	int mc_pwm, md_pwm;
	
    mc_pwm = PIN86[pin].PWMMC;
    md_pwm = PIN86[pin].PWMMD;
    
    crossbar_ioaddr = sb_Read16(0x64)&0xfffe;
    
	if (pin <= 9)
		io_outpb(crossbar_ioaddr + 2, 0x01); // GPIO port2: 0A, 0B, 0C, 3A
	else if (pin > 28)
    	io_outpb(crossbar_ioaddr, 0x03); // GPIO port0: 2A, 2B, 2C, 3C
	else
		io_outpb(crossbar_ioaddr + 3, 0x02); // GPIO port3: 1A, 1B, 1C, 3B
    
    // Init H/W PWM
    if(mc_md_inuse[pin] == 0)
	{
		mcpwm_SetOutMask(mc_pwm, md_pwm, MCPWM_HMASK_NONE + MCPWM_LMASK_NONE);
		mcpwm_SetOutPolarity(mc_pwm, md_pwm, MCPWM_HPOL_NORMAL + MCPWM_LPOL_NORMAL);
		mcpwm_SetDeadband(mc_pwm, md_pwm, 0L);
		mcpwm_ReloadOUT_Unsafe(mc_pwm, md_pwm, MCPWM_RELOAD_NOW);
		
		mcpwm_SetWaveform(mc_pwm, md_pwm, MCPWM_EDGE_A0I1);
		mcpwm_SetSamplCycle(mc_pwm, md_pwm, 0L);
    }
   
    mcpwm_SetWidth(mc_pwm, md_pwm, 20000L*SYSCLK, val);
    mcpwm_ReloadPWM(mc_pwm, md_pwm, MCPWM_RELOAD_PEREND);
    
    if(mc_md_inuse[pin] == 0)
	{
		mcpwm_Enable(mc_pwm, md_pwm);
		io_outpb(crossbar_ioaddr + 0x90 + PIN86[pin].gpN, 0x08);
		mc_md_inuse[pin] = 1;
    }  
}

static void _write(byte channel, long value) {
	int i;
	
	if(channel < MAX_SERVOS)   // ensure channel is valid
	{  
		//value = value - TRIM_DURATION;
		value = usToTicks(value);  // convert to ticks after compensating for interrupt overhead - 12 Aug 2009
		
		// if(servos[channel].ticks == value) return; // if set the same value to same pin, do nothong.
		servos[channel].ticks = value; // update date
		if(servos[channel].Pin.isActive == false) return; 
		
		if(isPWMPin(servos[channel].Pin.nbr) == true)
		{
			sendPWM(servos[channel].Pin.nbr, servos[channel].ticks);
			return;
		}
		
		update(servos[channel]);
		cp2irqservos();
		// setting and start fisrt pulse
		if(pwm_IsEnable == false)
		{
			for(i=0; i<(ServoSortCount+1); i++)
		    {
				(*(servosB + i)).pin = (*(servosA + i)).pin;
				(*(servosB + i)).value = (*(servosA + i)).value;
		    }
			ServoIRQCountB = ServoIRQCountA;
			over_periodB = over_periodA;
			pulse_init(true); // start pwm timer
			pwm_IsEnable = true;
		}
	} 
}

double Servo::read() {
	double x, in_min, in_max, out_min, out_max;
	
	x = this->readMicroseconds();
	in_min = SERVO_MIN();
	in_max = SERVO_MAX();
	out_min = _min,
	out_max = _max;
	return ((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);     
}

bool Servo::attached() {
	return servos[this->servoIndex].Pin.isActive;
}

void _attach_hw_servos(int pin) {
	if(isPWMPin(pin) == true)
	{  
		if(mcpwm_ReadReloadOUT(PIN86[pin].PWMMC, PIN86[pin].PWMMD) != 0L)
			mcpwm_ReloadOUT_Unsafe(PIN86[pin].PWMMC, PIN86[pin].PWMMD, MCPWM_RELOAD_CANCEL);
	}	
}

void Servo::angleMap(double angle_min, double angle_max) {
	if(angle_min < 0.0 || angle_max < 0.0) return;
	if(angle_min > 360.0 || angle_max > 360.0) return;
	
	_min = (angle_min > angle_max) ? angle_max : angle_min;
    _max = (angle_min < angle_max) ? angle_max : angle_min;
}

void Servo::setVelocity(unsigned long velocity) { // unit: us/s
	if(velocity == 0L)
	{
		speed_us = FULL_SPEED;
		return;
	}
    speed_us = velocity;
}

void Servo::setVelocity(double angular_velocity) {
	unsigned long tmp;
	if(angular_velocity <= 0.0)
	{
		speed_us = FULL_SPEED;
		return;
	}

	this->angle_resolution = ((SERVO_MAX() - SERVO_MIN()) / (_max - _min));
	speed_us = (long) (angular_velocity * this->angle_resolution);
	
	if(speed_us <= 0L) speed_us = FULL_SPEED;
}

void Servo::setPosition(long pos, unsigned long time) {
	if(pos <= 0L) return; // Added for ServoFrame.setPosition()

    if(had_target_pos[servoIndex] == 0)
	{
		if(isPWMPin(servos[servoIndex].Pin.nbr) == false) insert(servos[servoIndex]);
		had_target_pos[servoIndex] = 1;
	}
	
	if(pos <= SERVO_MIN())
		target_position = SERVO_MIN();
	else if(pos >= SERVO_MAX())
		target_position = SERVO_MAX();
	else
		target_position = pos;
	
	total_time = time;
}

void Servo::setPosition(double angle, unsigned long time) {
	if(angle == 0.0) return;
	
	if(had_target_pos[servoIndex] == 0)
	{
		if(isPWMPin(servos[servoIndex].Pin.nbr) == false) insert(servos[servoIndex]);
		had_target_pos[servoIndex] = 1;
	}
	
	if(angle <= _min)
		target_position = SERVO_MIN();
	else if(angle >= _max)
		target_position = SERVO_MAX();
	else
	{
		this->angle_resolution = ((SERVO_MAX() - SERVO_MIN()) / (_max - _min));
	    target_position = (unsigned long) ((angle - _min) * angle_resolution + SERVO_MIN());
	}
	
	total_time = time;
	/*
	printf("min = %ld max = %ld\n", SERVO_MIN(), SERVO_MAX());
	printf("target_position = %ld angle_resolution = %f\n", target_position, angle_resolution);
	*/
}

long Servo::getPosition(void) {
	if(sv86[servoIndex].state == SERVO_NONE) return 0L;
	
	return target_position;
}

void Servo::setOffset(long offset) {
    if(offset < -256L) offset = -256L;
    if(offset > 256L)  offset = 256L;
	pos_offset = offset;
}

void Servo::setRealTimeMixing(long mixoffset) {
    setRealTimeMixing(mixoffset, true);
}

void Servo::setRealTimeMixing(long mixoffset, bool interrupt) {
    if(interrupt) io_DisableINT();
	sv86[servoIndex].mixoffset = mixoffset;
	if(interrupt) io_RestoreINT();
}

void Servo::write(int value) {
	if(value < MIN_PULSE_WIDTH)
	{   // treat values less than 450 as angles in degrees (valid values in microseconds are handled as microseconds)
		if(value < _min) value = _min;
		if(value > _max) value = _max;
		value = map(value, _min, _max, SERVO_MIN(), SERVO_MAX());      
	}

	setPosition((unsigned long)value, total_time);
	run();
}

void Servo::writeMicroseconds(int pos) {
    write((unsigned long)pos);
}

void Servo::write(unsigned long pos) {
	setPosition(pos, total_time);
	run();
}

void Servo::write(double angle) {
    setPosition(angle, total_time);
	run();
}

unsigned long Servo::readMicroseconds(void) {
	unsigned long tmp;
    if(servoIndex >= MAX_SERVOS) return 0L;
    if(servos[servoIndex].Pin.isActive == false) return 0L;
	if(sv86[servoIndex].state == SERVO_NONE) return 0L;
	
	io_DisableINT();
	tmp = sv86[servoIndex].curposition - pos_offset;
	io_RestoreINT();
	
	return tmp;
}

static bool RTC_Timer_Used = false;
void Servo::run(void) {
	if(servoIndex >= MAX_SERVOS) return;
	if(servos[servoIndex].Pin.isActive == false) return;
	
	// 1. check whether the angle is over the limit value.
	if((target_position + pos_offset) < SERVO_MIN())
		target_position = SERVO_MIN() - pos_offset;
	else if((target_position + pos_offset) > SERVO_MAX())
		target_position = SERVO_MAX() - pos_offset;
	
	// 2. set servo parameters
	io_DisableINT();
	if(sv86[servoIndex].state == SERVO_MOVING || sv86[servoIndex].state == SERVO_IDLE)
    {
    	sv86[servoIndex].prevposition = sv86[servoIndex].curposition;
	}
	else if(sv86[servoIndex].state == SERVO_NONE)  // GO to HOME position
	{
		if(had_target_pos[servoIndex] == 0) {io_RestoreINT(); return;}
		_attach_hw_servos(servos[servoIndex].Pin.nbr);

		sv86[servoIndex].prevposition = sv86[servoIndex].curposition = target_position + pos_offset;
		sv86[servoIndex].state = SERVO_IDLE;
		io_RestoreINT();
		
		_write(servoIndex, sv86[servoIndex].curposition);
		
		// First call run(), enable RTC Timer
		if(RTC_Timer_Used == false)
		{
			RTC_initialize(15625L); // 15.625ms
			RTC_timer_start(15625L);
			
			//RTC_initialize(31250L); // 31.25ms
			//RTC_timer_start(31250L);
			
			//RTC_initialize(62500L); // 62.5ms
			//RTC_timer_start(62500L);
			RTC_Timer_Used = true;
		}
		
		return;
	}
	else // PAUSE state
	{
		io_RestoreINT();
		return;	
	}
	
	// 3. set new position to the servo
	sv86[servoIndex].targetposition = target_position + pos_offset;
	
	if(sv86[servoIndex].curposition == sv86[servoIndex].targetposition) // if target is equal to current, do nothing.
	{
		if(sv86[servoIndex].state == SERVO_MOVING) // if servo is moving and get the same target position, we need to update the endtime value
		{
            sv86[servoIndex].starttime = millis();
			sv86[servoIndex].endtime = sv86[servoIndex].starttime + total_time;
		}
		// if sv86[servoIndex].state is IDLE, do nothing.
		io_RestoreINT();
		return;
	}
	
	if(speed_us > 0 && speed_us != FULL_SPEED && total_time == 0L) // transform speed to time
	{
		if(sv86[servoIndex].targetposition > sv86[servoIndex].curposition)
			total_time = (sv86[servoIndex].targetposition - sv86[servoIndex].curposition) * 1000L / speed_us;
		else
			total_time = (sv86[servoIndex].curposition - sv86[servoIndex].targetposition) * 1000L / speed_us;
	}
	
	sv86[servoIndex].starttime = millis();
	sv86[servoIndex].endtime = sv86[servoIndex].starttime + total_time;
	sv86[servoIndex].state = SERVO_MOVING;
	io_RestoreINT();
}

void Servo::pause(void) {
	if(servoIndex >= MAX_SERVOS) return;
	if(servos[servoIndex].Pin.isActive == false) return;

	io_DisableINT();
	if(sv86[servoIndex].state != SERVO_MOVING) {io_RestoreINT(); return;}
	sv86[servoIndex].pausetime = millis();
	sv86[servoIndex].state = SERVO_PAUSE;
	io_RestoreINT();
}

void Servo::resume(void) {
	if(servoIndex >= MAX_SERVOS) return;
	if(servos[servoIndex].Pin.isActive == false) return;

	io_DisableINT();
    if(sv86[servoIndex].state != SERVO_PAUSE) {io_RestoreINT(); return;}
	sv86[servoIndex].pausetime = millis() - sv86[servoIndex].pausetime;
	sv86[servoIndex].starttime += sv86[servoIndex].pausetime;
	sv86[servoIndex].endtime += sv86[servoIndex].pausetime;
	sv86[servoIndex].state = SERVO_MOVING;
	io_RestoreINT();
}

void Servo::stop(void) {  
	if(servoIndex >= MAX_SERVOS) return;
	if(servos[servoIndex].Pin.isActive == false) return;
	
	io_DisableINT();
	target_position = sv86[servoIndex].prevposition = sv86[servoIndex].targetposition = sv86[servoIndex].curposition;
	sv86[servoIndex].starttime = 0L;
	sv86[servoIndex].endtime = 0L;
	sv86[servoIndex].pausetime = 0L;
	if(sv86[servoIndex].state != SERVO_NONE)
		sv86[servoIndex].state = SERVO_IDLE;
	total_time = 0L;
	speed_us = FULL_SPEED;
	io_RestoreINT();
}

void _detach(int index, int pin) {
	int mc_pwm, md_pwm;
	
	if(isPWMPin(pin) == false)
	{
		del(servos[index]);
		cp2irqservos();
	}
	else
	{
		mc_pwm = PIN86[pin].PWMMC;
		md_pwm = PIN86[pin].PWMMD;
		mcpwm_SetOutMask(mc_pwm, md_pwm, MCPWM_HMASK_INACTIVE);
		mcpwm_ReloadOUT_Unsafe(mc_pwm, md_pwm, MCPWM_RELOAD_PEREND);
		mc_md_inuse[pin] = 0;
	}
	had_target_pos[index] = 0;
}

void Servo::release(void) {
	if(servoIndex >= MAX_SERVOS) return;
	if(servos[servoIndex].Pin.isActive == false) return;
	if(had_target_pos[servoIndex] == 0) return;
	
	stop();
	_detach(servoIndex, servos[servoIndex].Pin.nbr);
	target_position = usToTicks(DEFAULT_PULSE_WIDTH) + pos_offset;
	
	io_DisableINT();
	sv86[servoIndex].state = SERVO_NONE;
	io_RestoreINT();
}

bool Servo::isMoving(void) {
    int tmp;
    if(servoIndex >= MAX_SERVOS) return false;
	if(servos[servoIndex].Pin.isActive == false) return false;
	
	io_DisableINT();
	tmp = sv86[servoIndex].state;
    io_RestoreINT();
    return (tmp == SERVO_MOVING) ? true : false;
}

void servoMultiRun(Servo &s1, Servo &s2, Servo &s3, Servo &s4, Servo &s5,
                   Servo &s6, Servo &s7, Servo &s8, Servo &s9, Servo &s10,
                   Servo &s11, Servo &s12, Servo &s13, Servo &s14, Servo &s15,
                   Servo &s16, Servo &s17, Servo &s18, Servo &s19, Servo &s20,
                   Servo &s21, Servo &s22, Servo &s23, Servo &s24, Servo &s25,
                   Servo &s26, Servo &s27, Servo &s28, Servo &s29, Servo &s30,
                   Servo &s31, Servo &s32, Servo &s33, Servo &s34, Servo &s35,
                   Servo &s36, Servo &s37, Servo &s38, Servo &s39, Servo &s40,
                   Servo &s41, Servo &s42, Servo &s43, Servo &s44, Servo &s45) {
		
		int i;
		
		if(&s1 == &nullServo) // no input servo class
		{
			for(i=0; i<45; i++) {if(Servoptr[i] != NULL) Servoptr[i]->run();}
			return;
		}
		
		s1.run();  s2.run();  s3.run();  s4.run();  s5.run();  s6.run();  s7.run();
		s8.run();  s9.run();  s10.run(); s11.run(); s12.run(); s13.run(); s14.run();
		s15.run(); s16.run(); s17.run(); s18.run(); s19.run(); s20.run(); s21.run();
		s22.run(); s23.run(); s24.run(); s25.run(); s26.run(); s27.run(); s28.run();
		s29.run(); s30.run(); s31.run(); s32.run(); s33.run(); s34.run(); s35.run();
		s36.run(); s37.run(); s38.run(); s39.run(); s40.run(); s41.run(); s42.run();
		s43.run(); s44.run(); s45.run();
}


void servoMultiPause(Servo &s1, Servo &s2, Servo &s3, Servo &s4, Servo &s5,
                     Servo &s6, Servo &s7, Servo &s8, Servo &s9, Servo &s10,
                     Servo &s11, Servo &s12, Servo &s13, Servo &s14, Servo &s15,
                     Servo &s16, Servo &s17, Servo &s18, Servo &s19, Servo &s20,
                     Servo &s21, Servo &s22, Servo &s23, Servo &s24, Servo &s25,
                     Servo &s26, Servo &s27, Servo &s28, Servo &s29, Servo &s30,
                     Servo &s31, Servo &s32, Servo &s33, Servo &s34, Servo &s35,
                     Servo &s36, Servo &s37, Servo &s38, Servo &s39, Servo &s40,
                     Servo &s41, Servo &s42, Servo &s43, Servo &s44, Servo &s45) {
	
		int i;
		
		if(&s1 == &nullServo) // no input servo class
		{
			for(i=0; i<45; i++) {if(Servoptr[i] != NULL) Servoptr[i]->pause();}
			return;
		}
		
		s1.pause();  s2.pause();  s3.pause();  s4.pause();  s5.pause();  s6.pause();  s7.pause();
		s8.pause();  s9.pause();  s10.pause(); s11.pause(); s12.pause(); s13.pause(); s14.pause();
		s15.pause(); s16.pause(); s17.pause(); s18.pause(); s19.pause(); s20.pause(); s21.pause();
		s22.pause(); s23.pause(); s24.pause(); s25.pause(); s26.pause(); s27.pause(); s28.pause();
		s29.pause(); s30.pause(); s31.pause(); s32.pause(); s33.pause(); s34.pause(); s35.pause();
		s36.pause(); s37.pause(); s38.pause(); s39.pause(); s40.pause(); s41.pause(); s42.pause();
		s43.pause(); s44.pause(); s45.pause();
}


void servoMultiResume(Servo &s1, Servo &s2, Servo &s3, Servo &s4, Servo &s5,
                      Servo &s6, Servo &s7, Servo &s8, Servo &s9, Servo &s10,
                      Servo &s11, Servo &s12, Servo &s13, Servo &s14, Servo &s15,
                      Servo &s16, Servo &s17, Servo &s18, Servo &s19, Servo &s20,
                      Servo &s21, Servo &s22, Servo &s23, Servo &s24, Servo &s25,
                      Servo &s26, Servo &s27, Servo &s28, Servo &s29, Servo &s30,
                      Servo &s31, Servo &s32, Servo &s33, Servo &s34, Servo &s35,
                      Servo &s36, Servo &s37, Servo &s38, Servo &s39, Servo &s40,
                      Servo &s41, Servo &s42, Servo &s43, Servo &s44, Servo &s45) {
	
		int i;
		
		if(&s1 == &nullServo) // no input servo class
		{
			for(i=0; i<45; i++) {if(Servoptr[i] != NULL) Servoptr[i]->resume();}
			return;
		}
		
		s1.resume();  s2.resume();  s3.resume();  s4.resume();  s5.resume();  s6.resume();  s7.resume();
		s8.resume();  s9.resume();  s10.resume(); s11.resume(); s12.resume(); s13.resume(); s14.resume();
		s15.resume(); s16.resume(); s17.resume(); s18.resume(); s19.resume(); s20.resume(); s21.resume();
		s22.resume(); s23.resume(); s24.resume(); s25.resume(); s26.resume(); s27.resume(); s28.resume();
		s29.resume(); s30.resume(); s31.resume(); s32.resume(); s33.resume(); s34.resume(); s35.resume();
		s36.resume(); s37.resume(); s38.resume(); s39.resume(); s40.resume(); s41.resume(); s42.resume();
		s43.resume(); s44.resume(); s45.resume();
}


void servoMultiStop(Servo &s1, Servo &s2, Servo &s3, Servo &s4, Servo &s5,
                    Servo &s6, Servo &s7, Servo &s8, Servo &s9, Servo &s10,
                    Servo &s11, Servo &s12, Servo &s13, Servo &s14, Servo &s15,
                    Servo &s16, Servo &s17, Servo &s18, Servo &s19, Servo &s20,
                    Servo &s21, Servo &s22, Servo &s23, Servo &s24, Servo &s25,
                    Servo &s26, Servo &s27, Servo &s28, Servo &s29, Servo &s30,
                    Servo &s31, Servo &s32, Servo &s33, Servo &s34, Servo &s35,
                    Servo &s36, Servo &s37, Servo &s38, Servo &s39, Servo &s40,
                    Servo &s41, Servo &s42, Servo &s43, Servo &s44, Servo &s45) {

        int i;
		
		if(&s1 == &nullServo) // no input servo class
		{
			for(i=0; i<45; i++) {if(Servoptr[i] != NULL) Servoptr[i]->stop();}
			return;
		}
		
		s1.stop();  s2.stop();  s3.stop();  s4.stop();  s5.stop();  s6.stop();  s7.stop();
		s8.stop();  s9.stop();  s10.stop(); s11.stop(); s12.stop(); s13.stop(); s14.stop();
		s15.stop(); s16.stop(); s17.stop(); s18.stop(); s19.stop(); s20.stop(); s21.stop();
		s22.stop(); s23.stop(); s24.stop(); s25.stop(); s26.stop(); s27.stop(); s28.stop();
		s29.stop(); s30.stop(); s31.stop(); s32.stop(); s33.stop(); s34.stop(); s35.stop();
		s36.stop(); s37.stop(); s38.stop(); s39.stop(); s40.stop(); s41.stop(); s42.stop();
		s43.stop(); s44.stop(); s45.stop();
}


bool isServoMultiMoving(Servo &s1, Servo &s2, Servo &s3, Servo &s4, Servo &s5,
                        Servo &s6, Servo &s7, Servo &s8, Servo &s9, Servo &s10,
                        Servo &s11, Servo &s12, Servo &s13, Servo &s14, Servo &s15,
                        Servo &s16, Servo &s17, Servo &s18, Servo &s19, Servo &s20,
                        Servo &s21, Servo &s22, Servo &s23, Servo &s24, Servo &s25,
                        Servo &s26, Servo &s27, Servo &s28, Servo &s29, Servo &s30,
                        Servo &s31, Servo &s32, Servo &s33, Servo &s34, Servo &s35,
                        Servo &s36, Servo &s37, Servo &s38, Servo &s39, Servo &s40,
                        Servo &s41, Servo &s42, Servo &s43, Servo &s44, Servo &s45) {
	
	int i;
	int tmp[45];
	
	for(i=0; i<45; i++) tmp[i] = false;
		
	if(&s1 == &nullServo) // no input servo class
	{
		for(i=0; i<45; i++) {if(Servoptr[i] != NULL) tmp[i] = Servoptr[i]->isMoving();}
		goto END;
	}
		
	tmp[0]  = s1.isMoving();   tmp[1] = s2.isMoving();   tmp[2] = s3.isMoving();  tmp[3]  = s4.isMoving();  tmp[4]  = s5.isMoving();  tmp[5]  = s6.isMoving();  tmp[6]  = s7.isMoving();
	tmp[7]  = s8.isMoving();   tmp[8] = s9.isMoving();   tmp[9] = s10.isMoving(); tmp[10] = s11.isMoving(); tmp[11] = s12.isMoving(); tmp[12] = s13.isMoving(); tmp[13] = s14.isMoving();
	tmp[14] = s15.isMoving(); tmp[15] = s16.isMoving(); tmp[16] = s17.isMoving(); tmp[17] = s18.isMoving(); tmp[18] = s19.isMoving(); tmp[19] = s20.isMoving(); tmp[20] = s21.isMoving();
	tmp[21] = s22.isMoving(); tmp[22] = s23.isMoving(); tmp[23] = s24.isMoving(); tmp[24] = s25.isMoving(); tmp[25] = s26.isMoving(); tmp[26] = s27.isMoving(); tmp[27] = s28.isMoving();
	tmp[28] = s29.isMoving(); tmp[29] = s30.isMoving(); tmp[30] = s31.isMoving(); tmp[31] = s32.isMoving(); tmp[32] = s33.isMoving(); tmp[33] = s34.isMoving(); tmp[34] = s35.isMoving();
	tmp[35] = s36.isMoving(); tmp[36] = s37.isMoving(); tmp[37] = s38.isMoving(); tmp[38] = s39.isMoving(); tmp[39] = s40.isMoving(); tmp[40] = s41.isMoving(); tmp[41] = s42.isMoving();
	tmp[42] = s43.isMoving(); tmp[43] = s44.isMoving(); tmp[44] = s45.isMoving();

END:	
	for(i=0; i<45; i++)
		if(tmp[i] == true) return true;
		
	return false;
}

void servoMultiRealTimeMixing(long* mixoffsets, Servo &s1, Servo &s2, Servo &s3, Servo &s4, Servo &s5,
                              Servo &s6, Servo &s7, Servo &s8, Servo &s9, Servo &s10,
                              Servo &s11, Servo &s12, Servo &s13, Servo &s14, Servo &s15,
                              Servo &s16, Servo &s17, Servo &s18, Servo &s19, Servo &s20,
                              Servo &s21, Servo &s22, Servo &s23, Servo &s24, Servo &s25,
                              Servo &s26, Servo &s27, Servo &s28, Servo &s29, Servo &s30,
                              Servo &s31, Servo &s32, Servo &s33, Servo &s34, Servo &s35,
                              Servo &s36, Servo &s37, Servo &s38, Servo &s39, Servo &s40,
                              Servo &s41, Servo &s42, Servo &s43, Servo &s44, Servo &s45) {
   
	int i;
		
	if(&s1 == &nullServo) // no input servo class
	{
		io_DisableINT();
		for(i=0; i<45; i++) {if(Servoptr[i] != NULL && mixoffsets[i] != 0L) Servoptr[i]->setRealTimeMixing(mixoffsets[i], false);}
		io_RestoreINT();
		return;
	}
	
	io_DisableINT();
	s1.setRealTimeMixing(mixoffsets[s1.servoIndex], false); s2.setRealTimeMixing(mixoffsets[s2.servoIndex], false); s3.setRealTimeMixing(mixoffsets[s3.servoIndex], false);
	s4.setRealTimeMixing(mixoffsets[s4.servoIndex], false); s5.setRealTimeMixing(mixoffsets[s5.servoIndex], false); s6.setRealTimeMixing(mixoffsets[s6.servoIndex], false);
	s7.setRealTimeMixing(mixoffsets[s7.servoIndex], false); s8.setRealTimeMixing(mixoffsets[s8.servoIndex], false); s9.setRealTimeMixing(mixoffsets[s9.servoIndex], false);
	s10.setRealTimeMixing(mixoffsets[s10.servoIndex], false); s11.setRealTimeMixing(mixoffsets[s11.servoIndex], false); s12.setRealTimeMixing(mixoffsets[s12.servoIndex], false);
	s13.setRealTimeMixing(mixoffsets[s13.servoIndex], false); s14.setRealTimeMixing(mixoffsets[s14.servoIndex], false); s15.setRealTimeMixing(mixoffsets[s15.servoIndex], false);
	s16.setRealTimeMixing(mixoffsets[s16.servoIndex], false); s17.setRealTimeMixing(mixoffsets[s17.servoIndex], false); s18.setRealTimeMixing(mixoffsets[s18.servoIndex], false);
	s19.setRealTimeMixing(mixoffsets[s19.servoIndex], false); s20.setRealTimeMixing(mixoffsets[s20.servoIndex], false); s21.setRealTimeMixing(mixoffsets[s21.servoIndex], false);
	s22.setRealTimeMixing(mixoffsets[s22.servoIndex], false); s23.setRealTimeMixing(mixoffsets[s23.servoIndex], false); s24.setRealTimeMixing(mixoffsets[s24.servoIndex], false);
	s25.setRealTimeMixing(mixoffsets[s25.servoIndex], false); s26.setRealTimeMixing(mixoffsets[s26.servoIndex], false); s27.setRealTimeMixing(mixoffsets[s27.servoIndex], false);
	s28.setRealTimeMixing(mixoffsets[s28.servoIndex], false); s29.setRealTimeMixing(mixoffsets[s29.servoIndex], false); s30.setRealTimeMixing(mixoffsets[s30.servoIndex], false);
	s31.setRealTimeMixing(mixoffsets[s31.servoIndex], false); s32.setRealTimeMixing(mixoffsets[s32.servoIndex], false); s33.setRealTimeMixing(mixoffsets[s33.servoIndex], false);
	s34.setRealTimeMixing(mixoffsets[s34.servoIndex], false); s35.setRealTimeMixing(mixoffsets[s35.servoIndex], false); s36.setRealTimeMixing(mixoffsets[s36.servoIndex], false);
	s37.setRealTimeMixing(mixoffsets[s37.servoIndex], false); s38.setRealTimeMixing(mixoffsets[s38.servoIndex], false); s39.setRealTimeMixing(mixoffsets[s39.servoIndex], false);
	s40.setRealTimeMixing(mixoffsets[s40.servoIndex], false); s41.setRealTimeMixing(mixoffsets[s41.servoIndex], false); s42.setRealTimeMixing(mixoffsets[s42.servoIndex], false);
	s43.setRealTimeMixing(mixoffsets[s43.servoIndex], false); s44.setRealTimeMixing(mixoffsets[s44.servoIndex], false); s45.setRealTimeMixing(mixoffsets[s45.servoIndex], false);
	io_RestoreINT();
}

/******************************** ServoFrame **********************************/

ServoFrame::ServoFrame() {
	int i;
	
	for(i=0; i<45; i++) positions[i] = 0L;
	used_servos = 0x00001FFFFFFFFFFFLL;
}

ServoFrame::ServoFrame(const char* dir) {
	int i;
	
	for(i=0; i<45; i++) positions[i] = 0L;
	load(dir);
	used_servos = 0x00001FFFFFFFFFFFLL;
}

void ServoFrame::setPositions(Servo &s1, Servo &s2, Servo &s3, Servo &s4, Servo &s5,
                              Servo &s6, Servo &s7, Servo &s8, Servo &s9, Servo &s10,
                              Servo &s11, Servo &s12, Servo &s13, Servo &s14, Servo &s15,
                              Servo &s16, Servo &s17, Servo &s18, Servo &s19, Servo &s20,
                              Servo &s21, Servo &s22, Servo &s23, Servo &s24, Servo &s25,
                              Servo &s26, Servo &s27, Servo &s28, Servo &s29, Servo &s30,
                              Servo &s31, Servo &s32, Servo &s33, Servo &s34, Servo &s35,
                              Servo &s36, Servo &s37, Servo &s38, Servo &s39, Servo &s40,
                              Servo &s41, Servo &s42, Servo &s43, Servo &s44, Servo &s45) {

    int i;
		
	if(&s1 == &nullServo) // no input servo class
	{
		for(i=0; i<45; i++) {if(Servoptr[i] != NULL && positions[i] != 0L) Servoptr[i]->setPosition(positions[i]);}
		return;
	}
		
	s1.setPosition(positions[s1.servoIndex]); s2.setPosition(positions[s2.servoIndex]); s3.setPosition(positions[s3.servoIndex]);
	s4.setPosition(positions[s4.servoIndex]); s5.setPosition(positions[s5.servoIndex]); s6.setPosition(positions[s6.servoIndex]);
	s7.setPosition(positions[s7.servoIndex]); s8.setPosition(positions[s8.servoIndex]); s9.setPosition(positions[s9.servoIndex]);
	s10.setPosition(positions[s10.servoIndex]); s11.setPosition(positions[s11.servoIndex]); s12.setPosition(positions[s12.servoIndex]);
	s13.setPosition(positions[s13.servoIndex]); s14.setPosition(positions[s14.servoIndex]); s15.setPosition(positions[s15.servoIndex]);
	s16.setPosition(positions[s16.servoIndex]); s17.setPosition(positions[s17.servoIndex]); s18.setPosition(positions[s18.servoIndex]);
	s19.setPosition(positions[s19.servoIndex]); s20.setPosition(positions[s20.servoIndex]); s21.setPosition(positions[s21.servoIndex]);
	s22.setPosition(positions[s22.servoIndex]); s23.setPosition(positions[s23.servoIndex]); s24.setPosition(positions[s24.servoIndex]);
	s25.setPosition(positions[s25.servoIndex]); s26.setPosition(positions[s26.servoIndex]); s27.setPosition(positions[s27.servoIndex]);
	s28.setPosition(positions[s28.servoIndex]); s29.setPosition(positions[s29.servoIndex]); s30.setPosition(positions[s30.servoIndex]);
	s31.setPosition(positions[s31.servoIndex]); s32.setPosition(positions[s32.servoIndex]); s33.setPosition(positions[s33.servoIndex]);
	s34.setPosition(positions[s34.servoIndex]); s35.setPosition(positions[s35.servoIndex]); s36.setPosition(positions[s36.servoIndex]);
	s37.setPosition(positions[s37.servoIndex]); s38.setPosition(positions[s38.servoIndex]); s39.setPosition(positions[s39.servoIndex]);
	s40.setPosition(positions[s40.servoIndex]); s41.setPosition(positions[s41.servoIndex]); s42.setPosition(positions[s42.servoIndex]);
	s43.setPosition(positions[s43.servoIndex]); s44.setPosition(positions[s44.servoIndex]); s45.setPosition(positions[s45.servoIndex]);
}

void ServoFrame::setPositions(unsigned long playtime, Servo &s1, Servo &s2, Servo &s3, Servo &s4, Servo &s5,
                              Servo &s6, Servo &s7, Servo &s8, Servo &s9, Servo &s10,
                              Servo &s11, Servo &s12, Servo &s13, Servo &s14, Servo &s15,
                              Servo &s16, Servo &s17, Servo &s18, Servo &s19, Servo &s20,
                              Servo &s21, Servo &s22, Servo &s23, Servo &s24, Servo &s25,
                              Servo &s26, Servo &s27, Servo &s28, Servo &s29, Servo &s30,
                              Servo &s31, Servo &s32, Servo &s33, Servo &s34, Servo &s35,
                              Servo &s36, Servo &s37, Servo &s38, Servo &s39, Servo &s40,
                              Servo &s41, Servo &s42, Servo &s43, Servo &s44, Servo &s45) {

    int i;
		
	if(&s1 == &nullServo) // no input servo class
	{
		for(i=0; i<45; i++) {if(Servoptr[i] != NULL && positions[i] != 0L) Servoptr[i]->setPosition(positions[i], playtime);}
		return;
	}
		
	s1.setPosition(positions[s1.servoIndex], playtime); s2.setPosition(positions[s2.servoIndex], playtime); s3.setPosition(positions[s3.servoIndex], playtime);
	s4.setPosition(positions[s4.servoIndex], playtime); s5.setPosition(positions[s5.servoIndex], playtime); s6.setPosition(positions[s6.servoIndex], playtime);
	s7.setPosition(positions[s7.servoIndex], playtime); s8.setPosition(positions[s8.servoIndex], playtime); s9.setPosition(positions[s9.servoIndex], playtime);
	s10.setPosition(positions[s10.servoIndex], playtime); s11.setPosition(positions[s11.servoIndex], playtime); s12.setPosition(positions[s12.servoIndex], playtime);
	s13.setPosition(positions[s13.servoIndex], playtime); s14.setPosition(positions[s14.servoIndex], playtime); s15.setPosition(positions[s15.servoIndex], playtime);
	s16.setPosition(positions[s16.servoIndex], playtime); s17.setPosition(positions[s17.servoIndex], playtime); s18.setPosition(positions[s18.servoIndex], playtime);
	s19.setPosition(positions[s19.servoIndex], playtime); s20.setPosition(positions[s20.servoIndex], playtime); s21.setPosition(positions[s21.servoIndex], playtime);
	s22.setPosition(positions[s22.servoIndex], playtime); s23.setPosition(positions[s23.servoIndex], playtime); s24.setPosition(positions[s24.servoIndex], playtime);
	s25.setPosition(positions[s25.servoIndex], playtime); s26.setPosition(positions[s26.servoIndex], playtime); s27.setPosition(positions[s27.servoIndex], playtime);
	s28.setPosition(positions[s28.servoIndex], playtime); s29.setPosition(positions[s29.servoIndex], playtime); s30.setPosition(positions[s30.servoIndex], playtime);
	s31.setPosition(positions[s31.servoIndex], playtime); s32.setPosition(positions[s32.servoIndex], playtime); s33.setPosition(positions[s33.servoIndex], playtime);
	s34.setPosition(positions[s34.servoIndex], playtime); s35.setPosition(positions[s35.servoIndex], playtime); s36.setPosition(positions[s36.servoIndex], playtime);
	s37.setPosition(positions[s37.servoIndex], playtime); s38.setPosition(positions[s38.servoIndex], playtime); s39.setPosition(positions[s39.servoIndex], playtime);
	s40.setPosition(positions[s40.servoIndex], playtime); s41.setPosition(positions[s41.servoIndex], playtime); s42.setPosition(positions[s42.servoIndex], playtime);
	s43.setPosition(positions[s43.servoIndex], playtime); s44.setPosition(positions[s44.servoIndex], playtime); s45.setPosition(positions[s45.servoIndex], playtime);
}

void ServoFrame::setPositions(int playtime, Servo &s1, Servo &s2, Servo &s3, Servo &s4, Servo &s5,
                              Servo &s6, Servo &s7, Servo &s8, Servo &s9, Servo &s10,
                              Servo &s11, Servo &s12, Servo &s13, Servo &s14, Servo &s15,
                              Servo &s16, Servo &s17, Servo &s18, Servo &s19, Servo &s20,
                              Servo &s21, Servo &s22, Servo &s23, Servo &s24, Servo &s25,
                              Servo &s26, Servo &s27, Servo &s28, Servo &s29, Servo &s30,
                              Servo &s31, Servo &s32, Servo &s33, Servo &s34, Servo &s35,
                              Servo &s36, Servo &s37, Servo &s38, Servo &s39, Servo &s40,
                              Servo &s41, Servo &s42, Servo &s43, Servo &s44, Servo &s45) {
	if(playtime <= 0) playtime = 0;
	setPositions((unsigned long) playtime, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20,
	              s21, s22, s23, s24, s25, s26, s27, s28, s29, s30, s31, s32, s33, s34, s35, s36, s37, s38, s39, s40,
	              s41, s42, s43, s44, s45);					  
}

static void get_real_path(const char* dir, char* path) {
    int i;
	const char* parent = "C:\\";
	
	if(dir[0] == 'c' || dir[0] == 'C' || dir[0] == 'a' || dir[0] == 'A')
	{
		if(dir[1] == ':' && dir[2] == '\\')
		{
			for(i=0; dir[i] != '\0'; i++) path[i] = dir[i];
			return;
		}
	}

	for(i=0; i<3; i++) path[i] = parent[i];
	for(i=0; dir[i] != '\0'; i++) path[i+3] = dir[i];

	return;
}

static bool analysis(const char* line, int* channel, long* value) {
	int i, j = 0;
	char tmp[256] = {'\0'};
	bool negative = false;
	
	for(i=0; line[i] != '\0' && line[i] != '\n'; i++)
	{
		if(line[i] == ';') break;
		if(line[i] != ' ') tmp[j++] = line[i];
	}
	
	if(strncmp("channel", tmp, 7) != 0) return false;

	if(isdigit(tmp[7]))
	{
		if(tmp[8] == '=')
		{
			*channel = tmp[7] - '0'; i = 9;
		}			
		else if(isdigit(tmp[8]) && tmp[9] == '=')
		{
			*channel = (tmp[7] - '0')*10 + (tmp[8] - '0'); i = 10;
		}
		else
			return false;
	}
	else
		return false;
	
	*value = 0;
	if(tmp[i] == '-') {negative = true; i++;}	
	for(;isdigit(tmp[i]) != 0; i++) *value = *value*10 + (tmp[i] - '0');
	
	if(tmp[i] != '\0') return false;
	if(negative == true) *value = (-1) * (*value);
	
	return true;
}

bool ServoFrame::load(const char* dir) {
	int i, channel;
	FILE *fp;
	char path[256] = {'\0'};
	char _line[256] = {'\0'};
	long tmp[45] = {0L}, value;
	
	if(dir == NULL) return false;

	get_real_path(dir, path);

	if((fp = fopen(path, "r")) == NULL) return false;

	while(fgets(_line, 256, fp))
	{
		for(i=0; _line[i] != '\0' && _line[i] != '\n'; i++)
			if(_line[i] == ' ') continue; else break;

		if(_line[i] == ';' || _line[i] == '\0' || _line[i] == '\n') continue;
		
		if(analysis(_line, &channel, &value) == true && channel < 45)
			tmp[channel] = value;
		else
		{
			fclose(fp);
			return false;
		}
	}
	
	fclose(fp);
	
	for(i=0; i<45; i++) positions[i] = tmp[i];

	return true;
}

bool ServoFrame::load(const String &s) {
	load(s.c_str());
}

void combine(char* result, int channel, long value) {
	int i, j;
	const char* head = "channel";
	char tmp[256] = {'\0'};
	char num[16]  = {'\0'};
	
	for(i=0; i<7; i++) tmp[i] = head[i];
	itoa(channel, num, 10);
	for(j=0; num[j] != '\0'; j++) tmp[i++] = num[j];
	tmp[i++] = '=';
	for(j=0; j<16; j++) num[j] = '\0';
	itoa(value, num, 10);
	for(j = 0; num[j] != '\0'; j++) tmp[i++] = num[j];
	tmp[i] = '\n';
	
	strncpy(result, tmp, 256);
	//printf("%s\n", result);
}

bool ServoFrame::save(const char* dir) {
    int i;
	FILE *fp;
	char path[256] = {'\0'};
	char _line[256] = {'\0'};
	
	if(dir == NULL) return false;

	get_real_path(dir, path);
	
	if((fp = fopen(path, "a")) == NULL) return false;
	
	for(i=0; i<45; i++)
	{
		if(positions[i] == 0L) continue;
		combine(_line, i, positions[i]);
		fputs(_line, fp);
	}
	fputs("\n", fp);
	fclose(fp);
	return true;
}

bool ServoFrame::save(const String &s) {
	save(s.c_str());
}

void ServoFrame::playPositions(Servo &s1, Servo &s2, Servo &s3, Servo &s4, Servo &s5,
                               Servo &s6, Servo &s7, Servo &s8, Servo &s9, Servo &s10,
                               Servo &s11, Servo &s12, Servo &s13, Servo &s14, Servo &s15,
                               Servo &s16, Servo &s17, Servo &s18, Servo &s19, Servo &s20,
                               Servo &s21, Servo &s22, Servo &s23, Servo &s24, Servo &s25,
                               Servo &s26, Servo &s27, Servo &s28, Servo &s29, Servo &s30,
                               Servo &s31, Servo &s32, Servo &s33, Servo &s34, Servo &s35,
                               Servo &s36, Servo &s37, Servo &s38, Servo &s39, Servo &s40,
                               Servo &s41, Servo &s42, Servo &s43, Servo &s44, Servo &s45) {
                   
	int i;
		
	if(&s1 == &nullServo) // no input servo class
	{  
		for(i=0; i<45; i++) {if(Servoptr[i] != NULL && positions[i] != 0L && (used_servos & (0x01LL<<i)) != 0LL) Servoptr[i]->setPosition(positions[i]);}
		for(i=0; i<45; i++) {if(Servoptr[i] != NULL && positions[i] != 0L && (used_servos & (0x01LL<<i)) != 0LL) Servoptr[i]->run();}
		return;
	}

	s1.setPosition(positions[s1.servoIndex]); s2.setPosition(positions[s2.servoIndex]); s3.setPosition(positions[s3.servoIndex]);
	s4.setPosition(positions[s4.servoIndex]); s5.setPosition(positions[s5.servoIndex]); s6.setPosition(positions[s6.servoIndex]);
	s7.setPosition(positions[s7.servoIndex]); s8.setPosition(positions[s8.servoIndex]); s9.setPosition(positions[s9.servoIndex]);
	s10.setPosition(positions[s10.servoIndex]); s11.setPosition(positions[s11.servoIndex]); s12.setPosition(positions[s12.servoIndex]);
	s13.setPosition(positions[s13.servoIndex]); s14.setPosition(positions[s14.servoIndex]); s15.setPosition(positions[s15.servoIndex]);
	s16.setPosition(positions[s16.servoIndex]); s17.setPosition(positions[s17.servoIndex]); s18.setPosition(positions[s18.servoIndex]);
	s19.setPosition(positions[s19.servoIndex]); s20.setPosition(positions[s20.servoIndex]); s21.setPosition(positions[s21.servoIndex]);
	s22.setPosition(positions[s22.servoIndex]); s23.setPosition(positions[s23.servoIndex]); s24.setPosition(positions[s24.servoIndex]);
	s25.setPosition(positions[s25.servoIndex]); s26.setPosition(positions[s26.servoIndex]); s27.setPosition(positions[s27.servoIndex]);
	s28.setPosition(positions[s28.servoIndex]); s29.setPosition(positions[s29.servoIndex]); s30.setPosition(positions[s30.servoIndex]);
	s31.setPosition(positions[s31.servoIndex]); s32.setPosition(positions[s32.servoIndex]); s33.setPosition(positions[s33.servoIndex]);
	s34.setPosition(positions[s34.servoIndex]); s35.setPosition(positions[s35.servoIndex]); s36.setPosition(positions[s36.servoIndex]);
	s37.setPosition(positions[s37.servoIndex]); s38.setPosition(positions[s38.servoIndex]); s39.setPosition(positions[s39.servoIndex]);
	s40.setPosition(positions[s40.servoIndex]); s41.setPosition(positions[s41.servoIndex]); s42.setPosition(positions[s42.servoIndex]);
	s43.setPosition(positions[s43.servoIndex]); s44.setPosition(positions[s44.servoIndex]); s45.setPosition(positions[s45.servoIndex]);
    servoMultiRun(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20,
	              s21, s22, s23, s24, s25, s26, s27, s28, s29, s30, s31, s32, s33, s34, s35, s36, s37, s38, s39, s40,
	              s41, s42, s43, s44, s45);
}

void ServoFrame::playPositions(unsigned long long enabled_servos, Servo &s1, Servo &s2, Servo &s3, Servo &s4, Servo &s5,
                               Servo &s6, Servo &s7, Servo &s8, Servo &s9, Servo &s10,
                               Servo &s11, Servo &s12, Servo &s13, Servo &s14, Servo &s15,
                               Servo &s16, Servo &s17, Servo &s18, Servo &s19, Servo &s20,
                               Servo &s21, Servo &s22, Servo &s23, Servo &s24, Servo &s25,
                               Servo &s26, Servo &s27, Servo &s28, Servo &s29, Servo &s30,
                               Servo &s31, Servo &s32, Servo &s33, Servo &s34, Servo &s35,
                               Servo &s36, Servo &s37, Servo &s38, Servo &s39, Servo &s40,
                               Servo &s41, Servo &s42, Servo &s43, Servo &s44, Servo &s45) {
	used_servos = enabled_servos;
	playPositions(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20,
	              s21, s22, s23, s24, s25, s26, s27, s28, s29, s30, s31, s32, s33, s34, s35, s36, s37, s38, s39, s40,
	              s41, s42, s43, s44, s45);
	used_servos = 0x00001FFFFFFFFFFFLL;
}

void ServoFrame::playPositions(unsigned long playtime, Servo &s1, Servo &s2, Servo &s3, Servo &s4, Servo &s5,
                               Servo &s6, Servo &s7, Servo &s8, Servo &s9, Servo &s10,
                               Servo &s11, Servo &s12, Servo &s13, Servo &s14, Servo &s15,
                               Servo &s16, Servo &s17, Servo &s18, Servo &s19, Servo &s20,
                               Servo &s21, Servo &s22, Servo &s23, Servo &s24, Servo &s25,
                               Servo &s26, Servo &s27, Servo &s28, Servo &s29, Servo &s30,
                               Servo &s31, Servo &s32, Servo &s33, Servo &s34, Servo &s35,
                               Servo &s36, Servo &s37, Servo &s38, Servo &s39, Servo &s40,
                               Servo &s41, Servo &s42, Servo &s43, Servo &s44, Servo &s45) {
                               
	int i;

	if(&s1 == &nullServo) // no input servo class
	{
		for(i=0; i<45; i++) {if(Servoptr[i] != NULL && positions[i] != 0L && (used_servos & (0x01LL<<i)) != 0LL) Servoptr[i]->setPosition(positions[i], playtime);}
		for(i=0; i<45; i++) {if(Servoptr[i] != NULL && positions[i] != 0L && (used_servos & (0x01LL<<i)) != 0LL) Servoptr[i]->run();}
		return;
	}

	s1.setPosition(positions[s1.servoIndex], playtime); s2.setPosition(positions[s2.servoIndex], playtime); s3.setPosition(positions[s3.servoIndex], playtime);
	s4.setPosition(positions[s4.servoIndex], playtime); s5.setPosition(positions[s5.servoIndex], playtime); s6.setPosition(positions[s6.servoIndex], playtime);
	s7.setPosition(positions[s7.servoIndex], playtime); s8.setPosition(positions[s8.servoIndex], playtime); s9.setPosition(positions[s9.servoIndex], playtime);
	s10.setPosition(positions[s10.servoIndex], playtime); s11.setPosition(positions[s11.servoIndex], playtime); s12.setPosition(positions[s12.servoIndex], playtime);
	s13.setPosition(positions[s13.servoIndex], playtime); s14.setPosition(positions[s14.servoIndex], playtime); s15.setPosition(positions[s15.servoIndex], playtime);
	s16.setPosition(positions[s16.servoIndex], playtime); s17.setPosition(positions[s17.servoIndex], playtime); s18.setPosition(positions[s18.servoIndex], playtime);
	s19.setPosition(positions[s19.servoIndex], playtime); s20.setPosition(positions[s20.servoIndex], playtime); s21.setPosition(positions[s21.servoIndex], playtime);
	s22.setPosition(positions[s22.servoIndex], playtime); s23.setPosition(positions[s23.servoIndex], playtime); s24.setPosition(positions[s24.servoIndex], playtime);
	s25.setPosition(positions[s25.servoIndex], playtime); s26.setPosition(positions[s26.servoIndex], playtime); s27.setPosition(positions[s27.servoIndex], playtime);
	s28.setPosition(positions[s28.servoIndex], playtime); s29.setPosition(positions[s29.servoIndex], playtime); s30.setPosition(positions[s30.servoIndex], playtime);
	s31.setPosition(positions[s31.servoIndex], playtime); s32.setPosition(positions[s32.servoIndex], playtime); s33.setPosition(positions[s33.servoIndex], playtime);
	s34.setPosition(positions[s34.servoIndex], playtime); s35.setPosition(positions[s35.servoIndex], playtime); s36.setPosition(positions[s36.servoIndex], playtime);
	s37.setPosition(positions[s37.servoIndex], playtime); s38.setPosition(positions[s38.servoIndex], playtime); s39.setPosition(positions[s39.servoIndex], playtime);
	s40.setPosition(positions[s40.servoIndex], playtime); s41.setPosition(positions[s41.servoIndex], playtime); s42.setPosition(positions[s42.servoIndex], playtime);
	s43.setPosition(positions[s43.servoIndex], playtime); s44.setPosition(positions[s44.servoIndex], playtime); s45.setPosition(positions[s45.servoIndex], playtime);
    servoMultiRun(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20,
	              s21, s22, s23, s24, s25, s26, s27, s28, s29, s30, s31, s32, s33, s34, s35, s36, s37, s38, s39, s40,
	              s41, s42, s43, s44, s45);
}

void ServoFrame::playPositions(int playtime, Servo &s1, Servo &s2, Servo &s3, Servo &s4, Servo &s5,
                               Servo &s6, Servo &s7, Servo &s8, Servo &s9, Servo &s10,
                               Servo &s11, Servo &s12, Servo &s13, Servo &s14, Servo &s15,
                               Servo &s16, Servo &s17, Servo &s18, Servo &s19, Servo &s20,
                               Servo &s21, Servo &s22, Servo &s23, Servo &s24, Servo &s25,
                               Servo &s26, Servo &s27, Servo &s28, Servo &s29, Servo &s30,
                               Servo &s31, Servo &s32, Servo &s33, Servo &s34, Servo &s35,
                               Servo &s36, Servo &s37, Servo &s38, Servo &s39, Servo &s40,
                               Servo &s41, Servo &s42, Servo &s43, Servo &s44, Servo &s45) {
	if(playtime <= 0) playtime = 0;
	playPositions((unsigned long) playtime, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20,
	              s21, s22, s23, s24, s25, s26, s27, s28, s29, s30, s31, s32, s33, s34, s35, s36, s37, s38, s39, s40,
	              s41, s42, s43, s44, s45);
}

void ServoFrame::playPositions(unsigned long playtime, unsigned long long enabled_servos, Servo &s1, Servo &s2, Servo &s3, Servo &s4, Servo &s5,
                               Servo &s6, Servo &s7, Servo &s8, Servo &s9, Servo &s10,
                               Servo &s11, Servo &s12, Servo &s13, Servo &s14, Servo &s15,
                               Servo &s16, Servo &s17, Servo &s18, Servo &s19, Servo &s20,
                               Servo &s21, Servo &s22, Servo &s23, Servo &s24, Servo &s25,
                               Servo &s26, Servo &s27, Servo &s28, Servo &s29, Servo &s30,
                               Servo &s31, Servo &s32, Servo &s33, Servo &s34, Servo &s35,
                               Servo &s36, Servo &s37, Servo &s38, Servo &s39, Servo &s40,
                               Servo &s41, Servo &s42, Servo &s43, Servo &s44, Servo &s45) {
	used_servos = enabled_servos;
	playPositions(playtime, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20,
	              s21, s22, s23, s24, s25, s26, s27, s28, s29, s30, s31, s32, s33, s34, s35, s36, s37, s38, s39, s40,
	              s41, s42, s43, s44, s45);
	used_servos = 0x00001FFFFFFFFFFFLL;
}

void ServoFrame::playPositions(int playtime, unsigned long long enabled_servos, Servo &s1, Servo &s2, Servo &s3, Servo &s4, Servo &s5,
                               Servo &s6, Servo &s7, Servo &s8, Servo &s9, Servo &s10,
                               Servo &s11, Servo &s12, Servo &s13, Servo &s14, Servo &s15,
                               Servo &s16, Servo &s17, Servo &s18, Servo &s19, Servo &s20,
                               Servo &s21, Servo &s22, Servo &s23, Servo &s24, Servo &s25,
                               Servo &s26, Servo &s27, Servo &s28, Servo &s29, Servo &s30,
                               Servo &s31, Servo &s32, Servo &s33, Servo &s34, Servo &s35,
                               Servo &s36, Servo &s37, Servo &s38, Servo &s39, Servo &s40,
                               Servo &s41, Servo &s42, Servo &s43, Servo &s44, Servo &s45) {
	if(playtime <= 0) playtime = 0;
	used_servos = enabled_servos;
	playPositions((unsigned long) playtime, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20,
	              s21, s22, s23, s24, s25, s26, s27, s28, s29, s30, s31, s32, s33, s34, s35, s36, s37, s38, s39, s40,
	              s41, s42, s43, s44, s45);
	used_servos = 0x00001FFFFFFFFFFFLL;
}

/******************************* End of Servo Frame ***************************/


/********************************* Servo Offset *******************************/
ServoOffset::ServoOffset() {
	int i;
	
	for(i=0; i<45; i++) offsets[i] = 0L;
}

ServoOffset::ServoOffset(const char* dir) {
	int i;
	
	for(i=0; i<45; i++) offsets[i] = 0L;
	load(dir);
}

void ServoOffset::setOffsets(Servo &s1, Servo &s2, Servo &s3, Servo &s4, Servo &s5,
                             Servo &s6, Servo &s7, Servo &s8, Servo &s9, Servo &s10,
                             Servo &s11, Servo &s12, Servo &s13, Servo &s14, Servo &s15,
                             Servo &s16, Servo &s17, Servo &s18, Servo &s19, Servo &s20,
                             Servo &s21, Servo &s22, Servo &s23, Servo &s24, Servo &s25,
                             Servo &s26, Servo &s27, Servo &s28, Servo &s29, Servo &s30,
                             Servo &s31, Servo &s32, Servo &s33, Servo &s34, Servo &s35,
                             Servo &s36, Servo &s37, Servo &s38, Servo &s39, Servo &s40,
                             Servo &s41, Servo &s42, Servo &s43, Servo &s44, Servo &s45) {
    
	int i;
		
	if(&s1 == &nullServo) // no input servo class
	{
		for(i=0; i<45; i++) {if(Servoptr[i] != NULL && offsets[i] != 0L) Servoptr[i]->setOffset(offsets[i]);}
		return;
	}
		
	s1.setOffset(offsets[s1.servoIndex]); s2.setOffset(offsets[s2.servoIndex]); s3.setOffset(offsets[s3.servoIndex]);
	s4.setOffset(offsets[s4.servoIndex]); s5.setOffset(offsets[s5.servoIndex]); s6.setOffset(offsets[s6.servoIndex]);
	s7.setOffset(offsets[s7.servoIndex]); s8.setOffset(offsets[s8.servoIndex]); s9.setOffset(offsets[s9.servoIndex]);
	s10.setOffset(offsets[s10.servoIndex]); s11.setOffset(offsets[s11.servoIndex]); s12.setOffset(offsets[s12.servoIndex]);
	s13.setOffset(offsets[s13.servoIndex]); s14.setOffset(offsets[s14.servoIndex]); s15.setOffset(offsets[s15.servoIndex]);
	s16.setOffset(offsets[s16.servoIndex]); s17.setOffset(offsets[s17.servoIndex]); s18.setOffset(offsets[s18.servoIndex]);
	s19.setOffset(offsets[s19.servoIndex]); s20.setOffset(offsets[s20.servoIndex]); s21.setOffset(offsets[s21.servoIndex]);
	s22.setOffset(offsets[s22.servoIndex]); s23.setOffset(offsets[s23.servoIndex]); s24.setOffset(offsets[s24.servoIndex]);
	s25.setOffset(offsets[s25.servoIndex]); s26.setOffset(offsets[s26.servoIndex]); s27.setOffset(offsets[s27.servoIndex]);
	s28.setOffset(offsets[s28.servoIndex]); s29.setOffset(offsets[s29.servoIndex]); s30.setOffset(offsets[s30.servoIndex]);
	s31.setOffset(offsets[s31.servoIndex]); s32.setOffset(offsets[s32.servoIndex]); s33.setOffset(offsets[s33.servoIndex]);
	s34.setOffset(offsets[s34.servoIndex]); s35.setOffset(offsets[s35.servoIndex]); s36.setOffset(offsets[s36.servoIndex]);
	s37.setOffset(offsets[s37.servoIndex]); s38.setOffset(offsets[s38.servoIndex]); s39.setOffset(offsets[s39.servoIndex]);
	s40.setOffset(offsets[s40.servoIndex]); s41.setOffset(offsets[s41.servoIndex]); s42.setOffset(offsets[s42.servoIndex]);
	s43.setOffset(offsets[s43.servoIndex]); s44.setOffset(offsets[s44.servoIndex]); s45.setOffset(offsets[s45.servoIndex]);
}

bool ServoOffset::load(const char* dir) {
	int i, channel;
	FILE *fp;
	char path[256] = {'\0'};
	char _line[256] = {'\0'};
	long tmp[45] = {0L}, value;
	
	if(dir == NULL) return false;    

	get_real_path(dir, path);

	if((fp = fopen(path, "r")) == NULL) return false;

	while(fgets(_line, 256, fp))
	{
		for(i=0; _line[i] != '\0' && _line[i] != '\n'; i++)
			if(_line[i] == ' ') continue; else break;

		if(_line[i] == ';' || _line[i] == '\0' || _line[i] == '\n') continue;
		
		if(analysis(_line, &channel, &value) == true && channel < 45)
			tmp[channel] = value;
		else
		{
			fclose(fp);
			return false;
		}
	}
	
	fclose(fp);
	
	for(i=0; i<45; i++) offsets[i] = tmp[i];

	return true;
}

bool ServoOffset::load(const String &s) {
	load(s.c_str());
}

bool ServoOffset::save(const char* dir) {
    int i;
	FILE *fp;
	char path[256] = {'\0'};
	char _line[256] = {'\0'};
	
	if(dir == NULL) return false;

	get_real_path(dir, path);
	
	if((fp = fopen(path, "a")) == NULL) return false;
	
	for(i=0; i<45; i++)
	{
		if(offsets[i] == 0L) continue;
		combine(_line, i, offsets[i]);
		fputs(_line, fp);
	}
	fputs("\n", fp);
	fclose(fp);
	return true;
}

bool ServoOffset::save(const String &s) {
	save(s.c_str());
}

/******************************* End of Servo Offset **************************/



/***************************** Other Servo Controler **************************/

ServoFrameInno::ServoFrameInno() : ServoFrame() {
	int i, j;
	
	frameno = 0; M1ID = -1; M2ID = -1; 
	for(i=0; i<32; i++)
	{
		en[i] = 0;
		time[i] = 0L;
		speed[i] = 0L;
		for(j=0; j<10; j++) mode[i][j] = '\0';
	}
	
	for(i=0; i<173; i++) frmfile[i] = 0L;
	
	for(i=0; i<2; i++)
	for(j=0; j<256; j++)
		all_string[i][j] = '\0';
}

ServoFrameInno::ServoFrameInno(const char* dir) : ServoFrame() {
    int i, j;
	
	frameno = 0; M1ID = -1; M2ID = -1; 
	for(i=0; i<32; i++)
	{
		en[i] = 0;
		time[i] = 0L;
		speed[i] = 0L;
		for(j=0; j<10; j++) mode[i][j] = '\0';
	}
	
	for(i=0; i<173; i++) frmfile[i] = 0L;
	
	for(i=0; i<2; i++)
	for(j=0; j<256; j++)
		all_string[i][j] = '\0';
	
	load(dir);
}

void clear_string_space(const char* src, char* result) {
    int i, j = 0;
	char tmp[256] = {'\0'};
	
	for(i=0; src[i] != '\0' && src[i] != '\n'; i++)
		if(src[i] != ' ') tmp[j++] = src[i];
	
	for(i=0; tmp[i] != '\0'; i++) result[i] = tmp[i];
	result[i] = '\0';
	//printf("%s\n", result);
}

bool get_inno_ch(const char* src, int *value) {
	int i;
	char *ch = "ch";
	long tmp = 0L;
	
	if(src[0] != ch[0] || src[1] != ch[1]) return false;

	for(i=0; i<2; i++)
	{
		if(isdigit(src[i+2]))
			tmp = tmp * 10L + (src[i+2] - '0');
		else
			return false;
	}
	if(src[i+2] != '=') return false;
	*value = tmp;
	//printf("ch:%d ", *value);
	return true;
}

bool get_inno_chen(const char* src, char *is_enable) {
	char *target = "=en:";
	char *result;

	if((result = strstr(src, target)) == NULL) return false;
	
	if(result[4] == '0') *is_enable = 0;
	else if(result[4] == '1') *is_enable = 1;
	else *is_enable = 0;
	
	return true;
}

bool get_inno_pos(const char* src, long *pos) {
    int i;
	char *target = ",pos:";
	char *result;
	long tmp = 0L;

	if((result = strstr(src, target)) == NULL) return false;

	for(i=0; isdigit(result[i+5]); i++)
		tmp = tmp * 10L + (result[i+5] - '0');
	if(result[i+5] != ',') return false;

	*pos = tmp;
	//printf("pos:%ld ", *pos);
	return true;
}

bool get_inno_mode(const char* src, char *mode) {
    int i;
	char *target = ",mode:";
	char *result;
	long tmp = 0L;

	if((result = strstr(src, target)) == NULL) return false;

	for(i=0; result[i+6] != ','; i++)
		mode[i] = result[i+6];

	// modes: full, time or speed
	if(i>5) return false;
	
	//printf("mode:%s ", mode);
	return true;
}

bool get_inno_speed(const char* src, long *speed) {
    int i;
	char *target = ",speed:";
	char *result;
	long tmp = 0L;

	if((result = strstr(src, target)) == NULL) return false;

	for(i=0; isdigit(result[i+7]); i++)
		tmp = tmp * 10L + (result[i+7] - '0');
	if(result[i+7] != ',') return false;

	*speed = tmp;
	//printf("speed:%ld ", *speed);
	return true;
}

bool get_inno_time(const char* src, long *_time) {
    int i;
	char *target = ",time:";
	char *result;
	long tmp = 0L;

	if((result = strstr(src, target)) == NULL) return false;

	for(i=0; isdigit(result[i+6]); i++)
		tmp = tmp * 10L + (result[i+6] - '0');
	if(result[i+6] != ' ' && result[i+6] != '\n' && result[i+6] != '\0') return false;

	*_time = tmp;
	//printf("time:%ld \n", *_time);
	return true;
}

bool inno_frame_analysis(const char* line, long* value, char* is_enable, long* _time, long* speed, char* mode) {
    int channel = 0;
    
	if(get_inno_ch(line, &channel) == false) return false;
    if(get_inno_chen(line, is_enable) == false) return false;
    if(get_inno_pos(line, value) == false) return false;
    if(get_inno_mode(line, mode) == false) return false;
    if(get_inno_speed(line, speed) == false) return false;
    if(get_inno_time(line, _time) == false) return false;
    
    return true;
}

bool is_innoFrameFile(char* _line, FILE* fp, int* frmno, int* m1id, int* m2id) {
    int i, tmp = 0;
    char result[256] = {'\0'};
    
	if(fgets(_line, 256, fp))
	{
		clear_string_space(_line, result);
		if(strncmp("[ID]", result, 4) != 0) {printf("[ID] false\n"); return false;}
	}
	else
		return false;
	
	if(fgets(_line, 256, fp))
	{
		clear_string_space(_line, result);
		if(strncmp("Signature=FRAME", result, 15) != 0) {printf("Signature=FRAME false\n"); return false;}
	}
	else
		return false;
	
	// Get frame number
	if(fgets(_line, 256, fp))
	{
		clear_string_space(_line, result);
		if(strncmp("Frame=", result, 6) != 0) {printf("Frame= false\n"); return false;}
		else
		{
			for(i=0; isdigit(result[i+6]); i++)
				tmp = tmp * 10 + (result[i+6] - '0');
			if(result[i+6] != ' ' && result[i+6] != '\n' && result[i+6] != '\0') return false;
			*frmno = tmp;
		}
	}
	else
		return false;
	
	// Get M1ID
	tmp = 0;
	if(fgets(_line, 256, fp))
	{
		clear_string_space(_line, result);
		if(strncmp("M1=", result, 3) != 0) {printf("M1= false\n"); return false;}
		else
		{
			for(i=0; isdigit(result[i+3]); i++)
				tmp = tmp * 10 + (result[i+3] - '0');
			if(result[i+3] != ' ' && result[i+3] != '\n' && result[i+3] != '\0') return false;
			*m1id = tmp;
		}
	}
	else
		return false;
		
	// Get M2ID
	tmp = 0;
	if(fgets(_line, 256, fp))
	{
		clear_string_space(_line, result);
		if(strncmp("M2=", _line, 3) != 0) {printf("M2= false\n"); return true;} // some inno board has no M2
		else
		{
			for(i=0; isdigit(result[i+3]); i++)
				tmp = tmp * 10 + (result[i+3] - '0');
			if(result[i+3] != ' ' && result[i+3] != '\n' && result[i+3] != '\0') return false;
			*m2id = tmp;
		}
	}
	else
		return false;
	
	return true;
}
	
bool ServoFrameInno::load(const char* dir) {
	int i, j, k = 0, channel = 0;
	FILE *fp;
	char *exten = NULL;
	char path[256] = {'\0'}, is_enable = 0, tmp_chen[45] = {0}, is_frmfile = 0, is_ftxtfile = 0;
	char _line[256] = {'\0'}, tmp_mode[45][10] = {'\0'}, tmp_ch[256] = {'\0'}, _mode[10] = {'\0'};
	long tmp_pos[45] = {0L}, tmp_time[45] = {0L}, tmp_speed[45] = {0L}, _pos, _time, _speed;
	
	if(dir == NULL) return false;    

	get_real_path(dir, path);
	
	if((exten = strstr(path, ".frm")) != NULL && exten[4] == '\0')
    {
		is_frmfile = 1;
    	if((fp = fopen(path, "rb")) == NULL) return false;
    }
    else if((exten = strstr(path, ".FRM")) != NULL && exten[4] == '\0')
    {
		is_frmfile = 1;
    	if((fp = fopen(path, "rb")) == NULL) return false;
    }
	else if((exten = strstr(path, ".ftxt")) != NULL && exten[5] == '\0')
	{
		is_ftxtfile = 1;
		if((fp = fopen(path, "r")) == NULL) return false;
	}
	else if((exten = strstr(path, ".FTXT")) != NULL && exten[5] == '\0')
	{
		is_ftxtfile = 1;
		if((fp = fopen(path, "r")) == NULL) return false;
	}
	else // the file is not supported
		return false;
	
	if(is_ftxtfile == 1)
	{
		if(is_innoFrameFile(_line, fp, &frameno, &M1ID, &M2ID) == false) {fclose(fp); return false;}

		while(fgets(_line, 256, fp))
		{
			for(i=0; _line[i] != '\0' && _line[i] != '\n'; i++)
				if(_line[i] == ' ') continue; else break;
	
			if(_line[i] == '\0' || _line[i] == '\n') continue;
			
			if(_line[0] == 'c' && _line[1] == 'h')
			{
				clear_string_space(_line, tmp_ch);
				if(inno_frame_analysis(tmp_ch, &_pos, &is_enable, &_time, &_speed, _mode) == true)
				{
					tmp_chen[channel]  = is_enable;
					tmp_pos[channel]   = _pos;
					tmp_time[channel]  = _time;
					tmp_speed[channel] = _speed;
					for(j=0; j<10; j++) tmp_mode[channel][j] = _mode[j];
					channel++;
				}
				else
				{
					fclose(fp);
					return false;
				}
			}
			else if(_line[0] == 'a' && _line[1] == 'l' && _line[2] == 'l')
			{
				strcpy(all_string[k], _line);
				k++; // k is module ID
			}
		}
	} // if(is_ftxtfile == 1) ...
	else if(is_frmfile == 1)
	{
		unsigned int index;
		fread(frmfile, 4, 173, fp);
		frameno = frmfile[0];
		M1ID    = frmfile[1];
		M2ID    = frmfile[2];
		
		for(i=3, j=0; i<173; i+=5) // include all_string
		{
			tmp_chen[j] = frmfile[i];
			tmp_pos[j]  = frmfile[i+1];
			if(frmfile[i+2] == 0) strcpy(tmp_mode[j], "time");
			else if(frmfile[i+2] == 1) strcpy(tmp_mode[j], "speed");
			else if(frmfile[i+2] == 2) strcpy(tmp_mode[j], "full");
			tmp_speed[j] = frmfile[i+3];
			tmp_time[j]  = frmfile[i+4];
			j++;
			
			if(j == 16 || j == 32)
			{
				i += 5;
				if(j == 16) index = 0; else index = 5;
				all_string2[index]   = frmfile[i];
				all_string2[index+1] = frmfile[i+1];
				all_string2[index+2] = frmfile[i+2];
				all_string2[index+3] = frmfile[i+3];
				all_string2[index+4] = frmfile[i+4];
			}
		}
	} 
	
	fclose(fp);

	for(i=0; i<32; i++)
	{
		//if(i != 0 && (i % 16) == 0) getchar();
		en[i] = tmp_chen[i];
		if(en[i] == 1) positions[i] = tmp_pos[i];
		time[i] = tmp_time[i];
		speed[i] = tmp_speed[i];
		for(j=0; j<10; j++) mode[i][j] = tmp_mode[i][j];
	}

	return true;
}

bool ServoFrameInno::load(const String &s) {
	load(s.c_str());
}

ServoOffsetInno::ServoOffsetInno() : ServoOffset() {
	int i;
	for(i=0; i<32; i++) ofsfile[i] = 0L;
}

ServoOffsetInno::ServoOffsetInno(const char* dir) : ServoOffset() {
	int i;
	for(i=0; i<32; i++) ofsfile[i] = 0L;
	load(dir);
}

bool is_innoOffsetFile(char* _line, FILE* fp) {
    int i, tmp = 0;
    char result[256] = {'\0'};
    
	if(fgets(_line, 256, fp))
	{
		clear_string_space(_line, result);
		if(strncmp("[ID]", result, 4) != 0) {printf("[ID] false\n"); return false;}
	}
	else
		return false;
	
	if(fgets(_line, 256, fp))
	{
		clear_string_space(_line, result);
		if(strncmp("Signature=OFFSET", result, 16) != 0) {printf("Signature=OFFSET false\n"); return false;}
	}
	else
		return false;
	
	return true;
}

bool inno_offset_analysis(char* line, long* offset) {
	int i, j = 0;
	char tmp[256] = {'\0'};
	bool negative = false;

	for(i=0; line[i] != '\0' && line[i] != '\n'; i++)
	{
		if(line[i] != ' ') tmp[j++] = line[i];
	}
	
	if(strncmp("ch", tmp, 2) != 0) return false;

	if(isdigit(tmp[2]) && isdigit(tmp[3]) && tmp[4] == '=')
	{
		for(i=5; tmp[i] == ' '; i++);
		
		*offset = 0;
		if(tmp[i] == '-') {negative = true; i++;}	
		for(;isdigit(tmp[i]); i++) *offset = *offset*10 + (tmp[i] - '0');
	}
	else
		return false;
	
	if(tmp[i] != '\0' && tmp[i] != '\n' && tmp[i] != ' ') return false;
	if(negative == true) *offset = (-1) * (*offset);
	
	return true;
}

bool ServoOffsetInno::load(const char* dir) {
    int i, channel = 0;
	FILE *fp;
	char *exten = NULL;
	char path[256] = {'\0'}, is_ofsfile = 0, is_otxtfile = 0;
	char _line[256] = {'\0'}, tmp_ch[256] = {'\0'};
	long tmp_offset[45] = {0L}, offset;
	
	if(dir == NULL) return false;    

	get_real_path(dir, path);
	
	if((exten = strstr(path, ".ofs")) != NULL && exten[4] == '\0')
    {
		is_ofsfile = 1;
    	if((fp = fopen(path, "rb")) == NULL) return false;
    }
    else if((exten = strstr(path, ".OFS")) != NULL && exten[4] == '\0')
    {
		is_ofsfile = 1;
    	if((fp = fopen(path, "rb")) == NULL) return false;
    }
	else if((exten = strstr(path, ".otxt")) != NULL && exten[5] == '\0')
	{
		is_otxtfile = 1;
		if((fp = fopen(path, "r")) == NULL) return false;
	}
	else if((exten = strstr(path, ".OTXT")) != NULL && exten[5] == '\0')
	{
		is_otxtfile = 1;
		if((fp = fopen(path, "r")) == NULL) return false;
	}
	else // the file is not supported
		return false;

    if(is_otxtfile == 1)
	{
		if(is_innoOffsetFile(_line, fp) == false) {fclose(fp); return false;}
	
		while(fgets(_line, 256, fp))
		{
			for(i=0; _line[i] != '\0' && _line[i] != '\n'; i++)
				if(_line[i] == ' ') continue; else break;
	
			if(_line[i] == '\0' || _line[i] == '\n') continue;
			
			if(_line[0] == 'c' && _line[1] == 'h')
			{
				clear_string_space(_line, tmp_ch);
				if(inno_offset_analysis(tmp_ch, &offset) == true)
				{
					tmp_offset[channel] = offset;
					channel++;
				}
				else
				{
					fclose(fp);
					return false;
				}
			}
		}
	} // if(is_otxtfile == 1) ...
	else if(is_ofsfile == 1)
	{
		fread(ofsfile, 4, 32, fp);
		for(i=0; i<32; i++) tmp_offset[i] = ofsfile[i];
	}
	
	fclose(fp);
	
	for(i=0; i<32; i++)
	{
		offsets[i] = tmp_offset[i];
		//printf("ch%d: offset %ld \n", i, tmp_offset[i]);
	}
	
	return true;
}

bool ServoOffsetInno::load(const String &s) {
	load(s.c_str());
}

/*
void offset_combine_value(char* target, char* src, int value, bool linkprev, bool next_line) {
	int i = 0, j;
	char tmp[256] = {'\0'};
	char num[16]  = {'\0'};
	char space;
	
	if(linkprev == true) for(i=0; i < 256 && target[i] != '\0'; i++);
	if(i >= 256) return;
	
	if(src[0] == 'c' || src[1] == 'h')
	{
		for(j=0; src[j] != '\0'; j++) target[i++] = src[j];
		itoa(value, num, 10);
		if(value < 10) target[i++] = '0';
		for(j=0; num[j] != '\0'; j++) target[i++] = num[j];
		target[i++] = '\0';
	}
	else
	{
		for(j=0; src[j] != '\0'; j++) target[i++] = src[j];
		itoa(value, num, 10);
		for(j=0; num[j] != '\0'; j++);
		for(space = (4 - j); space>0; space--) target[i++] = ' ';
		
		for(j=0; num[j] != '\0'; j++) target[i++] = num[j];
		if(next_line == true) target[i++] = '\n'; else target[i++] = ' ';
		target[i++] = '\0';
	}

	//printf("%s\n", result);
}

bool ServoOffsetInno::save(const char* dir) {
    char* line1 = "[ID]\n";
    char* line2 = "Signature=OFFSET\n";
    char* exten = NULL;
    char _line[256] = {'\0'}, is_ofsfile = 0, is_otxtfile = 0;
	int i, j;
	FILE *fp;

	char path[256] = {'\0'};
	
	if(dir == NULL) return false;

	get_real_path(dir, path);
	
	if((exten = strstr(path, ".ofs")) != NULL && exten[4] == '\0')
    {
		is_ofsfile = 1;
    	if((fp = fopen(path, "wb")) == NULL) return false;
    }
	else if((exten = strstr(path, ".otxt")) != NULL && exten[5] == '\0')
	{
		is_otxtfile = 1;
		if((fp = fopen(path, "w")) == NULL) return false;
	}
	else // the file is not supported
		return false;
	
	if(is_ofsfile == 1)
	{
		for(i=0; i<32; i++) ofsfile[i] = offsets[i];
		fwrite(ofsfile, 4, 32, fp);
		fclose(fp);
		return true;
	}
	
	fputs(line1, fp);
	fputs(line2, fp);
	
	fputs("[M1]\n", fp);
	for(i=0; i<16; i++)
	{
		offset_combine_value(_line, "ch", i, false, false);
		offset_combine_value(_line, "=", offsets[i], true, true);
		fputs(_line, fp);
	}
	
	fputs("[M2]\n", fp);
	for(i=16, j=0; i<32; i++, j++)
	{
		offset_combine_value(_line, "ch", j, false, false);
		offset_combine_value(_line, "=", offsets[i], true, true);
		fputs(_line, fp);
	}
	
	fclose(fp);
	return true;
}
*/


/************************* KONDO Servo Control Board **************************/

ServoFrameKondo::ServoFrameKondo() : ServoFrame() {}

ServoFrameKondo::ServoFrameKondo(const char* dir, const char* fname) : ServoFrame() {
	load(dir, fname);
}

long _capture(int pin) {
	unsigned long _nowtime;
	long width = 0L;
	
	// Switch GPIO to output lOW
	pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    
    // Send 50us pulse to the servo
    io_DisableINT();
	_nowtime = micros();
	digitalWrite(pin, HIGH);
    while((micros() - _nowtime) < 50);
    digitalWrite(pin, LOW);
    
    delayMicroseconds(50);
	
	// Read pusle width form the servo
	pinMode(pin, INPUT);
    width = pulseIn(pin, HIGH, 1000000L);
    delayMicroseconds(3500);
	
	// Switch the pin to output LOW
	pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    delayMicroseconds(250);
	io_RestoreINT();
    
    if(width < MIN_PULSE_WIDTH || width > MAX_PULSE_WIDTH) return 0L;
    return width;
}

#define CAP_WOFFSET    (12L)
long ServoFrameKondo::capture(Servo &s) {
	
	int i;
	// check whether all pins are released status, if not, return false.
	for(i=0; i<45; i++)
		if(Servoptr[i] != NULL && sv86[Servoptr[i]->servoIndex].state != SERVO_NONE)
			return 0L;
	
	if(&s == &nullServo) return 0L;
		
	return _capture(servos[s.servoIndex].Pin.nbr) + CAP_WOFFSET;;
}

bool is_kondoFrameFile(char* _line, FILE* fp) {
    int i, tmp = 0;
    char result[256] = {'\0'};
    
	if(fgets(_line, 256, fp))
	{
		clear_string_space(_line, result);
		if(strncmp("[GraphicalEdit]", result, 15) != 0) {printf("[GraphicalEdit] false\n"); return false;}
	}
	else
		return false;
	
	return true;
}

bool kondo_frame_analysis(char* ch, long* target) {
	int i, j = 0, k = 0;
	char start = 0, tmp_ch[10] = {'\0'};

	for(i=0; ch[i] != '\0'; i++)
	{
		if(isdigit(ch[i])) {tmp_ch[j++] = ch[i]; continue;}	
		if(start == 1)
		{
			tmp_ch[j] = '\0';
			target[k++] = atoi(tmp_ch);
			j = 0;
			continue;
		}
		start = 1;
		j = 0;
	}
	
	if(j != 0)
	{
		tmp_ch[j] = '\0';
		target[k++] = atoi(tmp_ch);
	}
	
	if(k != 24) return false;
	return true;
}

bool get_kondo_frame_name(char* ch, const char* fname) {
    int i;
    
    for(i=0; fname[i] != '\0'; i++)
    	if(fname[i] != ch[i]) return false;
    if(ch[i] == '\n' || ch[i] == '\0' || ch[i] == '\r') return true;
	return false;
}

bool ServoFrameKondo::load(const char* dir, const char* fname) {
    int i, channel = 0;
	FILE *fp;
	char *exten = NULL;
	char path[256] = {'\0'};
	char _line[256] = {'\0'}, tmp_ch[256] = {'\0'};
	long tmp_frame[24] = {0L};
	bool _handled = false;
	
	if(dir == NULL) return false;    

	get_real_path(dir, path);
	
	if((exten = strstr(path, ".rcb")) != NULL && exten[4] == '\0')
    {
    	if((fp = fopen(path, "r")) == NULL) return false;
    }
	else if((exten = strstr(path, ".RCB")) != NULL && exten[4] == '\0')
	{
		if((fp = fopen(path, "r")) == NULL) return false;
	}
	else // the file is not supported
		return false;
	
	if(is_kondoFrameFile(_line, fp) == false) {fclose(fp); return false;}
	
	while(fgets(_line, 256, fp))
	{
		for(i=0; _line[i] != '\0' && _line[i] != '\n'; i++)
			if(_line[i] == ' ') continue; else break;

		if(_line[i] == '\0' || _line[i] == '\n') continue;
		
		if((exten = strstr(_line, "[Item")) != NULL)
		{
			fgets(_line, 256, fp); // get next line
			clear_string_space(_line, tmp_ch);
			if((exten = strstr(tmp_ch, "Name=")) == NULL) continue;
			if(fname != NULL && get_kondo_frame_name(exten + 5, fname) == false) continue;

			for(i=0; i<7; i++) fgets(_line, 256, fp);
			clear_string_space(_line, tmp_ch);
			if((exten = strstr(tmp_ch, "Prm=")) != NULL)
			{
				if(kondo_frame_analysis(exten + 4, tmp_frame) == true)
				{
					for(i=0; i<24; i++)
					{
						if(tmp_frame[i] < 16039 || tmp_frame[i] > 16729) continue; // not servo position
						positions[i] = 1500L + (tmp_frame[i] - 16384L)*3L; // resolution is 3us
					}
					_handled = true;
				}
				break;
			}
			break;
		}
	}
	
	fclose(fp);
				
	return (_handled == true) ? true : false;
} 

bool ServoFrameKondo::load(const String &s, const String &s1) {
	load(s.c_str(), s1.c_str());
}


ServoOffsetKondo::ServoOffsetKondo() : ServoOffset() {}

ServoOffsetKondo::ServoOffsetKondo(const char* dir) : ServoOffset() {
	load(dir);
}

bool is_kondoOffsetFile(char* _line, FILE* fp) {
    int i, tmp = 0;
    char result[256] = {'\0'};
    
	if(fgets(_line, 256, fp))
	{
		clear_string_space(_line, result);
		if(strncmp("[TrimData]", result, 10) != 0) {printf("[TrimData] false\n"); return false;}
	}
	else
		return false;
	
	return true;
}

bool kondo_offset_analysis(char* line, long* offset) {
	int i, j = 0;
	char tmp[256] = {'\0'};
	bool negative = false;

	for(i=0; line[i] != '\0' && line[i] != '\n'; i++)
	{
		if(line[i] != ' ') tmp[j++] = line[i];
	}
	
	if(strncmp("CH", tmp, 2) != 0) return false;

	if(isdigit(tmp[2]) && isdigit(tmp[3]) && tmp[4] == '=') i = 5;
	else if(isdigit(tmp[2]) && tmp[3] == '=') i = 4;
	else return false;
	
	for(; tmp[i] == ' '; i++);
	
	*offset = 0;
	if(tmp[i] == '-') {negative = true; i++;}	
	for(;isdigit(tmp[i]); i++) *offset = *offset*10 + (tmp[i] - '0');

	if(tmp[i] != '\0' && tmp[i] != '\n' && tmp[i] != ' ') return false;
	if(negative == true) *offset = (-1) * (*offset);
	
	return true;
}

bool ServoOffsetKondo::load(const char* dir) {
    int i, channel = 0;
	FILE *fp;
	char *exten = NULL;
	char path[256] = {'\0'};
	char _line[256] = {'\0'}, tmp_ch[256] = {'\0'};
	long tmp_offset[45] = {0L}, offset;
	
	if(dir == NULL) return false;    

	get_real_path(dir, path);
	
	if((exten = strstr(path, ".rcb")) != NULL && exten[4] == '\0')
    {
    	if((fp = fopen(path, "r")) == NULL) return false;
    }
	else if((exten = strstr(path, ".RCB")) != NULL && exten[4] == '\0')
	{
		if((fp = fopen(path, "r")) == NULL) return false;
	}
	else // the file is not supported
		return false;

	if(is_kondoOffsetFile(_line, fp) == false) {fclose(fp); return false;}

	while(fgets(_line, 256, fp))
	{
		for(i=0; _line[i] != '\0' && _line[i] != '\n'; i++)
			if(_line[i] == ' ') continue; else break;

		if(_line[i] == '\0' || _line[i] == '\n') continue;
		
		if(_line[0] == 'C' && _line[1] == 'H')
		{
			clear_string_space(_line, tmp_ch);
			if(kondo_offset_analysis(tmp_ch, &offset) == true)
			{
				tmp_offset[channel] = offset;
				channel++;
			}
			else
			{
				fclose(fp);
				return false;
			}
		}
	}
	fclose(fp);

	for(i=0; i<24; i++)
	{
		if(tmp_offset[i] < 16039 || tmp_offset[i] > 16729) continue; // not servo position
		offsets[i] = (tmp_offset[i] - 16384L)*3L;
		// printf("CH%d: offset %ld \n", i, offsets[i]);
	}
	
	return true;
}

bool ServoOffsetKondo::load(const String &s) {
	load(s.c_str());
}



/************************ Pololu Servo Control Board **************************/

ServoFramePololu::ServoFramePololu() : ServoFrame() {}

ServoFramePololu::ServoFramePololu(const char* dir, const char* sname, const char* fname) : ServoFrame() {
	load(dir, sname, fname);
}

bool is_pololuFrameFile(char* _line, FILE* fp) {
    int i, tmp = 0;
    char result[256] = {'\0'};
    
	if(fgets(_line, 256, fp))
	{
		clear_string_space(_line, result);
		if(strncmp("<!--Pololu", result, 10) != 0) {printf("<!--Pololu false\n"); return false;}
	}
	else
		return false;
	
	return true;
}

bool pololu_frame_analysis(char* ch, long* target) {
	int i, j = 0, k = 0;
	char tmp_ch[10] = {'\0'};

	for(i=0; ch[i] != '<'; i++)
	{
		if(isdigit(ch[i])) {tmp_ch[j++] = ch[i]; continue;}
		tmp_ch[j] = '\0';
		target[k++] = atoi(tmp_ch);
		j = 0;
	}
	
	if(j != 0)
	{
		tmp_ch[j] = '\0';
		target[k++] = atoi(tmp_ch);
	}
	
	if(k != 24) return false;
	return true;
}

bool get_pololu_sequence_name(char* ch, const char* sname) {
    int i;

    for(i=1; ch[i] != '"'; i++)
    	if(sname[i-1] != ch[i]) return false;
    if(sname[i-1] == '\n' || sname[i-1] == '\r' || sname[i-1] == '\0')  return true;
	return false;
}

bool get_pololu_frame_name(char* ch, const char* fname) {
    int i;
    
    for(i=1; ch[i] != '"'; i++)
    	if(fname[i-1] != ch[i]) return false;
    if(fname[i-1] == '\n' || fname[i-1] == '\r' || fname[i-1] == '\0')  return true;
	return false;
}

bool ServoFramePololu::load(const char* dir, const char* sname, const char* fname) {
    int i, channel = 0;
	FILE *fp;
	char *exten = NULL;
	char path[256] = {'\0'};
	char _line[256] = {'\0'}, tmp_ch[256] = {'\0'};
	long tmp_frame[24] = {0L};
	
	char* sequence_start = "<Sequences>";
	char* sequence_end = "</Sequences>";
	
	bool _handled = false, find_sequences = false, find_sequence = false;
	
	if(dir == NULL) return false;    

	get_real_path(dir, path);
	
	if((exten = strstr(path, ".txt")) != NULL && exten[4] == '\0')
    {
    	if((fp = fopen(path, "r")) == NULL) return false;
    }
	else if((exten = strstr(path, ".TXT")) != NULL && exten[4] == '\0')
	{
		if((fp = fopen(path, "r")) == NULL) return false;
	}
	else // the file is not supported
		return false;
	
	if(is_pololuFrameFile(_line, fp) == false) {fclose(fp); return false;}
	
	// find the head of sequences 
	while(fgets(_line, 256, fp))
	{
		for(i=0; _line[i] != '\0' && _line[i] != '\n'; i++)
			if(_line[i] == ' ') continue; else break;

		if(_line[i] == '\0' || _line[i] == '\n') continue;
		
		clear_string_space(_line, tmp_ch);
		if(strcmp(tmp_ch, sequence_start) == 0) {find_sequences = true; break;}
	}
	
	if(find_sequences == false) return false;
	
	// find the sequence name
	while(fgets(_line, 256, fp))
	{
		for(i=0; _line[i] != '\0' && _line[i] != '\n'; i++)
			if(_line[i] == ' ') continue; else break;

		if(_line[i] == '\0' || _line[i] == '\n') continue;

		if((exten = strstr(_line, "<Sequence name=")) == NULL) continue;
		if(sname != NULL && get_pololu_sequence_name(exten + 15, sname) == false) continue;
		find_sequence = true;
		break;
	}
	
	if(find_sequence == false) return false;
		
	// find the frame name
	while(fgets(_line, 256, fp))
	{
		for(i=0; _line[i] != '\0' && _line[i] != '\n'; i++)
			if(_line[i] == ' ') continue; else break;

		if(_line[i] == '\0' || _line[i] == '\n') continue;

		if(strstr(_line, "</Sequence>") != NULL) break; // fine the final line of this sequence
		if((exten = strstr(_line, "Frame name=")) == NULL) continue;
		if(fname != NULL && get_pololu_frame_name(exten + 11, fname) == false) continue;
		
		if((exten = strstr(_line, ">")) != NULL)
		{
			if(pololu_frame_analysis(exten + 1, tmp_frame) == true)
			{
				for(i=0; i<24; i++)
				{
					positions[i] = tmp_frame[i]/4L;
				}
				_handled = true;
			}
			break;
		}
	}
	
	fclose(fp);
				
	return (_handled == true) ? true : false;
}

bool ServoFramePololu::load(const String &s, const String &s1, const String &s2) {
	load(s.c_str(), s1.c_str(), s2.c_str());
}

/*
ServoOffsetPololu::ServoOffsetPololu() : ServoOffset() {}

ServoOffsetPololu::ServoOffsetPololu(const char* dir) : ServoOffset() {
	load(dir);
}

bool is_pololuOffsetFile(char* _line, FILE* fp) {
    int i, tmp = 0;
    char result[256] = {'\0'};
    
	if(fgets(_line, 256, fp))
	{
		clear_string_space(_line, result);
		if(strncmp("<!--Pololu", result, 10) != 0) {printf("<!--Pololu false\n"); return false;}
	}
	else
		return false;
	
	return true;
}

bool pololu_offset_analysis(char* ch, long* offset) {
	int i, j = 0;
	char tmp_ch[10] = {'\0'};

	if(ch[0] != '"') return false;
	
	for(i=1; ch[i] != '"'; i++)
		if(isdigit(ch[i])) tmp_ch[j++] = ch[i];

	tmp_ch[j] = '\0';
	*offset = atoi(tmp_ch);

	return true;
}

bool ServoOffsetPololu::load(const char* dir) {
    int i, channel = 0;
	FILE *fp;
	char *exten = NULL;
	char path[256] = {'\0'};
	char _line[256] = {'\0'}, tmp_ch[256] = {'\0'};
	long tmp_offset[24] = {0L}, offset;
	
	bool _handled = false, find_channels = false;
	
	if(dir == NULL) return false;    

	get_real_path(dir, path);
	
	if((exten = strstr(path, ".txt")) != NULL && exten[4] == '\0')
    {
    	if((fp = fopen(path, "r")) == NULL) return false;
    }
	else if((exten = strstr(path, ".TXT")) != NULL && exten[4] == '\0')
	{
		if((fp = fopen(path, "r")) == NULL) return false;
	}
	else // the file is not supported
		return false;
	
	if(is_pololuFrameFile(_line, fp) == false) {fclose(fp); return false;}
	
	// find the sequence name
	while(fgets(_line, 256, fp))
	{
		for(i=0; _line[i] != '\0' && _line[i] != '\n'; i++)
			if(_line[i] == ' ') continue; else break;

		if(_line[i] == '\0' || _line[i] == '\n') continue;

		if((exten = strstr(_line, "<Channels MiniMaestroServoPeriod=")) == NULL) continue;
		find_channels = true;
		break;
	}
	
	if(find_channels == false) return false;
		
	// find the frame name
	while(fgets(_line, 256, fp))
	{
		for(i=0; _line[i] != '\0' && _line[i] != '\n'; i++)
			if(_line[i] == ' ') continue; else break;

		if(_line[i] == '\0' || _line[i] == '\n') continue;

		if(strstr(_line, "</Channels>") != NULL) break; // fine the final line of this sequence
		if((exten = strstr(_line, "neutral=")) == NULL) continue;

		if(pololu_offset_analysis(exten + 8, &offset) == true)
		{
			tmp_offset[channel] = offset;
			channel++;
		}
		else
		{
			fclose(fp);
			return false;
		}
	}
	fclose(fp);
	if(channel < 24) return false;

	for(i=0; i<24; i++)
	{
		offsets[i] = tmp_offset[i]/4L - 1500L;
	}
	
	return true;
}
*/

/************************************ RTC *************************************/

DMP_INLINE(unsigned long) interpolate(unsigned long cur, unsigned long target, unsigned long total, unsigned long t) {
	if (total == 0L) return target;

	return ((total-t)*cur + t*target)/total;
}

//volatile unsigned long __count = 0L;
static int timerrtc_isr_handler(int irq, void* data) {
	int i;
	unsigned char tmp, time_over[45];
	unsigned long tt, t, now_time;
	//__count = micros();
	io_outpb(0x70, 0x0C); // enable NMI and read RTC register C 
	tmp = io_inpb(0x71);  // clear RTC interrupt state

	if((tmp & 0x40) > 0)
	{
		//__count++;
		// ISR main function
		now_time = millis();
		for(i=0; i<MAX_SERVOS; i++)                       
		{
			if(sv86[i].state != SERVO_MOVING) continue;
			t  = now_time - sv86[i].starttime;
			tt = sv86[i].endtime - sv86[i].starttime;
			if(t >= tt)
				time_over[i] = 1;
			else
			{
				time_over[i] = 0;
				sv86[i].curposition = interpolate(sv86[i].prevposition, sv86[i].targetposition, tt, t);
			}
		}
		
		for(i=0; i<MAX_SERVOS; i++)                       
		{
			if(sv86[i].state != SERVO_MOVING)
			{
				if(sv86[i].state == SERVO_IDLE && sv86[i].mixoffset != 0L) // if the mixoffset != 0, do something
				{
					_write(i, (long)sv86[i].curposition + sv86[i].mixoffset);
				}	
				continue;
			}
			
			if(time_over[i] == 1)
			{
				sv86[i].curposition = sv86[i].targetposition;
				sv86[i].state = SERVO_IDLE;
			}
			_write(i, (long)sv86[i].curposition + sv86[i].mixoffset);
		}
	}
	
    if((tmp & 0x70) == 0) return ISR_NONE;
    //__count = micros() - __count;
    return ISR_HANDLED;
}

DMP_INLINE(unsigned char) inpb_cmos(unsigned char reg) {
    unsigned char tmp;
	io_DisableINT();
    io_outpb(0x70, 0x80 | reg); // disable NMI (by setting the 0x80 bit) and assign a RTC register address
    tmp = io_inpb(0x71);
    io_RestoreINT();
    return tmp;
}

DMP_INLINE(void) outpb_cmos(unsigned char reg, unsigned char data) {
    io_DisableINT();
    io_outpb(0x70, 0x80 | reg); // disable NMI (by setting the 0x80 bit) and assign a RTC register address
    io_outpb(0x71, data);
    io_RestoreINT();
}

void RTC_initialize(long microseconds) {
    unsigned char tmp;
    if(timerRTCInit == true) return;
    timerRTCInit = true;
    RTC_setPeriod(microseconds);
    
    tmp = inpb_cmos(0x0A);
    outpb_cmos(0x0A, (tmp & 0xF0) | _freq);
}

void RTC_timer_start(long microseconds) {
    unsigned char tmp;
    
    if(timerRTCInit == false) return;
    
    if(timerRTCEnable == false)
	{
		if(irq_Setting(RTCIRQ, IRQ_EDGE_TRIGGER) == false)
	    {
	        printf("MCM IRQ Setting fail\n"); return;
	    }
		if(irq_InstallISR(RTCIRQ, timerrtc_isr_handler, isrname_rtc) == false)
		{
		    printf("irq_install fail\n"); return;
		}
	}
	
    if(microseconds > 0) RTC_setPeriod(microseconds);
    
	tmp = inpb_cmos(0x0B);
    outpb_cmos(0x0B, tmp | 0x40);
    timerRTCEnable = true;
}

void RTC_timer_close() {
    unsigned char tmp;
    if(timerRTCInit == false || timerRTCEnable == false) return;
    
	tmp = inpb_cmos(0x0B);
    outpb_cmos(0x0B, tmp & ~(0x40));
    
    irq_UninstallISR(RTCIRQ, (void*)isrname_rtc);
    timerRTCEnable = false;
}

void RTC_setPeriod(long microseconds) {
	if(timerRTCInit == false || microseconds <= 0) return;
	
	if     (microseconds < 183L)    _freq = 3;
	else if(microseconds < 366L)    _freq = 4;
	else if(microseconds < 732L)    _freq = 5;
	else if(microseconds < 1464L)   _freq = 6;
	else if(microseconds < 2929L)   _freq = 7;
	else if(microseconds < 5859L)   _freq = 8;
	else if(microseconds < 11718L)  _freq = 9;
	else if(microseconds < 23437L)  _freq = 10;
	else if(microseconds < 46875L)  _freq = 11;
	else if(microseconds < 93750L)  _freq = 12;
	else if(microseconds < 187500L) _freq = 13;
	else if(microseconds < 375000L) _freq = 14;
	else if(microseconds < 500000L) _freq = 15;
	else                            _freq = 15;
}

