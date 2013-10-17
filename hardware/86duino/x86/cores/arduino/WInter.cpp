#include <stdio.h>
#include "Arduino.h"
#include "mcm.h"
#include "irq.h"


// MCM INTERRUPT SETTING FUNCTION //
static int mcint_offset[2] = {0, 24};
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
int mc = 0, md = 1;

static void write_mc_pcireg(unsigned idx, unsigned long val) {
    unsigned long cf8 = (0x01L << 31)  // Type-1 PCI configuration
                      + (0x00L << 16)  // Bus 0x00
                      + (DEVICE << 11) // Device 0x01
                      + (0x00L << 8)   // Fun 0x00;
                      + idx;

    io_DisableINT();
	io_outpdw(0x0cf8, cf8 & 0xfffffffcL);
	io_outpdw(0x0cfc, val);
    io_RestoreINT();
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
    io_RestoreINT();

    return tmp;
}

static unsigned char GetMCIRQ(void) {  
    return (unsigned char)(read_mc_pcireg(0x3c) & 0xffL);
}

static void Set_MCIRQ(unsigned char irq) {
    write_mc_pcireg(0x3c, (read_mc_pcireg(0x3c) & 0xffffff00L) | (unsigned long) irq);
    sb1_Write8(0xb4, (sb1_Read8(0xb4) & 0xf0) | int_routing_table[irq]);
}

static void clear_INTSTATUS(void) {
    mc_outp(mc, 0x04, 0xff0000ffL); //for EX
}

static void disable_MCINT(void) {
    mc_outp(mc, 0x00, 0x00L);  // disable mc interrupt
    mc_outp(MC_GENERAL, 0x38, mc_inp(MC_GENERAL, 0x38) | (1L << mc));
}

static void enable_MCINT(unsigned long used_int) {
	mc_outp(MC_GENERAL, 0x38, mc_inp(MC_GENERAL, 0x38) & ~(1L << mc));
	mc_outp(mc, 0x00, used_int<<mcint_offset[md]);
}

#define MCSIF_PFAU_CAPCTRLREG3  (0x14L)
#define MCSIF_PFAU_CAPCTRLREG2  (0x20L)
#define MCSIF_PFAU_CAPCTRLREG1  (0x2cL)
static unsigned long MCSIF_modOffset[2] = {0x08L, 0x8cL};
void mcpfau_SetCap1INT(int mc, int module, unsigned long interval)
{
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG1;
	interval = (interval << 11L) & 0x0000f800L;
    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffff07ffL) + interval);
}

void mcpfau_SetCap2INT(int mc, int module, unsigned long interval)
{
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG2;
	interval = (interval << 11L) & 0x0000f800L;
    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffff07ffL) + interval);
}

void mcpfau_SetCap3INT(int mc, int module, unsigned long interval)
{
    unsigned long reg = MCSIF_modOffset[module] + MCSIF_PFAU_CAPCTRLREG3;
	interval = (interval << 11L) & 0x0000f800L;
    mc_outp(mc, reg, (mc_inp(mc, reg) & 0xffff07ffL) + interval);
}






static void (*sifIntMode[3])(int, int, unsigned long) = {mcpfau_SetCapMode1, mcpfau_SetCapMode2, mcpfau_SetCapMode3}; 
static bool mcm_init[4] = {false, false, false, false};
static int user_int(int irq, void* data) {
	int i, m, n;
	
	// detect interrupt pin
	for(i=0; i<EXTERNAL_NUM_INTERRUPTS; i++)
	{
		m = i/3; // sif mc
		n = i%3; // offset
		if(mcm_init[m] == false) {i += 2; continue;}
		if((mc_inp(m, 0x04) & ((0x20000000L)<<n)) != 0L)
		{
			mc_outp(m, 0x04, (0x20000000L)<<n);
			break;
		}
	}
	
	// execute user function for the pin
	if(i < EXTERNAL_NUM_INTERRUPTS) _userfunc[i]();
	
	return ISR_HANDLED;
}

static uint8_t used_irq = 0xff;
static bool interrupt_init(void) {
	if(used_irq != 0xff) return true;
	
	if(irq_Init() == false) 
    {
        printf("irq_init fail\n"); return false;
    }
    
    if(irq_Setting(GetMCIRQ(), IRQ_LEVEL_TRIGGER) == false)
    {
        printf("%s\n", __FUNCTION__); return false;
    }
    
    if(irq_InstallISR(GetMCIRQ(), user_int, NULL) == false)
	{
	    printf("irq_install fail\n"); return false;
	}
	
	printf("BaseAddr = %08lxh irq = %d\n\n", mc_setbaseaddr(), GetMCIRQ());
	//Master_DX2();
	
	used_irq = GetMCIRQ();
	Set_MCIRQ(used_irq);
	return true;
}


static void mcmsif_init(void) {
    if(mcm_init[mc] == true) return;
	if(mc_SetMode(mc, MCMODE_PWM_SIFB) == false)
    {
    	printf("mc_SetMode() error\n"); return;
	}
    
    mcsif_SetInputFilter(mc, md, 0L);
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
	
	mcm_init[mc] = true;
}

static void mcmsif_close(void) {
	mcsif_Disable(mc, md);
	mcm_init[mc] = false;
}

void attachInterrupt(uint8_t interruptNum, void (*userFunc)(void), int mode) {
	int i;
	unsigned short crossbar_ioaddr;
	
	if(interruptNum >= EXTERNAL_NUM_INTERRUPTS)
	{
		printf("This interrupt%d has no one pin to use\n", interruptNum);
		return;
	}
    mc = interruptNum/3;
    md = MCSIF_MODULEB;
    
    if(_userfunc[interruptNum] != NULL) return;
	if(interrupt_init() == false) return;
	mcmsif_init();
	
    clear_INTSTATUS();
	enable_MCINT(0xe0);
	
	crossbar_ioaddr = sb_Read16(0x64)&0xfffe;
    if (mc == 0)
		io_outpb(crossbar_ioaddr + 2, 0x01); // GPIO port2: 0A, 0B, 0C, 3A
	else if (mc == 1)
    	io_outpb(crossbar_ioaddr + 3, 0x02); // GPIO port3: 1A, 1B, 1C, 3B
	else if(mc == 2)
		io_outpb(crossbar_ioaddr, 0x03); // GPIO port0: 2A, 2B, 2C, 3C
	else if(mc == 3)
	{
		io_outpb(crossbar_ioaddr + 2, 0x01);
		io_outpb(crossbar_ioaddr + 3, 0x02);
		io_outpb(crossbar_ioaddr, 0x03);
	}
	
	mcsif_Disable(mc, md);

	_userfunc[interruptNum] = userFunc;
	
	switch (mode) {
	case CHANGE:
		sifIntMode[interruptNum%3](mc, md, MCPFAU_CAP_BOTH);
		break;
	case FALLING:
		sifIntMode[interruptNum%3](mc, md, MCPFAU_CAP_1TO0);
		break;	
	case RISING:
	    sifIntMode[interruptNum%3](mc, md, MCPFAU_CAP_0TO1);
	    break;
	default:
		printf("No support this mode\n");
		return;
	}
	
	// switch crossbar to MCM_SIF_PIN
	io_outpb(crossbar_ioaddr + 0x90 + pin_offset[interruptNum], 0x08);//RICH IO
	mcsif_Enable(mc, md);
}

void detachInterrupt(uint8_t interruptNum) {
	int i;
	mc = interruptNum/3;
	
	if(interruptNum >= EXTERNAL_NUM_INTERRUPTS) return;
	if(_userfunc[interruptNum] == NULL) return;
	
	mcsif_Disable(mc, md);
	sifIntMode[interruptNum%3](mc, md, MCPFAU_CAP_DISABLE);
	_userfunc[interruptNum] = NULL;
	
	for(i=0; i<3; i++)
		if(_userfunc[mc*3+i] != NULL) break;
	if(i == 3) mcmsif_close(); else mcsif_Enable(mc, md);
	for(i=0; i<EXTERNAL_NUM_INTERRUPTS; i++)
		if(_userfunc[i] != NULL) break;
	if(i == EXTERNAL_NUM_INTERRUPTS)
		if(irq_UninstallISR(used_irq, NULL) == false)
		    printf("irq_install fail\n");
}

