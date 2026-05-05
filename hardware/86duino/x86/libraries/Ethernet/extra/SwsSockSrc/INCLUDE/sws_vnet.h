#ifndef __DMP_SWS_VNET_H
#define __DMP_SWS_VNET_H

#include "sws_vn.h"

int SwsVN_Initialize();
void SwsVN_Shutdown();

void SwsVN_AddDriver(int nic, VirtualNetwork *network);
void SwsVN_DelDriver(int nic);

#endif
