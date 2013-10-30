#include "Arduino.h"
#include "dmpcfg.h"
#include "mcm.h"
#include "irq.h"

static int mc = 3, md = 2;

static volatile int use_pin_tone  = 255;
static volatile int use_pin_times = 0;
static int8_t toneBegin(uint8_t _pin)
{  
}

/*
static bool findToneTimer(int* mc_tone, int* md_tone) {
	unsigned long reg = mc_inp(MC_GENERAL, MCG_ENABLEREG1);
    int i, j, timer; // That is Servo's timer. (mc*3 + 2)
    for(i=0; i<4; i++)
    {
    	for(j=0; j<3; j++)
    	{
    		if(i == 3 && j == 2) return false;
    		timer = i*3 + j;
			if ((reg & (0x01L << timer)) == 0L)
			{
				*mc_tone = i;
				*md_tone = j;
				return true;
			}
		}
	}
	return false;
}
*/

/**********/
/*  IRQ   */                                                                       
/**********/
static int mcint_offset[3] = {0, 8, 16};
static void clear_INTSTATUS(void) {
    mc_outp(mc, 0x04, 0x00ffffffL); //for EX
}

static void disable_MCINT(void) {
    mc_outp(mc, 0x00, 0x00L);  // disable mc interrupt
    mc_outp(MC_GENERAL, 0x38, mc_inp(MC_GENERAL, 0x38) | (1L << mc));
}

static void enable_MCINT(unsigned long used_int) {
	mc_outp(MC_GENERAL, 0x38, mc_inp(MC_GENERAL, 0x38) & ~(1L << mc));
	mc_outp(mc, 0x00, used_int<<mcint_offset[md]);
}


static bool toneInitInt = false;
static char* name = "Tone";
void noTone(uint8_t _pin)
{
	io_DisableINT();
	use_pin_times = 0;
	use_pin_tone  = 255;
	io_RestoreINT();
	//mcpwm_SetOutMask(mc, md, MCPWM_HMASK_NONE);
	//mcpwm_ReloadOUT_Unsafe(mc, md, MCPWM_RELOAD_NOW);
	disable_MCINT();
	mcpwm_Disable(mc, md);
	digitalWrite(_pin, LOW);
	if(toneInitInt == true)
		irq_UninstallISR(GetMCIRQ(), name);
	toneInitInt = false;
}


static int h_l = 0;
static int isr_handler(int irq, void* data) {
    mc_outp(mc, 0x04, (PULSE_END_INT << mcint_offset[md]));

    if(use_pin_times != 0)
	{
		if(h_l == 0)
		{
			digitalWrite(use_pin_tone, HIGH);
			if(use_pin_times > 0) use_pin_times--;
			h_l = 1;
		}
		else
		{
        	digitalWrite(use_pin_tone, LOW);
        	if(use_pin_times > 0) use_pin_times--;
        	h_l = 0;
		}
    }
    else
        noTone(use_pin_tone);
     
    return ISR_HANDLED;
}

static bool init_mc_irq(void) {
	if(toneInitInt == false)
	{
		Set_MCIRQ(GetMCIRQ());
		if(irq_Init() == false) 
		{
		    printf("irq_init fail\n"); return false;
		}
		
		if(irq_Setting(GetMCIRQ(), IRQ_LEVEL_TRIGGER) == false)
		{
		    printf("%s\n", __FUNCTION__); return false;
		}
		
		if(irq_InstallISR(GetMCIRQ(), isr_handler, name) == false)
		{
		    printf("irq_install fail\n"); return false;
		}
		toneInitInt = true;
	}
    return true;
}


void tone_INIT(uint8_t _pin, unsigned int frequency, unsigned long duration){
    pinMode(_pin, OUTPUT);
    
    mcpwm_ReloadPWM(mc, md, MCPWM_RELOAD_CANCEL);
    mcpwm_SetOutMask(mc, md, MCPWM_HMASK_NONE + MCPWM_LMASK_NONE);
    mcpwm_SetOutPolarity(mc, md, MCPWM_HPOL_NORMAL + MCPWM_LPOL_NORMAL);
    mcpwm_SetDeadband(mc, md, 0L);
    mcpwm_ReloadOUT_Unsafe(mc, md, MCPWM_RELOAD_NOW);
    mcpwm_SetWaveform(mc, md, MCPWM_EDGE_A0I1);
    mcpwm_SetSamplCycle(mc, md, 0x04L);   // sample cycle: 20ms
    mcpwm_SetOutPolarity(mc, md, MCPWM_HPOL_INVERSE + MCPWM_LPOL_INVERSE);
    mcpwm_SetDeadband(mc, md, 0L);
    mcpwm_ReloadOUT_Unsafe(mc, md, MCPWM_RELOAD_NOW);  
    
    disable_MCINT();
    clear_INTSTATUS();
	if(init_mc_irq() == false)
	{
		printf("Init MC IRQ fail\n");
		return;
    }
    enable_MCINT(PULSE_END_INT);
    
    if(frequency <= 0)
		mcpwm_SetWidth(mc, md, (duration*1000) * SYSCLK, (duration*1000)/2 * SYSCLK);  // period: (1.0/frequency)*1000000)ms
    else
		mcpwm_SetWidth(mc, md, ((1.0/frequency)*1000000)/2 * SYSCLK, ((1.0/frequency)*500000)/2 * SYSCLK);  // period: (1.0/frequency)*1000000)ms     
    //mcpwm_ReloadPWM(mc, md, MCPWM_RELOAD_PEREND);
    
	io_DisableINT();
    use_pin_tone = _pin;
	io_RestoreINT();
	
	mcpwm_Enable(mc, md);
}

void tone_UPDATE(uint8_t _pin, unsigned int frequency, unsigned long duration) {
    if(frequency <= 0)
		mcpwm_SetWidth(mc, md, (duration*1000) * SYSCLK,(duration*1000)/2 * SYSCLK);  // period: (1.0/frequency)*1000000)ms ³æ¦ìms 
    else
		mcpwm_SetWidth(mc, md, ((1.0/frequency)*1000000)/2 * SYSCLK, ((1.0/frequency)*500000)/2 * SYSCLK);  // period: (1.0/frequency)*1000000)ms    
    mcpwm_ReloadPWM(mc, md, MCPWM_RELOAD_PEREND);    
}

void tone(uint8_t _pin, unsigned int frequency, unsigned long duration) {
	io_DisableINT();
	if(frequency >= 10000)
		frequency = 10000;
    if(duration <= 0)
		use_pin_times = -1;
    else
	{  
		if(frequency > 0)    
			use_pin_times = duration / (((1.0/(float)frequency)*1000.0)/2.0);
		else
			use_pin_times = 0;
    }
	io_RestoreINT();
	
    if(use_pin_tone == 255)
	{
		tone_INIT(_pin, frequency, duration);
		return;  
    }
    
    if(use_pin_tone ==_pin)
	{
		tone_UPDATE(_pin, frequency, duration);
		return;
    }
}


// XXX: this function only works properly for timer 2 (the only one we use
// currently).  for the others, it should end the tone, but won't restore
// proper PWM functionality for the timer.
void disableTimer(uint8_t _timer)
{

}




