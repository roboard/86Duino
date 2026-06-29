/*
  scsi.cpp - DM&P Vortex86 SCSI Library for USB Device Mass Storage Class
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
#include <stdlib.h>
#include <string.h>
#include "scsi.h"
#include "disk.h"

#define MSC_CBW_SIGNATURE               (0x43425355)
#define MSC_CSW_SIGNATURE               (0x53425355)
#define MSC_CSW_CMD_PASSED              (0)
#define MSC_CSW_CMD_FAILED              (1)
#define MSC_CSW_PHASE_ERROR             (2)

#define SCSI_INQUIRY                    (0x12)
#define SCSI_READ_FORMAT_CAPACITIES     (0x23)
#define SCSI_READ_CAPACITY10            (0x25)
#define SCSI_READ10                     (0x28)
#define SCSI_MODE_SENSE6                (0x1A)
#define SCSI_REQUEST_SENSE              (0x03)
#define SCSI_TEST_UNIT_READY            (0x00)
#define SCSI_WRITE10                    (0x2A)
#define SCSI_PREVENT_ALLOW_MDM_RMVL     (0x1E)
#define SCSI_START_STOP                 (0x1B)

void SCSI_Inquiry(SCSI_MSC *scsi)
{
    scsi->TxBuf[0] = 0x00;
    scsi->TxBuf[1] = 0x80;
    scsi->TxBuf[2] = 0x00;
    scsi->TxBuf[3] = 0x01;
    scsi->TxBuf[4] = 0x1F;
    scsi->TxBuf[5] = 0x00;
    scsi->TxBuf[6] = 0x00;
    scsi->TxBuf[7] = 0x00;
    
    strcpy((char*)&scsi->TxBuf[ 8], "DMP");
    strcpy((char*)&scsi->TxBuf[16], "Vortex86 Drive");
    strcpy((char*)&scsi->TxBuf[32], "1.0");
    
    USBMSC_WriteData(scsi->msc, scsi->TxBuf, 36);
}

void SCSI_ReadFormatCapacities(SCSI_MSC *scsi)
{
    unsigned long block_size, block_count;
    
    block_size  = DC_GetBlockSize(scsi->dc);
    block_count = DC_GetBlockCount(scsi->dc);
    
    scsi->TxBuf[ 0] = 0x00;
    scsi->TxBuf[ 1] = 0x00;
    scsi->TxBuf[ 2] = 0x00;
    scsi->TxBuf[ 3] = 0x08;
    
    // Block Count
    scsi->TxBuf[ 4] = (block_count >> 24) & 0xFF;
    scsi->TxBuf[ 5] = (block_count >> 16) & 0xFF;
    scsi->TxBuf[ 6] = (block_count >>  8) & 0xFF;
    scsi->TxBuf[ 7] = (block_count >>  0) & 0xFF;
    
    scsi->TxBuf[ 8] = 0x02;
    
    // Block Size
    scsi->TxBuf[ 9] = (block_size >> 16) & 0xFF;
    scsi->TxBuf[10] = (block_size >>  8) & 0xFF;
    scsi->TxBuf[11] = (block_size >>  0) & 0xFF;
    
    USBMSC_WriteData(scsi->msc, scsi->TxBuf, 12);
}

void SCSI_ReadCapacity10(SCSI_MSC *scsi)
{
    unsigned long block_size, block_count;
    
    block_size  = DC_GetBlockSize(scsi->dc);
    block_count = DC_GetBlockCount(scsi->dc);
    
    scsi->TxBuf[0] = ((block_count - 1) >> 24) & 0xFF;
    scsi->TxBuf[1] = ((block_count - 1) >> 16) & 0xFF;
    scsi->TxBuf[2] = ((block_count - 1) >>  8) & 0xFF;
    scsi->TxBuf[3] = ((block_count - 1) >>  0) & 0xFF;
    
    scsi->TxBuf[4] = (block_size >> 24) & 0xFF;
    scsi->TxBuf[5] = (block_size >> 16) & 0xFF;
    scsi->TxBuf[6] = (block_size >>  8) & 0xFF;
    scsi->TxBuf[7] = (block_size >>  0) & 0xFF;
    
    USBMSC_WriteData(scsi->msc, scsi->TxBuf, 8);
}

bool SCSI_Read10(SCSI_MSC *scsi)
{
    USBMSC *msc = scsi->msc;
    USBMSC_CBW *cbw = &scsi->cbw;
    bool ret = false;
    unsigned char *buf;
    unsigned long ns;

    switch (scsi->read_state) {
    
    case 0:
        scsi->sector = (cbw->CBWCB[2] << 24 | cbw->CBWCB[3] << 16 | cbw->CBWCB[4] << 8 | cbw->CBWCB[5]);
        scsi->nsects = (cbw->CBWCB[7] <<  8 | cbw->CBWCB[8]);
        scsi->read_state = 1;
        break;
    case 1:
        buf = DC_ReadSector(scsi->dc, scsi->sector, scsi->nsects, &ns);
        if (ns <= 0) {
        
            ret = true;
            break;
        
        }
        scsi->sector += ns;
        scsi->nsects -= ns;
        USBMSC_WriteData(scsi->msc, &buf[0], ns * DC_GetBlockSize(scsi->dc));
        scsi->read_state = 2;
        break;
    case 2:
        if (USBMSC_WriteComplete(scsi->msc)) {
            
            DC_ReadComplete(scsi->dc);
            if (scsi->nsects > 0) {
                
                scsi->read_state = 1;
                
            } else {
                
                scsi->read_state = 0;
                ret = true;
                
            }
            
        }
        break;
    
    }
    
    return ret;
}

void SCSI_ModeSense6(SCSI_MSC *scsi)
{
    scsi->TxBuf[0] = 0x03;
    scsi->TxBuf[1] = 0x00;
    scsi->TxBuf[2] = 0x00;
    scsi->TxBuf[3] = 0x00;
    
    USBMSC_WriteData(scsi->msc, scsi->TxBuf, 4);
}

bool SCSI_Write10(SCSI_MSC *scsi)
{
    USBMSC *msc = scsi->msc;
    USBMSC_CBW *cbw = &scsi->cbw;
    bool ret = false;
    unsigned char *buf;
    unsigned long ns;
    
    switch (scsi->write_state) {
    
    case 0:
        scsi->sector = (cbw->CBWCB[2] << 24 | cbw->CBWCB[3] << 16 | cbw->CBWCB[4] << 8 | cbw->CBWCB[5]);
        scsi->nsects = (cbw->CBWCB[7] <<  8 | cbw->CBWCB[8]);
        scsi->write_state = 1;
        break;
    case 1:
        buf = DC_WriteSector(scsi->dc, scsi->sector, scsi->nsects, &ns);
        if (ns <= 0) {
        
            ret = true;
            break;
        
        }
        scsi->sector += ns;
        scsi->nsects -= ns;
        USBMSC_ReadData(scsi->msc, &buf[0], ns * DC_GetBlockSize(scsi->dc));
        scsi->write_state = 2;
        break;
    case 2:
        if (USBMSC_ReadComplete(scsi->msc)) {
            
            DC_WriteComplete(scsi->dc);
            if (scsi->nsects > 0) {
                
                scsi->write_state = 1;
                
            } else {
                
                scsi->write_state = 0;
                ret = true;
                
            }
            
        }
        break;
    
    }
    
    return ret;
}

void SCSI_RequestSense(SCSI_MSC *scsi)
{
    scsi->TxBuf[ 0] = 0x70;
    scsi->TxBuf[ 1] = 0x00;
    scsi->TxBuf[ 2] = 0x02;
    scsi->TxBuf[ 3] = 0x00;
    scsi->TxBuf[ 4] = 0x00;
    scsi->TxBuf[ 5] = 0x00;
    scsi->TxBuf[ 6] = 0x00;
    scsi->TxBuf[ 7] = 0x0A;
    scsi->TxBuf[ 8] = 0x00;
    scsi->TxBuf[ 9] = 0x00;
    scsi->TxBuf[10] = 0x00;
    scsi->TxBuf[11] = 0x00;
    scsi->TxBuf[12] = 0x3A;
    scsi->TxBuf[13] = 0x00;
    scsi->TxBuf[14] = 0x00;
    scsi->TxBuf[15] = 0x00;
    scsi->TxBuf[16] = 0x00;
    scsi->TxBuf[17] = 0x00;
    
    USBMSC_WriteData(scsi->msc, scsi->TxBuf, 18);
}

bool SCSI_Parser(SCSI_MSC *scsi)
{
    bool ret = false;
    USBMSC *msc = scsi->msc;
    USBMSC_CBW *cbw = &scsi->cbw;
    USBMSC_CSW *csw = &scsi->csw;
    
    switch (scsi->parse_state) {
    
    case 0:
        if (cbw->dCBWSignature == MSC_CBW_SIGNATURE) {
        
            csw->dCSWSignature = MSC_CSW_SIGNATURE;
            csw->dCSWTag = cbw->dCBWTag;
            csw->dCSWDataResidue = cbw->dCBWDataTransferLength;
            
            if (cbw->bCBWLUN != 0 || cbw->bCBWCBLength < 1 || cbw->bCBWCBLength > 16) {
            
                csw->bCSWStatus = MSC_CSW_CMD_FAILED;
            
            } else {
            
                switch (cbw->CBWCB[0]) {
                
                case SCSI_INQUIRY:
                    SCSI_Inquiry(scsi);
                    csw->bCSWStatus = MSC_CSW_CMD_PASSED;
                    scsi->parse_state = 1;
                    break;
                case SCSI_READ_FORMAT_CAPACITIES:
                    SCSI_ReadFormatCapacities(scsi);
                    csw->bCSWStatus = MSC_CSW_CMD_PASSED;
                    scsi->parse_state = 1;
                    break;
                case SCSI_READ_CAPACITY10:
                    SCSI_ReadCapacity10(scsi);
                    csw->bCSWStatus = MSC_CSW_CMD_PASSED;
                    scsi->parse_state = 1;
                    break;
                case SCSI_READ10:
                    if (!(cbw->bmCBWFlags & 0x80)) {
                        
                        csw->bCSWStatus = MSC_CSW_PHASE_ERROR;
                        scsi->parse_state = 1;
                        
                    } else if (SCSI_Read10(scsi)) {
                        
                        csw->bCSWStatus = MSC_CSW_CMD_PASSED;
                        scsi->parse_state = 1;
                        
                    }
                    break;
                case SCSI_MODE_SENSE6:
                    SCSI_ModeSense6(scsi);
                    csw->bCSWStatus = MSC_CSW_CMD_PASSED;
                    scsi->parse_state = 1;
                    break;
                case SCSI_WRITE10:
                    if (cbw->bmCBWFlags & 0x80) {
                        
                        csw->bCSWStatus = MSC_CSW_PHASE_ERROR;
                        scsi->parse_state = 1;
                        
                    } else if (SCSI_Write10(scsi)) {
                        
                        csw->bCSWStatus = MSC_CSW_CMD_PASSED;
                        scsi->parse_state = 1;
                        
                    }
                    break;
                case SCSI_REQUEST_SENSE:
                    SCSI_RequestSense(scsi);
                    csw->bCSWStatus = MSC_CSW_CMD_PASSED;
                    scsi->parse_state = 1;
                    break;
                case SCSI_TEST_UNIT_READY:
                    if (scsi->disconnect)
                        csw->bCSWStatus = MSC_CSW_CMD_FAILED;
                    else
                        csw->bCSWStatus = MSC_CSW_CMD_PASSED;
                    ret = true;
                    break;
                case SCSI_PREVENT_ALLOW_MDM_RMVL:
                    csw->bCSWStatus = MSC_CSW_CMD_PASSED;
                    ret = true;
                    break;
                case SCSI_START_STOP:
                    csw->bCSWStatus = MSC_CSW_CMD_PASSED;
                    scsi->disconnect = true;
                    ret = true;
                    break;
                default:
                    csw->bCSWStatus = MSC_CSW_CMD_FAILED;
                    ret = true;
                    break;
                }
            
            }
            
        
        }
        break;
    case 1:
        if (USBMSC_WriteComplete(msc)) {
            
            csw->dCSWDataResidue = 0;
            scsi->parse_state = 0;
            ret = true;
            
        }
        break;
    }
    
    return ret;
}

void MSC_Ready(SCSI_MSC *scsi);
void MSC_CommandTransport(SCSI_MSC *scsi);
void MSC_DataTransport(SCSI_MSC *scsi);
void MSC_StatusTransport(SCSI_MSC *scsi);
void MSC_EndTransport(SCSI_MSC *scsi);

void MSC_Ready(SCSI_MSC *scsi)
{
    if (USBMSC_ReadCBW(scsi->msc, &scsi->cbw))
        scsi->poll_state = MSC_CommandTransport;
}

void MSC_CommandTransport(SCSI_MSC *scsi)
{
    if (USBMSC_ReadComplete(scsi->msc))
        scsi->poll_state = MSC_DataTransport;
}

void MSC_DataTransport(SCSI_MSC *scsi)
{
    if (SCSI_Parser(scsi))
        scsi->poll_state = MSC_StatusTransport;
}

void MSC_StatusTransport(SCSI_MSC *scsi)
{
    if (USBMSC_WriteCSW(scsi->msc, &scsi->csw))
        scsi->poll_state = MSC_EndTransport;
}

void MSC_EndTransport(SCSI_MSC *scsi)
{
    if (USBMSC_WriteComplete(scsi->msc))
        scsi->poll_state = MSC_Ready;
}

void SCSI_Reset(SCSI_MSC *scsi)
{
    scsi->poll_state = MSC_Ready;
    scsi->read_state = 0;
    scsi->write_state = 0;
    scsi->parse_state = 0;
}

void SCSI_Suspend(SCSI_MSC *scsi)
{
    scsi->disconnect = false;
}

void SCSI_Poll(SCSI_MSC *scsi)
{
    
    if (!USBMSC_CheckReady(scsi->msc)) {
        
        SCSI_Reset(scsi);
        return;
        
    }
    
    scsi->poll_state(scsi);
}

SCSI_MSC *SCSI_Init(USBMSC *msc, DISK_CTRL *dc)
{
    SCSI_MSC *scsi;
    
    scsi = (SCSI_MSC *)malloc(sizeof(SCSI_MSC));
    if (scsi == NULL)
        return NULL;
    
    scsi->msc = msc;
    scsi->dc  = dc;
    
    scsi->disconnect = false;
    
    scsi->parse_state = 0;
    scsi->write_state = 0;
    scsi->read_state  = 0;
    scsi->poll_state  = MSC_Ready;
    
    scsi->sector = 0;
    scsi->nsects = 0;
    
    return scsi;
}

void SCSI_Close(SCSI_MSC *scsi)
{
    free(scsi);
}
