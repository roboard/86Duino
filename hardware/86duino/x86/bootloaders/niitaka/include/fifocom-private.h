#ifndef __FIFOCOM_PRIVATE_H
#define __FIFOCOM_PRIVATE_H

#if defined(linux) && defined(__GNUC__) || defined(DJGPP)
    #ifndef PACKED
        #define PACKED __attribute__((__packed__))
    #endif
#else
    #error Not supported platform.
#endif


struct frame_pool;
struct fcom_frame {
    
    struct fcom_frame *next;
    struct frame_pool *pool;

    uint8_t size;
    uint8_t type;
    uint8_t data[0];

} PACKED;

/* Frame Types */
#define FCOM_TYPE_NONE  (0)
#define FCOM_TYPE_HEAD  (1)
#define FCOM_TYPE_DATA  (2)
#define FCOM_TYPE_TAIL  (3)

/* FIFO-COM Frame Structure Size */
#define FCOM_FRAME_STRUCT_SIZE  (64)
/* FIFO-COM Frame Maximum Data Size */
#define FIFOCOM_FRAME_SIZE      (54)

/* Critical Section Lock/Unlock */
#define ENTER_CRITICAL_SECTION()    fcom_lock()
#define EXIT_CRITICAL_SECTION()     fcom_unlock()
void fcom_lock();
void fcom_unlock();

/* Indicate FIFO-COM Maximum Frame Memory Size */
extern unsigned long fcom_pool_max_size;
/* Indicate FIFO-COM Current Frame Memory Size */
extern unsigned long fcom_pool_size;
/* Indicate FIFO-COM Maximum Frame Count */
extern unsigned long fcom_max_frame_num;
/* Indicate FIFO-COM Current Frame Count */
extern unsigned long fcom_use_frame_num;
/* Request One Frame */
struct fcom_frame *fcom_request_frame();
/* Release One Frame */
void fcom_release_frame(struct fcom_frame *frame);

/* Receive One Frame */
struct fcom_frame *fcom_recv_frame();
/* Send One Frame */
int fcom_send_frame(struct fcom_frame *frame);
/* Send One Frame First */
int fcom_send_frame_first(struct fcom_frame *frame);
/* Start Transmission */
void fcom_start_transfer();

/* FIFO-COM flush rx */
void fcom_rx_flush();
/* FIFO-COM flush tx */
void fcom_tx_flush();
/* FIFO-COM wait tx data flushed */
void fcom_wait_tx_flush();

#endif