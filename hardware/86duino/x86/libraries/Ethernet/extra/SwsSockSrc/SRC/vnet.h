#ifndef __DMP_VNET_H
#define __DMP_VNET_H

typedef void *SWS_VN_t;
typedef void *SWS_VN_FRAME_t;

int SwsVN_GetDriverCount();
int SwsVN_GetInterfaceNo(int idx);

SWS_VN_t SwsVN_GetVirtualNetwork(int nic);

void SwsVN_RegisterReceivedCB(SWS_VN_t handle, void (*callback)(void *), void *user_data);
void SwsVN_RegisterSendCompletedCB(SWS_VN_t handle, void (*callback)(void *), void *user_data);
void SwsVN_SetMacAddress(SWS_VN_t handle, uint8_t *mac_address);
void SwsVN_GetMacAddress(SWS_VN_t handle, uint8_t *mac_address);

SWS_VN_FRAME_t SwsVN_RequestPacket(SWS_VN_t handle);
void SwsVN_SendPacket(SWS_VN_t handle, SWS_VN_FRAME_t packet, size_t size);

SWS_VN_FRAME_t SwsVN_RecvPacket(SWS_VN_t handle, size_t *psize);
void SwsVN_ReleasePacket(SWS_VN_t handle, SWS_VN_FRAME_t packet);

/* Set RX mode.
    1   turn off receiver
    2   receive only packets sent to this interface
    3   mode 2 plus broadcast packets
    4   mode 3 plus limited multicast packets
    5   mode 3 plus all multicast packets
    6   all packets
*/
int SwsVN_SetRxMode(SWS_VN_t handle, int mode);
int SwsVN_SetMulticastList(SWS_VN_t handle, void *pData, size_t size);

#endif
