#ifndef __IPCFIFO_H
#define __IPCFIFO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IPCFIFO_REAL_TIME_VER

void ipcfifo_SETTINGS_receive_timtout_ms(int timeout);
void ipcfifo_SETTINGS_transmit_timtout_ms(int timeout);
void ipcfifo_SETTINGS_max_pool_size(uint32_t size);

#ifdef IPCFIFO_REAL_TIME_VER

    /* IPC-FIFO Polling Mode Enable */
    // #define IPCFIFO_POLLING_MODE_ENABLE
    
    #ifdef IPCFIFO_POLLING_MODE_ENABLE
    
        /* IPC-FIFO Polling Mode DPC Enable */
        // #define IPCFIFO_DPC_ENABLE
    
        /* Polling function when using polling mode */
        void ipcfifo_process();
        /* Set Polling function ipcfifo_process() deadline (microseconds) */
        void ipcfifo_set_deadline(unsigned long deadline);
        /* Get deadline (microseconds) */
        unsigned long ipcfifo_get_deadline();
        /* Clear Polling function ipcfifo_process() deadline (microseconds) */
        void ipcfifo_clear_deadline();
        /* To check deadline is expired or not */
        bool ipcfifo_deadline_expired();
        /* To check deadline is expired or not */
        bool ipcfifo_assigned_deadline_expired(unsigned long deadline);
        
    #endif
    
    /* Check First Frame List Empty */
    bool ipcfifo_first_frame_empty();
    
#else
    
    void ipcfifo_SETTINGS_overflow_behavior(int option);
    #define IPCFIFO_NORMAL          (0) // If user does not receive until buffer is fulled,
                                        // it will discard following data.
    #define IPCFIFO_DATA_NOT_LOSS   (1) // If master and slave system use write blocking function to send 
                                        // large data more than Software RX Buffer size, they will wait to
                                        // send data for each other.

#endif

/* IPC-FIFO Library Initialize */
int ipcfifo_initialize();
/* IPC-FIFO Library Deinitialize */
void ipcfifo_shutdown();

/* The data in the Transmit Buffer had been sended. */
bool ipcfifo_check_send_completed();
/* Receive Buffer has some data. */
bool ipcfifo_check_data_ready();
/* Check the number of data in Receive Buffer is greater than the specifed size. */
bool ipcfifo_check_data_size_ready(int size);

/* Receive data from Receive Buffer. */
int ipcfifo_recv_nowait(uint8_t *buf, uint32_t len);
int ipcfifo_recv(uint8_t *buf, uint32_t len);
/* Receive one byte from Receive Buffer. */
static inline int ipcfifo_read_byte(uint8_t *pval) {
    if (ipcfifo_check_data_size_ready(1))
        return ipcfifo_recv_nowait(pval, 1);
    return 0;
}
/* Receive one word (two bytes) from Receive Buffer. */
static inline int ipcfifo_read_word(uint16_t *pval) {
    if (ipcfifo_check_data_size_ready(2))
        return ipcfifo_recv_nowait((uint8_t*)pval, 2);
    return 0;
}
/* Receive one double-word (four bytes) from Receive Buffer. */
static inline int ipcfifo_read_dword(uint32_t *pval) {
    if (ipcfifo_check_data_size_ready(4))
        return ipcfifo_recv_nowait((uint8_t*)pval, 4);
    return 0;
}

/* Send data to Transmit Buffer. */
int ipcfifo_send_nowait(uint8_t *buf, uint32_t len);
int ipcfifo_send(uint8_t *buf, uint32_t len);
/* Send one byte to Transmit Buffer. */
static inline int ipcfifo_write_byte(uint8_t val) {
    return ipcfifo_send(&val, 1);
}
/* Send one word (two bytes) to Transmit Buffer. */
static inline int ipcfifo_write_word(uint16_t val) {
    return ipcfifo_send((uint8_t*)&val, 2);
}
/* Send one double-word (four bytes) to Transmit Buffer. */
static inline int ipcfifo_write_dword(uint32_t val) {
    return ipcfifo_send((uint8_t*)&val, 4);
}

#ifdef __cplusplus
}
#endif

#endif