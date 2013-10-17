#include "Arduino.h"
#include "dmpcfg.h"
#include "mcm.h"
#include "irq.h"

static int mc = MC_list[3], md = MD_list[2];

static int arduint_to_ex[14]={0,0,0,23,0,20,19,0,0,17,28,27,0,25};
static int use_pin_tone=255;
static int use_pin_times=0;
static int8_t toneBegin(uint8_t _pin)
{
  
}



///////////////////////////////////////////////////////////////////////
//IRQ
///////////////////////////////////////////////////////////////////////
static int mcint_offset[3] = {0, 8, 16};
static unsigned char int_routing_table[16] = {0xff, 0x08, 0xff, 0x02, 0x04, 0x05, 0x07, 0x06, 0xff, 0x01, 0x03, 0x09, 0x0b, 0xff, 0x0d, 0x0f};

#define PULSE_END_INT    (0x01L)
#define SC_END_INT       (0x02L)
#define USER_EVT_INT     (0x04L)
#define LDRDY_SERVO_INT  (0x08L)
#define LDRDY_EV_INT     (0x10L)
#define LDRDY_OCTRL_INT  (0x20L)
#define PULSE_FB_INT     (0x40L)
#define DDAFIFO_INT      (0x80L) // for EX

static unsigned long MCM_baseaddr = 0L;

static unsigned char read_sbf1_regb(unsigned char idx) {
    unsigned char tmp;

    io_DisableINT();
	io_outpdw(0x0cf8, (0x80003900L+(unsigned long)idx) & 0xfffffffcL);
    tmp = 0xff & (unsigned char)(io_inpdw(0x0cfc) >> ((idx & 0x03) * 8));
    io_EnableINT();

    return tmp;
}
static void write_sbf1_regb(unsigned char idx, unsigned char val) {
	int i = (idx & 0x03) * 8;

    io_DisableINT();
	io_outpdw(0x0cf8, (0x80003900L+(unsigned long)idx) & 0xfffffffcL);
	io_outpdw(0x0cfc, (io_inpdw(0x0cfc) & (~(0x000000ffL << i))) | ((unsigned long)val << i));
    io_EnableINT();
}
static void write_mc_pcireg(unsigned idx, unsigned long val) {
    unsigned long cf8 = (0x01L << 31)  // Type-1 PCI configuration
                      + (0x00L << 16)  // Bus 0x00
                      + (DEVICE << 11) // Device 0x01
                      + (0x00L << 8)   // Fun 0x00;
                      + idx;

    io_DisableINT();
	  io_outpdw(0x0cf8, cf8 & 0xfffffffcL);
	  io_outpdw(0x0cfc, val);
    io_EnableINT();
}
static unsigned long read_mc_pcireg(unsigned idx) {
    unsigned long tmp;
    unsigned long cf8 = (0x01L << 31)  // Type-1 PCI configuration
                      + (0x00L << 16)  // Bus 0x00
                      + (DEVICE << 11)  // Device 0x01
                      + (0x00L << 8)   // Fun 0x00;
                      + idx;

    io_DisableINT();
	  io_outpdw(0x0cf8, cf8 & 0xfffffffcL);
    tmp = io_inpdw(0x0cfc);
    io_EnableINT();

    return tmp;
}
static unsigned char GetMCIRQ(void) {  
    return (unsigned char)(read_mc_pcireg(0x3c) & 0xffL);
}
static void Set_MCIRQ(unsigned char irq) {
    write_mc_pcireg(0x3c, (read_mc_pcireg(0x3c) & 0xffffff00L) | (unsigned long) irq);
    write_sbf1_regb(0xb4, (read_sbf1_regb(0xb4) & 0xf0) | int_routing_table[irq]);
}
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
void noTone(uint8_t _pin)
{
      use_pin_times=0;
      use_pin_tone=255;
      disable_MCINT();
      mcpwm_Disable(mc, md);
      digitalWrite(_pin, LOW);
}
static volatile unsigned long intdata = 200000L;
static int h_l=0;
static int isr_handler(int irq, void* data) {
   
    //if(irq != GetMCIRQ())return ISR_NONE;
    io_outpdw(MCM_baseaddr + 0xd0L, 1UL<<(mc+1));
    io_outpdw(MCM_baseaddr + 0x04, (PULSE_END_INT << mcint_offset[md])); // clear int status

    //do something
    if(use_pin_times != 0){
      if((h_l == 0 )){
        digitalWrite(use_pin_tone, HIGH);
        if(use_pin_times>0)
          use_pin_times--;
        h_l=1;}
      else{
        digitalWrite(use_pin_tone, LOW);
        if(use_pin_times>0)
          use_pin_times--;
        h_l=0;}
    }
    else{
        noTone(use_pin_tone);
    } 
    return ISR_HANDLED;
}
////////////////////////////////////////////////////////////////////////////////
static bool init_mc_irq(void) {
    static int firist=0; 
    if(firist==0){
      firist=1;
      if(irq_Init() == false) 
      {
          printf("irq_init fail\n"); return false;
      }
      
      if(irq_Setting(GetMCIRQ(), IRQ_LEVEL_TRIGGER) == false)
      {
          printf("%s\n", __FUNCTION__); return false;
      }
      
      if(irq_InstallISR(GetMCIRQ(), isr_handler, NULL) == false)
      {
          printf("irq_install fail\n"); return false;
      }
    }
    return true;
}
///////////////////////////////////////////////////////////////////////
void tone_INIT(uint8_t _pin, unsigned int frequency, unsigned long duration){
    
    Set_MCIRQ(GetMCIRQ());    
    MCM_baseaddr = mc_setbaseaddr();
    //Master_DX2();   
    pinMode(_pin, OUTPUT);
    if (mc_SetMode(mc, MCMODE_PWM) == false){
      printf("ERROR: fail to change MC mode!\n");
      //return -1;
    }
    mcpwm_SetOutMask(mc, md, MCPWM_HMASK_NONE + MCPWM_LMASK_NONE);
    mcpwm_SetOutPolarity(mc, md, MCPWM_HPOL_NORMAL + MCPWM_LPOL_NORMAL);
    mcpwm_SetDeadband(mc, md, 0L);
    mcpwm_ReloadOUT_Unsafe(mc, md, MCPWM_RELOAD_NOW);
    mcpwm_SetWaveform(mc, md, MCPWM_EDGE_A0I1);
    mcpwm_SetSamplCycle(mc, md, 0x04L);   // sample cycle: 20ms
    mcpwm_SetOutPolarity(mc, md, MCPWM_HPOL_INVERSE + MCPWM_LPOL_INVERSE);
    mcpwm_SetDeadband(mc, md, 3L);
    mcpwm_ReloadOUT_Unsafe(mc, md, MCPWM_RELOAD_PEREND);  
    disable_MCINT();
    if(init_mc_irq() == false){
      printf("Init MC IRQ fail\n");
      return;
    }
    clear_INTSTATUS();
    enable_MCINT(PULSE_END_INT);
    if(frequency <= 0)
      mcpwm_SetWidth(mc, md, (duration*1000) * SYSCLK,(duration*1000)/2 * SYSCLK);  // period: (1.0/frequency)*1000000)ms 單位ms
    else
      mcpwm_SetWidth(mc, md, ((1.0/frequency)*1000000)/2 * SYSCLK, ((1.0/frequency)*500000)/2 * SYSCLK);  // period: (1.0/frequency)*1000000)ms 單位ms     
    mcpwm_ReloadPWM(mc, md, MCPWM_RELOAD_PEREND);
    mcpwm_Enable(mc, md);
    use_pin_tone=_pin;
}
void tone_UPDATE(uint8_t _pin, unsigned int frequency, unsigned long duration){
   
    if(frequency <= 0)
      mcpwm_SetWidth(mc, md, (duration*1000) * SYSCLK,(duration*1000)/2 * SYSCLK);  // period: (1.0/frequency)*1000000)ms 單位ms 
    else
      mcpwm_SetWidth(mc, md, ((1.0/frequency)*1000000)/2 * SYSCLK, ((1.0/frequency)*500000)/2 * SYSCLK);  // period: (1.0/frequency)*1000000)ms 單位ms     
    mcpwm_ReloadPWM(mc, md, MCPWM_RELOAD_PEREND);    
}
void tone(uint8_t _pin, unsigned int frequency, unsigned long duration)
{
    if(frequency>=10000)
      frequency=10000;
    if(duration<=0)
      use_pin_times=-1;
    else{  
      if(frequency > 0)    
        use_pin_times=duration / (((1.0/(float)frequency)*1000.0)/2.0);
      else
        use_pin_times=0;
    }
    if(use_pin_tone==255 ){
      tone_INIT(_pin,frequency,duration);
      return;  
    }
    if(use_pin_tone ==_pin){
      tone_UPDATE(_pin,frequency,duration);
      return;
    }
}


// XXX: this function only works properly for timer 2 (the only one we use
// currently).  for the others, it should end the tone, but won't restore
// proper PWM functionality for the timer.
void disableTimer(uint8_t _timer)
{

}




