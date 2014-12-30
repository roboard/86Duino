/*
  Servo.cpp - Servo library for 86Duino
  Copyright (c) 2013 Android Lin <acen@dmp.com.tw>. All right reserved.

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

#include "Servo.h"
#include "io.h"
#include "irq.h"
#include "pins_arduino.h"

typedef struct {
  int pin;
  long value;
} irqservo_t;

#define MAX_SERVOS        (45)
#define usToTicks(_us)    ((_us)*SYSCLK)     // converts microseconds to tick
#define ticksToUs(_ticks) (( (unsigned)_ticks * 8)/ clockCyclesPerMicrosecond() ) // converts from ticks back to microseconds
#define ENDPIN    (999)

#define TRIM_DURATION       2                               // compensation ticks to trim adjust for digitalWrite delays // 12 August 2009

//#define NBR_TIMERS        (MAX_SERVOS / SERVOS_PER_TIMER)

static servo_t servos[MAX_SERVOS];                          // static array of servo structures
static servo_t sorted[MAX_SERVOS];

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

#define SERVO_MIN() (MIN_PULSE_WIDTH - this->min * 4)  // minimum value in uS for this servo
#define SERVO_MAX() (MAX_PULSE_WIDTH - this->max * 4)  // maximum value in uS for this servo 
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
    int mcpwm = arduino_to_mc_md[0][pin];
	int mdpwm = arduino_to_mc_md[1][pin];
	if(mcpwm == NOPWM || mdpwm == NOPWM) return false;
	if(mcpwm == MC_MODULE3 && mdpwm == MCPWM_MODULEB) return false;// this pin is uesd by tone and IRrmote lib after version 103
    return true;
}



static char mc_md_inuse[PINS];
static void sendPWM(uint8_t pin, unsigned int val);
Servo::Servo() {
	if(ServoCount < MAX_SERVOS)
	{
		if(ServoCount == 0) initServoIRQ(); 
		this->servoIndex = ServoCount++; // assign a servo index to this instance
		servos[this->servoIndex].ticks = usToTicks(DEFAULT_PULSE_WIDTH);   // store default values  - 12 Aug 2009
		servos[this->servoIndex].Pin.isActive = false;
	}
	else
		this->servoIndex = INVALID_SERVO ;  // too many servos 
}

uint8_t Servo::attach(int pin) {
	return this->attach(pin, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH);
}

uint8_t Servo::attach(int pin, int min, int max) {
	if(pin >= PINS) return 0;
	if(servos[this->servoIndex].Pin.isActive == true) return this->servoIndex; // if pin have been active, ingore sort process
	if(this->servoIndex < MAX_SERVOS)
	{
		servos[this->servoIndex].Pin.nbr = pin;
		this->min  = (MIN_PULSE_WIDTH - min)/4; //resolution of min/max is 4 uS
		this->max  = (MAX_PULSE_WIDTH - max)/4; 
		if(isPWMPin(pin) == false)
		{
			pinMode(pin, OUTPUT);                  // set servo pin to output
			digitalWrite(pin, LOW);
			// if(isISRActive() == false) initServoIRQ(); we don't need this here   
			insert(servos[this->servoIndex]);
			cp2irqservos();
		}
		else
		{
			if(mcpwm_ReadReloadOUT(arduino_to_mc_md[0][pin], arduino_to_mc_md[1][pin]) != 0L)
				mcpwm_ReloadOUT_Unsafe(arduino_to_mc_md[0][pin], arduino_to_mc_md[1][pin], MCPWM_RELOAD_CANCEL);
		}
		servos[this->servoIndex].Pin.isActive = true;  // this must be set after the check for isISRActive
	} 
	return this->servoIndex;
}

void Servo::detach() {
	uint8_t pin = servos[this->servoIndex].Pin.nbr;
	int mc_pwm, md_pwm;
	
	if(servos[this->servoIndex].Pin.isActive == false) return; // if pin have been inactive, ingore sort process
	servos[this->servoIndex].Pin.isActive = false;
	if(isPWMPin(pin) == false)
	{
		del(servos[this->servoIndex]);
		cp2irqservos();
	}
	else
	{
		mc_pwm = arduino_to_mc_md[0][pin];
		md_pwm = arduino_to_mc_md[1][pin];
		mcpwm_SetOutMask(mc_pwm, md_pwm, MCPWM_HMASK_INACTIVE); // Low sigh is sensor interface
		mcpwm_ReloadOUT_Unsafe(mc_pwm, md_pwm, MCPWM_RELOAD_PEREND);
		mc_md_inuse[pin] = 0;
	}
}

void Servo::write(int value) {
	if(value < MIN_PULSE_WIDTH)
	{   // treat values less than 544 as angles in degrees (valid values in microseconds are handled as microseconds)
		if(value < 0) value = 0;
		if(value > 180) value = 180;
		value = map(value, 0, 180, SERVO_MIN(),  SERVO_MAX());      
	}
	this->writeMicroseconds(value);
}

void Servo::writeMicroseconds(int value) {
	int i;
	byte channel = this->servoIndex;
	if(channel < MAX_SERVOS)   // ensure channel is valid
	{  
		if(value < SERVO_MIN())          // ensure pulse width is valid
		  value = SERVO_MIN();
		else if(value > SERVO_MAX())
		  value = SERVO_MAX();   
		
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

int Servo::read() {
	return  map( this->readMicroseconds()+1, SERVO_MIN(), SERVO_MAX(), 0, 180);     
}

int Servo::readMicroseconds() {
	unsigned int pulsewidth;
	if(this->servoIndex != INVALID_SERVO)
		pulsewidth = ticksToUs(servos[this->servoIndex].ticks) + TRIM_DURATION ;   // 12 aug 2009
	else 
		pulsewidth = 0;
	return pulsewidth;   
}

bool Servo::attached() {
	return servos[this->servoIndex].Pin.isActive;
}


static void sendPWM(uint8_t pin, unsigned int val) {
    unsigned short crossbar_ioaddr = 0;
	int mc_pwm, md_pwm;
	
    mc_pwm = arduino_to_mc_md[0][pin];
    md_pwm = arduino_to_mc_md[1][pin];
    
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
		io_outpb(crossbar_ioaddr + 0x90 + pinMap[pin], 0x08);
		mc_md_inuse[pin] = 1;
    }  
}
