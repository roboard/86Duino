#include "sws_vnet.h"
#include "vnet.h"
#include "list.h"
#include "io.h"

#include <stdlib.h>
#include <string.h>

typedef struct {

    list_head node;

    VirtualNetwork *network;
    int nic;
    int dummy;

} SwsVN_Driver_t;

typedef struct {

    list_head DriverHead;
    int DriverCount;

} SwsVN_Private_t;

static SwsVN_Private_t *_swsvn_priv = NULL;

int SwsVN_Initialize()
{
    SwsVN_Private_t *priv;

    if (_swsvn_priv)
        return 0;

    if (io_Init() == false)
        return -1;

    if ((priv = (SwsVN_Private_t *)malloc(sizeof(*priv))) == NULL)
    {
        io_Close();
        return -1;
    }
    memset(priv, 0, sizeof(*priv));
    INIT_LIST_HEAD(&priv->DriverHead);

    _swsvn_priv = priv;

    return 0;
}

void SwsVN_Shutdown()
{
    SwsVN_Driver_t *driver;
    list_head *list, *temp;

    if (_swsvn_priv == NULL)
        return;

    list_for_each_safe(list, temp, &_swsvn_priv->DriverHead)
    {
        driver = list_entry(list, SwsVN_Driver_t, node);
        list_del(&driver->node);
        free(driver);
    }
    free(_swsvn_priv);
    io_Close();
    _swsvn_priv = NULL;
}

void SwsVN_AddDriver(int nic, VirtualNetwork *network)
{
    SwsVN_Driver_t *driver;
    list_head *list;

    if (_swsvn_priv == NULL)
        return;
    if (network == NULL)
        return;

    list_for_each(list, &_swsvn_priv->DriverHead)
    {
        driver = list_entry(list, SwsVN_Driver_t, node);
        if (driver->nic == nic)
            return;
    }

    if ((driver = (SwsVN_Driver_t *)malloc(sizeof(*driver))) == NULL)
        return;
    memset(driver, 0, sizeof(*driver));
    driver->nic = nic;
    driver->network = network;
    INIT_LIST_HEAD(&driver->node);

    list_add_tail(&driver->node, &_swsvn_priv->DriverHead);
    _swsvn_priv->DriverCount++;
}

void SwsVN_DelDriver(int nic)
{
    SwsVN_Driver_t *driver;
    list_head *list, *temp;

    if (_swsvn_priv == NULL)
        return;
 
    list_for_each_safe(list, temp, &_swsvn_priv->DriverHead)
    {
        driver = list_entry(list, SwsVN_Driver_t, node);
        if (driver->nic == nic) {
            list_del(&driver->node);
            free(driver);
            break;
        }
    }
}

int SwsVN_GetDriverCount()
{
    if (_swsvn_priv == NULL)
        return 0;
    return _swsvn_priv->DriverCount;
}

int SwsVN_GetInterfaceNo(int idx)
{
    list_head *list;
    int n = 0;

    if (_swsvn_priv == NULL)
        return -1;
 
    list_for_each(list, &_swsvn_priv->DriverHead)
    {
        if (n++ == idx)
            return list_entry(list, SwsVN_Driver_t, node)->nic;
    }

    return -1;
}

SWS_VN_t SwsVN_GetVirtualNetwork(int nic)
{
    SwsVN_Driver_t *driver;
    list_head *list;

    if (_swsvn_priv == NULL)
        return NULL;

    list_for_each(list, &_swsvn_priv->DriverHead)
    {
        driver = list_entry(list, SwsVN_Driver_t, node);
        if (driver->nic == nic)
            return &driver->dummy;
    }

    return NULL;
}

void SwsVN_RegisterReceivedCB(SWS_VN_t handle, void (*callback)(void *), void *user_data)
{
    SwsVN_Driver_t *driver;

    if (handle == NULL)
        return;

    driver = container_of(handle, SwsVN_Driver_t, dummy);
    driver->network->attachReceivedCallback(callback, user_data);
}

void SwsVN_RegisterSendCompletedCB(SWS_VN_t handle, void (*callback)(void *), void *user_data)
{
    SwsVN_Driver_t *driver;

    if (handle == NULL)
        return;

    driver = container_of(handle, SwsVN_Driver_t, dummy);
    driver->network->attachSendCompletedCallback(callback, user_data);
}

void SwsVN_SetMacAddress(SWS_VN_t handle, uint8_t *mac_address)
{
    SwsVN_Driver_t *driver;

    if (handle == NULL)
        return;

    driver = container_of(handle, SwsVN_Driver_t, dummy);
    driver->network->setMACAddress(mac_address);
}

void SwsVN_GetMacAddress(SWS_VN_t handle, uint8_t *mac_address)
{
    SwsVN_Driver_t *driver;

    if (handle == NULL)
        return;

    driver = container_of(handle, SwsVN_Driver_t, dummy);
    driver->network->getMACAddress(mac_address);
}

SWS_VN_FRAME_t SwsVN_RequestPacket(SWS_VN_t handle)
{
    SwsVN_Driver_t *driver;

    if (handle == NULL)
        return NULL;

    driver = container_of(handle, SwsVN_Driver_t, dummy);

    return driver->network->requestFrame();
}

void SwsVN_SendPacket(SWS_VN_t handle, SWS_VN_FRAME_t frame, size_t size)
{
    SwsVN_Driver_t *driver;

    if (handle == NULL)
        return;

    driver = container_of(handle, SwsVN_Driver_t, dummy);
    driver->network->sendFrame(frame, size);
}

SWS_VN_FRAME_t SwsVN_RecvPacket(SWS_VN_t handle, size_t *psize)
{
    SwsVN_Driver_t *driver;

    if (handle == NULL)
        return NULL;

    driver = container_of(handle, SwsVN_Driver_t, dummy);

    return driver->network->receiveFrame(psize);
}

void SwsVN_ReleasePacket(SWS_VN_t handle, SWS_VN_FRAME_t frame)
{
    SwsVN_Driver_t *driver;

    if (handle == NULL)
        return;

    driver = container_of(handle, SwsVN_Driver_t, dummy);
    driver->network->releaseFrame(frame);
}

int SwsVN_SetRxMode(SWS_VN_t handle, int mode)
{
    SwsVN_Driver_t *driver;

    if (handle == NULL)
        return -1;

    driver = container_of(handle, SwsVN_Driver_t, dummy);

    return driver->network->setMode(mode);
}

int SwsVN_SetMulticastList(SWS_VN_t handle, void *pData, size_t size)
{
    SwsVN_Driver_t *driver;

    if (handle == NULL)
        return -1;

    driver = container_of(handle, SwsVN_Driver_t, dummy);

    return driver->network->setMulticastList(pData, size);
}
