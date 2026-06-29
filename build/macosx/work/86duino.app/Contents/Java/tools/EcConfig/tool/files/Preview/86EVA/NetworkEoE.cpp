#include "NetworkEoE.h"
#include "list.h"
#include "pt.h"
#include "io.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MAX_FRAMES      (256)
#define MAX_FRAME_SIZE  (1536)

struct NetworkFrame_t
{
    list_head node;
    size_t size;
    uint8_t data[MAX_FRAME_SIZE];
};

struct NetworkTransceiver_t
{
    NetworkFrame_t pool[MAX_FRAMES];
    NetworkFrame_t *frame;
    list_head free;
    list_head head;
    struct pt pt;
};

struct NetworkEoE_t
{
    NetworkTransceiver_t tx;
    NetworkTransceiver_t rx;
    uint32_t inProgress:1;
};

NetworkEoE::NetworkEoE(EthercatDevice &device, uint8_t port)
{
    this->_privateData = NULL;
    this->_device = &device;
    this->_master = NULL;
    this->_slaveNo = 0;
    this->_port = port;
}

NetworkEoE::NetworkEoE(EthercatMaster &master, uint16_t slave_no, uint8_t port)
{
    this->_privateData = NULL;
    this->_device = NULL;
    this->_master = &master;
    this->_slaveNo = slave_no;
    this->_port = port;
}

NetworkEoE::~NetworkEoE()
{
    end();
}

int NetworkEoE::begin()
{
    NetworkEoE_t *network;

    if (this->_privateData)
        return  0;
    if (io_Init() == false)
        return -1;
    if ((network = (NetworkEoE_t *)malloc(sizeof(NetworkEoE_t))) == NULL) {
        io_Close();
        return -1;
    }
    memset(network, 0, sizeof(NetworkEoE_t));
    PT_INIT(&network->tx.pt);
    PT_INIT(&network->rx.pt);
    INIT_LIST_HEAD(&network->tx.free);
    INIT_LIST_HEAD(&network->tx.head);
    INIT_LIST_HEAD(&network->rx.free);
    INIT_LIST_HEAD(&network->rx.head);
    for (int i = 0; i < MAX_FRAMES; i++) {
        INIT_LIST_HEAD(&network->tx.pool[i].node);
        list_add_tail(&network->tx.pool[i].node, &network->tx.free);
    }
    for (int i = 0; i < MAX_FRAMES; i++) {
        INIT_LIST_HEAD(&network->rx.pool[i].node);
        list_add_tail(&network->rx.pool[i].node, &network->rx.free);
    }
    this->_privateData = network;

    return 0;
}

void NetworkEoE::end()
{
    NetworkEoE_t *network = (NetworkEoE_t *)this->_privateData;

    if (network == NULL)
        return;

    if (this->_device) {
        this->_device->eoeRead_End();
        this->_device->eoeWrite_End();
    } else {
        this->_master->eoeRead_End(this->_slaveNo);
        this->_master->eoeWrite_End(this->_slaveNo);
    }
    this->_privateData = NULL;
    free(network);
    io_Close();
}

NetIfaceType NetworkEoE::getType() const
{
    return NIF_TYPE_EOE;
}

int NetworkEoE::processRxFrame()
{
    NetworkEoE_t *network = (NetworkEoE_t *)this->_privateData;
    int status;

    PT_BEGIN(&network->rx.pt);
    PT_WAIT_UNTIL(&network->rx.pt, !list_empty(&network->rx.free));
    network->rx.frame = list_first_entry(&network->rx.free, NetworkFrame_t, node);
    network->rx.frame->size = 0;
    if (this->_device) {
        PT_WAIT_UNTIL(&network->rx.pt, this->_device->eoeRead_Begin(this->_port, network->rx.frame->data) >= 0);
        PT_WAIT_UNTIL(&network->rx.pt, (status = this->_device->eoeRead_Status()) != ECAT_EOE_STATUS_BUSY);
        if (status == ECAT_EOE_STATUS_SUCCESS)
            network->rx.frame->size = this->_device->eoeRead_ReadLength();
        this->_device->eoeRead_End();
    } else {
        PT_WAIT_UNTIL(&network->rx.pt, this->_master->eoeRead_Begin(this->_slaveNo, this->_port, network->rx.frame->data) >= 0);
        PT_WAIT_UNTIL(&network->rx.pt, (status = this->_master->eoeRead_Status(this->_slaveNo)) != ECAT_EOE_STATUS_BUSY);
        if (status == ECAT_EOE_STATUS_SUCCESS)
            network->rx.frame->size = this->_master->eoeRead_ReadLength(this->_slaveNo);
        this->_master->eoeRead_End(this->_slaveNo);
    }
    if (this->_promiscuous == false && network->rx.frame->size > sizeof(this->_macAddress) &&
       (network->rx.frame->data[0] != 0xFF || network->rx.frame->data[1] != 0xFF ||
        network->rx.frame->data[2] != 0xFF || network->rx.frame->data[3] != 0xFF ||
        network->rx.frame->data[4] != 0xFF || network->rx.frame->data[5] != 0xFF) &&
       (network->rx.frame->data[0] != this->_macAddress[0] || network->rx.frame->data[1] != this->_macAddress[1] ||
        network->rx.frame->data[2] != this->_macAddress[2] || network->rx.frame->data[3] != this->_macAddress[3] ||
        network->rx.frame->data[4] != this->_macAddress[4] || network->rx.frame->data[5] != this->_macAddress[5]))
        network->rx.frame->size = 0;
    if (network->rx.frame->size > 0) {
        list_move_tail(&network->rx.frame->node, &network->rx.head);
        if (this->_receivedCallback)
            this->_receivedCallback(this->_receivedData);
    }
    PT_END(&network->rx.pt);
}

int NetworkEoE::processTxFrame()
{
    NetworkEoE_t *network = (NetworkEoE_t *)this->_privateData;

    PT_BEGIN(&network->tx.pt);
    PT_WAIT_UNTIL(&network->tx.pt, !list_empty(&network->tx.head));
    network->tx.frame = list_first_entry(&network->tx.head, NetworkFrame_t, node);
    if (this->_device) {
        PT_WAIT_UNTIL(&network->tx.pt, this->_device->eoeWrite_Begin(this->_port, network->tx.frame->data, network->tx.frame->size) >= 0);
        PT_WAIT_UNTIL(&network->tx.pt, this->_device->eoeWrite_Status() != ECAT_EOE_STATUS_BUSY);
        this->_device->eoeWrite_End();
    } else {
        PT_WAIT_UNTIL(&network->tx.pt, this->_master->eoeWrite_Begin(this->_slaveNo, this->_port, network->tx.frame->data, network->tx.frame->size) >= 0);
        PT_WAIT_UNTIL(&network->tx.pt, this->_master->eoeWrite_Status(this->_slaveNo) != ECAT_EOE_STATUS_BUSY);
        this->_master->eoeWrite_End(this->_slaveNo);
    }
    list_move_tail(&network->tx.frame->node, &network->tx.free);
    if (this->_sendCompletedCallback)
        this->_sendCompletedCallback(this->_sendCompletedData, 0);
    PT_END(&network->tx.pt);
}

void NetworkEoE::process()
{
    NetworkEoE_t *network = (NetworkEoE_t *)this->_privateData;

    if (network == NULL)
        return;
    if (network->inProgress)
        return;

    network->inProgress = 1;
    if (this->_device == NULL) {
        this->_master->update();
    } else
    if (this->_device->getMaster()) {
        this->_device->getMaster()->update();
    }
    processRxFrame();
    processTxFrame();
    network->inProgress = 0;
}

EthernetFrame NetworkEoE::requestFrame()
{
    NetworkEoE_t *network = (NetworkEoE_t *)this->_privateData;
    NetworkFrame_t *netframe = NULL;

    if (network == NULL)
        return NULL;

    io_DisableINT();
    if (list_empty(&network->tx.free) == 0)
    {
        netframe = list_first_entry(&network->tx.free, NetworkFrame_t, node);
        list_del(&netframe->node);
    }
    io_RestoreINT();

    return (netframe == NULL) ? (NULL) : (&netframe->data);
}

void NetworkEoE::sendFrame(EthernetFrame frame, size_t size)
{
    NetworkEoE_t *network = (NetworkEoE_t *)this->_privateData;
    NetworkFrame_t *netframe;

    if (network == NULL)
        return;
    if (frame == NULL)
        return;

    netframe = container_of(frame, NetworkFrame_t, data);
    netframe->size = size;

    io_DisableINT();
    list_add_tail(&netframe->node, &network->tx.head);
    io_RestoreINT();
}

EthernetFrame NetworkEoE::receiveFrame(size_t *psize)
{
    NetworkEoE_t *network = (NetworkEoE_t *)this->_privateData;
    NetworkFrame_t *netframe = NULL;

    if (network == NULL)
        return NULL;

    io_DisableINT();
    if (list_empty(&network->rx.head) == 0)
    {
        netframe = list_first_entry(&network->rx.head, NetworkFrame_t, node);
        psize[0] = netframe->size;
        list_del(&netframe->node);
    }
    io_RestoreINT();

    return (netframe == NULL) ? (NULL) : (&netframe->data);
}

void NetworkEoE::releaseFrame(EthernetFrame frame)
{
    NetworkEoE_t *network = (NetworkEoE_t *)this->_privateData;
    NetworkFrame_t *netframe;

    if (network == NULL)
        return;
    if (frame == NULL)
        return;

    netframe = container_of(frame, NetworkFrame_t, data);

    io_DisableINT();
    list_add_tail(&netframe->node, &network->rx.free);
    io_RestoreINT();
}
