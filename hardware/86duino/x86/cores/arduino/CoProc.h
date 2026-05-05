#ifndef __COPROC_H
#define __COPROC_H

#include "CoProcDeviceList.h"
#include <stdint.h>
#include <stdlib.h>

#define __COPROC_LIB
#ifdef _86DUINO
    #undef __COPROC_LIB
    #include "v86clock.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* CoProc State. */
#define COPROC_OFFLINE      (0x00)
#define COPROC_CONFIGURED   (0x01)
#define COPROC_READY        (0x02)
#define COPROC_RUNNING      (0x03)

/* CoProc Error Code. */
#define CPERR_NONE                      (0)
#define CPERR_SEND_EVENT_FAIL           (-1)
#define CPERR_OFFLINE_IO_FAIL           (-101)
#define CPERR_OFFLINE_ALLOC_RXBUF_FAIL  (-102)
#define CPERR_OFFLINE_ALLOC_TXBUF_FAIL  (-103)
#define CPERR_OFFLINE_IPCNET_FAIL       (-104)
#define CPERR_OFFLINE_SOCKET_FAIL       (-105)
#define CPERR_READY_REMOTE_DISCONNECT   (-301)

#if defined(__COPROC_SERVER)

/* CoProc Error Code (Server). */
#define CPERR_TOO_MUCH_ENTRIES              (-2)
#define CPERR_ALLOC_RES_FAIL                (-3)
#define CPERR_OFFLINE_VDPMEM_FAIL           (-106)
#define CPERR_OFFLINE_BIND_FAIL             (-107)
#define CPERR_OFFLINE_LISTEN_FAIL           (-108)
#define CPERR_CONFIGURED_TIMEOUT_ACCEPT     (-201)
#define CPERR_CONFIGURED_TIMEOUT_SEND_VER   (-202)
#define CPERR_READY_TIMEOUT_IDLE            (-302)
#define CPERR_READY_TIMEOUT_SEND_RES        (-303)

#elif defined(__COPROC_CLIENT)

/* CoProc Error Code (Client). */
#define CPERR_NO_SUCH_DEVICE_ENTRY              (-2)
#define CPERR_SUCH_DEVICE_IN_USED               (-3)
#define CPERR_SUCH_DEVICE_NO_RTEVT              (-4)
#define CPERR_CONFIGURED_TIMEOUT_CONNECT        (-201)
#define CPERR_CONFIGURED_TIMEOUT_RECV_VER       (-202)
#define CPERR_CONFIGURED_REMOTE_DISCONNECT      (-203)
#define CPERR_CONFIGURED_VERSION_HEADER_FAIL    (-204)
#define CPERR_CONFIGURED_VERSION_ID_FAIL        (-205)
#define CPERR_CONFIGURED_VERSION_NUMBER_FAIL    (-206)
#define CPERR_CONFIGURED_VERSION_CHECKSUM_FAIL  (-207)
#define CPERR_CONFIGURED_VDPMEM_FAIL            (-208)
#define CPERR_READY_TIMEOUT_SEND_CMD            (-302)
#define CPERR_READY_NO_ENTRY                    (-303)
#define CPERR_READY_TOO_MUCH_ENTRIES            (-304)
#define CPERR_READY_ALLOC_RES_FAIL              (-305)
#define CPERR_READY_TIMEOUT_RECV_RES            (-306)
#define CPERR_READY_TIMEOUT_WAIT_CLOSE          (-307)
   
#endif

/* CoProc Class */
class CoProcClass {

public:
    /* Class Constructor */
    CoProcClass();
    /* Class Destructor */
    ~CoProcClass();
    
    /* Initialize (Blocking). */
    int initialize();
    /* Initialize (Non-Blocking). */
    int runInitialize();

    /* Close CoProc. */
    void shutdown();

    /* CoProc User Event Callback. */
    void (*userEvent)(uint32_t);
    /* Send User Event. */
    int sendEvent(uint32_t event);
    
#if defined(__COPROC_SERVER)

    /* Register Resource Table Initialize Callback. */
    void attachCallback_ResTableInit(void (*callback)(void));
    /* Register Device Initialize Callback. */
    void attachCallback_DeviceInit(void (*callback)(void));
    
    /* Add Resource Table Entry. Invoke in Resource Table Initialize Callback. */
    int addResTableEntry(uint16_t devID, uint16_t ctrlPort);
    /* Add Resource Table Entry. Invoke in Resource Table Initialize Callback. */
    int addResTableEntry(uint16_t devID, uint16_t ctrlPort, uint8_t evtID);
    
#elif defined(__COPROC_CLIENT)

    /* Get Device Count. */
    int getDeviceCount();
    /* Get Device Control Port. */
    int getDeviceCtrlPort(uint16_t devID);
    /* Get Device Real Time Event ID. */
    int getDeviceRtEvtID(uint16_t devID);

    /* Request Device. Return Control Port or Error. */
    int requestDevice(uint16_t devID);
    /* Release Device. */
    int releaseDevice(uint16_t devID);
    
#endif
    
private:
    
    /* CoProc Version. */
    uint32_t version;
    /* CoProc State. */
    int state;
    /* Initial Socket. */
    int initSock;
    /* Resource Table. */
    void *resTable;
    /* Resource Table Entry Count. */
    uint32_t resEntryCount;
    /* RX Buffer for Initial Socket. */
    uint8_t *rxBuf;
    int rxCur;
    int rxLen;
    /* TX Buffer for Initial Socket. */
    uint8_t *txBuf;
    int txCur;
    int txLen;
    /* Used for Time Out. */
    unsigned long preTime;
    /* Indicate IO Library had been initialized. */
    int ioInited;
    /* Indicate IPC-NET had been initialized. */
    int ipcnetInited;
    /* Indicate VDPMEM had been initialized. */
    int vdpmInited;
    
#if defined(__COPROC_SERVER)
    /* CoProc Server Listen Socket. (Port: COPROC_INIT_PORT) */
    int listenSock;
    /* Resource Table Initialize Callback. */
    void (*resourceTableInitialize)(void);
    /* Device Initialize Callback. */
    void (*deviceInitialize)(void);
#endif

};

extern CoProcClass CoProc;

#if defined(__COPROC_SERVER)

/* Resource Table Initialize Function. */
extern void resTableInit() __attribute__((weak));
/* Device Initialize Function. */
extern void deviceInit() __attribute__((weak));

#endif

#ifdef __cplusplus
}
#endif

#endif
