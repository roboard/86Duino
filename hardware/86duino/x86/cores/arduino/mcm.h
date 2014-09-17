/*
  mcm.h - DM&P Vortex86DX2/EX MCM library
  Copyright (c) 2013 AAA <aaa@dmp.com.tw>. All right reserved.

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

  Modified 01 November 2013 by Android Lin, DY Hung, and Hellion Chuang
  
  (If you need a commercial license, please contact soc@dmp.com.tw 
   to get more information.)
*/

#ifndef __MCEX_H
#define __MCEX_H

// #include "defines.h"
#include "dmpcfg.h"
#define SYSCLK    (100)
#define DEVICE    (0x10)
#ifdef __cplusplus
extern "C" {
#endif


void set_MMIO(void);
void set_PortIO(void);


/* void Master_DX2(void);
void Master_8051B(void);
unsigned long mc8051_setbaseaddr(void);
void b8051_outpb(unsigned int addr, unsigned char val);
unsigned char b8051_inpb(unsigned int addr);
// enable/disable 8051 INT1
void enable_8051_int1(void);
void disable_8051_int1(void);
// get/clear 8051 INT1 counter
unsigned char read_8051_int1_cnt(void);
void clear_8051_int1_cnt(void);
unsigned char read_8051_tcon(void);
unsigned char read_8051_ie(void); */

// MC no.
#define MC_GENERAL          (-1)
#define MC_MODULE0          (0)
#define MC_MODULE1          (1)
#define MC_MODULE2          (2)
#define MC_MODULE3          (3)
// #define MC_MODULE4          (4)
// #define MC_MODULE5          (5)
// #define MC_MODULE6          (6)
// #define MC_MODULE7          (7)
// #define MC_MODULE8          (8)
// #define MC_MODULE9          (9)
// #define MC_MODULE10         (10)
// #define MC_MODULE11         (11)

// Module no.
#define MCPWM_MODULEA       (0)
#define MCPWM_MODULEB       (1)
#define MCPWM_MODULEC       (2)
#define MCSERVO_MODULEA     (0)
#define MCSERVO_MODULEB     (1)
#define MCSERVO_MODULEC     (2)
#define MCSIF_MODULEA       (0)
#define MCSIF_MODULEB       (1)

unsigned long mc_setbaseaddr(void);
int mc_getIRQ(void);

void mc_outp(int mc, unsigned long idx, unsigned long val);
void mc_outpb(int mc, unsigned long idx, unsigned char val);
unsigned long mc_inp(int mc, unsigned long idx);
unsigned char mc_inpb(int mc, unsigned long idx);
bool mc_IsEnabled(int mc);


/***************************  General Settings  *******************************/

#define MCMODE_SYNCPWM          (0x00L)
#define MCMODE_PWM              (0x01L)
#define MCMODE_SYNCPWM_SIFB     (0x02L)
#define MCMODE_PWM_SIFB         (0x03L)
#define MCMODE_SERVO            (0x04L)
#define MCMODE_SIF              (0x05L)
bool mc_SetMode(int mc, unsigned long mode);


void mc_LockLDRDYs(unsigned long group1, unsigned long group2, unsigned long group3);
void mc_UnlockLDRDYs(unsigned long group1, unsigned long group2, unsigned long group3);

void mc_LockLDRDY(int mc, int module);
void mc_UnlockLDRDY(int mc, int module);


#define MCMULTI_NONE            (0L)
#define MCMULTI_MC0PWMA         (1L << 0)
#define MCMULTI_MC0PWMB         (1L << 1)
#define MCMULTI_MC0PWMC         (1L << 2)
#define MCMULTI_MC1PWMA         (1L << 3)
#define MCMULTI_MC1PWMB         (1L << 4)
#define MCMULTI_MC1PWMC         (1L << 5)
#define MCMULTI_MC2PWMA         (1L << 6)
#define MCMULTI_MC2PWMB         (1L << 7)
#define MCMULTI_MC2PWMC         (1L << 8)
#define MCMULTI_MC3PWMA         (1L << 9)
#define MCMULTI_MC3PWMB         (1L << 10)
#define MCMULTI_MC3PWMC         (1L << 11)
#define MCMULTI_MC0SERVOA       (1L << 0)
#define MCMULTI_MC0SERVOB       (1L << 1)
#define MCMULTI_MC0SERVOC       (1L << 2)
#define MCMULTI_MC1SERVOA       (1L << 3)
#define MCMULTI_MC1SERVOB       (1L << 4)
#define MCMULTI_MC1SERVOC       (1L << 5)
#define MCMULTI_MC2SERVOA       (1L << 6)
#define MCMULTI_MC2SERVOB       (1L << 7)
#define MCMULTI_MC2SERVOC       (1L << 8)
#define MCMULTI_MC3SERVOA       (1L << 9)
#define MCMULTI_MC3SERVOB       (1L << 10)
#define MCMULTI_MC3SERVOC       (1L << 11)
#define MCMULTI_MC0SIFA         (1L << 0)
#define MCMULTI_MC0SIFB         (1L << 16)
#define MCMULTI_MC1SIFA         (1L << 3)
#define MCMULTI_MC1SIFB         (1L << 17)
#define MCMULTI_MC2SIFA         (1L << 6)
#define MCMULTI_MC2SIFB         (1L << 18)
#define MCMULTI_MC3SIFA         (1L << 9)
#define MCMULTI_MC3SIFB         (1L << 19)
void mc_EnableMulti(unsigned long group1, unsigned long group2, unsigned long group3);
void mc_DisableMulti(unsigned long group1, unsigned long group2, unsigned long group3);

unsigned long mc_ReadEnableREG1(void);
unsigned long mc_ReadEnableREG2(void);


#define MCNET_MC2SIF_NONE       (0L)
#define MCNET_MC2SIF_ALL        (0xbfeffbfeL)
#define MCNET_MC0_TO_MC0SIFB    (1L << 1)
#define MCNET_MC0_TO_MC1SIFA    (1L << 2)
#define MCNET_MC0_TO_MC1SIFB    (1L << 3)
#define MCNET_MC0_TO_MC2SIFA    (1L << 4)
#define MCNET_MC0_TO_MC2SIFB    (1L << 5)
#define MCNET_MC0_TO_MC3SIFA    (1L << 6)
#define MCNET_MC0_TO_MC3SIFB    (1L << 7)
#define MCNET_MC1_TO_MC0SIFA    (1L << 8)
#define MCNET_MC1_TO_MC0SIFB    (1L << 9)
#define MCNET_MC1_TO_MC1SIFB    (1L << 11)
#define MCNET_MC1_TO_MC2SIFA    (1L << 12)
#define MCNET_MC1_TO_MC2SIFB    (1L << 13)
#define MCNET_MC1_TO_MC3SIFA    (1L << 14)
#define MCNET_MC1_TO_MC3SIFB    (1L << 15)
#define MCNET_MC2_TO_MC0SIFA    (1L << 16)
#define MCNET_MC2_TO_MC0SIFB    (1L << 17)
#define MCNET_MC2_TO_MC1SIFA    (1L << 18)
#define MCNET_MC2_TO_MC1SIFB    (1L << 19)
#define MCNET_MC2_TO_MC2SIFB    (1L << 21)
#define MCNET_MC2_TO_MC3SIFA    (1L << 22)
#define MCNET_MC2_TO_MC3SIFB    (1L << 23)
#define MCNET_MC3_TO_MC0SIFA    (1L << 24)
#define MCNET_MC3_TO_MC0SIFB    (1L << 25)
#define MCNET_MC3_TO_MC1SIFA    (1L << 26)
#define MCNET_MC3_TO_MC1SIFB    (1L << 27)
#define MCNET_MC3_TO_MC2SIFA    (1L << 28)
#define MCNET_MC3_TO_MC2SIFB    (1L << 29)
#define MCNET_MC3_TO_MC3SIFB    (1L << 31)
void mc_SetRouting_MC2SIF(unsigned long group1, unsigned long group2, unsigned long group3);
void mc_RemoveRouting_MC2SIF(unsigned long group1, unsigned long group2, unsigned long group3);

#define MCNET_SIF2MC_NONE       (0L)
#define MCNET_SIF2MC_ALL        (0xffffffffL)
#define MCNET_MC0SIFA_TO_MC1    (1L << 8)
#define MCNET_MC0SIFA_TO_MC2    (1L << 16)
#define MCNET_MC0SIFA_TO_MC3    (1L << 24)
#define MCNET_MC0SIFB_TO_MC0    (1L << 1)
#define MCNET_MC0SIFB_TO_MC1    (1L << 9)
#define MCNET_MC0SIFB_TO_MC2    (1L << 17)
#define MCNET_MC0SIFB_TO_MC3    (1L << 25)
#define MCNET_MC1SIFA_TO_MC0    (1L << 2)
#define MCNET_MC1SIFA_TO_MC2    (1L << 18)
#define MCNET_MC1SIFA_TO_MC3    (1L << 26)
#define MCNET_MC1SIFB_TO_MC0    (1L << 3)
#define MCNET_MC1SIFB_TO_MC1    (1L << 11)
#define MCNET_MC1SIFB_TO_MC2    (1L << 19)
#define MCNET_MC1SIFB_TO_MC3    (1L << 27)
#define MCNET_MC2SIFA_TO_MC0    (1L << 4)
#define MCNET_MC2SIFA_TO_MC1    (1L << 12)
#define MCNET_MC2SIFA_TO_MC3    (1L << 28)
#define MCNET_MC2SIFB_TO_MC0    (1L << 5)
#define MCNET_MC2SIFB_TO_MC1    (1L << 13)
#define MCNET_MC2SIFB_TO_MC2    (1L << 21)
#define MCNET_MC2SIFB_TO_MC3    (1L << 29)
#define MCNET_MC3SIFA_TO_MC0    (1L << 6)
#define MCNET_MC3SIFA_TO_MC1    (1L << 14)
#define MCNET_MC3SIFA_TO_MC2    (1L << 22)
#define MCNET_MC3SIFB_TO_MC0    (1L << 7)
#define MCNET_MC3SIFB_TO_MC1    (1L << 15)
#define MCNET_MC3SIFB_TO_MC2    (1L << 23)
#define MCNET_MC3SIFB_TO_MC3    (1L << 31)
#define MCNET_8051B_TO_MC0      (1L << 0)
#define MCNET_8051B_TO_MC1      (1L << 10)
#define MCNET_8051B_TO_MC2      (1L << 20)
#define MCNET_8051B_TO_MC3      (1L << 30)
void mc_SetRouting_SIF2MC(unsigned long group1, unsigned long group2, unsigned long group3);
void mc_RemoveRouting_SIF2MC(unsigned long group1, unsigned long group2, unsigned long group3);

/*-----------------------  end of General Settings  --------------------------*/



/*****************************  PWM Settings  *********************************/

void mcpwm_Enable(int mc, int module);
void mcpwm_Disable(int mc, int module);
void mcpwm_DisableProtect(int mc, int module);



#define MCPWM_EDGE_I0A1     (0x00L << 30)
#define MCPWM_EDGE_A0I1     (0x02L << 30)
#define MCPWM_CENTER_I0A1   (0x01L << 30)
#define MCPWM_CENTER_A0I1   (0x03L << 30)
void mcpwm_SetWaveform(int mc, int module, unsigned long mode);

void mcpwm_SetWidth(int mc, int module, unsigned long period, unsigned long phase0);

void mcpwm_SetSamplCycle(int mc, int module, unsigned long sc);

void mcpwm_SetSCRESET(int mc, int module);
void mcpwm_ClearSCRESET(int mc, int module);

void mcpwm_SetSCLAST(int mc, int module);
void mcpwm_ClearSCLAST(int mc, int module);
unsigned long mcpwm_ReadSCLAST(int mc, int module);
unsigned long mcpwm_ReadSCREG(int mc, int module);



unsigned long mcpwm_ReadSTATREG1(int mc, int module);
unsigned long mcpwm_ReadSTATREG2(int mc, int module);



void mcpwm_SetDeadband(int mc, int module, unsigned long db);

#define MCPWM_HMASK_NONE        (0x00L << 26)
#define MCPWM_HMASK_INACTIVE    (0x01L << 26)
#define MCPWM_HMASK_TRISTATE    (0x03L << 26)
#define MCPWM_LMASK_NONE        (0x00L << 24)
#define MCPWM_LMASK_INACTIVE    (0x01L << 24)
#define MCPWM_LMASK_TRISTATE    (0x03L << 24)
void mcpwm_SetOutMask(int mc, int module, unsigned long mask);

#define MCPWM_HPOL_NORMAL       (0x00L << 29)
#define MCPWM_HPOL_INVERSE      (0x01L << 29)
#define MCPWM_LPOL_NORMAL       (0x00L << 28)
#define MCPWM_LPOL_INVERSE      (0x01L << 28)
void mcpwm_SetOutPolarity(int mc, int module, unsigned long pol);

#define MCPWM_MPWM_DISABLE      (0x00L << 30)
#define MCPWM_MPWM_INACTIVE     (0x02L << 30)
#define MCPWM_MPWM_ACTIVE       (0x03L << 30)
void mcpwm_SetMPWM(int mc, int module, unsigned long mpwm);



#define MCPWM_EVT_DISABLE           (0x00L)
#define MCPWM_EVT_PERSC             (0x01L)
#define MCPWM_EVT_PERPWM            (0x02L)
#define MCPWM_EVT_PERPWM2           (0x03L)
void mcpwm_SetEVTMode(int mc, int module, unsigned long mode);

void mcpwm_SetEVT(int mc, int module, unsigned long pwm, unsigned long pwmstat, unsigned long sc);



#define MCPWM_FAU_DISABLE           (0x00L)
#define MCPWM_FAU_PEREND            (0x01L)
#define MCPWM_FAU_PERCE             (0x02L)
#define MCPWM_FAU_SCEND             (0x03L)
#define MCPWM_FAU_NOWAIT            (0x0fL)
void mcpwm_SetFAU1Mode(int mc, int module, unsigned long mode);
void mcpwm_SetFAU2Mode(int mc, int module, unsigned long mode);
void mcpwm_SetFAU3Mode(int mc, int module, unsigned long mode);

#define MCPWM_FAUOUT_HI_LI          (0x05L)
#define MCPWM_FAUOUT_HI_LP          (0x06L)
#define MCPWM_FAUOUT_HI_LT          (0x07L)
#define MCPWM_FAUOUT_HP_LI          (0x09L)
#define MCPWM_FAUOUT_NONE           (0x0aL)
#define MCPWM_FAUOUT_HP_LT          (0x0bL)
#define MCPWM_FAUOUT_HT_LI          (0x0dL)
#define MCPWM_FAUOUT_HT_LP          (0x0eL)
#define MCPWM_FAUOUT_HT_LT          (0x0fL)
void mcpwm_SetFAU1Output(int mc, int module, unsigned long mode);
void mcpwm_SetFAU2Output(int mc, int module, unsigned long mode);
void mcpwm_SetFAU3Output(int mc, int module, unsigned long mode);



#define MCPWM_RELOAD_CANCEL         (0x00<<4)
#define MCPWM_RELOAD_PEREND         (0x01<<4)
#define MCPWM_RELOAD_PERCE          (0x02<<4)
#define MCPWM_RELOAD_SCEND          (0x03<<4)
#define MCPWM_RELOAD_SIFTRIG        (0x05<<4)
#define MCPWM_RELOAD_EVT            (0x06<<4)
#define MCPWM_RELOAD_NOW            (0x07<<4)
void mcpwm_ReloadPWM(int mc, int module, unsigned char mode);
void mcpwm_ReloadEVT(int mc, int module, unsigned char mode);
void mcpwm_ReloadOUT(int mc, int module, unsigned char mode);
void mcpwm_ReloadOUT_Unsafe(int mc, int module, unsigned char mode);
void mcspwm_ReloadOUT_Unsafe(int mc, unsigned char mode);
unsigned char mcpwm_ReadReloadPWM(int mc, int module);
unsigned char mcpwm_ReadReloadEVT(int mc, int module);
unsigned char mcpwm_ReadReloadOUT(int mc, int module);

/*------------------------  end of PWM Settings  -----------------------------*/



/****************************  Servo Settings  ********************************/

void mcsv_Enable(int mc, int module);
void mcsv_Disable(int mc, int module);

#define MCSV_PDIR_I0A1      (0x00L << 29)
#define MCSV_PDIR_A0I1      (0x04L << 29)
#define MCSV_CWCCW_I0A1     (0x01L << 29)
#define MCSV_CWCCW_A0I1     (0x05L << 29)
#define MCSV_PAB_I0A1       (0x02L << 29)
#define MCSV_PAB_A0I1       (0x06L << 29)
#define MCSV_PFEEDBACK      (0x07L << 29)
void mcsv_SetMode(int mc, int module, unsigned long mode);


void mcsv_SetWidth(int mc, int module, unsigned long period, unsigned long phase0);
void mcsv_SetSamplCycle(int mc, int module, unsigned long sc);


/************************** 2012-04-26 Modify for EX **************************/
#define MCSV_DDADIR_NONE    (0x00L) // for counter-mode
#define MCSV_DDADIR_CW      (0x00L) // for stat-mode
#define MCSV_DDADIR_CCW     (0x01L) // for stat-mode
void mcsv_SetDDA(int mc, int module, unsigned long dir, unsigned long dda);
unsigned long mcsv_ReadDDA(int mc, int module);
void mcsv_SetStatPulse(int mc, int module);
void mcsv_ClearStatPulse(int mc, int module);
void mcsv_SetDirOwner(int mc, int module);
void mcsv_ClearDirOwner(int mc, int module);

#define MCSV_DDAINT_EMPTY  (0x00L)
#define MCSV_DDAINT_LESS1  (0x01L)
#define MCSV_DDAINT_LESS2  (0x02L)
#define MCSV_DDAINT_LESS3  (0x03L)
#define MCSV_DDAINT_LESS4  (0x04L)
#define MCSV_DDAINT_LESS5  (0x05L)
#define MCSV_DDAINT_LESS6  (0x06L)
#define MCSV_DDAINT_LESS7  (0x07L)
void mcsv_SetDDAInt(int mc, int module, unsigned long ddaint);

#define MCSV_DDA_COUNT_POPLAST   (0x00L)
#define MCSV_DDA_COUNT_POPZERO   (0x01L)
#define MCSV_DDA_PATTERN_POPFULL (0x02L)
#define MCSV_DDA_PATTERN_POPZERO (0x03L)
void mcsv_SetDDAMode(int mc, int module, unsigned long mode);
#define MCSV_DDAFIFO_REALY	(0x00L)
#define MCSV_DDAFIFO_FULL	(0x01L)
#define MCSV_DDAFIFO_EMPTY	(0x03L)
unsigned long mcsv_ReadDDASTAT(int mc, int module);
unsigned long mcsv_ReadDDAMode(int mc, int module);
unsigned long mcsv_ReadDDAInt(int mc, int module);
unsigned long mcsv_ReadDirOwner(int mc, int module);
unsigned long mcsv_ReadStatPulse(int mc, int module);
/******************************** End for EX **********************************/


void mcsv_SetSCRESET(int mc, int module);
void mcsv_ClearSCRESET(int mc, int module);

void mcsv_SetSCLAST(int mc, int module);
void mcsv_ClearSCLAST(int mc, int module);
unsigned long mcsv_ReadSCLAST(int mc, int module);
unsigned long mcsv_ReadSCREG(int mc, int module);

#define MCSV_PFB_SUCCESS        (0x00L)
#define MCSV_PFB_ERR_NOFB       (0x01L)
#define MCSV_PFB_ERR_WRONGFB    (0x02L)
#define MCSV_PFB_BUSY           (0x03L)
unsigned long mcsv_ReadFBSTAT(int mc, int module);



unsigned long mcsv_ReadSTATREG1(int mc, int module);
unsigned long mcsv_ReadSTATREG2(int mc, int module);



#define MCSV_PAB_INIT00         (0x00 << 2)
#define MCSV_PAB_INIT01         (0x01 << 2)
#define MCSV_PAB_INIT10         (0x02 << 2)
#define MCSV_PAB_INIT11         (0x03 << 2)
void mcsv_SetPABInitOut(int mc, int module, unsigned char mode);

#define MCSV_P1MASK_NONE        (0x00L << 26)
#define MCSV_P1MASK_INACTIVE    (0x01L << 26)
#define MCSV_P1MASK_TRISTATE    (0x03L << 26)
#define MCSV_P2MASK_NONE        (0x00L << 24)
#define MCSV_P2MASK_INACTIVE    (0x01L << 24)
#define MCSV_P2MASK_TRISTATE    (0x03L << 24)
void mcsv_SetOutMask(int mc, int module, unsigned long mask);

#define MCSV_POL_NORMAL         (0x00L << 29)
#define MCSV_POL_INVERSE        (0x01L << 29)
void mcsv_SetOutPolarity(int mc, int module, unsigned long pol);

#define MCSV_DIR_CW             (0x00L << 30)
#define MCSV_DIR_CCW            (0x01L << 30)
void mcsv_SetDIR(int mc, int module, unsigned long dir);

void mcsv_SetDIRDeadband(int mc, int module, unsigned long db);



#define MCSV_EVT_DISABLE            (0x00L)
#define MCSV_EVT_PERSC              (0x01L)
#define MCSV_EVT_PERPULSE           (0x03L)
#define MCSV_PAB_NFMODE_DISABLE     (0x00L)
#define MCSV_PAB_NFMODE_4           (0x01L)
#define MCSV_PAB_NFMODE_16          (0x02L)
#define MCSV_PAB_NFMODE_64          (0x03L)
void mcsv_SetEVTMode(int mc, int module, unsigned long mode);

void mcsv_SetEVT(int mc, int module, unsigned long pcnt, unsigned long sc);

#define MCSV_FAU_DISABLE            (0x00L)
#define MCSV_FAU_PEREND             (0x01L)
#define MCSV_FAU_SCEND              (0x03L)
#define MCSV_FAU_NOWAIT             (0x0fL)
void mcsv_SetFAU1Mode(int mc, int module, unsigned long mode);
void mcsv_SetFAU2Mode(int mc, int module, unsigned long mode);
void mcsv_SetFAU3Mode(int mc, int module, unsigned long mode);

#define MCSV_FAUOUT_HA_LA           (0x00L)
#define MCSV_FAUOUT_HA_LI           (0x01L)
#define MCSV_FAUOUT_HA_LP           (0x02L)
#define MCSV_FAUOUT_HA_LL           (0x03L)
#define MCSV_FAUOUT_HI_LA           (0x04L)
#define MCSV_FAUOUT_HI_LI           (0x05L)
#define MCSV_FAUOUT_HI_LP           (0x06L)
#define MCSV_FAUOUT_HI_LL           (0x07L)
#define MCSV_FAUOUT_HP_LA           (0x08L)
#define MCSV_FAUOUT_HP_LI           (0x09L)
#define MCSV_FAUOUT_NONE            (0x0aL)
#define MCSV_FAUOUT_HP_LL           (0x0bL)
#define MCSV_FAUOUT_HL_LA           (0x0cL)
#define MCSV_FAUOUT_HL_LI           (0x0dL)
#define MCSV_FAUOUT_HL_LP           (0x0eL)
#define MCSV_FAUOUT_HL_LL           (0x0fL)
void mcsv_SetFAU1Output(int mc, int module, unsigned long mode);
void mcsv_SetFAU2Output(int mc, int module, unsigned long mode);
void mcsv_SetFAU3Output(int mc, int module, unsigned long mode);

void mcsv_SetDDAPOS(int mc, int module, unsigned long ddapos);
unsigned long mcsv_ReadDDAPOS(int mc, int module);
void mcsv_SetSWPIN(int mc, int module);
void mcsv_ClearSWPIN(int mc, int module);

#define MCSV_RELOAD_CANCEL              (0x00<<4)
#define MCSV_PDIR_RELOAD_PEREND         (0x01<<4)
#define MCSV_PDIR_RELOAD_SCEND          (0x03<<4)
#define MCSV_PDIR_RELOAD_SIFTRIG        (0x05<<4)
#define MCSV_PDIR_RELOAD_EVT            (0x06<<4)
#define MCSV_PDIR_RELOAD_PEREND_NODDA   (0x09<<4)
#define MCSV_PDIR_RELOAD_SCEND_NODDA    (0x0b<<4)
#define MCSV_PDIR_RELOAD_SIFTRIG_NODDA  (0x0d<<4)
#define MCSV_PDIR_RELOAD_EVT_NODDA      (0x0e<<4)
#define MCSV_CWCCW_RELOAD_PEREND        (0x01<<4)
#define MCSV_CWCCW_RELOAD_SCEND         (0x03<<4)
#define MCSV_CWCCW_RELOAD_SIFTRIG       (0x05<<4)
#define MCSV_CWCCW_RELOAD_EVT           (0x06<<4)
#define MCSV_CWCCW_RELOAD_PEREND_NODDA  (0x09<<4)
#define MCSV_CWCCW_RELOAD_SCEND_NODDA   (0x0b<<4)
#define MCSV_CWCCW_RELOAD_SIFTRIG_NODDA (0x0d<<4)
#define MCSV_CWCCW_RELOAD_EVT_NODDA     (0x0e<<4)
#define MCSV_PAB_RELOAD_STEND           (0x01<<4)
#define MCSV_PAB_RELOAD_SCEND           (0x03<<4)
#define MCSV_PAB_RELOAD_SIFTRIG         (0x05<<4)
#define MCSV_PAB_RELOAD_EVT             (0x06<<4)
#define MCSV_PAB_RELOAD_STEND_NODDA    	(0x09<<4)
#define MCSV_PAB_RELOAD_SCEND_NODDA     (0x0b<<4)
#define MCSV_PAB_RELOAD_SIFTRIG_NODDA   (0x0d<<4)
#define MCSV_PAB_RELOAD_EVT_NODDA       (0x0e<<4)
#define MCSV_PFB_RELOAD_START           (0x02<<4)
#define MCSV_PFB_NORELOAD_START         (0x04<<4)  // no use in mcsv_ReloadEVT() & mcsv_ReloadOUT()
#define MCSV_RELOAD_NOW                 (0x07<<4)
#define MCSV_RELOAD_NOW_NODDA           (0x0f<<4)
void mcsv_ReloadSV(int mc, int module, unsigned char mode);
void mcsv_ReloadEVT(int mc, int module, unsigned char mode);
void mcsv_ReloadOUT(int mc, int module, unsigned char mode);
unsigned char mcsv_ReadReloadSV(int mc, int module);
unsigned char mcsv_ReadReloadEVT(int mc, int module);
unsigned char mcsv_ReadReloadOUT(int mc, int module);
/*-----------------------  end of Servo Settings  ----------------------------*/



/*****************************  SIF Settings  *********************************/

void mcsif_Enable(int mc, int module);
void mcsif_Disable(int mc, int module);

#define MCSIF_ENC_PDIR          (0x00L << 13)
#define MCSIF_ENC_CWCCW         (0x01L << 13)
#define MCSIF_ENC_PAB           (0x02L << 13)
#define MCSIF_HALL              (0x03L << 13)
#define MCSIF_PFAU              (0x04L << 13)
#define MCSIF_SSI               (0x05L << 13)
void mcsif_SetMode(int mc, int module, unsigned long mode);



#define MCSIF_SWSTART_NOW       (0x00L << 4)
#define MCSIF_SWSTART_DISABLE   (0x01L << 4)
#define MCSIF_SWSTART_EXTRIG    (0x02L << 4)
#define MCSIF_SWSTART_PWMA1     (0x04L << 4)
#define MCSIF_SWSTART_PWMB1     (0x05L << 4)
#define MCSIF_SWSTART_PWMC1     (0x06L << 4)
#define MCSIF_SWSTART_PWMA0     (0x08L << 4)
#define MCSIF_SWSTART_PWMB0     (0x09L << 4)
#define MCSIF_SWSTART_PWMC0     (0x0aL << 4)
#define MCSIF_SWSTART_PWMAEVT   (0x0cL << 4)
#define MCSIF_SWSTART_PWMBEVT   (0x0dL << 4)
#define MCSIF_SWSTART_PWMCEVT   (0x0eL << 4)
#define MCSIF_SWEND_DISABLE     (0x00L << 0)
#define MCSIF_SWEND_NOW         (0x01L << 0)
#define MCSIF_SWEND_EXTRIG      (0x02L << 0)
#define MCSIF_SWEND_PWMA1       (0x04L << 0)
#define MCSIF_SWEND_PWMB1       (0x05L << 0)
#define MCSIF_SWEND_PWMC1       (0x06L << 0)
#define MCSIF_SWEND_PWMA0       (0x08L << 0)
#define MCSIF_SWEND_PWMB0       (0x09L << 0)
#define MCSIF_SWEND_PWMC0       (0x0aL << 0)
#define MCSIF_SWEND_PWMAEVT     (0x0cL << 0)
#define MCSIF_SWEND_PWMBEVT     (0x0dL << 0)
#define MCSIF_SWEND_PWMCEVT     (0x0eL << 0)
void mcsif_SetSamplWin(int mc, int module, unsigned long mode);

#define MCSIF_SWPOL_REMAIN      (0x00L << 8)
#define MCSIF_SWPOL_OUT0        (0x02L << 8)
#define MCSIF_SWPOL_OUT1        (0x03L << 8)
void mcsif_SetSWPOL(int mc, int module, unsigned long mode);

void mcsif_SetSWDeadband(int mc, int module, unsigned long db);
unsigned long mcsif_ReadSWSTAT(int mc, int module);

#define MCSIF_FILTER_SYNC       (0x80000000L)
void mcsif_SetInputFilter(int mc, int module, unsigned long filtime);



#define MCENC_INT_PCNTUV        (1L << 0)
#define MCENC_INT_PCNTOV        (1L << 1)
#define MCENC_INT_DIR           (1L << 2)
#define MCENC_INT_INERR         (1L << 3)
#define MCENC_INT_CAPINT        (1L << 5)
#define MCENC_INT_USREVT        (1L << 6)
#define MCHALL_INT_HCNTUV       (1L << 0)
#define MCHALL_INT_HCNTOV       (1L << 1)
#define MCHALL_INT_DIR          (1L << 2)
#define MCHALL_INT_INERR        (1L << 3)
#define MCHALL_INT_STERR        (1L << 4)
#define MCHALL_INT_HCAPINT      (1L << 5)
#define MCHALL_INT_USREVT       (1L << 6)
#define MCPFAU_INT_FAU1STAT     (1L << 2)
#define MCPFAU_INT_FAU2STAT     (1L << 3)
#define MCPFAU_INT_FAU3STAT     (1L << 4)
#define MCPFAU_INT_CAP1INT      (1L << 5)
#define MCPFAU_INT_CAP2INT      (1L << 6)
#define MCPFAU_INT_CAP3INT      (1L << 7)
void mcsif_EnableINT(int mc, int module, unsigned long intsort);
void mcsif_DisableINT(int mc, int module, unsigned long intsort);
bool mcsif_CheckINT(int mc, int module, unsigned long intsort);
void mcsif_ClearINT(int mc, int module, unsigned long intsort);



// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^   Encoder Modes  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#define MCENC_PDIR_DIR0_DISABLE     (0x00L << 12)
#define MCENC_PDIR_DIR0_DEC1TO0     (0x01L << 12)
#define MCENC_PDIR_DIR0_DEC0TO1     (0x02L << 12)
#define MCENC_PDIR_DIR0_DECBOTH     (0x03L << 12)
#define MCENC_PDIR_DIR0_INC1TO0     (0x05L << 12)
#define MCENC_PDIR_DIR0_INC0TO1     (0x06L << 12)
#define MCENC_PDIR_DIR0_INCBOTH     (0x07L << 12)
#define MCENC_PDIR_DIR1_DISABLE     (0x00L << 8)
#define MCENC_PDIR_DIR1_DEC1TO0     (0x01L << 8)
#define MCENC_PDIR_DIR1_DEC0TO1     (0x02L << 8)
#define MCENC_PDIR_DIR1_DECBOTH     (0x03L << 8)
#define MCENC_PDIR_DIR1_INC1TO0     (0x05L << 8)
#define MCENC_PDIR_DIR1_INC0TO1     (0x06L << 8)
#define MCENC_PDIR_DIR1_INCBOTH     (0x07L << 8)
#define MCENC_CWCCW_CW_DISABLE      (0x00L << 12)
#define MCENC_CWCCW_CW_DEC1TO0      (0x01L << 12)
#define MCENC_CWCCW_CW_DEC0TO1      (0x02L << 12)
#define MCENC_CWCCW_CW_DECBOTH      (0x03L << 12)
#define MCENC_CWCCW_CW_INC1TO0      (0x05L << 12)
#define MCENC_CWCCW_CW_INC0TO1      (0x06L << 12)
#define MCENC_CWCCW_CW_INCBOTH      (0x07L << 12)
#define MCENC_CWCCW_CCW_DISABLE     (0x00L << 8)
#define MCENC_CWCCW_CCW_DEC1TO0     (0x01L << 8)
#define MCENC_CWCCW_CCW_DEC0TO1     (0x02L << 8)
#define MCENC_CWCCW_CCW_DECBOTH     (0x03L << 8)
#define MCENC_CWCCW_CCW_INC1TO0     (0x05L << 8)
#define MCENC_CWCCW_CCW_INC0TO1     (0x06L << 8)
#define MCENC_CWCCW_CCW_INCBOTH     (0x07L << 8)
#define MCENC_PAB_DIR0_DISABLE      (0x00L << 12)
#define MCENC_PAB_DIR0_DECA         (0x01L << 12)
#define MCENC_PAB_DIR0_DECB         (0x02L << 12)
#define MCENC_PAB_DIR0_DECAB        (0x03L << 12)
#define MCENC_PAB_DIR0_INCA         (0x05L << 12)
#define MCENC_PAB_DIR0_INCB         (0x06L << 12)
#define MCENC_PAB_DIR0_INCAB        (0x07L << 12)
#define MCENC_PAB_DIR1_DISABLE      (0x00L << 8)
#define MCENC_PAB_DIR1_DECA         (0x01L << 8)
#define MCENC_PAB_DIR1_DECB         (0x02L << 8)
#define MCENC_PAB_DIR1_DECAB        (0x03L << 8)
#define MCENC_PAB_DIR1_INCA         (0x05L << 8)
#define MCENC_PAB_DIR1_INCB         (0x06L << 8)
#define MCENC_PAB_DIR1_INCAB        (0x07L << 8)
void mcenc_SetCntMode(int mc, int module, unsigned long mode);

#define MCENC_RESET_INC_DISABLE     (0x00L << 4)
#define MCENC_RESET_INC_CNTMIN      (0x01L << 4)
#define MCENC_RESET_INC_CNTMAX      (0x02L << 4)
#define MCENC_RESET_INC_CNTIDX      (0x04L << 4)
#define MCENC_RESET_INC_CNTMINIDX   (0x05L << 4)
#define MCENC_RESET_INC_CNTMAXIDX   (0x06L << 4)
#define MCENC_RESET_DEC_DISABLE     (0x00L << 0)
#define MCENC_RESET_DEC_CNTMAX      (0x01L << 0)
#define MCENC_RESET_DEC_CNTMIN      (0x02L << 0)
#define MCENC_RESET_DEC_CNTIDX      (0x04L << 0)
#define MCENC_RESET_DEC_CNTMAXIDX   (0x05L << 0)
#define MCENC_RESET_DEC_CNTMINIDX   (0x06L << 0)
void mcenc_SetResetMode(int mc, int module, unsigned long mode);

#define MCENC_TRIGRESET_IDXCOND_DISABLE    (0x00L << 29)
#define MCENC_TRIGRESET_IDXCOND_0TO1       (0x01L << 29)
#define MCENC_TRIGRESET_IDXCOND_1TO0       (0x02L << 29)
#define MCENC_TRIGRESET_IDXCOND_HOLD       (0x03L << 29)
#define MCENC_TRIGRESET_EXTRIG_DISABLE     (0x00L << 31)
#define MCENC_TRIGRESET_EXTRIG_ENABLE      (0x01L << 31)
void mcenc_SetTrigResetMode(int mc, int module, unsigned long mode);



void mcenc_SetPulCnt(int mc, int module, unsigned long pcnt);
unsigned long mcenc_ReadPulCnt(int mc, int module);

unsigned long mcenc_ReadDIR(int mc, int module);
unsigned long mcenc_ReadPINS(int mc, int module);

void mcenc_SetCntMin(int mc, int module, unsigned long cmin);
void mcenc_SetCntMax(int mc, int module, unsigned long cmax);
void mcenc_SetCntIdx(int mc, int module, unsigned long cidx);
void mcenc_SetCntEvt(int mc, int module, unsigned long cevt);



#define MCENC_PDIR_IDXCOND_DISABLE   (0x00L << 5)
#define MCENC_PDIR_IDXCOND_Z         (0x01L << 5)
#define MCENC_PDIR_IDXCOND_AZ        (0x05L << 5)
#define MCENC_PDIR_IDXCOND_ABZ       (0x07L << 5)
#define MCENC_CWCCW_IDXCOND_DISABLE  (0x00L << 5)
#define MCENC_CWCCW_IDXCOND_Z        (0x01L << 5)
#define MCENC_CWCCW_IDXCOND_BZ       (0x03L << 5)
#define MCENC_CWCCW_IDXCOND_AZ       (0x05L << 5)
#define MCENC_CWCCW_IDXCOND_AZ_BZ    (0x06L << 5)
#define MCENC_CWCCW_IDXCOND_ABZ      (0x07L << 5)
#define MCENC_PAB_IDXCOND_DISABLE    (0x00L << 5)
#define MCENC_PAB_IDXCOND_Z          (0x01L << 5)
#define MCENC_PAB_IDXCOND_BZ         (0x03L << 5)
#define MCENC_PAB_IDXCOND_AZ         (0x05L << 5)
#define MCENC_PAB_IDXCOND_AZ_BZ      (0x06L << 5)
#define MCENC_PAB_IDXCOND_ABZ        (0x07L << 5)
#define MCENC_ICA0                   (0x00L << 2)
#define MCENC_ICA1                   (0x01L << 2)
#define MCENC_ICB0                   (0x00L << 1)
#define MCENC_ICB1                   (0x01L << 1)
#define MCENC_ICZ0                   (0x00L << 0)
#define MCENC_ICZ1                   (0x01L << 0)
void mcenc_SetIdxCond(int mc, int module, unsigned long mode, unsigned long abz);

bool mcenc_CheckIDXERR(int mc, int module);
void mcenc_ClearIDXERR(int mc, int module);
bool mcenc_CheckIDXSTAT(int mc, int module);



#define MCENC_CAP_PCNT_DISABLE       (0x00L << 16)
#define MCENC_CAP_PCNT_ENABLE        (0x01L << 16)
#define MCENC_CAP_PCNT_ENABLE_CLEAR  (0x03L << 16)
#define MCENC_CAP_EXTRIG_DISABLE     (0x00L << 18)
#define MCENC_CAP_EXTRIG_ENABLE      (0x01L << 18)
#define MCENC_CAP_IDXCOND_DISABLE    (0x00L << 19)
#define MCENC_CAP_IDXCOND_ENABLE     (0x01L << 19)
void mcenc_SetCapMode(int mc, int module, unsigned long mode);

void mcenc_SetCapInterval(int mc, int module, unsigned long capivl);

#define MCENC_CAPFIFO_HASDATA        (0x00L)
#define MCENC_CAPFIFO_OVERFLOW       (0x02L)
#define MCENC_CAPFIFO_EMPTY          (0x03L)
unsigned long mcenc_ReadCAPSTAT(int mc, int module);

#define MCENC_PDIR_CAP_1TO0DIR0      (0x00L << 28)
#define MCENC_PDIR_CAP_0TO1DIR0      (0x02L << 28)
#define MCENC_PDIR_CAP_1TO0DIR1      (0x05L << 28)
#define MCENC_PDIR_CAP_0TO1DIR1      (0x07L << 28)
#define MCENC_CWCCW_CAP_1TO0CW       (0x00L << 28)
#define MCENC_CWCCW_CAP_0TO1CW       (0x02L << 28)
#define MCENC_CWCCW_CAP_1TO0CCW      (0x04L << 28)
#define MCENC_CWCCW_CAP_0TO1CCW      (0x05L << 28)
#define MCENC_PAB_CAP_TO00DIR0       (0x00L << 28)
#define MCENC_PAB_CAP_TO01DIR0       (0x01L << 28)
#define MCENC_PAB_CAP_TO10DIR0       (0x02L << 28)
#define MCENC_PAB_CAP_TO11DIR0       (0x03L << 28)
#define MCENC_PAB_CAP_TO00DIR1       (0x04L << 28)
#define MCENC_PAB_CAP_TO01DIR1       (0x05L << 28)
#define MCENC_PAB_CAP_TO10DIR1       (0x06L << 28)
#define MCENC_PAB_CAP_TO11DIR1       (0x07L << 28)
#define MCENC_CAP_CAPCNT_OVERFLOW    (0x08L << 28)
#define MCENC_CAP_PCNT_OVERFLOW      (0x0aL << 28)
#define MCENC_CAP_PCNT_UNDERFLOW     (0x0bL << 28)
#define MCENC_CAP_EXTRIG             (0x0cL << 28)
#define MCENC_CAP_IDXCOND            (0x0dL << 28)
#define MCENC_CAP_FIFOEMPTY          (0x0fL << 28)
unsigned long mcenc_ReadCAPFIFO(int mc, int module, unsigned long* data) ;

#define MCENC_RLDTRIG_DISABLE        (0x00L << 20)
#define MCENC_RLDTRIG_NORMAL         (0x01L << 20)
#define MCENC_RLDTRIG_ONESHOT        (0x02L << 20)
void mcenc_SetRLDTRIG(int mc, int module, unsigned long mode);
unsigned long mcenc_ReadRLDTRIG(int mc, int module);

unsigned long mcenc_ReadPINAInput(int mc, int module);
unsigned long mcenc_ReadPINBInput(int mc, int module);
unsigned long mcenc_ReadPINZInput(int mc, int module);


void mcenc_SetCtrlREG(int mc, int module, unsigned long val);
unsigned long mcenc_ReadCtrlREG(int mc, int module);

void mcenc_SetIdxCondREG(int mc, int module, unsigned long val);
unsigned long mcenc_ReadIdxCondREG(int mc, int module);

void mcenc_SetCapCtrlREG(int mc, int module, unsigned long val);
unsigned long mcenc_ReadCapCtrlREG(int mc, int module);



// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^   Hall Mode  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#define MCHALL_DIR0_DISABLE         (0x00L << 12)
#define MCHALL_DIR0_DEC             (0x03L << 12)
#define MCHALL_DIR0_INC             (0x07L << 12)
#define MCHALL_DIR1_DISABLE         (0x00L << 8)
#define MCHALL_DIR1_DEC             (0x03L << 8)
#define MCHALL_DIR1_INC             (0x07L << 8)
void mchall_SetCntMode(int mc, int module, unsigned long mode);

#define MCHALL_RESET_INC_DISABLE    (0x00L << 4)
#define MCHALL_RESET_INC_CNTMIN     (0x01L << 4)
#define MCHALL_RESET_INC_CNTMAX     (0x02L << 4)
#define MCHALL_RESET_DEC_DISABLE    (0x00L << 0)
#define MCHALL_RESET_DEC_CNTMAX     (0x01L << 0)
#define MCHALL_RESET_DEC_CNTMIN     (0x02L << 0)
void mchall_SetResetMode(int mc, int module, unsigned long mode);

void mchall_SetDirPattern(int mc, int module, unsigned long pattern);



void mchall_SetHallCnt(int mc, int module, unsigned long pcnt);
unsigned long mchall_ReadHallCnt(int mc, int module);

unsigned long mchall_ReadDIR(int mc, int module);
unsigned long mchall_ReadPINS(int mc, int module);

void mchall_SetCntMin(int mc, int module, unsigned long cmin);
void mchall_SetCntMax(int mc, int module, unsigned long cmax);
void mchall_SetCntEvt(int mc, int module, unsigned long cevt);



bool mchall_CheckSTERR(int mc, int module);
void mchall_ClearSTERR(int mc, int module);



#define MCHALL_CAP_SCNT_DISABLE      (0x00L << 16)
#define MCHALL_CAP_SCNT_ENABLE       (0x01L << 16)
#define MCHALL_CAP_SCNT_ENABLE_CLEAR (0x03L << 16)
#define MCHALL_CAP_EXTRIG_DISABLE    (0x00L << 18)
#define MCHALL_CAP_EXTRIG_ENABLE     (0x01L << 18)
void mchall_SetCapMode(int mc, int module, unsigned long mode);

void mchall_SetCapInterval(int mc, int module, unsigned long capivl);

#define MCHALL_CAPFIFO_HASDATA       (0x00L)
#define MCHALL_CAPFIFO_OVERFLOW      (0x02L)
#define MCHALL_CAPFIFO_EMPTY         (0x03L)
unsigned long mchall_ReadCAPSTAT(int mc, int module);

#define MCHALL_CAP_TO000             (0x00L << 28)
#define MCHALL_CAP_TO001             (0x01L << 28)
#define MCHALL_CAP_TO010             (0x02L << 28)
#define MCHALL_CAP_TO011             (0x03L << 28)
#define MCHALL_CAP_TO100             (0x04L << 28)
#define MCHALL_CAP_TO101             (0x05L << 28)
#define MCHALL_CAP_TO110             (0x06L << 28)
#define MCHALL_CAP_TO111             (0x07L << 28)
#define MCHALL_CAP_CAPCNT_OVERFLOW   (0x08L << 28)
#define MCHALL_CAP_SCNT_OVERFLOW     (0x0aL << 28)
#define MCHALL_CAP_SCNT_UNDERFLOW    (0x0bL << 28)
#define MCHALL_CAP_EXTRIG            (0x0cL << 28)
#define MCHALL_CAP_FIFOEMPTY         (0x0fL << 28)
unsigned long mchall_ReadCAPFIFO(int mc, int module, unsigned long* data) ;



void mchall_SetCtrlREG(int mc, int module, unsigned long val);
unsigned long mchall_ReadCtrlREG(int mc, int module);

void mchall_SetCapCtrlREG(int mc, int module, unsigned long val);
unsigned long mchall_ReadCapCtrlREG(int mc, int module);

unsigned long mchall_ReadPINAInput(int mc, int module);
unsigned long mchall_ReadPINBInput(int mc, int module);
unsigned long mchall_ReadPINZInput(int mc, int module);


// ^^^^^^^^^^^^^^^^^^^^^^^   Pulse & Fault Capture Mode  ^^^^^^^^^^^^^^^^^^^^^^^

#define MCPFAU_CAP_DISABLE           (0x00L << 16)
#define MCPFAU_CAP_1TO0              (0x01L << 16)
#define MCPFAU_CAP_0TO1              (0x02L << 16)
#define MCPFAU_CAP_BOTH              (0x03L << 16)
#define MCPFAU_CAP_1TO0_CLEAR        (0x05L << 16)
#define MCPFAU_CAP_0TO1_CLEAR        (0x06L << 16)
#define MCPFAU_CAP_BOTH_CLEAR        (0x07L << 16)
void mcpfau_SetCapMode1(int mc, int module, unsigned long mode);
void mcpfau_SetCapMode2(int mc, int module, unsigned long mode);
void mcpfau_SetCapMode3(int mc, int module, unsigned long mode);

void mcpfau_SetCapInterval1(int mc, int module, unsigned long capivl);
void mcpfau_SetCapInterval2(int mc, int module, unsigned long capivl);
void mcpfau_SetCapInterval3(int mc, int module, unsigned long capivl);

#define MCPFAU_CAPFIFO_HASDATA       (0x00L)
#define MCPFAU_CAPFIFO_OVERFLOW      (0x02L)
#define MCPFAU_CAPFIFO_EMPTY         (0x03L)
unsigned long mcpfau_ReadCAPSTAT1(int mc, int module);
unsigned long mcpfau_ReadCAPSTAT2(int mc, int module);
unsigned long mcpfau_ReadCAPSTAT3(int mc, int module);

#define MCPFAU_CAP_1TO0EDGE          (0x00L << 28)
#define MCPFAU_CAP_0TO1EDGE          (0x02L << 28)
#define MCPFAU_CAP_CAPCNT_OVERFLOW   (0x08L << 28)
#define MCPFAU_CAP_FIFOEMPTY         (0x0fL << 28)
unsigned long mcpfau_ReadCAPFIFO1(int mc, int module, unsigned long* data);
unsigned long mcpfau_ReadCAPFIFO2(int mc, int module, unsigned long* data);
unsigned long mcpfau_ReadCAPFIFO3(int mc, int module, unsigned long* data);

unsigned long mcpfau_ReadPIN1Input(int mc, int module);
unsigned long mcpfau_ReadPIN2Input(int mc, int module);
unsigned long mcpfau_ReadPIN3Input(int mc, int module);

#define MCPFAU_POL_NORMAL            (0x00L << 31)
#define MCPFAU_POL_INVERSE           (0x01L << 31)
void mcpfau_SetPolarity1(int mc, int module, unsigned long pol);
void mcpfau_SetPolarity2(int mc, int module, unsigned long pol);
void mcpfau_SetPolarity3(int mc, int module, unsigned long pol);

#define MCPFAU_MASK_NONE             (0x00L << 29)
#define MCPFAU_MASK_INACTIVE         (0x01L << 29)
#define MCPFAU_MASK_ACTIVE           (0x02L << 29)
void mcpfau_SetMask1(int mc, int module, unsigned long mask);
void mcpfau_SetMask2(int mc, int module, unsigned long mask);
void mcpfau_SetMask3(int mc, int module, unsigned long mask);



#define MCPFAU_RLDTRIG_DISABLE       (0x00L << 20)
#define MCPFAU_RLDTRIG_NORMAL        (0x01L << 20)
#define MCPFAU_RLDTRIG_ONESHOT       (0x02L << 20)
void mcpfau_SetRLDTRIG1(int mc, int module, unsigned long mode);
void mcpfau_SetRLDTRIG2(int mc, int module, unsigned long mode);
void mcpfau_SetRLDTRIG3(int mc, int module, unsigned long mode);
unsigned long mcpfau_ReadRLDTRIG1(int mc, int module);
unsigned long mcpfau_ReadRLDTRIG2(int mc, int module);
unsigned long mcpfau_ReadRLDTRIG3(int mc, int module);



#define MCPFAU_EXTRIG_DISABLE        (0x00L << 22)
#define MCPFAU_EXTRIG_NORMAL         (0x01L << 22)
#define MCPFAU_EXTRIG_ONESHOT        (0x02L << 22)
void mcpfau_SetEXTRIG1(int mc, int module, unsigned long mode);
void mcpfau_SetEXTRIG2(int mc, int module, unsigned long mode);
void mcpfau_SetEXTRIG3(int mc, int module, unsigned long mode);
unsigned long mcpfau_ReadEXTRIG1(int mc, int module);
unsigned long mcpfau_ReadEXTRIG2(int mc, int module);
unsigned long mcpfau_ReadEXTRIG3(int mc, int module);



unsigned long mcpfau_ReadFAU1STAT(int mc, int module);
unsigned long mcpfau_ReadFAU2STAT(int mc, int module);
unsigned long mcpfau_ReadFAU3STAT(int mc, int module);
void mcpfau_ClearFAU1STAT(int mc, int module);
void mcpfau_ClearFAU2STAT(int mc, int module);
void mcpfau_ClearFAU3STAT(int mc, int module);

#define MCPFAU_FAURELS_INPUT0        (0x01L << 26)
#define MCPFAU_FAURELS_FSTAT0        (0x02L << 26)
#define MCPFAU_FAURELS_BOTH          (0x03L << 26)
void mcpfau_SetFAU1RELS(int mc, int module, unsigned long mode);
void mcpfau_SetFAU2RELS(int mc, int module, unsigned long mode);
void mcpfau_SetFAU3RELS(int mc, int module, unsigned long mode);

#define MCPFAU_FAUTRIG_DISABLE       (0x00L << 24)
#define MCPFAU_FAUTRIG_INPUT1        (0x01L << 24)
#define MCPFAU_FAUTRIG_CAPEVT        (0x02L << 24)
void mcpfau_SetFAU1TRIG(int mc, int module, unsigned long mode);
void mcpfau_SetFAU2TRIG(int mc, int module, unsigned long mode);
void mcpfau_SetFAU3TRIG(int mc, int module, unsigned long mode);



unsigned long mcpfau_ReadCap1CtrlREG(int mc, int module);
unsigned long mcpfau_ReadCap2CtrlREG(int mc, int module);
unsigned long mcpfau_ReadCap3CtrlREG(int mc, int module);

unsigned long mcpfau_ReadCapStatREG(int mc, int module);

void mcpfau_SetCap1INT(int mc, int module, unsigned long interval);
void mcpfau_SetCap2INT(int mc, int module, unsigned long interval);
void mcpfau_SetCap3INT(int mc, int module, unsigned long interval);

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^   SSI Mode  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

void mcssi_SetPulCnt(int mc, int module, unsigned long pcnt);
unsigned long mcssi_ReadPulCnt(int mc, int module);
unsigned long mcssi_ReadData(int mc, int module);
void mcssi_SetClock(int mc, int module, unsigned long clk);
unsigned long mcssi_ReadClock(int mc, int module);
void mcssi_SetWaitTime(int mc, int module, unsigned long waittime);
unsigned long mcssi_ReadWaitTime(int mc, int module);

#define MCSSI_LATCH_PHASE0    (0x00L << 31)
#define MCSSI_LATCH_PHASE1    (0x01L << 31)
void mcssi_SetLatchPhase(int mc, int module, unsigned long phase);

unsigned long mcssi_ReadLatchPhase(int mc, int module);
void mcssi_SetLatchTime(int mc, int module, unsigned long latchtime);
unsigned long mcssi_ReadLatchTime(int mc, int module);
void mcssi_SetNumberBITS(int mc, int module, unsigned long numberbits);
unsigned long mcssi_ReadNumberBITS(int mc, int module);
void mcssi_SetNumberRUNS(int mc, int module, unsigned long numberruns);
unsigned long mcssi_ReadNumberRUNS(int mc, int module);

#define MCSSI_COUNT            (0x00L << 12)
#define MCSSI_CONTINUE         (0x01L << 12)
#define MCSSI_COUNT_1TO0       (0x04L << 12)
#define MCSSI_CONTINUE_1TO0    (0x05L << 12)
#define MCSSI_COUNT_0TO1       (0x08L << 12)
#define MCSSI_CONTINUE_0TO1    (0x09L << 12)
#define MCSSI_COUNT_BOTH       (0x0cL << 12)
#define MCSSI_CONTINUE_BOTH    (0x0dL << 12)
void mcssi_SetCntMode(int mc, int module, unsigned long mode);
void mcssi_SetExternalBit(int mc, int module);
void mcssi_ClearExternalBit(int mc, int module, unsigned long mode);
void mcssi_SetChkErrBit(int mc, int module);
void mcssi_ClearChkErrBit(int mc, int module, unsigned long mode);
void mcssi_SetGAY2BINBit(int mc, int module);
void mcssi_ClearGAY2BINBit(int mc, int module, unsigned long mode);
unsigned long mcssi_ReadRunNumber(int mc, int module);

#define MCSSI_STAT_IDLE       (0x00L)
#define MCSSI_STAT_READING    (0x01L)
#define MCSSI_STAT_WAIT       (0x02L)
unsigned long mcssi_ReadSSISTAT(int mc, int module);

bool mcssi_CheckERRBit(int mc, int module);
void mcssi_ClearERRBit(int mc, int module);

#define MCSSI_CAPFIFO_HASDATA        (0x00L)
#define MCSSI_CAPFIFO_OVERFLOW       (0x02L)
#define MCSSI_CAPFIFO_EMPTY          (0x03L)
unsigned long mcssi_ReadCAPSTAT(int mc, int module);

#define MCSSI_RLDTRIG_DISABLE        (0x00L << 20)
#define MCSSI_RLDTRIG_NORMAL         (0x01L << 20)
#define MCSSI_RLDTRIG_ONESHOT        (0x02L << 20)
void mcssi_SetRLDTRIG(int mc, int module, unsigned long mode);

#define MCSSI_CAP_PCNT_DISABLE       (0x00L << 16)
#define MCSSI_CAP_PCNT_ENABLE        (0x01L << 16)
void mcssi_SetCAPMode(int mc, int module, unsigned long mode);

void mcssi_SetCAPINT(int mc, int module, unsigned long number);
void mcssi_SetCapInterval(int mc, int module, unsigned long number);
unsigned long mcssi_ReadCAPFIFO(int mc, int module, unsigned long* data);

unsigned long mcssi_ReadCtrlREG(int mc, int module);

// MCM INTERRUPT FUNCTION 
#define PULSE_END_INT    (0x01L)
#define SC_END_INT       (0x02L)
#define USER_EVT_INT     (0x04L)
#define LDRDY_SERVO_INT  (0x08L)
#define LDRDY_EV_INT     (0x10L)
#define LDRDY_OCTRL_INT  (0x20L)
#define PULSE_FB_INT     (0x40L)
#define DDAFIFO_INT      (0x80L) // for EX
unsigned char GetMCIRQ(void);
void Set_MCIRQ(unsigned char irq);


#ifdef __cplusplus
}
#endif 
#endif

