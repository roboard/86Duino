#include "fifocom.h"
#include "fifocom-private.h"

#include <stdlib.h>
#include <string.h>
#include "io.h"
#include "io_atom.h"
#include "irq.h"

#define POOL_MAXIMUM_SIZE_ENABLE

/* FIFO-COM Configuration */
static int fcom_conf = FCOM_DATA_NOT_LOSS;

/* Indicate FIFO-COM in Interrupt Space or User Space */
#define FIFOCOM_IN_USER_SPACE   (0)
#define FIFOCOM_IN_INTR_SPACE   (1)
static volatile int fcom_status = FIFOCOM_IN_USER_SPACE;

/* FIFO-COM User Read/Write API Timeout */
#define FIFOCOM_RX_TIMEOUT  (1000UL)
#define FIFOCOM_TX_TIMEOUT  (1000UL)
static unsigned long fcom_rx_timeout = FIFOCOM_RX_TIMEOUT;
static unsigned long fcom_tx_timeout = FIFOCOM_TX_TIMEOUT;

/* FIFO-COM RX Frame Timeout for ISR */
#define FIFOCOM_RX_FRAME_TIMEOUT_US (1000000)
static volatile unsigned long fcom_rx_preclocks = 0;
static unsigned long fcom_frame_timeout = 0;

/* FIFO-COM Frame Header */
#define FIFOCOM_FRAME_HEADER    (0xA5)

/* FIFO-COM Control Register Temporary */
static volatile unsigned long fcom_ctrl = 0;
/* FIFO-COM RX FIFO Not Empty Flag */
#define FIFOCOM_RX_EMPTY        (0)
#define FIFOCOM_RX_NOT_EMPTY    (1)
static volatile int fcom_rx_status = FIFOCOM_RX_EMPTY;
/* FIFO-COM TX Status Not Clear Flag */
#define FIFOCOM_TX_CLEAR        (0)
#define FIFOCOM_TX_NOT_CLEAR    (1)
static volatile int fcom_tx_status = FIFOCOM_TX_CLEAR;

/* FIFO-COM Initialze Variables */
static int fcom_initialize = 0;
static unsigned int fcom_irq = 0;
static unsigned long fcom_base = 0;
static void *fcom_mmio = NULL;

/* FIFO-COM Interrupt Routing Table */
static const unsigned int fcom_route_irq[] = {
    0, 9, 3, 10, 4, 5, 7, 6, 1, 11, 0, 12, 0, 14, 0, 15
};

/*****************************************************************************
 *                                                                           *
 *                   FIFO-COM Lock & Unlock Functions                        *
 *                                                                           *
 *****************************************************************************/
ATOMIC_INT_t fcom_lock_cnt = ATOMIC_INT_INIT(0);

/* Use disable FIFO-COM interrupt as Critical Section Lock */
void fcom_lock()
{
    if (ATOMIC_INT_TEST(&fcom_lock_cnt, 0)) {
        io_Out32(fcom_mmio, 0x00, 0x00);
        io_In32(fcom_mmio, 0x00); // while (io_In32(fcom_mmio, 0x00) != 0);
    }
    ATOMIC_INT_INC(&fcom_lock_cnt);
    ATOMIC_MEM_BARRIER();
}

/* Use enable FIFO-COM interrupt as Critical Section Unlock */
void fcom_unlock()
{
    ATOMIC_MEM_BARRIER();
    if (!ATOMIC_INT_TEST(&fcom_lock_cnt, 0)) {
        ATOMIC_INT_DEC(&fcom_lock_cnt);
        if (ATOMIC_INT_TEST(&fcom_lock_cnt, 0))
            io_Out32(fcom_mmio, 0x00, fcom_ctrl);
    }
}

/* Critical Section Lock/Unlock: This will disable all interrupt. */
#define ENTER_CRITICAL_SECTION_FAST()   io_DisableINT()
#define EXIT_CRITICAL_SECTION_FAST()    io_RestoreINT()

/*****************************************************************************
 *                                                                           *
 *               FIFO-COM Frame Pool Functions and Variables                 *
 *                                                                           *
 *****************************************************************************/
#define FCOM_DEFAULT_POOL_FRAME_NR  (1024 * 16) // FIFO-COM Static Pool Size
#define FCOM_EXTRA_POOL_FRAME_NR    (1024)      // FIFO-COM Extra Pool Size

/* Frame Pool Structure */
struct frame_pool {
    uint32_t recycle_cnt;
};
static struct frame_pool *fcom_default_pool = NULL;
static struct fcom_frame *fcom_frame_head = NULL;
unsigned long fcom_pool_max_size = 0;
unsigned long fcom_pool_size = 0;
unsigned long fcom_max_frame_num = 0;
unsigned long fcom_use_frame_num = 0;

/* Allocate New Frame Pool */
inline struct fcom_frame *alloc_frame_pool(unsigned int size)
{
    unsigned long mem_size;
    struct frame_pool *pool;
    struct fcom_frame *frame, *f;
    
    if (size <= 1)
        return NULL;
    
    mem_size = FCOM_FRAME_STRUCT_SIZE * size;
    
#ifdef POOL_MAXIMUM_SIZE_ENABLE
    if (fcom_pool_max_size && fcom_pool_size >= fcom_pool_max_size)
        return NULL;
#endif

    pool = (struct frame_pool *)malloc(mem_size);
    if (pool == NULL)
        return NULL;
    
    pool->recycle_cnt = 0;
    size--;
    
#ifdef POOL_MAXIMUM_SIZE_ENABLE
    fcom_pool_size += mem_size;
#endif

    frame = (struct fcom_frame *)((uint8_t *)pool + FCOM_FRAME_STRUCT_SIZE);

    f = frame;
    while (1) {
        f->pool = pool;
        f->size = 0;
        f->type = 0;
        f->next = (struct fcom_frame *)((uint8_t *)f + FCOM_FRAME_STRUCT_SIZE);
        if (--size == 0)
            break;
        f = f->next;
    }
    f->next = NULL;
    
    return frame;
}

/* Free Frame Pool */
inline void free_frame_pool(struct frame_pool *pool)
{
    if (pool) {
#ifdef POOL_MAXIMUM_SIZE_ENABLE
        if (pool == fcom_default_pool)
            fcom_pool_size -= FCOM_FRAME_STRUCT_SIZE * FCOM_DEFAULT_POOL_FRAME_NR;
        else
            fcom_pool_size -= FCOM_FRAME_STRUCT_SIZE * FCOM_EXTRA_POOL_FRAME_NR;
#endif
        free(pool);
    }
}

/* Request one frame from pools */
struct fcom_frame *fcom_request_frame()
{
    struct fcom_frame *frame = NULL;

    /* If no frame to request, then FIFO-COM in User Space, allocate new frame pool. */
    if (fcom_frame_head == NULL && fcom_status == FIFOCOM_IN_USER_SPACE)
        fcom_frame_head = alloc_frame_pool(FCOM_EXTRA_POOL_FRAME_NR);
    
    if (fcom_frame_head) {
        frame = fcom_frame_head;
        fcom_frame_head = frame->next;
        frame->next = NULL;
        frame->type = FCOM_TYPE_NONE;
        fcom_use_frame_num++;
    }
    
    return frame;
}

/* Release one frame */
void fcom_release_frame(struct fcom_frame *frame)
{
    struct fcom_frame *f;
    uint32_t count;
    
    if (frame == NULL)
        return;

    fcom_use_frame_num--;
    if (frame->pool == fcom_default_pool || fcom_status == FIFOCOM_IN_INTR_SPACE) {
        frame->next = fcom_frame_head;
        fcom_frame_head = frame;
    } else {
        frame->pool->recycle_cnt++;
        fcom_max_frame_num--;
        /* If all of frame are recycled, free frame pool. */
        if (frame->pool->recycle_cnt == FCOM_EXTRA_POOL_FRAME_NR - 1) {
            free_frame_pool(frame->pool);
            fcom_max_frame_num += FCOM_EXTRA_POOL_FRAME_NR - 1;
        }
    }
}

/*****************************************************************************
 *                                                                           *
 *                FIFO-COM RX Handle Functions and Variables                 *
 *                                                                           *
 *****************************************************************************/
/* RX Frame Linker List */
static volatile struct fcom_frame *rx_frame_head = NULL;
static volatile struct fcom_frame *rx_frame_tail = NULL;
/* Temporary RX Frame for ISR */
static struct fcom_frame *rx_frame_temp = NULL;
/* Temporary RX Frame for UAPI */
static struct fcom_frame *rx_frame_peek = NULL;
/* Indicate the end pointer of RX frame data. */
static volatile uint32_t *rx_frame_end = NULL;
/* Indicate the current pointer of RX frame data. */
static volatile uint32_t *rx_frame_now = NULL;

/* RX Handle function, it will be called by ISR or UAPI. */
inline void rx_handle_irq()
{
    uint32_t data;
    uint8_t *p;
    int len, count, n;
    
    /* If timeout and RX Frame receives data not finished, discard this RX Frame. */
    if (rx_frame_temp) {
        if (timer_GetClocks() - fcom_rx_preclocks > fcom_frame_timeout) {
            fcom_release_frame(rx_frame_temp);
            rx_frame_temp = NULL;
        }
    }
    
    fcom_rx_status = FIFOCOM_RX_EMPTY;
    
    do {
        len = (io_In32(fcom_mmio, 0x04) >> 16) & 0x1F; // Get RX FIFO Size
        if (len <= 0)
            break;
        
        if (rx_frame_temp == NULL) {
            rx_frame_temp = fcom_request_frame(); // Request one frame
            if (rx_frame_temp == NULL) {
                if (fcom_conf == FCOM_DATA_NOT_LOSS) {
                    fcom_rx_status = FIFOCOM_RX_NOT_EMPTY;
                    break;
                }
                io_In32(fcom_mmio, 0x0C);
            } else {
                data = io_In32(fcom_mmio, 0x0C);
                p = (uint8_t*)&data;
                if (p[0] != FIFOCOM_FRAME_HEADER ||
                    p[1]  > FIFOCOM_FRAME_SIZE   || p[1] == 0) {
                    fcom_release_frame(rx_frame_temp);
                    rx_frame_temp = NULL;
                    continue;
                }
                
                rx_frame_temp->size = p[1];
                
                rx_frame_end = (uint32_t*)((uint8_t*)rx_frame_temp + FCOM_FRAME_STRUCT_SIZE);
                if (rx_frame_temp->size <= 2) {
                    rx_frame_now = rx_frame_end - 1;
                    *rx_frame_now++ = data;
                } else
                    rx_frame_now = rx_frame_end - ((rx_frame_temp->size + 3) >> 2);
            }
        } else {
            count = rx_frame_end - rx_frame_now;
            if (count > len)
                count = len;
            
            /* This loop unrolling method references from
               URL: https://en.wikipedia.org/wiki/Duff's_device. */
            n = (count + 7) >> 3;
            switch (count & 0x07) {
            case 0: do { *rx_frame_now++ = io_In32(fcom_mmio, 0x0C);
            case 7:      *rx_frame_now++ = io_In32(fcom_mmio, 0x0C);
            case 6:      *rx_frame_now++ = io_In32(fcom_mmio, 0x0C);
            case 5:      *rx_frame_now++ = io_In32(fcom_mmio, 0x0C);
            case 4:      *rx_frame_now++ = io_In32(fcom_mmio, 0x0C);
            case 3:      *rx_frame_now++ = io_In32(fcom_mmio, 0x0C);
            case 2:      *rx_frame_now++ = io_In32(fcom_mmio, 0x0C);
            case 1:      *rx_frame_now++ = io_In32(fcom_mmio, 0x0C);
                    } while (--n > 0);
            }
        }
        
        if (rx_frame_temp) {
            if (rx_frame_now == rx_frame_end) {
                rx_frame_temp->next = rx_frame_tail->next;
                rx_frame_tail->next = rx_frame_temp;
                rx_frame_tail = rx_frame_temp;
                rx_frame_temp = NULL;
            } else
                fcom_rx_preclocks = timer_GetClocks(); // Update receive timeout clocks
        }
        
    } while (1);
}

/*****************************************************************************
 *                                                                           *
 *                FIFO-COM TX Handle Functions and Variables                 *
 *                                                                           *
 *****************************************************************************/
/* TX Frame Linker List */
static volatile struct fcom_frame *tx_frame_head = NULL;
static volatile struct fcom_frame *tx_frame_tail = NULL;
/* TX Frame Wait Queue Linker List */
static volatile struct fcom_frame *tx_frame_wait_head = NULL;
static volatile struct fcom_frame *tx_frame_wait_tail = NULL;
static bool tx_frame_wait_in_preparation = false;
/* Temporary TX Frame Head for Insert Wait Queue */
static struct fcom_frame *tx_frame_head_temp = NULL;
/* Temporary TX Frame for ISR */
static struct fcom_frame *tx_frame_temp = NULL;
/* Indicate the end pointer of TX frame data. */
static volatile uint32_t *tx_frame_end = NULL;
/* Indicate the current pointer of TX frame data. */
static volatile uint32_t *tx_frame_now = NULL;

/* TX Handle function, it will be called by ISR or UAPI. */
inline void tx_handle_irq()
{
    struct fcom_frame *temp, *tail = NULL;
    uint32_t data;
    uint8_t *p;
    int len, count, n;
    
    do {
        
        len = (io_In32(fcom_mmio, 0x04) >> 24) & 0x1F;  // Get FIFO-COM TX Free Size
        if (len <= 0)
            break;
        
        if (tx_frame_temp == NULL) {
            /* Insert TX First Wait Queue */
            if (tx_frame_wait_head &&
               (tx_frame_head->next->type == FCOM_TYPE_NONE ||
                tx_frame_head->next->type == FCOM_TYPE_HEAD)) {
                
                if (tx_frame_wait_head->type == FCOM_TYPE_NONE) {
                    temp = (struct fcom_frame*)tx_frame_wait_head;
                    while (temp->next) {
                        if (temp->next->type != FCOM_TYPE_NONE)
                            break;
                        temp = temp->next;
                    }
                    tail = temp;
                } else if (tx_frame_wait_head->type == FCOM_TYPE_HEAD) {
                    temp = tx_frame_wait_head->next;
                    while (temp) {
                        if (temp->type == FCOM_TYPE_TAIL)
                            tail = temp;
                        temp = temp->next;
                    }
                }
                
                if (tail) {
                    temp = tail->next;
                    tail->next = tx_frame_head_temp->next;
                    tx_frame_head_temp->next = (struct fcom_frame*)tx_frame_wait_head;
                    tx_frame_head_temp = tail;
                    if (tx_frame_tail == tx_frame_head)
                        tx_frame_tail = tx_frame_head_temp;
                    tx_frame_wait_head = temp;
                    if (tx_frame_wait_head == NULL)
                        tx_frame_wait_tail = NULL;
                }
            }

            tx_frame_temp = tx_frame_head->next;
            if (tx_frame_temp == tx_frame_head) {
                tx_frame_temp = NULL;
                break;
            }
            
            tx_frame_end = (uint32_t*)((uint8_t*)tx_frame_temp + FCOM_FRAME_STRUCT_SIZE);
            if (tx_frame_temp->size <= 2) {
                tx_frame_now = tx_frame_end - 1;
                data = *tx_frame_now++;
            } else
                tx_frame_now = tx_frame_end - ((tx_frame_temp->size + 3) >> 2);
            
            p = (uint8_t*)&data;
            p[0] = FIFOCOM_FRAME_HEADER;
            p[1] = tx_frame_temp->size;

            io_Out32(fcom_mmio, 0x08, data);
            fcom_tx_status = FIFOCOM_TX_NOT_CLEAR; // TX Empty Status must to be Clear
        } else {
            count = tx_frame_end - tx_frame_now;
            if (count > len)
                count = len;

            /* This loop unrolling method references from
               URL: https://en.wikipedia.org/wiki/Duff's_device. */
            n = (count + 7) >> 3;
            switch (count & 0x07) {
            case 0: do { io_Out32(fcom_mmio, 0x08, *tx_frame_now++);
            case 7:      io_Out32(fcom_mmio, 0x08, *tx_frame_now++);
            case 6:      io_Out32(fcom_mmio, 0x08, *tx_frame_now++);
            case 5:      io_Out32(fcom_mmio, 0x08, *tx_frame_now++);
            case 4:      io_Out32(fcom_mmio, 0x08, *tx_frame_now++);
            case 3:      io_Out32(fcom_mmio, 0x08, *tx_frame_now++);
            case 2:      io_Out32(fcom_mmio, 0x08, *tx_frame_now++);
            case 1:      io_Out32(fcom_mmio, 0x08, *tx_frame_now++);
                    } while (--n > 0);
            }
            
            fcom_tx_status = FIFOCOM_TX_NOT_CLEAR; // TX Empty Status must to be Clear
        }
        
        if (tx_frame_temp) {
            if (tx_frame_now == tx_frame_end) {
                tx_frame_head->next = tx_frame_temp->next;
                if (tx_frame_tail == tx_frame_temp)
                    tx_frame_tail = tx_frame_head;
                if (tx_frame_head_temp == tx_frame_temp)
                    tx_frame_head_temp = (struct fcom_frame*)tx_frame_head;
                fcom_release_frame(tx_frame_temp);
                tx_frame_temp = NULL;
            }
        }
        
    } while (1);
    
    if (fcom_ctrl & (1UL << 4)) {
        if (tx_frame_head == tx_frame_tail) {
            fcom_ctrl &= ~(1UL << 4); // If no data, disable TX Empty Interrupt.
        }
    } else {
        if (tx_frame_head != tx_frame_tail) {
            fcom_ctrl |= (1UL << 4); // If it has data to transmit, enable TX Empty Interrupt.
        }
    }
}

/*****************************************************************************
 *                                                                           *
 *                    FIFO-COM Interrupt Service Runtine                     *
 *                                                                           *
 *****************************************************************************/
static int FIFOCOM_ISR(int irq, void *data)
{
    unsigned long status;
    int ret = ISR_NONE;

    fcom_status = FIFOCOM_IN_INTR_SPACE; // FIFO-COM enter Interrupt Space

    status = io_In32(fcom_mmio, 0x04);
    io_Out32(fcom_mmio, 0x04, status);

    /* Receive data from RX FIFO */
    if (status & 1UL) {

        ret = ISR_HANDLED;
        
        rx_handle_irq();
        
    }

    /* Send data to TX FIFO */
    if (status & (1UL << 4)) {

        ret = ISR_HANDLED;

        fcom_tx_status = FIFOCOM_TX_CLEAR; // TX Empty Status was Cleared
        tx_handle_irq();
        
    }

    fcom_status = FIFOCOM_IN_USER_SPACE; // FIFO-COM exit Interrupt Space

    return ret;
}

/*****************************************************************************
 *                                                                           *
 *                    FIFO-COM User Application Interface                    *
 *                                                                           *
 *****************************************************************************/
/* FIFO-COM Initialize */
int fcom_init()
{
    int route;
    unsigned long temp;
    
    if (fcom_initialize != 0) {
        fcom_initialize++;
        return 0;
    }
    
    /* IO Library initialize */
    if (io_Init() == false)
        return -1;
    
    /* Check CPU ID */
    if (vx86_CpuID() != CPU_VORTEX86EX2M && vx86_CpuID() != CPU_VORTEX86EX2S) {
        io_Close();
        return -2;
    }
    
    /* Config RX Timeout */
    fcom_frame_timeout = FIFOCOM_RX_FRAME_TIMEOUT_US * vx86_CpuCLK();
    
    /* Get FIFO COM IRQ number */
    route = (sb1_Read(0xB4) >> 16) & 0x0F;
    if (fcom_route_irq[route] == 0) {
        io_Close();
        return -3;
    }
    fcom_irq = fcom_route_irq[route];
    
    /* Get FIFO COM base address */
    temp = nb_Read(0xBC);
    if (!(temp & 0x01) || !(temp & ~0x0FUL)) {
        io_Close();
        return -4;
    }
    fcom_base = temp & ~0x0FUL;
    
    /* Request memory space */
    fcom_mmio = io_Alloc(IO_USE_MMIO, fcom_base, 16);
    if (fcom_mmio == NULL) {
        io_Close();
        return -5;
    }
    
    /* Alloc Frame Pool */
    fcom_frame_head = alloc_frame_pool(FCOM_DEFAULT_POOL_FRAME_NR);
    if (fcom_frame_head == NULL) {
        io_Free(fcom_mmio);
        io_Close();
        return -6;
    }
    fcom_default_pool = fcom_frame_head->pool;
    
    rx_frame_head = fcom_request_frame();
    rx_frame_head->next = (struct fcom_frame*)rx_frame_head;
    rx_frame_tail = rx_frame_head;
    
    tx_frame_head = fcom_request_frame();
    tx_frame_head->next = (struct fcom_frame*)tx_frame_head;
    tx_frame_tail = tx_frame_head;
    tx_frame_head_temp = (struct fcom_frame*)tx_frame_head;
    
    /* IRQ Library initialize */
    if (irq_Init() == false) {
        free_frame_pool(fcom_default_pool);
        io_Free(fcom_mmio);
        io_Close();
        return -7;
    }
    
    /* RX FIFO Reset */
    io_Out32(fcom_mmio, 0x00, (1UL << 31));
    
    /* Install FIFO COM ISR */
    irq_Setting(fcom_irq, IRQ_LEVEL_TRIGGER);// + IRQ_DISABLE_INTR);
    irq_InstallISR(fcom_irq, FIFOCOM_ISR, NULL);
    
    /* TX FIFO Empty and RX FIFO Not Empty Interrupt Enable */
    fcom_ctrl = 1UL;
    io_Out32(fcom_mmio, 0x00, fcom_ctrl);

    fcom_initialize++;

    return 0;
}

/* FIFO-COM Deinitialize */
void fcom_close()
{
    if (fcom_initialize == 0)
        return;
    fcom_initialize--;
    if (fcom_initialize != 0)
        return;

    /* Interrupt Disable */
    io_Out32(fcom_mmio, 0x00, 0x00);
    /* RX FIFO Reset */
    io_Out32(fcom_mmio, 0x00, (1UL << 31));
    /* Uninstall FIFO COM ISR */
    irq_UninstallISR(fcom_irq, NULL);
    /* IRQ Library deinitialize */
    irq_Close();
    /* Flush RX */    
    fcom_rx_flush();
    /* Flush TX */    
    fcom_tx_flush();
    /* Free Default Frame Pool */    
    free_frame_pool(fcom_default_pool);
    /* Release memory space */
    io_Free(fcom_mmio);
    /* IO Library deinitialize */
    io_Close();
}

/* FIFO-COM set receive timeout */
void fcom_set_rx_timtout_ms(uint32_t timeout)
{
    fcom_rx_timeout = timeout;
}

/* FIFO-COM set send timeout */
void fcom_set_tx_timtout_ms(uint32_t timeout)
{
    fcom_tx_timeout = timeout;
}

/* FIFO-COM set maximum pool number */
void fcom_set_max_pool_size(uint32_t size)
{
    uint32_t extra_num, temp;
    
    fcom_pool_max_size = size;
    
    temp = fcom_pool_max_size - FCOM_DEFAULT_POOL_FRAME_NR * FCOM_FRAME_STRUCT_SIZE;
    extra_num = (temp % (FCOM_EXTRA_POOL_FRAME_NR * 64)) ? 1 : 0;
    extra_num = (temp / (FCOM_EXTRA_POOL_FRAME_NR * 64)) + extra_num;
    temp = extra_num * FCOM_EXTRA_POOL_FRAME_NR + FCOM_DEFAULT_POOL_FRAME_NR;
    fcom_max_frame_num = temp - extra_num - 1;
}

/* FIFO-COM configure mode */
void fcom_config(int config)
{
    if (config == FCOM_NORMAL)
        fcom_conf = FCOM_NORMAL;
    if (config == FCOM_DATA_NOT_LOSS)
        fcom_conf = FCOM_DATA_NOT_LOSS;
}

inline bool fcom_query_rx_size(int size)
{
    struct fcom_frame *frame;
    int count = 0;
    
    ENTER_CRITICAL_SECTION();
    frame = (struct fcom_frame*)rx_frame_head;
    while (frame != frame->next) {
        count += frame->size;
        if (count >= size) {
            EXIT_CRITICAL_SECTION();
            return true;
        }
        frame = frame->next;
    }
    EXIT_CRITICAL_SECTION();
    
    return size;
}

/* FIFO-COM check RX has data */
bool fcom_rx_not_empty()
{
    bool ret;
    
    ENTER_CRITICAL_SECTION();
    ret = (rx_frame_head != rx_frame_tail) ? true : false;
    if (ret == false && fcom_rx_status == FIFOCOM_RX_NOT_EMPTY) {
        rx_handle_irq();
        ret = (rx_frame_head != rx_frame_tail) ? true : false;
    }
    EXIT_CRITICAL_SECTION();
    
    return ret;
}

/* FIFO-COM check TX empty */
bool fcom_tx_empty()
{
    bool ret;
    
    ENTER_CRITICAL_SECTION();
    ret = (tx_frame_head == tx_frame_tail) ? true : false;
    EXIT_CRITICAL_SECTION();
    
    return ret;
}

/* FIFO-COM receive data non-blocking function */
int fcom_recv_nowait(uint8_t *buf, uint32_t len)
{
    int min, size = 0;
    uint8_t *fbuf;

    while (size < len) {

        if (rx_frame_peek == NULL) {
            ENTER_CRITICAL_SECTION_FAST();
            if (rx_frame_head == rx_frame_tail) {
                EXIT_CRITICAL_SECTION_FAST();
                break;
            }
            rx_frame_peek = rx_frame_head->next;
            rx_frame_head->next = rx_frame_peek->next;
            if (rx_frame_tail == rx_frame_peek)
                rx_frame_tail = rx_frame_head;
            EXIT_CRITICAL_SECTION_FAST();
        }

        min = len - size;
        if (min > rx_frame_peek->size)
            min = rx_frame_peek->size;
        fbuf = (uint8_t*)rx_frame_peek + FCOM_FRAME_STRUCT_SIZE - rx_frame_peek->size;
        memcpy(&buf[size], fbuf, min);
        
        size += min;

        rx_frame_peek->size -= min;
        if (rx_frame_peek->size <= 0) {
            ENTER_CRITICAL_SECTION();
            fcom_release_frame(rx_frame_peek);
            EXIT_CRITICAL_SECTION();
            rx_frame_peek = NULL;
        }
    }

    if (fcom_rx_not_empty()) {
        if (len > size)
            size += fcom_recv_nowait(&buf[size], len - size);
    }
    
    return size;
}

/* FIFO-COM receive data blocking function */
int fcom_recv(uint8_t *buf, uint32_t len)
{
    unsigned long pre, now;
    int cur = 0;
    
    pre = timer_NowTime();
    do {
        
        cur += fcom_recv_nowait(&buf[cur], len - cur);
        if (cur >= len)
            break;
        
        now = timer_NowTime();
        
    } while (now - pre <= fcom_rx_timeout);

    return cur;
}

/* FIFO-COM read byte blocking function */
int fcom_read_byte(uint8_t *pval)
{
    return fcom_recv(pval, 1);
}

/* FIFO-COM read word blocking function */
int fcom_read_word(uint16_t *pval)
{
    unsigned long pre, now;
    int cur = 0;

    pre = timer_NowTime();
    do {
        
        if (fcom_query_rx_size(2)) {
            cur = fcom_recv_nowait((uint8_t*)pval, 2);
            break;
        }
        
        now = timer_NowTime();
        
    } while (now - pre <= fcom_rx_timeout);

    return cur;
}

/* FIFO-COM read double word blocking function */
int fcom_read_dword(uint32_t *pval)
{
    unsigned long pre, now;
    int cur = 0;

    pre = timer_NowTime();
    do {
        
        if (fcom_query_rx_size(4)) {
            cur = fcom_recv_nowait((uint8_t*)pval, 4);
            break;
        }
        
        now = timer_NowTime();
        
    } while (now - pre <= fcom_rx_timeout);

    return cur;
}

/* FIFO-COM flush rx */
void fcom_rx_flush()
{
    struct fcom_frame *frame;
    
    ENTER_CRITICAL_SECTION();
    do {
        if (rx_frame_head == rx_frame_tail)
            break;
        frame = rx_frame_head->next;
        rx_frame_head->next = frame->next;
        if (rx_frame_tail == frame)
            rx_frame_tail = rx_frame_head;
        fcom_release_frame(frame);
    } while (1);
    EXIT_CRITICAL_SECTION();
}

/* FIFO-COM flush tx */
void fcom_tx_flush()
{
    struct fcom_frame *frame;
    
    ENTER_CRITICAL_SECTION();
    do {
        if (tx_frame_wait_head == NULL)
            break;
        frame = (struct fcom_frame*)tx_frame_wait_head;
        tx_frame_wait_head = frame->next;
        fcom_release_frame(frame);
    } while (1);
    tx_frame_wait_tail = NULL;
    tx_frame_head_temp = NULL;
    do {
        if (tx_frame_head == tx_frame_tail)
            break;
        frame = tx_frame_head->next;
        tx_frame_head->next = frame->next;
        if (tx_frame_tail == frame)
            tx_frame_tail = tx_frame_head;
        fcom_release_frame(frame);
    } while (1);
    EXIT_CRITICAL_SECTION();
}

/* FIFO-COM wait tx data flushed */
void fcom_wait_tx_flush()
{
    #define FCOM_TX_FLUSH_TIMEOUT   (1000)
    struct fcom_frame *frame, *next, *temp = NULL;
    unsigned long pretime;
    
    do {
        ENTER_CRITICAL_SECTION_FAST();
        frame = (struct fcom_frame*)tx_frame_head;
        next = tx_frame_head->next;
        EXIT_CRITICAL_SECTION_FAST();
        if (temp != frame) {
            pretime = timer_NowTime();
            temp = frame;
        } else if (timer_NowTime() - pretime > FCOM_TX_FLUSH_TIMEOUT)
            break;
    } while (frame != next);
}

/* FIFO-COM send data non-blocking function */
int fcom_send_nowait(uint8_t *buf, uint32_t len)
{
    struct fcom_frame *frame;
    int min, size = 0;
    uint8_t *fbuf;

    while (size < len) {
        
        ENTER_CRITICAL_SECTION();
        frame = fcom_request_frame();
        EXIT_CRITICAL_SECTION();
        
        if (!frame)
            break;

        min = len - size;
        if (min > FIFOCOM_FRAME_SIZE)
            min = FIFOCOM_FRAME_SIZE;

        fbuf = (uint8_t*)frame + FCOM_FRAME_STRUCT_SIZE - min;
        memcpy(fbuf, &buf[size], min);
        size += min;
        frame->size = min;

        ENTER_CRITICAL_SECTION_FAST();
        frame->next = tx_frame_tail->next;
        tx_frame_tail->next = frame;
        tx_frame_tail = frame;
        EXIT_CRITICAL_SECTION_FAST();
    }

    fcom_start_transfer();
    
    return size;
}

/* FIFO-COM send data blocking function */
int fcom_send(uint8_t *buf, uint32_t len)
{
    unsigned long pre, now;
    int cur = 0;
    
    pre = timer_NowTime();
    do {
        
        cur += fcom_send_nowait(&buf[cur], len - cur);
        if (cur >= len)
            break;
        
        now = timer_NowTime();
        
    } while (now - pre <= fcom_tx_timeout);

    return cur;
}

/* FIFO-COM write byte blocking function */
int fcom_write_byte(uint8_t val)
{
    return fcom_send(&val, 1);
}

/* FIFO-COM write word blocking function */
int fcom_write_word(uint16_t val)
{
    return fcom_send((uint8_t*)&val, 2);
}

/* FIFO-COM write double word blocking function */
int fcom_write_dword(uint32_t val)
{
    return fcom_send((uint8_t*)&val, 4);
}

/*****************************************************************************
 *                                                                           *
 *            FIFO-COM Send/Receive Frame Application Interface              *
 *                                                                           *
 *****************************************************************************/
/* Receive One Frame */
struct fcom_frame *fcom_recv_frame()
{
    struct fcom_frame *frame = NULL;
    
    ENTER_CRITICAL_SECTION_FAST();
    if (rx_frame_head != rx_frame_tail) {
        frame = rx_frame_head->next;
        rx_frame_head->next = frame->next;
        if (rx_frame_tail == frame)
            rx_frame_tail = rx_frame_head;
    }
    EXIT_CRITICAL_SECTION_FAST();
    
    if (fcom_rx_not_empty()) {
        if (frame == NULL)
            frame = fcom_recv_frame();
    }
    
    return frame;
}

/* Start Transmission */
void fcom_start_transfer()
{
    tx_frame_wait_in_preparation = false;
    
    ENTER_CRITICAL_SECTION();
    if (!(fcom_ctrl & (1UL << 4))) {
        if (fcom_tx_status == FIFOCOM_TX_NOT_CLEAR) {
            io_Out32(fcom_mmio, 0x04, (1UL << 4));
            fcom_tx_status = FIFOCOM_TX_CLEAR;
        }
        tx_handle_irq();
    }
    EXIT_CRITICAL_SECTION();
}

/* Send One Frame */
int fcom_send_frame(struct fcom_frame *frame)
{
    if (frame == NULL)
        return -1;
    
    ENTER_CRITICAL_SECTION_FAST();
    frame->next = tx_frame_tail->next;
    tx_frame_tail->next = frame;
    tx_frame_tail = frame;
    EXIT_CRITICAL_SECTION_FAST();
    
    return 0;
}

/* Send One Frame First */
int fcom_send_frame_first(struct fcom_frame *frame)
{
    if (frame == NULL)
        return -1;

    ENTER_CRITICAL_SECTION_FAST();
    if (tx_frame_wait_head == NULL) {
        tx_frame_wait_head = frame;
        tx_frame_wait_tail = frame;
    } else {
        frame->next = tx_frame_wait_tail->next;
        tx_frame_wait_tail->next = frame;
        tx_frame_wait_tail = frame;
    }
    EXIT_CRITICAL_SECTION_FAST();
    
    tx_frame_wait_in_preparation = true;
    
    return 0;
}
