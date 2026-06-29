/*
  d1061.h - DM&P Vortex86EX2 USB Device
  Copyright (c) 2018 Johnson Hung <Dyhung@dmp.com.tw>. All right reserved.

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
#ifndef __D1061_H
#define __D1061_H

#include <dpmi.h>

#define D1061_HIGH_SPEED

#define D1061_REG_LEN                       (0xB4)
#define D1061_REG_SIZE                      (0x04)
                    
#define D1061_GBLCFG_ADDR                   (0x00)
#define D1061_GBLCTL_ADDR                   (0x04)
#define D1061_GBLSTS_ADDR                   (0x08)
#define D1061_INTENABLE1_ADDR               (0x0C)
#define D1061_INTENABLE2_ADDR               (0x10)
#define D1061_INTSTS1_ADDR                  (0x14)
#define D1061_INTSTS2_ADDR                  (0x18)
#define D1061_FRAMENUM_ADDR                 (0x1C)
#define D1061_TEST_ADDR                     (0x20)
#define D1061_EP0INCFG_ADDR                 (0x24)
#define D1061_EP0OUTCFG_ADDR                (0x28)
#define D1061_EP0INCTLSTS_ADDR              (0x2C)
#define D1061_EP0OUTCTLSTS_ADDR             (0x30)
#define D1061_EP0SETUPDBA_ADDR              (0x34)
#define D1061_EP0INDDC_ADDR                 (0x38)
#define D1061_EP0INDDA_ADDR                 (0x3C)
#define D1061_EP0OUTDDC_ADDR                (0x40)
#define D1061_EP0OUTDDA_ADDR                (0x44)
#define D1061_EP1INCFG_ADDR                 (0x48)
#define D1061_EP1OUTCFG_ADDR                (0x4C)
#define D1061_EP1INCTLSTS_ADDR              (0x50)
#define D1061_EP1OUTCTLSTS_ADDR             (0x54)
#define D1061_EP1INDDC_ADDR                 (0x5C)
#define D1061_EP1INDDA_ADDR                 (0x60)
#define D1061_EP1OUTDDC_ADDR                (0x64)
#define D1061_EP1OUTDDA_ADDR                (0x68)
#define D1061_EP2INCFG_ADDR                 (0x6C)
#define D1061_EP2OUTCFG_ADDR                (0x70)
#define D1061_EP2INCTLSTS_ADDR              (0x74)
#define D1061_EP2OUTCTLSTS_ADDR             (0x78)
#define D1061_EP2INDDC_ADDR                 (0x80)
#define D1061_EP2INDDA_ADDR                 (0x84)
#define D1061_EP2OUTDDC_ADDR                (0x88)
#define D1061_EP2OUTDDA_ADDR                (0x8C)
#define D1061_EP3INCFG_ADDR                 (0x90)
#define D1061_EP3OUTCFG_ADDR                (0x94)
#define D1061_EP3INCTLSTS_ADDR              (0x98)
#define D1061_EP3OUTCTLSTS_ADDR             (0x9C)
#define D1061_EP3INDDC_ADDR                 (0xA4)
#define D1061_EP3INDDA_ADDR                 (0xA8)
#define D1061_EP3OUTDDC_ADDR                (0xAC)
#define D1061_EP3OUTDDA_ADDR                (0xB0)
        
#define D1061_EPxREGS_OFFSET(number)        (0x24 + 0x24 * number)
#define D1061_EPxINCFG_ADDR                 (0x00)
#define D1061_EPxOUTCFG_ADDR                (0x04)
#define D1061_EPxINCTLSTS_ADDR              (0x08)
#define D1061_EPxOUTCTLSTS_ADDR             (0x0C)
#define D1061_EPxINDDC_ADDR                 (0x14)
#define D1061_EPxINDDA_ADDR                 (0x18)
#define D1061_EPxOUTDDC_ADDR                (0x1C)
#define D1061_EPxOUTDDA_ADDR                (0x20)

#define D1061MASK(name)                     (D1061_##name##_RANGE << D1061_##name##_SHIFT)
#define D1061_GBLCFG_FULL_SHIFT             (8)
#define D1061_GBLCFG_DEVADDR_SHIFT          (0)
#define D1061_GBLCTL_TEST_SHIFT             (11)
#define D1061_GBLCTL_WAKEUP_SHIFT           (8)
#define D1061_GBLCTL_SUSPEND_SHIFT          (7)
#define D1061_GBLCTL_PULLUP_SHIFT           (6)
#define D1061_GBLCTL_GBLINTE_SHIFT          (5)
#define D1061_GBLCTL_DEVCTLE_SHIFT          (4)
#define D1061_GBLCTL_SWRST_SHIFT            (1)
#define D1061_GBLCTL_HWRST_SHIFT            (0)
#define D1061_GBLSTS_WAKEUP_SHIFT           (5)
#define D1061_GBLSTS_TEST_SHIFT             (4)
#define D1061_GBLSTS_SUSPEND_SHIFT          (3)
#define D1061_GBLSTS_HIGH_SHIFT             (2)
#define D1061_GBLSTS_CONNECT_SHIFT          (1)
#define D1061_GBLSTS_ACTIVE_SHIFT           (0)
#define D1061_INTENABLE1_EP2PLERR_SHIFT     (29)
#define D1061_INTENABLE1_EP2OUTDMA_SHIFT    (28)
#define D1061_INTENABLE1_EP2INDMA_SHIFT     (27)
#define D1061_INTENABLE1_EP1PLERR_SHIFT     (21)
#define D1061_INTENABLE1_EP1OUTDMA_SHIFT    (20)
#define D1061_INTENABLE1_EP1INDMA_SHIFT     (19)
#define D1061_INTENABLE1_EP0PLERR_SHIFT     (13)
#define D1061_INTENABLE1_EP0OUTDMA_SHIFT    (12)
#define D1061_INTENABLE1_EP0INDMA_SHIFT     (11)
#define D1061_INTENABLE1_EP0STATUS_SHIFT    (10)
#define D1061_INTENABLE1_EP0SETUP_SHIFT     (9)
#define D1061_INTENABLE1_SOF_SHIFT          (7)
#define D1061_INTENABLE1_WAKEUP_SHIFT       (6)
#define D1061_INTENABLE1_RESUME_SHIFT       (5)
#define D1061_INTENABLE1_SUSPEND_SHIFT      (4)
#define D1061_INTENABLE1_BUSRST_SHIFT       (2)
#define D1061_INTENABLE1_CNTSTSCHG_SHIFT    (1)
#define D1061_INTENABLE1_SYSERR_SHIFT       (0)
#define D1061_INTENABLE2_EP3PLERR_SHIFT     (5)
#define D1061_INTENABLE2_EP3OUTDMA_SHIFT    (4)
#define D1061_INTENABLE2_EP3INDMA_SHIFT     (3)
#define D1061_INTSTS1_EP2PLERR_SHIFT        (29)
#define D1061_INTSTS1_EP2OUTDMA_SHIFT       (28)
#define D1061_INTSTS1_EP2INDMA_SHIFT        (27)
#define D1061_INTSTS1_EP1PLERR_SHIFT        (21)
#define D1061_INTSTS1_EP1OUTDMA_SHIFT       (20)
#define D1061_INTSTS1_EP1INDMA_SHIFT        (19)
#define D1061_INTSTS1_EP0PLERR_SHIFT        (13)
#define D1061_INTSTS1_EP0OUTDMA_SHIFT       (12)
#define D1061_INTSTS1_EP0INDMA_SHIFT        (11)
#define D1061_INTSTS1_EP0STATUS_SHIFT       (10)
#define D1061_INTSTS1_EP0SETUP_SHIFT        (9)
#define D1061_INTSTS1_SOF_SHIFT             (7)
#define D1061_INTSTS1_WAKEUP_SHIFT          (6)
#define D1061_INTSTS1_RESUME_SHIFT          (5)
#define D1061_INTSTS1_SUSPEND_SHIFT         (4)
#define D1061_INTSTS1_BUSRST_SHIFT          (2)
#define D1061_INTSTS1_CNTSTSCHG_SHIFT       (1)
#define D1061_INTSTS1_SYSERR_SHIFT          (0)
#define D1061_INTSTS2_EP3PLERR_SHIFT        (5)
#define D1061_INTSTS2_EP3OUTDMA_SHIFT       (4)
#define D1061_INTSTS2_EP3INDMA_SHIFT        (3)
#define D1061_FRAMENUM_FN_SHIFT             (16)
#define D1061_FRAMENUM_MFC_SHIFT            (0)
#define D1061_TEST_PKT_SHIFT                (3)
#define D1061_TEST_SE0NAK_SHIFT             (2)
#define D1061_TEST_K_SHIFT                  (1)
#define D1061_TEST_J_SHIFT                  (0)
#define D1061_EP0SETUPDBA_SETUPADDR_SHIFT   (0)
#define D1061_EPCFG_NOTPMF_SHIFT            (27)
#define D1061_EPCFG_MAXPKTSIZE_SHIFT        (16)
#define D1061_EPCFG_EPNUM_SHIFT             (8)
#define D1061_EPCFG_ISOCZEROPKT_SHIFT       (5)
#define D1061_EPCFG_EPTYPE_SHIFT            (1)
#define D1061_EPCFG_EPENABLE_SHIFT          (0)
#define D1061_EPCTLSTS_TOGGLEERR_SHIFT      (18)
#define D1061_EPCTLSTS_DRDYMISS_SHIFT       (17)
#define D1061_EPCTLSTS_ACK_SHIFT            (16)
#define D1061_EPCTLSTS_DESCOFF_SHIFT        (8)
#define D1061_EPCTLSTS_DMASTOP_SHIFT        (5)
#define D1061_EPCTLSTS_DMASTART_SHIFT       (4)
#define D1061_EPCTLSTS_STALL_SHIFT          (2)
#define D1061_EPCTLSTS_ACKSET_SHIFT         (1)
#define D1061_EPCTLSTS_EPRST_SHIFT          (0)
#define D1061_EPDDC_NUMDESC_SHIFT           (0)
#define D1061_EPDDA_DESCADDR_SHIFT          (0)
    
#define D1061_GBLCFG_FULL_RANGE             (0x00000001UL)
#define D1061_GBLCFG_DEVADDR_RANGE          (0x0000007FUL)
#define D1061_GBLCTL_TEST_RANGE             (0x00000001UL)
#define D1061_GBLCTL_WAKEUP_RANGE           (0x00000001UL)
#define D1061_GBLCTL_SUSPEND_RANGE          (0x00000001UL)
#define D1061_GBLCTL_PULLUP_RANGE           (0x00000001UL)
#define D1061_GBLCTL_GBLINTE_RANGE          (0x00000001UL)
#define D1061_GBLCTL_DEVCTLE_RANGE          (0x00000001UL)
#define D1061_GBLCTL_SWRST_RANGE            (0x00000001UL)
#define D1061_GBLCTL_HWRST_RANGE            (0x00000001UL)
#define D1061_GBLSTS_WAKEUP_RANGE           (0x00000001UL)
#define D1061_GBLSTS_TEST_RANGE             (0x00000001UL)
#define D1061_GBLSTS_SUSPEND_RANGE          (0x00000001UL)
#define D1061_GBLSTS_HIGH_RANGE             (0x00000001UL)
#define D1061_GBLSTS_CONNECT_RANGE          (0x00000001UL)
#define D1061_GBLSTS_ACTIVE_RANGE           (0x00000001UL)
#define D1061_INTENABLE1_EP2PLERR_RANGE     (0x00000001UL)
#define D1061_INTENABLE1_EP2OUTDMA_RANGE    (0x00000001UL)
#define D1061_INTENABLE1_EP2INDMA_RANGE     (0x00000001UL)
#define D1061_INTENABLE1_EP1PLERR_RANGE     (0x00000001UL)
#define D1061_INTENABLE1_EP1OUTDMA_RANGE    (0x00000001UL)
#define D1061_INTENABLE1_EP1INDMA_RANGE     (0x00000001UL)
#define D1061_INTENABLE1_EP0PLERR_RANGE     (0x00000001UL)
#define D1061_INTENABLE1_EP0OUTDMA_RANGE    (0x00000001UL)
#define D1061_INTENABLE1_EP0INDMA_RANGE     (0x00000001UL)
#define D1061_INTENABLE1_EP0STATUS_RANGE    (0x00000001UL)
#define D1061_INTENABLE1_EP0SETUP_RANGE     (0x00000001UL)
#define D1061_INTENABLE1_SOF_RANGE          (0x00000001UL)
#define D1061_INTENABLE1_WAKEUP_RANGE       (0x00000001UL)
#define D1061_INTENABLE1_RESUME_RANGE       (0x00000001UL)
#define D1061_INTENABLE1_SUSPEND_RANGE      (0x00000001UL)
#define D1061_INTENABLE1_BUSRST_RANGE       (0x00000001UL)
#define D1061_INTENABLE1_CNTSTSCHG_RANGE    (0x00000001UL)
#define D1061_INTENABLE1_SYSERR_RANGE       (0x00000001UL)
#define D1061_INTENABLE2_EP3PLERR_RANGE     (0x00000001UL)
#define D1061_INTENABLE2_EP3OUTDMA_RANGE    (0x00000001UL)
#define D1061_INTENABLE2_EP3INDMA_RANGE     (0x00000001UL)
#define D1061_INTSTS1_EP2PLERR_RANGE        (0x00000001UL)
#define D1061_INTSTS1_EP2OUTDMA_RANGE       (0x00000001UL)
#define D1061_INTSTS1_EP2INDMA_RANGE        (0x00000001UL)
#define D1061_INTSTS1_EP1PLERR_RANGE        (0x00000001UL)
#define D1061_INTSTS1_EP1OUTDMA_RANGE       (0x00000001UL)
#define D1061_INTSTS1_EP1INDMA_RANGE        (0x00000001UL)
#define D1061_INTSTS1_EP0PLERR_RANGE        (0x00000001UL)
#define D1061_INTSTS1_EP0OUTDMA_RANGE       (0x00000001UL)
#define D1061_INTSTS1_EP0INDMA_RANGE        (0x00000001UL)
#define D1061_INTSTS1_EP0STATUS_RANGE       (0x00000001UL)
#define D1061_INTSTS1_EP0SETUP_RANGE        (0x00000001UL)
#define D1061_INTSTS1_SOF_RANGE             (0x00000001UL)
#define D1061_INTSTS1_WAKEUP_RANGE          (0x00000001UL)
#define D1061_INTSTS1_RESUME_RANGE          (0x00000001UL)
#define D1061_INTSTS1_SUSPEND_RANGE         (0x00000001UL)
#define D1061_INTSTS1_BUSRST_RANGE          (0x00000001UL)
#define D1061_INTSTS1_CNTSTSCHG_RANGE       (0x00000001UL)
#define D1061_INTSTS1_SYSERR_RANGE          (0x00000001UL)
#define D1061_INTSTS2_EP3PLERR_RANGE        (0x00000001UL)
#define D1061_INTSTS2_EP3OUTDMA_RANGE       (0x00000001UL)
#define D1061_INTSTS2_EP3INDMA_RANGE        (0x00000001UL)
#define D1061_FRAMENUM_FN_RANGE             (0x000003FFUL)
#define D1061_FRAMENUM_MFC_RANGE            (0x00000007UL)
#define D1061_TEST_PKT_RANGE                (0x00000001UL)
#define D1061_TEST_SE0NAK_RANGE             (0x00000001UL)
#define D1061_TEST_K_RANGE                  (0x00000001UL)
#define D1061_TEST_J_RANGE                  (0x00000001UL)
#define D1061_EP0SETUPDBA_SETUPADDR_RANGE   (0xFFFFFFFFUL)
#define D1061_EPCFG_NOTPMF_RANGE            (0x00000003UL)
#define D1061_EPCFG_MAXPKTSIZE_RANGE        (0x000007FFUL)
#define D1061_EPCFG_EPNUM_RANGE             (0x0000000FUL)
#define D1061_EPCFG_ISOCZEROPKT_RANGE       (0x00000001UL)
#define D1061_EPCFG_EPTYPE_RANGE            (0x00000003UL)
#define D1061_EPCFG_EPENABLE_RANGE          (0x00000001UL)
#define D1061_EPCTLSTS_TOGGLEERR_RANGE      (0x00000001UL)
#define D1061_EPCTLSTS_DRDYMISS_RANGE       (0x00000001UL)
#define D1061_EPCTLSTS_ACK_RANGE            (0x00000001UL)
#define D1061_EPCTLSTS_DESCOFF_RANGE        (0x0000003FUL)
#define D1061_EPCTLSTS_DMASTOP_RANGE        (0x00000001UL)
#define D1061_EPCTLSTS_DMASTART_RANGE       (0x00000001UL)
#define D1061_EPCTLSTS_STALL_RANGE          (0x00000001UL)
#define D1061_EPCTLSTS_ACKSET_RANGE         (0x00000001UL)
#define D1061_EPCTLSTS_EPRST_RANGE          (0x00000001UL)
#define D1061_EPDDC_NUMDESC_RANGE           (0x0000003FUL)
#define D1061_EPDDA_DESCADDR_RANGE          (0xFFFFFFFFUL)
    
#define D1061_GBLCFG_FULL                   D1061MASK(GBLCFG_FULL)
#define D1061_GBLCFG_DEVADDR                D1061MASK(GBLCFG_DEVADDR)
#define D1061_GBLCTL_TEST                   D1061MASK(GBLCTL_TEST)
#define D1061_GBLCTL_WAKEUP                 D1061MASK(GBLCTL_WAKEUP)
#define D1061_GBLCTL_SUSPEND                D1061MASK(GBLCTL_SUSPEND)
#define D1061_GBLCTL_PULLUP                 D1061MASK(GBLCTL_PULLUP)
#define D1061_GBLCTL_GBLINTE                D1061MASK(GBLCTL_GBLINTE)
#define D1061_GBLCTL_DEVCTLE                D1061MASK(GBLCTL_DEVCTLE)
#define D1061_GBLCTL_SWRST                  D1061MASK(GBLCTL_SWRST)
#define D1061_GBLCTL_HWRST                  D1061MASK(GBLCTL_HWRST)
#define D1061_GBLSTS_WAKEUP                 D1061MASK(GBLSTS_WAKEUP)
#define D1061_GBLSTS_TEST                   D1061MASK(GBLSTS_TEST)
#define D1061_GBLSTS_SUSPEND                D1061MASK(GBLSTS_SUSPEND)
#define D1061_GBLSTS_HIGH                   D1061MASK(GBLSTS_HIGH)
#define D1061_GBLSTS_CONNECT                D1061MASK(GBLSTS_CONNECT)
#define D1061_GBLSTS_ACTIVE                 D1061MASK(GBLSTS_ACTIVE)
#define D1061_INTENABLE1_EP2PLERR           D1061MASK(INTENABLE1_EP2PLERR)
#define D1061_INTENABLE1_EP2OUTDMA          D1061MASK(INTENABLE1_EP2OUTDMA)
#define D1061_INTENABLE1_EP2INDMA           D1061MASK(INTENABLE1_EP2INDMA)
#define D1061_INTENABLE1_EP1PLERR           D1061MASK(INTENABLE1_EP1PLERR)
#define D1061_INTENABLE1_EP1OUTDMA          D1061MASK(INTENABLE1_EP1OUTDMA)
#define D1061_INTENABLE1_EP1INDMA           D1061MASK(INTENABLE1_EP1INDMA)
#define D1061_INTENABLE1_EP0PLERR           D1061MASK(INTENABLE1_EP0PLERR)
#define D1061_INTENABLE1_EP0OUTDMA          D1061MASK(INTENABLE1_EP0OUTDMA)
#define D1061_INTENABLE1_EP0INDMA           D1061MASK(INTENABLE1_EP0INDMA)
#define D1061_INTENABLE1_EP0STATUS          D1061MASK(INTENABLE1_EP0STATUS)
#define D1061_INTENABLE1_EP0SETUP           D1061MASK(INTENABLE1_EP0SETUP)
#define D1061_INTENABLE1_SOF                D1061MASK(INTENABLE1_SOF)
#define D1061_INTENABLE1_WAKEUP             D1061MASK(INTENABLE1_WAKEUP)
#define D1061_INTENABLE1_RESUME             D1061MASK(INTENABLE1_RESUME)
#define D1061_INTENABLE1_SUSPEND            D1061MASK(INTENABLE1_SUSPEND)
#define D1061_INTENABLE1_BUSRST             D1061MASK(INTENABLE1_BUSRST)
#define D1061_INTENABLE1_CNTSTSCHG          D1061MASK(INTENABLE1_CNTSTSCHG)
#define D1061_INTENABLE1_SYSERR             D1061MASK(INTENABLE1_SYSERR)
#define D1061_INTENABLE2_EP3PLERR           D1061MASK(INTENABLE2_EP3PLERR)
#define D1061_INTENABLE2_EP3OUTDMA          D1061MASK(INTENABLE2_EP3OUTDMA)
#define D1061_INTENABLE2_EP3INDMA           D1061MASK(INTENABLE2_EP3INDMA)
#define D1061_INTSTS1_EP2PLERR              D1061MASK(INTSTS1_EP2PLERR)
#define D1061_INTSTS1_EP2OUTDMA             D1061MASK(INTSTS1_EP2OUTDMA)
#define D1061_INTSTS1_EP2INDMA              D1061MASK(INTSTS1_EP2INDMA)
#define D1061_INTSTS1_EP1PLERR              D1061MASK(INTSTS1_EP1PLERR)
#define D1061_INTSTS1_EP1OUTDMA             D1061MASK(INTSTS1_EP1OUTDMA)
#define D1061_INTSTS1_EP1INDMA              D1061MASK(INTSTS1_EP1INDMA)
#define D1061_INTSTS1_EP0PLERR              D1061MASK(INTSTS1_EP0PLERR)
#define D1061_INTSTS1_EP0OUTDMA             D1061MASK(INTSTS1_EP0OUTDMA)
#define D1061_INTSTS1_EP0INDMA              D1061MASK(INTSTS1_EP0INDMA)
#define D1061_INTSTS1_EP0STATUS             D1061MASK(INTSTS1_EP0STATUS)
#define D1061_INTSTS1_EP0SETUP              D1061MASK(INTSTS1_EP0SETUP)
#define D1061_INTSTS1_SOF                   D1061MASK(INTSTS1_SOF)
#define D1061_INTSTS1_WAKEUP                D1061MASK(INTSTS1_WAKEUP)
#define D1061_INTSTS1_RESUME                D1061MASK(INTSTS1_RESUME)
#define D1061_INTSTS1_SUSPEND               D1061MASK(INTSTS1_SUSPEND)
#define D1061_INTSTS1_BUSRST                D1061MASK(INTSTS1_BUSRST)
#define D1061_INTSTS1_CNTSTSCHG             D1061MASK(INTSTS1_CNTSTSCHG)
#define D1061_INTSTS1_SYSERR                D1061MASK(INTSTS1_SYSERR)
#define D1061_INTSTS2_EP3PLERR              D1061MASK(INTSTS2_EP3PLERR)
#define D1061_INTSTS2_EP3OUTDMA             D1061MASK(INTSTS2_EP3OUTDMA)
#define D1061_INTSTS2_EP3INDMA              D1061MASK(INTSTS2_EP3INDMA)
#define D1061_FRAMENUM_FN                   D1061MASK(FRAMENUM_FN)
#define D1061_FRAMENUM_MFC                  D1061MASK(FRAMENUM_MFC)
#define D1061_TEST_PKT                      D1061MASK(TEST_PKT)
#define D1061_TEST_SE0NAK                   D1061MASK(TEST_SE0NAK)
#define D1061_TEST_K                        D1061MASK(TEST_K)
#define D1061_TEST_J                        D1061MASK(TEST_J)
#define D1061_EP0SETUPDBA_SETUPADDR         D1061MASK(EP0SETUPDBA_SETUPADDR)
#define D1061_EPCFG_NOTPMF                  D1061MASK(EPCFG_NOTPMF)
#define D1061_EPCFG_MAXPKTSIZE              D1061MASK(EPCFG_MAXPKTSIZE)
#define D1061_EPCFG_EPNUM                   D1061MASK(EPCFG_EPNUM)
#define D1061_EPCFG_ISOCZEROPKT             D1061MASK(EPCFG_ISOCZEROPKT)
#define D1061_EPCFG_EPTYPE                  D1061MASK(EPCFG_EPTYPE)
#define D1061_EPCFG_EPENABLE                D1061MASK(EPCFG_EPENABLE)
#define D1061_EPCTLSTS_TOGGLEERR            D1061MASK(EPCTLSTS_TOGGLEERR)
#define D1061_EPCTLSTS_DRDYMISS             D1061MASK(EPCTLSTS_DRDYMISS)
#define D1061_EPCTLSTS_ACK                  D1061MASK(EPCTLSTS_ACK)
#define D1061_EPCTLSTS_DESCOFF              D1061MASK(EPCTLSTS_DESCOFF)
#define D1061_EPCTLSTS_DMASTOP              D1061MASK(EPCTLSTS_DMASTOP)
#define D1061_EPCTLSTS_DMASTART             D1061MASK(EPCTLSTS_DMASTART)
#define D1061_EPCTLSTS_STALL                D1061MASK(EPCTLSTS_STALL)
#define D1061_EPCTLSTS_ACKSET               D1061MASK(EPCTLSTS_ACKSET)
#define D1061_EPCTLSTS_EPRST                D1061MASK(EPCTLSTS_EPRST)
#define D1061_EPDDC_NUMDESC                 D1061MASK(EPDDC_NUMDESC)
#define D1061_EPDDA_DESCADDR                D1061MASK(EPDDA_DESCADDR)

#define D1061_INTR_MASK1_ALL               (D1061_INTENABLE1_EP2PLERR  |    \
                                            D1061_INTENABLE1_EP2OUTDMA |    \
                                            D1061_INTENABLE1_EP2INDMA  |    \
                                            D1061_INTENABLE1_EP1PLERR  |    \
                                            D1061_INTENABLE1_EP1OUTDMA |    \
                                            D1061_INTENABLE1_EP1INDMA  |    \
                                            D1061_INTENABLE1_EP0PLERR  |    \
                                            D1061_INTENABLE1_EP0OUTDMA |    \
                                            D1061_INTENABLE1_EP0INDMA  |    \
                                            D1061_INTENABLE1_EP0STATUS |    \
                                            D1061_INTENABLE1_EP0SETUP  |    \
                                            D1061_INTENABLE1_WAKEUP    |    \
                                            D1061_INTENABLE1_RESUME    |    \
                                            D1061_INTENABLE1_SUSPEND   |    \
                                            D1061_INTENABLE1_BUSRST    |    \
                                            D1061_INTENABLE1_CNTSTSCHG |    \
                                            D1061_INTENABLE1_SYSERR    )
#define D1061_INTR_MASK2_ALL               (D1061_INTENABLE2_EP3PLERR  |    \
                                            D1061_INTENABLE2_EP3OUTDMA |    \
                                            D1061_INTENABLE2_EP3INDMA  )
#define D1061_INTR_MASK1_CTRL              (D1061_INTENABLE1_EP0PLERR  |    \
                                            D1061_INTENABLE1_EP0OUTDMA |    \
                                            D1061_INTENABLE1_EP0INDMA  |    \
                                            D1061_INTENABLE1_EP0STATUS |    \
                                            D1061_INTENABLE1_EP0SETUP  |    \
                                            D1061_INTENABLE1_WAKEUP    |    \
                                            D1061_INTENABLE1_RESUME    |    \
                                            D1061_INTENABLE1_SUSPEND   |    \
                                            D1061_INTENABLE1_BUSRST    |    \
                                            D1061_INTENABLE1_CNTSTSCHG |    \
                                            D1061_INTENABLE1_SYSERR    )
#define D1061_INTR_MASK1_EP1               (D1061_INTENABLE1_EP1PLERR  |    \
                                            D1061_INTENABLE1_EP1OUTDMA |    \
                                            D1061_INTENABLE1_EP1INDMA  )
#define D1061_INTR_MASK1_EP2               (D1061_INTENABLE1_EP2PLERR  |    \
                                            D1061_INTENABLE1_EP2OUTDMA |    \
                                            D1061_INTENABLE1_EP2INDMA  )
#define D1061_INTR_MASK2_EP3               (D1061_INTR_MASK2_ALL)

#define D1061_EP_NUM                        (8)
#define D1061_EP0_IN                        (0)
#define D1061_EP0_OUT                       (1)
#define D1061_EP1_IN                        (2)
#define D1061_EP1_OUT                       (3)
#define D1061_EP2_IN                        (4)
#define D1061_EP2_OUT                       (5)
#define D1061_EP3_IN                        (6)
#define D1061_EP3_OUT                       (7)

#define D1061_EPTYPE_CTRL                   (0)
#define D1061_EPTYPE_ISOC                   (1)
#define D1061_EPTYPE_BULK                   (2)
#define D1061_EPTYPE_INTR                   (3)
                
#define D1061_EPIDX_EP0                     (0)
#define D1061_EPIDX_EP1                     (1)
#define D1061_EPIDX_EP2                     (2)
#define D1061_EPIDX_EP3                     (3)
                
#define D1061_EPDIR_IN                      (0)
#define D1061_EPDIR_OUT                     (1)

#if defined(D1061_HIGH_SPEED)
    #define D1061_EP0SETUP_SIZE             (8)
    #define D1061_EP0IN_MAXPACKETSIZE       (64)
    #define D1061_EP0OUT_MAXPACKETSIZE      (64)
    #define D1061_EP1IN_MAXPACKETSIZE       (512)
    #define D1061_EP1OUT_MAXPACKETSIZE      (512)
    #define D1061_EP2IN_MAXPACKETSIZE       (512)
    #define D1061_EP2OUT_MAXPACKETSIZE      (512)
    #define D1061_EP3IN_MAXPACKETSIZE       (512)
    #define D1061_EP3OUT_MAXPACKETSIZE      (512)
#else
    #define D1061_EP0SETUP_SIZE             (8)
    #define D1061_EP0IN_MAXPACKETSIZE       (64)
    #define D1061_EP0OUT_MAXPACKETSIZE      (64)
    #define D1061_EP1IN_MAXPACKETSIZE       (64)
    #define D1061_EP1OUT_MAXPACKETSIZE      (64)
    #define D1061_EP2IN_MAXPACKETSIZE       (64)
    #define D1061_EP2OUT_MAXPACKETSIZE      (64)
    #define D1061_EP3IN_MAXPACKETSIZE       (64)
    #define D1061_EP3OUT_MAXPACKETSIZE      (64)
#endif

#define D1061_DMA_CTL_DATA_LENGTH_MASK      (0xFFFFUL)
#define D1061_DMA_CTL_TERMINATE             (1UL << 16)
#define D1061_DMA_CTL_INTR_ON_COMPLETION    (1UL << 17)
#define D1061_DMA_CTL_ACTIVE                (1UL << 18)
#define D1061_DMA_STS_HALT                  (1UL << 19)
#define D1061_DMA_STS_STALL                 (1UL << 20)
#define D1061_DMA_STS_SHORT_PACKET          (1UL << 21)
#define D1061_DMA_STS_INCOMING_SETUP        (1UL << 22)
#define D1061_DMA_STS_PROTOCOL_ERROR        (1UL << 23)
#define D1061_DMA_STS_OTHER_ERROR           (1UL << 24)
struct d1061_data_descriptor {
    unsigned long dataptr;
    unsigned long ctrlsts;
} __attribute((packed));

static inline int d1061_get_frame(struct usb_device *dev)
{
    return (io_In32(dev->io, D1061_FRAMENUM_ADDR) >> D1061_FRAMENUM_FN_SHIFT) & D1061_FRAMENUM_FN_RANGE;
}

static inline void d1061_rmt_wakeup(struct usb_device *dev)
{
    io_Out32(dev->io, D1061_GBLCTL_ADDR, io_In32(dev->io, D1061_GBLCTL_ADDR) | D1061_GBLCTL_WAKEUP);
}

static inline void d1061_set_disconnect(struct usb_device *dev)
{
    io_Out32(dev->io, D1061_GBLCTL_ADDR, io_In32(dev->io, D1061_GBLCTL_ADDR) & ~D1061_GBLCTL_PULLUP);
}

static inline bool d1061_connect_status(struct usb_device *dev)
{
    return io_In32(dev->io, D1061_GBLSTS_ADDR) & D1061_GBLSTS_CONNECT;
}

static inline void d1061_clear_disconnect(struct usb_device *dev)
{
    io_Out32(dev->io, D1061_GBLCTL_ADDR, io_In32(dev->io, D1061_GBLCTL_ADDR) | D1061_GBLCTL_PULLUP);
}

static inline void d1061_write_interrupt_enable1(struct usb_device *dev, unsigned long mask)
{
    io_Out32(dev->io, D1061_INTENABLE1_ADDR, mask);
}

static inline void d1061_write_interrupt_enable2(struct usb_device *dev, unsigned long mask)
{
    io_Out32(dev->io, D1061_INTENABLE2_ADDR, mask);
}

static inline unsigned long d1061_read_interrupt_enable1(struct usb_device *dev)
{
    return io_In32(dev->io, D1061_INTENABLE1_ADDR);
}

static inline unsigned long d1061_read_interrupt_enable2(struct usb_device *dev)
{
    return io_In32(dev->io, D1061_INTENABLE2_ADDR);
}

static inline void d1061_reinit(struct usb_device *dev)
{
    #if defined(D1061_HIGH_SPEED)
        io_Out32(dev->io, D1061_GBLCFG_ADDR, io_In32(dev->io, D1061_GBLCFG_ADDR) & ~D1061_GBLCFG_FULL);
    #else
        io_Out32(dev->io, D1061_GBLCFG_ADDR, io_In32(dev->io, D1061_GBLCFG_ADDR) | D1061_GBLCFG_FULL);
    #endif
    
    io_Out32(dev->io, D1061_GBLCTL_ADDR, D1061_GBLCTL_DEVCTLE | D1061_GBLCTL_GBLINTE);
}

static inline unsigned long d1061_read_interrupt_status1(struct usb_device *dev)
{
    return io_In32(dev->io, D1061_INTSTS1_ADDR);
}

static inline unsigned long d1061_read_interrupt_status2(struct usb_device *dev)
{
    return io_In32(dev->io, D1061_INTSTS2_ADDR);
}

static inline void d1061_write_interrupt_status1(struct usb_device *dev, unsigned long isr1)
{
    io_Out32(dev->io, D1061_INTSTS1_ADDR, isr1);
}

static inline void d1061_write_interrupt_status2(struct usb_device *dev, unsigned long isr2)
{
    io_Out32(dev->io, D1061_INTSTS1_ADDR, isr2);
}

static void d1061_ep_enable_interrupt(struct usb_ep *ep)
{
    unsigned long value;
    
    if (ep->num < D1061_EPIDX_EP3) {
        value = d1061_read_interrupt_enable1(ep->dev);
        if (ep->num == D1061_EPIDX_EP1 && !ep->in) value |= D1061_INTENABLE1_EP1OUTDMA;
        if (ep->num == D1061_EPIDX_EP1 &&  ep->in) value |= D1061_INTENABLE1_EP1INDMA;
        if (ep->num == D1061_EPIDX_EP2 && !ep->in) value |= D1061_INTENABLE1_EP2OUTDMA;
        if (ep->num == D1061_EPIDX_EP2 &&  ep->in) value |= D1061_INTENABLE1_EP2INDMA;
        d1061_write_interrupt_enable1(ep->dev, value);
    } else {
        value = d1061_read_interrupt_enable2(ep->dev);
        if (ep->num == D1061_EPIDX_EP3 && !ep->in) value |= D1061_INTENABLE2_EP3OUTDMA;
        if (ep->num == D1061_EPIDX_EP3 &&  ep->in) value |= D1061_INTENABLE2_EP3INDMA;
        d1061_write_interrupt_enable2(ep->dev, value);
    }
}

static void d1061_ep_disable_interrupt(struct usb_ep *ep)
{
    unsigned long value;
    
    if (ep->num < D1061_EPIDX_EP3) {
        value = d1061_read_interrupt_enable1(ep->dev);
        if (ep->num == D1061_EPIDX_EP1 && !ep->in) value &= ~D1061_INTENABLE1_EP1OUTDMA;
        if (ep->num == D1061_EPIDX_EP1 &&  ep->in) value &= ~D1061_INTENABLE1_EP1INDMA;
        if (ep->num == D1061_EPIDX_EP2 && !ep->in) value &= ~D1061_INTENABLE1_EP2OUTDMA;
        if (ep->num == D1061_EPIDX_EP2 &&  ep->in) value &= ~D1061_INTENABLE1_EP2INDMA;
        d1061_write_interrupt_enable1(ep->dev, value);
    } else {
        value = d1061_read_interrupt_enable2(ep->dev);
        if (ep->num == D1061_EPIDX_EP3 && !ep->in) value &= ~D1061_INTENABLE2_EP3OUTDMA;
        if (ep->num == D1061_EPIDX_EP3 &&  ep->in) value &= ~D1061_INTENABLE2_EP3INDMA;
        d1061_write_interrupt_enable2(ep->dev, value);
    }
}

static void d1061_ep_enable(struct usb_ep *ep, struct usb_endpoint_descriptor *desc)
{
    unsigned long value;
    unsigned long offset = D1061_EPxREGS_OFFSET(ep->num);
    offset += (ep->in) ? D1061_EPxINCFG_ADDR : D1061_EPxOUTCFG_ADDR;
    
    value = (desc->wMaxPacketSize & D1061_EPCFG_MAXPKTSIZE_RANGE) << 16;
    value |= (ep->num & D1061_EPCFG_EPNUM_RANGE) << 8;
    value |= (desc->bmAttributes & D1061_EPCFG_EPTYPE_RANGE) << 1;
    value |= D1061_EPCFG_EPENABLE;
    
    io_Out32(ep->dev->io, offset, value);
    
    d1061_ep_enable_interrupt(ep);
}

static void d1061_ep_disable(struct usb_ep *ep)
{
    unsigned long offset = D1061_EPxREGS_OFFSET(ep->num);
    offset += (ep->in) ? D1061_EPxINCFG_ADDR : D1061_EPxOUTCFG_ADDR;
    
    d1061_ep_disable_interrupt(ep);
    io_Out32(ep->dev->io, offset, io_In32(ep->dev->io, offset) & ~D1061_EPCFG_EPENABLE);
}

static inline void d1061_setup_ep0(struct usb_device *dev)
{
    unsigned long value[2];
    unsigned long offset = D1061_EPxREGS_OFFSET(dev->ep[0].num);
    
    value[0] = (dev->ep[0].in) ? D1061_EP0IN_MAXPACKETSIZE : D1061_EP0OUT_MAXPACKETSIZE;
    value[0] = (value[0] & D1061_EPCFG_MAXPKTSIZE_RANGE) << 16;
    value[0] |= (D1061_EPTYPE_CTRL & D1061_EPCFG_EPTYPE_RANGE) << 1;
    value[0] |= D1061_EPCFG_EPENABLE;
    
    value[1] = (dev->ep[1].in) ? D1061_EP0IN_MAXPACKETSIZE : D1061_EP0OUT_MAXPACKETSIZE;
    value[1] = (value[1] & D1061_EPCFG_MAXPKTSIZE_RANGE) << 16;
    value[1] |= (D1061_EPTYPE_CTRL & D1061_EPCFG_EPTYPE_RANGE) << 1;
    value[1] |= D1061_EPCFG_EPENABLE;
    
    io_Out32(dev->io, D1061_EP0SETUPDBA_ADDR, dev->dma_setup);
    io_Out32(dev->io, offset + D1061_EPxINCFG_ADDR, value[0]);
    io_Out32(dev->io, offset + D1061_EPxOUTCFG_ADDR, value[1]);
}

static void d1061_ep_set_stall(struct usb_ep *ep)
{
    unsigned long offset = D1061_EPxREGS_OFFSET(ep->num);
    offset += (ep->in) ? D1061_EPxINCTLSTS_ADDR : D1061_EPxOUTCTLSTS_ADDR;
    
    d1061_ep_disable_interrupt(ep);
    io_Out32(ep->dev->io, offset, io_In32(ep->dev->io, offset) | D1061_EPCTLSTS_STALL);
}

static void d1061_ep_clr_stall(struct usb_ep *ep)
{
    unsigned long offset = D1061_EPxREGS_OFFSET(ep->num);
    offset += (ep->in) ? D1061_EPxINCTLSTS_ADDR : D1061_EPxOUTCTLSTS_ADDR;
    
    io_Out32(ep->dev->io, offset, D1061_EPCTLSTS_EPRST);
    while (io_In32(ep->dev->io, offset) & D1061_EPCTLSTS_EPRST);
    io_Out32(ep->dev->io, offset, D1061_EPCTLSTS_DMASTART);
    d1061_ep_enable(ep, ep->desc);
}

static void d1061_set_ack(struct usb_ep *ep)
{
    unsigned long offset = D1061_EPxREGS_OFFSET(ep->num);
    offset += (ep->in) ? D1061_EPxINCTLSTS_ADDR : D1061_EPxOUTCTLSTS_ADDR;
    
    io_Out32(ep->dev->io, offset, io_In32(ep->dev->io, offset) | D1061_EPCTLSTS_ACKSET);
}

static void d1061_hardware_reset(struct usb_device *dev)
{
    io_Out32(dev->io, D1061_GBLCTL_ADDR, io_In32(dev->io, D1061_GBLCTL_ADDR) | D1061_GBLCTL_HWRST);
    while (io_In32(dev->io, D1061_GBLCTL_ADDR) & D1061_GBLCTL_HWRST);
}

static void d1061_software_reset(struct usb_device *dev)
{
    io_Out32(dev->io, D1061_GBLCTL_ADDR, io_In32(dev->io, D1061_GBLCTL_ADDR) | D1061_GBLCTL_SWRST);
    while (io_In32(dev->io, D1061_GBLCTL_ADDR) & D1061_GBLCTL_SWRST);
}

static void d1061_reconnect(struct usb_device *dev)
{
    d1061_clear_disconnect(dev);
}

static bool d1061_ep_is_busy(struct usb_ep *ep)
{
    unsigned long offset = D1061_EPxREGS_OFFSET(ep->num);
    offset += (ep->in) ? D1061_EPxINCTLSTS_ADDR : D1061_EPxOUTCTLSTS_ADDR;
    
    return (io_In32(ep->dev->io, offset) & D1061_EPCTLSTS_DMASTART) ? true : false;
}

static void d1061_set_address(struct usb_device *dev, int address)
{
    unsigned long config;
    
    config = io_In32(dev->io, D1061_GBLCFG_ADDR) & ~D1061_GBLCFG_DEVADDR;
    config = config | dev->address;
    io_Out32(dev->io, D1061_GBLCFG_ADDR, config);
}

static unsigned int d1061_ep_done(struct usb_ep *ep, unsigned long *status)
{
    struct d1061_data_descriptor desc;
    unsigned int size;
    
    dosmemget(ep->dma_desc, sizeof(struct d1061_data_descriptor), &desc);
    
    *status = desc.ctrlsts;
    
    size = desc.ctrlsts & 0xFFFF;
    
    if (size > 0)
        dosmemget(ep->dma_buf, size, ep->buf);
    
    return size;
}


static void d1061_ep_init(struct usb_ep *ep, unsigned int size)
{
    struct usb_device *dev;
    struct d1061_data_descriptor desc;
    unsigned long offset;
    
    dev = ep->dev;
    offset = D1061_EPxREGS_OFFSET(ep->num);
    
    if (ep->in) {
        
        desc.dataptr = ep->dma_buf;
        desc.ctrlsts = (size & 0xFFFF) | D1061_DMA_CTL_TERMINATE | D1061_DMA_CTL_INTR_ON_COMPLETION | D1061_DMA_CTL_ACTIVE;
        
        dosmemput(ep->buf, size, ep->dma_buf);
        dosmemput(&desc, sizeof(struct d1061_data_descriptor), ep->dma_desc);
        
        io_Out32(dev->io, offset + D1061_EPxINDDC_ADDR, 0x00UL);
        io_Out32(dev->io, offset + D1061_EPxINDDA_ADDR, ep->dma_desc);
        io_Out32(dev->io, offset + D1061_EPxINCTLSTS_ADDR, D1061_EPCTLSTS_DMASTART);
        
    } else {
        
        desc.dataptr = ep->dma_buf;
        desc.ctrlsts = (size & 0xFFFF) | D1061_DMA_CTL_TERMINATE | D1061_DMA_CTL_INTR_ON_COMPLETION | D1061_DMA_CTL_ACTIVE;
        
        dosmemput(&desc, sizeof(struct d1061_data_descriptor), ep->dma_desc);
        
        io_Out32(dev->io, offset + D1061_EPxOUTDDC_ADDR, 0x00UL);
        io_Out32(dev->io, offset + D1061_EPxOUTDDA_ADDR, ep->dma_desc);
        io_Out32(dev->io, offset + D1061_EPxOUTCTLSTS_ADDR, D1061_EPCTLSTS_DMASTART);
        
    }
}

static inline void d1061_bus_reset(struct usb_device *dev)
{
    unsigned long value = D1061_INTR_MASK1_CTRL;
    
    if (dev->sof_interrupt)
        value |= D1061_INTENABLE1_SOF;
    
    d1061_software_reset(dev);
    d1061_setup_ep0(dev);
    d1061_write_interrupt_enable1(dev, value);
    d1061_write_interrupt_enable2(dev, 0);
}

static inline void d1061_suspend(struct usb_device *dev)
{
    struct class_register *reg;
    
    if (dev->reset)
        dev->reset = true;
    
    
    reg = dev->class_reg;
    while (reg) {
        
        reg->suspend(reg->data);
        reg = reg->next;
        
    }
}

static inline void d1061_sof(struct usb_device *dev)
{
    if (dev->start_of_frame)
        dev->start_of_frame();
}

static inline void d1061_status_stage(struct usb_device *dev)
{
    if (dev->address) {
        
        d1061_set_address(dev, dev->address);
        dev->address = 0;
        
    }

    d1061_setup_ep0(dev);
}

static inline void d1061_setup_stage(struct usb_device *dev)
{
    struct usb_ctrlrequest *setup = &dev->setup;
    struct usb_ep *ep = dev->in;
    struct class_register *reg;
    
    dosmemget(dev->dma_setup, 8, setup);

    switch (setup->bRequest) {
    case USB_REQ_SET_CONFIGURATION:
        if (setup->bRequestType == USB_RECIP_DEVICE) {
            
            d1061_set_ack(ep);
            reg = dev->class_reg;
            while (reg) {
                
                reg->classenable(reg->data);
                reg = reg->next;
                
            }
            
        }
        break;

    case USB_REQ_SET_INTERFACE:
        if (setup->bRequestType == USB_RECIP_INTERFACE)
            d1061_set_ack(ep);
        break;

    case USB_REQ_SET_ADDRESS:
        if (setup->bRequestType == USB_RECIP_DEVICE) {
            dev->address = setup->wValue & 0x7F;
            d1061_set_ack(ep);
        }
        break;
        
    case USB_REQ_CLEAR_FEATURE:
        if (setup->bRequestType == USB_RECIP_ENDPOINT &&
            setup->wValue       == USB_ENDPOINT_HALT  &&
            setup->wLength      == 0) {
            int idx = ((setup->wIndex & 0x7f) << 1) + ((setup->wIndex & 0x80) ? 0 : 1);
            d1061_set_ack(ep);
            if (dev->ep[idx].stall) {
                
                d1061_ep_clr_stall(&dev->ep[idx]);
                dev->ep[idx].stall = false;
                
            }
        } else if (setup->bRequestType == (USB_DIR_OUT | USB_RECIP_INTERFACE))
            d1061_set_ack(ep);
        break;
        
    case USB_REQ_SET_FEATURE:
        if (setup->bRequestType == USB_RECIP_ENDPOINT &&
            setup->wValue       == USB_ENDPOINT_HALT  &&
            setup->wLength      == 0) {
            int idx = (setup->wIndex & 0x7f) << 1;
            d1061_set_ack(ep);
            d1061_ep_set_stall(&dev->ep[idx]);
        } else if (setup->bRequestType == (USB_DIR_OUT | USB_RECIP_INTERFACE))
            d1061_set_ack(ep);
        break;

    default:
        if (setup->bRequestType & USB_DIR_IN) {
            if (setup->wLength == 0)
                d1061_set_ack(dev->out);
        } else {
            if (setup->wLength == 0)
                d1061_set_ack(ep);
        }
        break;
    }
    
    usb_control_request(dev, setup);
    d1061_setup_ep0(dev);
}

static inline void d1061_data_in(struct usb_ep *ep)
{
    int i, len;
    
    if (d1061_ep_is_busy(ep))
        return;

    i = 0;
    while (i < ep->maxpacket && (ep->dataptr - ep->databuf) < ep->datalen) {
        
        ep->buf[i++] = ep->dataptr[0];
        ep->dataptr++;
        
    }
    
    if (i > 0) {
        
        d1061_ep_init(ep, i);
        if (ep->dev->data_in)
            ep->dev->data_in();
        
    } else if ((len = ep->dataptr - ep->databuf) >= ep->datalen) {
        
        if (ep->complete)
            ep->complete(ep->data, len);
        
    }
}

static inline void d1061_data_out(struct usb_ep *ep)
{
    unsigned long size, status;
    int i, len;

    size = d1061_ep_done(ep, &status);
    
    if ((status & D1061_DMA_STS_HALT) && !(status & D1061_DMA_STS_SHORT_PACKET)) {
        
        d1061_ep_set_stall(ep);
        return;
        
    }
    
    i = 0;
    while (i < size && (ep->dataptr - ep->databuf) < ep->datalen) {
        
        ep->dataptr[0] = ep->buf[i++];
        ep->dataptr++;
    
    }
    
    if ((len = ep->dataptr - ep->databuf) >= ep->datalen || size < ep->maxpacket) {
        
        if (ep->num == 0)
            d1061_set_ack(ep->dev->in);
        if (ep->complete)
            ep->complete(ep->data, len);
        if (ep->dev->data_out)
            ep->dev->data_out();
        
    }

}

static inline void d1061_ep_init_transfer(struct usb_ep *ep)
{
    int size = ep->datalen;
    
    if (size > ep->maxpacket)
        size = ep->maxpacket;
    
    if (ep->num == 0 && ep->in)
        d1061_set_ack(ep->dev->out);
    
    if (ep->in)
        d1061_data_in(ep);
    else
        d1061_ep_init(ep, size);
}

static inline bool d1061_initialize(struct usb_device *dev)
{
    int i;
    
    for (i = 0; i < USB_EP_NUM; i++) {
        
        dev->ep[i].dev = dev;
        dev->ep[i].num = i / 2;
        dev->ep[i].in = ~i & 1;
        
        if (dev->ep[i].num == 0 && dev->ep[i].in) {
            
            dev->in = USBDev_InitEndpoint(dev, &control_in, NULL, NULL);
            USBDev_EnableEndpoint(dev->in);
            
        } else if (dev->ep[i].num == 0) {
        
            dev->out = USBDev_InitEndpoint(dev, &control_out, NULL, NULL);
            USBDev_EnableEndpoint(dev->out);
        
        }
        
    }
    
    if (!dev->in || !dev->out) {
        
        if (dev->in) {
            USBDev_DisableEndpoint(dev->in);
            USBDev_DeinitEndpoint(dev->in);
        }
        if (dev->out) {
            USBDev_DisableEndpoint(dev->out);
            USBDev_DeinitEndpoint(dev->out);
        }
        
        return false;
        
    }
    
    dev->dma_setup_handle = dma_Alloc(sizeof(struct usb_ctrlrequest), &dev->dma_setup);
    if (dev->dma_setup_handle == DMA_FAIL) {
        
        printf("ERROR: %s() alloc DMA buffer failed.\n", __FUNCTION__);
        return false;
        
    }
    
    d1061_hardware_reset(dev);
    d1061_reinit(dev);
    d1061_setup_ep0(dev);
    d1061_set_disconnect(dev);
    
    return true;
}

static inline void d1061_deinitialize(struct usb_device *dev)
{
    d1061_hardware_reset(dev);
    dma_Free(dev->dma_setup_handle);
    USBDev_DisableEndpoint(dev->in);
    USBDev_DisableEndpoint(dev->out);
    USBDev_DeinitEndpoint(dev->in);
    USBDev_DeinitEndpoint(dev->out);
}

static inline void d1061_enable_sof_int(struct usb_device *dev)
{
    unsigned long value;
    
    io_DisableINT();
    value = d1061_read_interrupt_enable1(dev);
    value |= D1061_INTENABLE1_SOF;
    d1061_write_interrupt_enable1(dev, value);
    io_RestoreINT();
}

static inline void d1061_disable_sof_int(struct usb_device *dev)
{
    unsigned long value;
    
    io_DisableINT();
    value = d1061_read_interrupt_enable1(dev);
    value &= ~D1061_INTENABLE1_SOF;
    d1061_write_interrupt_enable1(dev, value);
    io_RestoreINT();
}

static int d1061_isr(struct usb_device *dev)
{
    int status = ISR_NONE;
    unsigned long isr1, isr2;
    
    isr1 = io_In32(dev->io, D1061_INTSTS1_ADDR);
    isr2 = io_In32(dev->io, D1061_INTSTS2_ADDR);
    io_Out32(dev->io, D1061_INTSTS1_ADDR, isr1);
    io_Out32(dev->io, D1061_INTSTS2_ADDR, isr2);
    
    if (isr1 & D1061_INTSTS1_SYSERR) {
        
        status = ISR_HANDLED;
        
    }
    
    if (isr1 & D1061_INTSTS1_CNTSTSCHG) {
        
        status = ISR_HANDLED;
        
    }
    
    if (isr1 & D1061_INTSTS1_BUSRST) {
        
        d1061_bus_reset(dev);
        status = ISR_HANDLED;
        
    }
    
    if (isr1 & D1061_INTSTS1_SUSPEND) {
        
        d1061_suspend(dev);
        status = ISR_HANDLED;
        
    }
    
    if (isr1 & D1061_INTSTS1_RESUME) {
        
        status = ISR_HANDLED;
        
    }
    
    if (isr1 & D1061_INTSTS1_WAKEUP) {
        
        status = ISR_HANDLED;
        
    }
    
    if (isr1 & D1061_INTSTS1_SOF) {
        
        d1061_sof(dev);
        status = ISR_HANDLED;
        
    }
    
    if (isr1 & D1061_INTSTS1_EP0STATUS) {
        
        d1061_status_stage(dev);
        status = ISR_HANDLED;
        
    }
    
    if (isr1 & D1061_INTSTS1_EP0INDMA) {
        
        d1061_data_in(dev->in);
        status = ISR_HANDLED;
        
    }
    
    if (isr1 & D1061_INTSTS1_EP0OUTDMA) {
        
        d1061_data_out(dev->out);
        status = ISR_HANDLED;
        
    }
    
    if (isr1 & D1061_INTSTS1_EP0PLERR) {
        
        status = ISR_HANDLED;
        
    }
    
    if (isr1 & D1061_INTSTS1_EP0SETUP) {
        
        d1061_setup_stage(dev);
        status = ISR_HANDLED;
        
    }
    
    if (isr1 & D1061_INTSTS1_EP1INDMA) {
        
        d1061_data_in(&dev->ep[D1061_EP1_IN]);
        status = ISR_HANDLED;
        
    }
    
    if (isr1 & D1061_INTSTS1_EP1OUTDMA) {
        
        d1061_data_out(&dev->ep[D1061_EP1_OUT]);
        status = ISR_HANDLED;
        
    }
    
    if (isr1 & D1061_INTSTS1_EP1PLERR) {
        
        status = ISR_HANDLED;
        
    }
    
    if (isr1 & D1061_INTSTS1_EP2INDMA) {
        
        d1061_data_in(&dev->ep[D1061_EP2_IN]);
        status = ISR_HANDLED;
        
    }
    
    if (isr1 & D1061_INTSTS1_EP2OUTDMA) {
        
        d1061_data_out(&dev->ep[D1061_EP2_OUT]);
        status = ISR_HANDLED;
        
    }
    
    if (isr1 & D1061_INTSTS1_EP2PLERR) {
        
        status = ISR_HANDLED;
        
    }
    
    if (isr2 & D1061_INTSTS2_EP3INDMA) {
        
        d1061_data_in(&dev->ep[D1061_EP3_IN]);
        status = ISR_HANDLED;
        
    }
    
    if (isr2 & D1061_INTSTS2_EP3OUTDMA) {

        d1061_data_out(&dev->ep[D1061_EP3_OUT]);
        status = ISR_HANDLED;
        
    }
    
    if (isr2 & D1061_INTSTS2_EP3PLERR) {
        
        status = ISR_HANDLED;
        
    }
    
    return status;
}

bool d1061_start(USBDev *dev)
{
    unsigned long  value;
    
    d1061_setup_ep0(dev);
    
    value = D1061_INTR_MASK1_CTRL;
    if (dev->sof_interrupt)
        value |= D1061_INTENABLE1_SOF;
    
    io_DisableINT();
    d1061_write_interrupt_enable1(dev, value);
    d1061_write_interrupt_enable2(dev, 0);
    io_RestoreINT();
    
    d1061_clear_disconnect(dev);
    
    return true;
}

void d1061_stop(USBDev *dev)
{
    d1061_set_disconnect(dev);
    
    io_DisableINT();
    d1061_write_interrupt_enable1(dev, 0);
    d1061_write_interrupt_enable2(dev, 0);
    io_RestoreINT();
}

bool d1061_ep_dma_alloc(struct usb_ep *ep)
{
    ep->dma_handle = dma_Alloc(ep->maxpacket + 8, &ep->dma_desc);
    if (ep->dma_handle == DMA_FAIL) {
        
        printf("ERROR: %s() alloc DMA buffer failed.\n", __FUNCTION__);
        return false;
        
    }
    ep->dma_buf = ep->dma_desc + 8;
    
    return true;
}

struct usb_devive_ops d1061_ops = {
    
    d1061_isr,
    d1061_start,
    d1061_stop,
    d1061_initialize,
    d1061_deinitialize,
    d1061_enable_sof_int,
    d1061_disable_sof_int,
    
    d1061_ep_dma_alloc,
    d1061_ep_enable,
    d1061_ep_disable,
    d1061_ep_init_transfer,
    d1061_ep_set_stall,
    
};

#endif
