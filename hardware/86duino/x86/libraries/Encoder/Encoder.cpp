/*
  Encoder.cpp - DM&P Vortex86 Encoder library
  Copyright (c) 2014 Android Lin <acen@dmp.com.tw>. All right reserved.

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Arduino.h"

#include "mcm.h"
#include "irq.h"
#include "Encoder.h"

static uint8_t used_irq = 0xff;
static char* name = "attachInt";
static int mcint_offset[2] = {0, 24};

static void clear_INTSTATUS(int mc) {
    mc_outp(mc, 0x04, 0xff000000L); //for EX SIFB
}

#define MCMINT_ENABLE_REG  (0x00)
#define MCMINT_STAT_REG    (0x04)

// define SIFB interrupt bits for encoder mode
#define SIFB_TRIGRESETBIT      (31)
#define SIFB_USEREVTBIT        (30)
#define SIFB_PCNT_OV           (25)
#define SIFB_PCNT_UV           (24)

// define SIFB interrupt bits for pulse & capture mode
#define SIFB_CAP1INTBIT        (29)
#define SIFB_CAP2INTBIT        (30)
#define SIFB_CAP3INTBIT        (31)
static void enable_MCINT(int mc, int bit) {
	mc_outp(mc, 0x00, mc_inp(mc, 0x00) | (0x01<<bit));
}

static void disable_MCINT(int mc, int bit) {
	mc_outp(mc, 0x00, mc_inp(mc, 0x00) & ~(0x01<<bit));
}

static void clear_interrupt_state(int mc, int bit) {
    mc_outp(mc, MCMINT_STAT_REG, (0x01)<<bit);
}

static bool check_interrupt_state(int mc, int bit) {
    if((mc_inp(mc, MCMINT_STAT_REG) & (0x01<<bit)) != 0L) return true;
	return false;
}

static bool check_interrupt_enable(int mc, int bit) {
	if((mc_inp(mc, MCMINT_ENABLE_REG) & (0x01<<bit)) != 0L) return true;
	return false;
}

static void (*(_pcapfunc[4][3]))(int) = {{NULL, NULL, NULL}, {NULL, NULL, NULL}, {NULL, NULL, NULL}, {NULL, NULL, NULL}};
static void (*(_encfunc[4]))(int) = {NULL, NULL, NULL, NULL};
static void (*(sifIntMode[3]))(int, int, unsigned long) = {mcpfau_SetCapMode1, mcpfau_SetCapMode2, mcpfau_SetCapMode3}; 
static void (*(sifSetPol[3]))(int, int, unsigned long) = {mcpfau_SetPolarity1, mcpfau_SetPolarity2, mcpfau_SetPolarity3}; 
static void (*(sifSetRelease[3]))(int, int, unsigned long) = {mcpfau_SetFAU1RELS, mcpfau_SetFAU2RELS, mcpfau_SetFAU3RELS}; 
static void (*(sifClearStat[3]))(int, int) = {mcpfau_ClearFAU1STAT, mcpfau_ClearFAU2STAT, mcpfau_ClearFAU3STAT}; 
static void (*(sifSetMask[3]))(int, int, unsigned long) = {mcpfau_SetMask1, mcpfau_SetMask2, mcpfau_SetMask3}; 
static void (*(sifSetInt[3]))(int, int, unsigned long) = {mcpfau_SetCap1INT, mcpfau_SetCap2INT, mcpfau_SetCap3INT};
static unsigned long (*(readCapStat[3]))(int, int) = {mcpfau_ReadCAPSTAT1, mcpfau_ReadCAPSTAT2, mcpfau_ReadCAPSTAT3};
static unsigned long (*(readCapFIFO[3]))(int, int, unsigned long*) = {mcpfau_ReadCAPFIFO1, mcpfau_ReadCAPFIFO2, mcpfau_ReadCAPFIFO3};
static volatile unsigned long _mcmode[4] = {MODE_NOSET, MODE_NOSET, MODE_NOSET, MODE_NOSET};
static volatile unsigned long _encmode[4] = {0, 0, 0, 0};
static volatile unsigned long long int pulseInData[4][3][2] = {0L};
static unsigned long long int ovdata[3] = {0L, 0L, 0L};
static volatile int capture_state = 0; // for MODE_CAPTURE
//static int test_count[3] = {0, 0, 0}; // for debugging
//static unsigned long test_count = 0;
static int user_int(int irq, void* data) {
	int i, mc, irq_handled = 0, ovcounter = 0, uvcounter = 0, idxcounter = 0;
	unsigned long capdata, stat;
	
	// detect all sensor interface
	for(mc=0; mc<4; mc++)
	{
		if(_mcmode[mc] == MODE_NOSET) continue;
		
		if(_mcmode[mc] != MODE_SSI && _mcmode[mc] != MODE_CAPTURE)
		{
			while(mcenc_ReadCAPSTAT(mc, MCSIF_MODULEB) != MCENC_CAPFIFO_EMPTY)
			{
				stat = mcenc_ReadCAPFIFO(mc, MCSIF_MODULEB, &capdata);
				if(stat == MCENC_CAP_PCNT_OVERFLOW)
				{
					clear_interrupt_state(mc, SIFB_PCNT_OV);
					irq_handled |= 0x01;
					ovcounter++;
				}
				else if(stat == MCENC_CAP_PCNT_UNDERFLOW)
				{
					clear_interrupt_state(mc, SIFB_PCNT_UV);
					irq_handled |= 0x01;
					uvcounter++;
				}
				else if(stat == MCENC_CAP_IDXCOND)
				{
					clear_interrupt_state(mc, SIFB_TRIGRESETBIT);
					irq_handled |= 0x01;
					idxcounter++;
				}
			}
			
			if((_encmode[mc] & INTR_OVERFLOW) == INTR_OVERFLOW && _encfunc[mc] != NULL)
			{
				for(;ovcounter > 0; ovcounter--)
					_encfunc[mc](INTR_OVERFLOW);
			}
			
			if((_encmode[mc] & INTR_UNDERFLOW) == INTR_UNDERFLOW && _encfunc[mc] != NULL)
			{
				for(;uvcounter > 0; uvcounter--)
					_encfunc[mc](INTR_UNDERFLOW);
			}
			
			if((_encmode[mc] & INTR_INDEX) == INTR_INDEX && _encfunc[mc] != NULL)
			{
				for(;idxcounter > 0; idxcounter--)
					_encfunc[mc](INTR_INDEX);
			}
			
			if(check_interrupt_enable(mc, SIFB_USEREVTBIT) == true && check_interrupt_state(mc, SIFB_USEREVTBIT) == true) // USER EVT
			{
				clear_interrupt_state(mc, SIFB_USEREVTBIT);
				irq_handled |= 0x02;
				if((_encmode[mc] & INTR_COMPARE) == INTR_COMPARE && _encfunc[mc] != NULL) _encfunc[mc](INTR_COMPARE);
			}
		}
		else if(_mcmode[mc] == MODE_CAPTURE)
		{
			for(i=0; i<3; i++)
			{
				if(check_interrupt_enable(mc, SIFB_CAP1INTBIT+i) == true && check_interrupt_state(mc, SIFB_CAP1INTBIT+i) == true) // USER EVT
				{
					clear_interrupt_state(mc, SIFB_CAP1INTBIT+i);
					irq_handled |= 0x04;
					                                           
					while(readCapStat[i](mc, MCSIF_MODULEB) != MCPFAU_CAPFIFO_EMPTY)
					{
						stat = readCapFIFO[i](mc, MCSIF_MODULEB, &capdata);
						if(stat == MCPFAU_CAP_CAPCNT_OVERFLOW)
							ovdata[i] += 0x10000000L;
						else if(stat == MCPFAU_CAP_1TO0EDGE)
						{
							pulseInData[mc][i][1] = capdata + ovdata[i];
							ovdata[i] = 0L;
							if(i == 0)      capture_state = INTR_A_PULSE_HIGH;
							else if(i == 1) capture_state = INTR_B_PULSE_HIGH;
							else            capture_state = INTR_Z_PULSE_HIGH;
							if(_pcapfunc[mc][i] != NULL) _pcapfunc[mc][i](capture_state);
							capture_state = 0;
						}
						else if(stat == MCPFAU_CAP_0TO1EDGE)
						{
							pulseInData[mc][i][0] = capdata + ovdata[i];
							ovdata[i] = 0L;
							if(i == 0)      capture_state = INTR_A_PULSE_LOW;
							else if(i == 1) capture_state = INTR_B_PULSE_LOW;
							else            capture_state = INTR_Z_PULSE_LOW;
							if(_pcapfunc[mc][i] != NULL) _pcapfunc[mc][i](capture_state);
							capture_state = 0;
						}
					}
				}
			}
		}
		// SSI mode have no interrupt event
	}
	
	if(irq_handled == 0x00) return ISR_NONE;
	return ISR_HANDLED;
}

/*
unsigned long Encoder::readtestcount() {
	return test_count;
}
*/

static void _filterAndSampleWindowInit(int mc, int md) {
	mcsif_SetInputFilter(mc, md, 0L);
  	mcsif_SetSWDeadband(mc, md, 0L);
  	mcsif_SetSWPOL(mc, md, MCSIF_SWPOL_REMAIN);
  	mcsif_SetSamplWin(mc, md, MCSIF_SWSTART_DISABLE + MCSIF_SWEND_NOW);
  	mcsif_SetSamplWin(mc, md, MCSIF_SWSTART_NOW + MCSIF_SWEND_DISABLE);
}

static void _defaultEncoderSetting(int mc, int md) {
    mcenc_SetIdxCond(mc, md, MCENC_PDIR_IDXCOND_DISABLE, 0L);
    mcenc_SetCapMode(mc, md, MCENC_CAP_PCNT_DISABLE + MCENC_CAP_EXTRIG_DISABLE + MCENC_CAP_IDXCOND_DISABLE);
    
	mcenc_SetCntMin(mc, md, 0L);
    mcenc_SetCntMax(mc, md, 0xffffffffL);
    mcenc_SetResetMode(mc, md, MCENC_RESET_INC_CNTMIN + MCENC_RESET_DEC_CNTMAX);
    mcenc_SetCntIdx(mc, md, 0L);
    mcenc_SetTrigResetMode(mc, md, MCENC_TRIGRESET_IDXCOND_0TO1);
    mcenc_SetPulCnt(mc, md, 0L);
}

Encoder::Encoder(int mc) {
	mdn = 1; // for 86Duino
	mcn = mc;
	mode = MODE_NOSET;
	_setZPol = false;
	_dir = 0;
}


void Encoder::_pdirInit(int samplerate) {
  	_filterAndSampleWindowInit(mcn, mdn);
	
	mcsif_SetMode(mcn, mdn, MCSIF_ENC_PDIR);
    
    _defaultEncoderSetting(mcn, mdn);
    
    io_DisableINT();
	if(samplerate == 1)
	{
    	mcenc_SetCntMode(mcn, mdn, MCENC_PDIR_DIR0_INC0TO1 + MCENC_PDIR_DIR1_DEC0TO1);
    	_mcmode[mcn] = MODE_STEP_DIR;
    }
    else
    {
    	mcenc_SetCntMode(mcn, mdn, MCENC_PDIR_DIR0_INCBOTH + MCENC_PDIR_DIR1_DECBOTH);
    	_mcmode[mcn] = MODE_STEP_DIR_x2;
    }
	io_RestoreINT();
}

void Encoder::_cwccwInit(int samplerate) {
  	_filterAndSampleWindowInit(mcn, mdn);
	
	mcsif_SetMode(mcn, mdn, MCSIF_ENC_CWCCW);
    
    _defaultEncoderSetting(mcn, mdn);
    
    io_DisableINT();
    if(samplerate == 1)
    {
    	mcenc_SetCntMode(mcn, mdn, MCENC_CWCCW_CW_INC0TO1 + MCENC_CWCCW_CCW_DEC0TO1);
		_mcmode[mcn] = MODE_CWCCW;
	}
    else
    {
    	mcenc_SetCntMode(mcn, mdn, MCENC_CWCCW_CW_INCBOTH + MCENC_CWCCW_CCW_DECBOTH);
    	_mcmode[mcn] = MODE_CWCCW_x2;
    }
	io_RestoreINT();
}

void Encoder::_pabInit(int samplerate) {
  	_filterAndSampleWindowInit(mcn, mdn);
	
	mcsif_SetMode(mcn, mdn, MCSIF_ENC_PAB);
    
    _defaultEncoderSetting(mcn, mdn);
    
    io_DisableINT();
    if(samplerate == 1)
    {
		mcenc_SetCntMode(mcn, mdn, MCENC_PAB_DIR0_INCA + MCENC_PAB_DIR1_DECA);
		_mcmode[mcn] = MODE_AB_PHASE;
	}
	else
	{
		mcenc_SetCntMode(mcn, mdn, MCENC_PAB_DIR0_INCAB + MCENC_PAB_DIR1_DECAB);
		_mcmode[mcn] = MODE_AB_PHASE_x2;
	}
	io_RestoreINT();
}                                                   

void Encoder::_pcapInit(void) {
	_filterAndSampleWindowInit(mcn, mdn);
	
	mcsif_SetMode(mcn, mdn, MCSIF_PFAU);
	
    mcpfau_SetCapMode1(mcn, mdn, MCPFAU_CAP_BOTH_CLEAR);
    mcpfau_SetCapInterval1(mcn, mdn, 1L);             
    mcpfau_SetCap1INT(mcn, mdn, 0L);
    mcpfau_SetPolarity1(mcn, mdn, MCPFAU_POL_NORMAL);
    mcpfau_SetMask1(mcn, mdn, MCPFAU_MASK_NONE);
    mcpfau_SetRLDTRIG1(mcn, mdn, MCPFAU_RLDTRIG_DISABLE);
    mcpfau_SetFAU1TRIG(mcn, mdn, MCPFAU_FAUTRIG_INPUT1);
    mcpfau_SetFAU1RELS(mcn, mdn, MCPFAU_FAURELS_INPUT0);
	
    mcpfau_SetCapMode2(mcn, mdn, MCPFAU_CAP_BOTH_CLEAR);     
    mcpfau_SetCapInterval2(mcn, mdn, 1L);
    mcpfau_SetCap2INT(mcn, mdn, 0L);
    mcpfau_SetPolarity2(mcn, mdn, MCPFAU_POL_NORMAL);
    mcpfau_SetMask2(mcn, mdn, MCPFAU_MASK_NONE);
    mcpfau_SetRLDTRIG2(mcn, mdn, MCPFAU_RLDTRIG_DISABLE);
    mcpfau_SetFAU2TRIG(mcn, mdn, MCPFAU_FAUTRIG_INPUT1);
    mcpfau_SetFAU2RELS(mcn, mdn, MCPFAU_FAURELS_INPUT0);

    mcpfau_SetCapMode3(mcn, mdn, MCPFAU_CAP_BOTH_CLEAR);
    mcpfau_SetCapInterval3(mcn, mdn, 1L);
    mcpfau_SetCap3INT(mcn, mdn, 0L);
    mcpfau_SetPolarity3(mcn, mdn, MCPFAU_POL_NORMAL);
    mcpfau_SetMask3(mcn, mdn, MCPFAU_MASK_NONE);
    mcpfau_SetRLDTRIG3(mcn, mdn, MCPFAU_RLDTRIG_DISABLE);          
	mcpfau_SetFAU3TRIG(mcn, mdn, MCPFAU_FAUTRIG_INPUT1);
	mcpfau_SetFAU3RELS(mcn, mdn, MCPFAU_FAURELS_INPUT0);
	
	io_DisableINT();
	_mcmode[mcn] = MODE_CAPTURE;
	io_RestoreINT();
}

void Encoder::_ssiInit(unsigned long bits, unsigned long clk, unsigned long wtime, bool gray2bin) {
  	_filterAndSampleWindowInit(mcn, mdn);
	
	mcsif_SetMode(mcn, mdn, MCSIF_SSI);

    if(clk == 0L) clk = 1000000L; // default value
    if(wtime == 0L) wtime = 20L; // default value
	if(bits > 32) bits = 32L; // default value
	
    clk = ((100000000L)/(clk))/2L; // = ((1/clk)x1000000x100)/2
    wtime = wtime * 100L;
    if(wtime <= clk) return;
    
	mcssi_SetClock(mcn, mdn, clk-1L);
	mcssi_SetWaitTime(mcn, mdn, wtime-1L);
	mcssi_SetLatchPhase(mcn, mdn, MCSSI_LATCH_PHASE0);
    mcssi_SetLatchTime(mcn, mdn, 0L);
	mcssi_SetNumberBITS(mcn, mdn, bits-1L);
	mcssi_SetCntMode(mcn, mdn, MCSSI_CONTINUE);

    if(gray2bin == true) mcssi_SetGAY2BINBit(mcn, mdn);
    
	io_DisableINT();
	_mcmode[mcn] = MODE_SSI;
	io_RestoreINT();
}

static void open_mcm_crossbar(int mc) {
	unsigned short crossbar_ioaddr;
	
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
}

static void open_encoder_pin(int mc, int pin) {
	unsigned short crossbar_ioaddr;
	crossbar_ioaddr = sb_Read16(0x64)&0xfffe;
	io_outpb(crossbar_ioaddr + 0x90 + pin_offset[mc*3+pin], 0x08);//RICH IO
}

void Encoder::begin(int sifmode, unsigned long bits, unsigned long clk, unsigned long wtime, bool gray2bin) {
	if(mode != MODE_NOSET) return;
	
	mode = sifmode;
	if(mode == MODE_CWCCW)            _cwccwInit(1);
	else if(mode == MODE_AB_PHASE)    _pabInit(1);
	else if(mode == MODE_STEP_DIR_x2) _pdirInit(2);
	else if(mode == MODE_CWCCW_x2)    _cwccwInit(2);
	else if(mode == MODE_AB_PHASE_x2) _pabInit(2);
	else if(mode == MODE_CAPTURE)     _pcapInit();
	else if(mode == MODE_SSI)         _ssiInit(bits, clk, wtime, gray2bin);
	else 
	{
		mode = MODE_STEP_DIR;
		_pdirInit(1);
	}
	
	open_mcm_crossbar(mcn);
	open_encoder_pin(mcn, 0);
	open_encoder_pin(mcn, 1);
	open_encoder_pin(mcn, 2);
	
	mcsif_Enable(mcn, mdn);
}

void Encoder::end() {
	int i, j;
	if(mode == MODE_NOSET) return;
	
	detachInterrupt();
	
	io_DisableINT();
	mcsif_Disable(mcn, mdn);
	
	if(mode == MODE_CAPTURE)
	{
		disable_MCINT(mcn, SIFB_CAP1INTBIT);
		disable_MCINT(mcn, SIFB_CAP2INTBIT);
		disable_MCINT(mcn, SIFB_CAP3INTBIT);
	}
	else
	{
		disable_MCINT(mcn, SIFB_TRIGRESETBIT);
		disable_MCINT(mcn, SIFB_USEREVTBIT);
	}

	mode = _mcmode[mcn] = MODE_NOSET;
	_setZPol = false;
	_dir = 0;
	io_RestoreINT();
	
	for(i=0; i<4; i++)
	{
		if(_encfunc[i] != NULL) break;
		for(j=0; j<3; j++)
			if(_pcapfunc[i][j] != NULL) break;
	}
	
	if(i == 4 && j == 3)
	{
		mc_outp(MC_GENERAL, 0x38, mc_inp(MC_GENERAL, 0x38) | (1L << mcn));
		if(used_irq == 0xff) return;
		if(irq_UninstallISR(used_irq, (void*)name) == false)
		    printf("irq_uninstall fail\n");
		else
			used_irq = 0xff;
	}
}

void Encoder::write(unsigned long cnt) {
	if(mode == MODE_NOSET || mode == MODE_CAPTURE) return;
	
	mcsif_Disable(mcn, mdn);
	if(mode == MODE_SSI)
		mcssi_SetPulCnt(mcn, mdn, cnt);
	else	
		mcenc_SetPulCnt(mcn, mdn, cnt);
	mcsif_Enable(mcn, mdn);
}

static bool _pcapAttchINT = false; //only for MODE_CAPTURE
void Encoder::setDigitalFilter(unsigned long width) {
    if(mode == MODE_NOSET) return;
    if(mode == MODE_CAPTURE && _pcapAttchINT == true) return;
    
    mcsif_Disable(mcn, mdn);
    mcsif_SetInputFilter(mcn, mdn, width);
    mcsif_Enable(mcn, mdn);
}

void Encoder::setIndexReset(bool condition) {
	if(mode == MODE_NOSET || mode == MODE_CAPTURE || mode == MODE_SSI) return;
	
	mcsif_Disable(mcn, mdn);
	
	if(condition == true)
	{
		mcenc_SetResetMode(mcn, mdn, MCENC_RESET_INC_CNTMINIDX + MCENC_RESET_DEC_CNTMAXIDX);
        mcenc_SetCapMode(mcn, mdn, MCENC_CAP_PCNT_DISABLE + MCENC_CAP_EXTRIG_DISABLE + MCENC_CAP_IDXCOND_ENABLE); // start to use FIFO (mean that capture event will occur)

		if(_setZPol == false) // if you use setInputPolarity() to set Z pin's pol to inverse before setIndexReset()
		{
			// In fact, below actions are same as mcenc_SetIdxCond(mcn, mdn, MCENC_PDIR_IDXCOND_Z, MCENC_ICZ1);
			if(mode == MODE_STEP_DIR || mode == MODE_STEP_DIR_x2)
				mcenc_SetIdxCond(mcn, mdn, MCENC_PDIR_IDXCOND_Z, MCENC_ICZ1);
			else if(mode == MODE_CWCCW || mode == MODE_CWCCW_x2)
				mcenc_SetIdxCond(mcn, mdn, MCENC_CWCCW_IDXCOND_Z, MCENC_ICZ1);
			else if(mode == MODE_AB_PHASE || mode == MODE_AB_PHASE_x2)
				mcenc_SetIdxCond(mcn, mdn, MCENC_PAB_IDXCOND_Z, MCENC_ICZ1);
		}
		
		io_DisableINT();
		_encmode[mcn] |= INTR_INDEX;
		io_RestoreINT();
	}
	else
	{
		if((_encmode[mcn] & (INTR_INDEX | INTR_OVERFLOW | INTR_UNDERFLOW)) == 0)
			mcenc_SetCapMode(mcn, mdn, MCENC_CAP_PCNT_DISABLE + MCENC_CAP_EXTRIG_DISABLE + MCENC_CAP_IDXCOND_DISABLE);
		mcenc_SetIdxCond(mcn, mdn, MCENC_PDIR_IDXCOND_DISABLE, 0L);
		mcenc_SetResetMode(mcn, mdn, MCENC_RESET_INC_CNTMIN + MCENC_RESET_DEC_CNTMAX);
		
		io_DisableINT();
		_encmode[mcn] &= ~(INTR_INDEX);
		io_RestoreINT();
	}
    
	mcsif_Enable(mcn, mdn);
}

int Encoder::indexRead(void) {
	if(mode == MODE_NOSET || mode == MODE_CAPTURE || mode == MODE_SSI) return 0xff;
	return ((mcpfau_ReadCapStatREG(mcn, mdn) & 0x20) != 0L) ? HIGH : LOW;
}

void Encoder::setComparator(unsigned long evncnt, bool condition) {
	if(mode == MODE_NOSET || mode == MODE_CAPTURE || mode == MODE_SSI) return;
	
	mcsif_Disable(mcn, mdn);
	
	mcenc_SetCntEvt(mcn, mdn, evncnt);
	
	io_DisableINT();
	if(condition == true)
		_encmode[mcn] |= INTR_COMPARE;
	else
		_encmode[mcn] &= ~(INTR_COMPARE);
	io_RestoreINT();
	
	mcsif_Enable(mcn, mdn);
}

void Encoder::setRange(unsigned long val, bool condition) {
	if(mode == MODE_NOSET || mode == MODE_CAPTURE || mode == MODE_SSI) return;
	if(val == 0L) return;
    
	mcsif_Disable(mcn, mdn);
    
	if(condition == true)
	{
		mcenc_SetCapMode(mcn, mdn, MCENC_CAP_PCNT_DISABLE + MCENC_CAP_EXTRIG_DISABLE + MCENC_CAP_IDXCOND_ENABLE); // start to use FIFO (mean that capture event will occur)
		io_DisableINT();
		_encmode[mcn] |= INTR_OVERFLOW;
		_encmode[mcn] |= INTR_UNDERFLOW;
		io_RestoreINT();
	}
	else
	{
		if((_encmode[mcn] & (INTR_INDEX | INTR_OVERFLOW | INTR_UNDERFLOW)) == 0 && condition == false)
			mcenc_SetCapMode(mcn, mdn, MCENC_CAP_PCNT_DISABLE + MCENC_CAP_EXTRIG_DISABLE + MCENC_CAP_IDXCOND_DISABLE);
		io_DisableINT();
		_encmode[mcn] &= ~(INTR_OVERFLOW);
		_encmode[mcn] &= ~(INTR_UNDERFLOW);
		io_RestoreINT();
	}
	
	mcenc_SetCntMax(mcn, mdn, val);
	
	mcsif_Enable(mcn, mdn);
}

// return
// 1 : Forward
// -1 : Reverse
// 0xff : error mode
int Encoder::directionRead(void) {
	unsigned long ret;
	if(mode == MODE_NOSET || mode == MODE_CAPTURE || mode == MODE_SSI) return 0xff;
	if(mcenc_ReadDIR(mcn, mdn) == 0L) ret = 1; else ret = -1;
	return ((_dir == 0) ? ret : ((ret == 1) ? -1 : 1));
}

unsigned long Encoder::read() {
	if(mode == MODE_NOSET) return 0L;

	if(mode == MODE_CAPTURE)
	{
		if(capture_state == INTR_A_PULSE_LOW)
			return (pulseInData[mcn][0][0]/100L);
		else if(capture_state == INTR_A_PULSE_HIGH)
			return (pulseInData[mcn][0][1]/100L);
		else if(capture_state == INTR_B_PULSE_LOW)
			return (pulseInData[mcn][1][0]/100L);
		else if(capture_state == INTR_B_PULSE_HIGH)
			return (pulseInData[mcn][1][1]/100L);
		else if(capture_state == INTR_Z_PULSE_LOW)
			return (pulseInData[mcn][2][0]/100L);
		else if(capture_state == INTR_Z_PULSE_HIGH)
			return (pulseInData[mcn][2][1]/100L);
		else
			return 0L;
	}
	
	if(mode == MODE_SSI) return mcssi_ReadData(mcn, mdn);

	return mcenc_ReadPulCnt(mcn, mdn);
}

unsigned long Encoder::readNanoseconds() {
	if(mode == MODE_NOSET) return 0L;
	
	if(mode == MODE_CAPTURE)
	{
		if(capture_state == INTR_A_PULSE_LOW)
			return (pulseInData[mcn][0][0]*10L);
		else if(capture_state == INTR_A_PULSE_HIGH)
			return (pulseInData[mcn][0][1]*10L);
		else if(capture_state == INTR_B_PULSE_LOW)
			return (pulseInData[mcn][1][0]*10L);
		else if(capture_state == INTR_B_PULSE_HIGH)
			return (pulseInData[mcn][1][1]*10L);
		else if(capture_state == INTR_Z_PULSE_LOW)
			return (pulseInData[mcn][2][0]*10L);
		else if(capture_state == INTR_Z_PULSE_HIGH)
			return (pulseInData[mcn][2][1]*10L);
		else
			return 0L; // is not called in callback function, return 0.
	}

	return 0L;
}

static bool interrupt_init(int mc) {
	if(used_irq != 0xff) return true;
    
    if(irq_InstallISR(GetMCIRQ(), user_int, (void*)name) == false)
	{
	    printf("irq_install fail\n"); return false;
	}
	
	// enable mcm general interrupt function
	mc_outp(MC_GENERAL, 0x38, mc_inp(MC_GENERAL, 0x38) & ~(1L << mc));
	used_irq = GetMCIRQ();
	return true;
}

void Encoder::attachInterrupt(void (*callback)(int)) {
	if(mode == MODE_NOSET) return;
	
	if(mode == MODE_CAPTURE && callback != NULL)
	{
		if(_pcapfunc[mcn][0] != NULL || _pcapfunc[mcn][1] != NULL || _pcapfunc[mcn][2] != NULL)	return;
		if(interrupt_init(mcn) == false) return;
		
		mcsif_Disable(mcn, mdn);
	
		io_DisableINT();
		_pcapfunc[mcn][0] = callback;
		_pcapfunc[mcn][1] = callback;
		_pcapfunc[mcn][2] = callback;
		io_RestoreINT();

		clear_INTSTATUS(mcn);
		enable_MCINT(mcn, SIFB_CAP1INTBIT);
		enable_MCINT(mcn, SIFB_CAP2INTBIT);
		enable_MCINT(mcn, SIFB_CAP3INTBIT);
		
		// Enable interrupt option
		for(int i=0; i<3; i++)
		{
			sifIntMode[i](mcn, mdn, MCPFAU_CAP_BOTH_CLEAR);
			sifSetInt[i](mcn, mdn, 1L);
	    }

		mcsif_Enable(mcn, mdn);
		_pcapAttchINT = true;
		return;
	}
	
	if(callback != NULL)
	{
		if(_encfunc[mcn] != NULL) return;
		if(interrupt_init(mcn) == false) return;
		
		mcsif_Disable(mcn, mdn);
		
		io_DisableINT();
		_encfunc[mcn] = callback;
		io_RestoreINT();

		clear_INTSTATUS(mcn);
		enable_MCINT(mcn, SIFB_TRIGRESETBIT);
		enable_MCINT(mcn, SIFB_USEREVTBIT);
		enable_MCINT(mcn, SIFB_PCNT_OV);
		enable_MCINT(mcn, SIFB_PCNT_UV);
		
		// Enable interrupt option
		mcenc_SetCapInterval(mcn, mdn, 1L);
		mcsif_Enable(mcn, mdn);
	}
}

// only for COMPAREMATCH and INDEXRESET condition
void Encoder::detachInterrupt() {
    int i;

	if(mode == MODE_CAPTURE)
	{
		if(_pcapfunc[mcn][0] == NULL || _pcapfunc[mcn][1] == NULL || _pcapfunc[mcn][2] == NULL)	return;
		mcsif_Disable(mcn, mdn);
		
		// Disable interrupt option
		for(i=0; i<3; i++) sifSetInt[i](mcn, mdn, 0L);
		
		io_DisableINT();
		_pcapfunc[mcn][0] = NULL;
		_pcapfunc[mcn][1] = NULL;
		_pcapfunc[mcn][2] = NULL;
		io_RestoreINT();
		
		_pcapAttchINT = false;
		mcsif_Enable(mcn, mdn);
		return;
	}
	
	
	if(_encfunc[mcn] == NULL) return;
	
	mcsif_Disable(mcn, mdn);
	
	// Disable interrupt option
	mcenc_SetCapInterval(mcn, mdn, 0L);
	
	io_DisableINT();
	_encfunc[mcn] = NULL;
	_encmode[mcn] &= ~(INTR_COMPARE);
	_encmode[mcn] &= ~(INTR_INDEX);
	_encmode[mcn] &= ~(INTR_OVERFLOW);
	_encmode[mcn] &= ~(INTR_UNDERFLOW);
	io_RestoreINT();
	
	// re-enable sif
	mcsif_Enable(mcn, mdn);
}

unsigned long long int Encoder::_pulseIn(uint8_t pin, uint8_t state, unsigned long timeout) {
    unsigned long data, _timeout;
	unsigned long long int povdata = 0L, result = 0L;
	int stat;
	
	if(_pcapAttchINT == true || mode != MODE_CAPTURE || pin > 2) return 0L;
	           
	_timeout = timer_NowTime() + timeout;
	if(state == HIGH)
	{
		while(1)
		{
			if(timeout != 0L && timer_NowTime() > _timeout) return 0L;
			if(readCapStat[pin](mcn, MCSIF_MODULEB) != MCENC_CAPFIFO_EMPTY &&
			   readCapFIFO[pin](mcn, MCSIF_MODULEB, &data) == MCPFAU_CAP_0TO1EDGE)
				break;
		}
		
		while(1)
		{
			if(timeout != 0L && timer_NowTime() > _timeout) return 0L;
			if(readCapStat[pin](mcn, MCSIF_MODULEB) != MCENC_CAPFIFO_EMPTY)
			{
				stat = readCapFIFO[pin](mcn, MCSIF_MODULEB, &data);
				if(stat == MCPFAU_CAP_CAPCNT_OVERFLOW)
					povdata += 0x10000000L;
				else if(stat == MCPFAU_CAP_1TO0EDGE)
				{
					result = data + povdata;
					break;
				}
			}
		}
	}
	else
	{
		while(1)
		{
			if(timeout != 0L && timer_NowTime() > _timeout) return 0L;
			if(readCapStat[pin](mcn, MCSIF_MODULEB) != MCENC_CAPFIFO_EMPTY &&
			   readCapFIFO[pin](mcn, MCSIF_MODULEB, &data) == MCPFAU_CAP_1TO0EDGE)
				break;
		}
		
		while(1)
		{
			if(timeout != 0L && timer_NowTime() > _timeout) return 0L;
			if(readCapStat[pin](mcn, MCSIF_MODULEB) != MCENC_CAPFIFO_EMPTY)
			{
				stat = readCapFIFO[pin](mcn, MCSIF_MODULEB, &data);
				if(stat == MCPFAU_CAP_CAPCNT_OVERFLOW)
					povdata += 0x10000000L;
				else if(stat == MCPFAU_CAP_0TO1EDGE)
				{
					result = data + povdata;
					break;
				}
			}
		}
	}
	
	return result;
}

unsigned long Encoder::pulseIn(uint8_t pin, uint8_t state) {
	return (_pulseIn(pin, state, 0L)/100L);
}

unsigned long Encoder::pulseIn(uint8_t pin, uint8_t state, unsigned long timeout) {
	return (_pulseIn(pin, state, timeout)/100L);
}

unsigned long Encoder::pulseInNanoseconds(uint8_t pin, uint8_t state, unsigned long timeout) {
	return (_pulseIn(pin, state, timeout)*10L);
}


void Encoder::setInputPolarity(bool pinA, bool pinB, bool pinZ) {
	if(mode == MODE_NOSET || mode == MODE_CAPTURE || mode == MODE_SSI) return;
	
	mcsif_Disable(mcn, mdn);
	if(mode == MODE_STEP_DIR)
	{
		// 8 conditions
		if(pinA == LOW && pinB == HIGH)
		{
			mcenc_SetCntMode(mcn, mdn, MCENC_PDIR_DIR0_INC1TO0 + MCENC_PDIR_DIR1_DEC1TO0);
			_dir = 0;
		}
		else if(pinA == HIGH && pinB == LOW)
		{
			mcenc_SetCntMode(mcn, mdn, MCENC_PDIR_DIR0_DEC0TO1 + MCENC_PDIR_DIR1_INC0TO1);
			_dir = 1;
		}
		else if(pinA == LOW && pinB == LOW)
		{
			mcenc_SetCntMode(mcn, mdn, MCENC_PDIR_DIR0_DEC1TO0 + MCENC_PDIR_DIR1_INC1TO0);
			_dir = 1;
		}
		else // all HIGH
		{
			mcenc_SetCntMode(mcn, mdn, MCENC_PDIR_DIR0_INC0TO1 + MCENC_PDIR_DIR1_DEC0TO1);
			_dir = 0;
		}
		
		if(pinZ == LOW)
		{
			mcenc_SetIdxCond(mcn, mdn, MCENC_PDIR_IDXCOND_Z, MCENC_ICZ0);
			_setZPol = true;
		}
		else
			mcenc_SetIdxCond(mcn, mdn, MCENC_PDIR_IDXCOND_Z, MCENC_ICZ1);
	}
	else if(mode == MODE_CWCCW)
	{
		if(pinA == LOW && pinB == HIGH)
		{
			mcenc_SetCntMode(mcn, mdn, MCENC_CWCCW_CW_INC1TO0 + MCENC_CWCCW_CCW_DEC0TO1);
		}
		else if(pinA == HIGH && pinB == LOW)
		{
			mcenc_SetCntMode(mcn, mdn, MCENC_CWCCW_CW_INC0TO1 + MCENC_CWCCW_CCW_DEC1TO0);
		}
		else if(pinA == LOW && pinB == LOW)
		{
			mcenc_SetCntMode(mcn, mdn, MCENC_CWCCW_CW_INC1TO0 + MCENC_CWCCW_CCW_DEC1TO0);
		}
		else // all NORMAL
		{
			mcenc_SetCntMode(mcn, mdn, MCENC_CWCCW_CW_INC0TO1 + MCENC_CWCCW_CCW_DEC0TO1);
		}
		
		if(pinZ == LOW)
		{
			mcenc_SetIdxCond(mcn, mdn, MCENC_CWCCW_IDXCOND_Z, MCENC_ICZ0);
			_setZPol = true;
		}
		else
			mcenc_SetIdxCond(mcn, mdn, MCENC_CWCCW_IDXCOND_Z, MCENC_ICZ1);
	}
	else if(mode == MODE_AB_PHASE)
	{
		if((pinA == LOW && pinB == LOW) || (pinA == HIGH && pinB == HIGH))
		{
			mcenc_SetCntMode(mcn, mdn, MCENC_PAB_DIR0_INCA + MCENC_PAB_DIR1_DECA);
			_dir = 0;
		}
		else
		{
			mcenc_SetCntMode(mcn, mdn, MCENC_PAB_DIR0_DECA + MCENC_PAB_DIR1_INCA);
			_dir = 1;
		}
		
		if(pinZ == LOW)
		{
			mcenc_SetIdxCond(mcn, mdn, MCENC_PAB_IDXCOND_Z, MCENC_ICZ0);
			_setZPol = true;
		}
		else
			mcenc_SetIdxCond(mcn, mdn, MCENC_PAB_IDXCOND_Z, MCENC_ICZ1);
	}
	// 2X samplerate cases
	else if(mode == MODE_STEP_DIR_x2)
	{
		// 4 conditions
		if(pinB == LOW)
		{
			mcenc_SetCntMode(mcn, mdn, MCENC_PDIR_DIR0_DECBOTH + MCENC_PDIR_DIR1_INCBOTH);
			_dir = 1;
		}
		else // all HIGH
		{
			mcenc_SetCntMode(mcn, mdn, MCENC_PDIR_DIR0_INCBOTH + MCENC_PDIR_DIR1_DECBOTH);
			_dir = 0;
		}
		
		if(pinZ == LOW)
		{
			mcenc_SetIdxCond(mcn, mdn, MCENC_PDIR_IDXCOND_Z, MCENC_ICZ0);
			_setZPol = true;
		}
		else
			mcenc_SetIdxCond(mcn, mdn, MCENC_PDIR_IDXCOND_Z, MCENC_ICZ1);
	}
	else if(mode == MODE_CWCCW_x2)
	{
		if(pinZ == LOW)
		{
			mcenc_SetIdxCond(mcn, mdn, MCENC_CWCCW_IDXCOND_Z, MCENC_ICZ0);
			_setZPol = true;
		}
		else
			mcenc_SetIdxCond(mcn, mdn, MCENC_CWCCW_IDXCOND_Z, MCENC_ICZ1);
	}
	else if(mode == MODE_AB_PHASE_x2)
	{
		if((pinA == LOW && pinB == LOW) || (pinA == HIGH && pinB == HIGH))
		{
			mcenc_SetCntMode(mcn, mdn, MCENC_PAB_DIR0_INCAB + MCENC_PAB_DIR1_DECAB);
			_dir = 0;
		}
		else
		{
			mcenc_SetCntMode(mcn, mdn, MCENC_PAB_DIR0_DECAB + MCENC_PAB_DIR1_INCAB);
			_dir = 1;
		}
		
		if(pinZ == LOW)
		{
			mcenc_SetIdxCond(mcn, mdn, MCENC_PAB_IDXCOND_Z, MCENC_ICZ0);
			_setZPol = true;
		}
		else
			mcenc_SetIdxCond(mcn, mdn, MCENC_PAB_IDXCOND_Z, MCENC_ICZ1);
	}
	mcsif_Enable(mcn, mdn);
}

#if defined __86DUINO_ONE || defined __86DUINO_ZERO || defined __86DUINO_EDUCAKE
Encoder Enc0(0);
Encoder Enc1(1);
Encoder Enc2(2);
Encoder Enc3(3);
#endif
