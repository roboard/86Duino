#include "fifolan.h"
#include <stdio.h>
#include <string.h>
#include "fifocom.h"
#include "fifocom-private.h"
#include "io.h"

/*****************************************************************************
 *                                                                           *
 *                   FIFO-LAN Socket Request and Release                     *
 *                                                                           *
 *****************************************************************************/
/* FIFO-LAN Socket Flags */
#define FLAN_FLAG_NONE                      (0)
#define FLAN_FLAG_DATA                      (1)
#define FLAN_FLAG_CONNECT                   (2)
#define FLAN_FLAG_CONNECT_ACK               (3)
#define FLAN_FLAG_CONNEXTION_EXIST          (4)
#define FLAN_FLAG_REMOVE_CONNECT            (5)
#define FLAN_FLAG_NOT_ENOUGH_CONNECTIONS    (6)
#define FLAN_FLAG_NEW_PORT                  (7)
#define FLAN_FLAG_DISCONNECT                (8)
#define FLAN_FLAG_DISCONNECT_ACK            (9)
#define FLAN_FLAG_PING                      (10)
#define FLAN_FLAG_PING_ACK                  (11)

/* FIFO-LAN Connection Wait Queue Structure */
struct wait_queue;
/* FIFO-LAN Socket Structure */
struct flan_socket {
    struct flan_socket *next;
    struct flan_socket *father;
    struct fcom_frame *data_head;   /* RX FIFO-COM Frame Head */
    struct fcom_frame *data_tail;   /* RX FIFO-COM Frame Tail */
    size_t connections;             /* Maximum Connections */
    uint16_t local;                 /* Local Port */
    uint16_t remote;                /* Remote Port */
    
    size_t tx_length;               /* Temporary for RX Packet Length */
    
    struct wait_queue *wait_queue;  /* FIFO-LAN Connection Wait Queue */

    int busy;
    unsigned long sndtimeo;         /* Send Timeout */
    unsigned long rcvtimeo;         /* Receive Timeout */
    int non_blocking;               /* Indicate Non-Blocking flag */
    
    unsigned long pretime;
    
    /* FIFO-LAN State Machine */
    int state;
    #define FLAN_STATE_NONE                     (0)
    #define FLAN_STATE_CREATE                   (1)
    #define FLAN_STATE_LISTEN                   (2)
    #define FLAN_STATE_WAIT_CONNECT_ACK         (3)
    #define FLAN_STATE_WAIT_NEW_PORT            (4)
    #define FLAN_STATE_CONNECTED                (5)
    #define FLAN_STATE_WAIT_DISCONNECT_ACK      (6)
    #define FLAN_STATE_DISCONNECT_READY         (7)
    #define FLAN_STATE_INTERNAL                 (8)
    #define FLAN_STATE_PING                     (9)
    
    int errno;
    
    uint16_t ping_port;
    uint16_t ping_ack_port;
    int ping_status;
    int ping_state;
    #define PING_STATE_SEND (0)
    #define PING_STATE_RECV (1)
    
    struct flan_linger linger;
    
};

/*****************************************************************************
 *                                                                           *
 *         FIFO-LAN Register/Unregister port from Port Lookup Table          *
 *                                                                           *
 *****************************************************************************/
/* TCP/UDP Well-Known Ports */
#define WELL_KNOWN_PORTS    (256)
/* FIFO-LAN Socket number in Port Lookup Table */
#define FLAN_PLT_SOCKET_NR  (1024)
/* FIFO-LAN any port for not assign port */
#define FLAN_ANY_PORT       (-1)
/* FIFO-LAN Port Lookup Table */
static struct flan_socket *port_lookup_table[FLAN_PLT_SOCKET_NR];

/* Register one port */
inline int flan_register_port(struct flan_socket *sock, int port)
{
    if (port == FLAN_ANY_PORT) {
        for (port = WELL_KNOWN_PORTS; port < FLAN_PLT_SOCKET_NR; port++)
            if (port_lookup_table[port] == NULL)
                break;
        if (port >= FLAN_PLT_SOCKET_NR)
            return -1;
    } else if (port_lookup_table[port] != NULL) {
            return -1;
    }
    port_lookup_table[port] = sock;

    return port;
}

/* Unregister one port */
inline void flan_unregister_port(int port)
{
    port_lookup_table[port] = NULL;
}

/*****************************************************************************
 *                                                                           *
 *                   FIFO-LAN Request/Release Wait Queue                     *
 *                                                                           *
 *****************************************************************************/
/* FIFO-LAN Wait Queue Size per pool */
#define FLAN_WAIT_QUEUE_POOL_NR (64)

/* Wait Queue Pool Structure */
struct wait_queue_pool {
    uint32_t recycle_cnt;
};
/* FIFO-LAN Connection Wait Queue Structure */
struct wait_queue {
    struct wait_queue *next;
    struct wait_queue_pool *pool;
    uint16_t remote;
};
static struct wait_queue *wait_queue_head = NULL;

/* Allocate New Wait Queue Pool */
inline struct wait_queue *alloc_wait_queue_pool(unsigned int size)
{
    struct wait_queue_pool *pool;
    struct wait_queue *queue, *q;
    
    if (size <= 1)
        return NULL;

    pool = (struct wait_queue_pool*)malloc(sizeof(struct wait_queue) * size);
    if (pool == NULL)
        return NULL;
    
    pool->recycle_cnt = 0;
    size--;

    queue = (struct wait_queue*)((uint8_t*)pool + sizeof(struct wait_queue));

    q = queue;
    while (1) {
        q->pool = pool;
        q->remote = 0;
        q->next = (struct wait_queue*)((uint8_t*)q + sizeof(struct wait_queue));
        if (--size == 0)
            break;
        q = q->next;
    }
    q->next = NULL;
    
    return queue;
}

/* Free Wait Queue Pool */
inline void free_wait_queue_pool(struct wait_queue_pool *pool)
{
    if (pool)
        free(pool);
}

/* Request wait queue from pools */
struct wait_queue *flan_request_wait_queue()
{
    struct wait_queue *q = NULL;

    /* If no wait queue to request, allocate new wait queue pool. */
    if (wait_queue_head == NULL)
        wait_queue_head = alloc_wait_queue_pool(FLAN_WAIT_QUEUE_POOL_NR);
    
    if (wait_queue_head) {
        q = wait_queue_head;
        wait_queue_head = q->next;
        q->next = NULL;
    }
    
    return q;
}

/* Release wait queue */
void flan_release_wait_queue(struct wait_queue *q)
{
    if (q == NULL)
        return;

    q->pool->recycle_cnt++;
    if (q->pool->recycle_cnt == FLAN_WAIT_QUEUE_POOL_NR - 1)
        free_wait_queue_pool(q->pool);
}

/*****************************************************************************
 *                                                                           *
 *                   FIFO-LAN Request/Release one socket                     *
 *                                                                           *
 *****************************************************************************/
#define FLAN_SOCKET_TABLE_INITIAL_NR    (64)
/* FIFO-LAN Socket Table */
static struct flan_socket *socket_table = NULL;
/* FIFO-LAN Socket Table Maximum Size */
static unsigned long flan_socket_table_max = 0;
/* FIFO-LAN Socket Table Request Counter */
static unsigned long flan_socket_table_cnt = 0;

/* Request one socket */
inline int flan_request_socket()
{
    struct flan_socket *temp_table;
    int fd, size;
    
    for (fd = 0; fd < flan_socket_table_max; fd++)
        if (socket_table[fd].busy == 0)
            break;
    
    if (fd == flan_socket_table_max) {
        if (flan_socket_table_max == 0)
            size = FLAN_SOCKET_TABLE_INITIAL_NR;
        else
            size = flan_socket_table_max << 1;
        temp_table = (struct flan_socket *)malloc(sizeof(struct flan_socket) * size);
        if (temp_table == NULL)
            return -1;
        memset(temp_table, 0, sizeof(struct flan_socket) * size);
        if (socket_table) {
            memcpy(temp_table, socket_table, sizeof(struct flan_socket) * flan_socket_table_max);
            free(socket_table);
        }
        socket_table = temp_table;
        for (fd = flan_socket_table_max; fd < size; fd++)
            if (socket_table[fd].busy == 0)
                break;
        flan_socket_table_max = size;
    }
    flan_socket_table_cnt++;
    
    socket_table[fd].busy = 1;
    socket_table[fd].next = NULL;
    socket_table[fd].father = NULL;
    socket_table[fd].data_head = NULL;
    socket_table[fd].data_tail = NULL;
    socket_table[fd].connections = 0;
    socket_table[fd].local = 0;
    socket_table[fd].remote = 0;
    socket_table[fd].tx_length = 0;
    socket_table[fd].state = FLAN_STATE_CREATE;
    socket_table[fd].sndtimeo = 1000UL;
    socket_table[fd].rcvtimeo = 1000UL;
    socket_table[fd].non_blocking = 0;
    socket_table[fd].wait_queue = NULL;
    socket_table[fd].pretime = 0;
    socket_table[fd].errno = 0;
    socket_table[fd].ping_port = 0;
    socket_table[fd].ping_ack_port = 0;
    socket_table[fd].ping_status = FLAN_FLAG_NONE;
    socket_table[fd].ping_state = PING_STATE_SEND;
    socket_table[fd].linger.l_onoff = 1;
    socket_table[fd].linger.l_linger = 1;
    
    return fd;
}

/* Release one socket */
inline void flan_release_socket(int fd)
{
    struct wait_queue *q;
    
    socket_table[fd].busy = 0;
    
    while (socket_table[fd].wait_queue) {
        q = socket_table[fd].wait_queue;
        socket_table[fd].wait_queue = q->next;
        flan_release_wait_queue(q);
    }
    
    flan_socket_table_cnt--;
    if (flan_socket_table_cnt == 0) {
        flan_socket_table_max = 0;
        free(socket_table);
        socket_table = NULL;
    }
}

/*****************************************************************************
 *                                                                           *
 *                   FIFO-LAN Receive and Process Packet                     *
 *                                                                           *
 *****************************************************************************/
/* RX Packet Timeout */
#define FLAN_RX_PACKET_TIMEOUT  (1000)
static unsigned long flan_rx_pretime = 0;

/* FIFO-LAN Packet Header */
#define FLAN_PACKET_HEADER  (0x55AAFFFF)

/* RX Packet Linker List */
static struct fcom_frame *rx_packet_head = NULL;
static struct fcom_frame *rx_packet_tail = NULL;
static struct fcom_frame *rx_packet_temp = NULL;
/* Temporary variable for receive RX Packet */
static int rx_packet_length = 0;

/* Send Socket Packet */
inline int flan_send_packet(struct flan_socket *sock, void *buf, size_t count, int flag, int (*send)(struct fcom_frame*))
{
    struct fcom_frame *frame = NULL;
    size_t temp, size = 0;
    uint8_t *fbuf;
    int ccount = 0;
    static uint32_t need_size, free_size;

    ENTER_CRITICAL_SECTION();
    do {
        
        if (sock->tx_length == 0) {
            
            sock->tx_length = count - size;
            if (sock->tx_length > 0xFFFF)
                sock->tx_length = 0xFFFF;
            
            need_size = sock->tx_length + 12;
            need_size = need_size / FIFOCOM_FRAME_SIZE + ((need_size % FIFOCOM_FRAME_SIZE) ? 1 : 0);
            free_size = fcom_max_frame_num - fcom_use_frame_num;
            if (need_size > free_size) {
                sock->tx_length = 0;
                break;
            }
            
            frame = fcom_request_frame();
            
            temp = sock->tx_length + 12;
            if (temp > FIFOCOM_FRAME_SIZE)
                temp = FIFOCOM_FRAME_SIZE;
            frame->size = temp;
            frame->type = FCOM_TYPE_HEAD;
            
            fbuf = (uint8_t*)frame + FCOM_FRAME_STRUCT_SIZE - temp;
            ((uint32_t*)fbuf)[0] = FLAN_PACKET_HEADER;
            ((uint16_t*)fbuf)[2] = sock->local;
            ((uint16_t*)fbuf)[3] = sock->remote;
            ((uint16_t*)fbuf)[4] = sock->tx_length;
            ((uint16_t*)fbuf)[5] = flag;
            
            temp = frame->size - 12;
            
        } else {
            
            frame = fcom_request_frame();
            
            temp = sock->tx_length;
            if (temp > FIFOCOM_FRAME_SIZE)
                temp = FIFOCOM_FRAME_SIZE;
            frame->size = temp;
            if (sock->tx_length != temp)
                frame->type = FCOM_TYPE_DATA;
            else
                frame->type = FCOM_TYPE_TAIL;
            
        }
        
        if (temp > 0) {
            fbuf = (uint8_t*)frame + FCOM_FRAME_STRUCT_SIZE - temp;
            memcpy(fbuf, (uint8_t*)buf + size, temp);
        }
        
        size += temp;
        sock->tx_length -= temp;

        fbuf = (uint8_t*)frame + FCOM_FRAME_STRUCT_SIZE - frame->size;
        // if (flag != FLAN_FLAG_DATA) {
            // printf("Send: ");
            // for (int i = 0; i < frame->size; i++)
                // printf("%02x ", fbuf[i]);
            // printf("\n");
        // }
        send(frame);
        if (size >= count || sock->tx_length <= 0) {
            fcom_start_transfer();
            break;
        }
    } while (1);
    
    EXIT_CRITICAL_SECTION();

    return frame ? size : -1;
}

/* Discard Socket Packet */
inline void flan_discard_packet()
{
    struct fcom_frame *frame;
    
    while (rx_packet_head) {
        frame = rx_packet_head;
        rx_packet_head = rx_packet_head->next;
        ENTER_CRITICAL_SECTION();
        fcom_release_frame(frame);
        EXIT_CRITICAL_SECTION();
    }
    rx_packet_tail = NULL;
}

/* Receive Packet Data from FIFO-COM */
inline void flan_receive_packet_from_fifocom()
{
    struct fcom_frame *frame;
    struct flan_socket *sock;
    uint8_t *fbuf;
    uint16_t dest;

    if (rx_packet_temp) {
        if (timer_NowTime() - flan_rx_pretime > FLAN_RX_PACKET_TIMEOUT) {
            rx_packet_head = rx_packet_temp;
            rx_packet_temp = NULL;
            flan_discard_packet();
        }
    }
    
    while (rx_packet_head == NULL) {
        
        frame = fcom_recv_frame();
        if (frame == NULL)
            return;
        frame->next = NULL;

        if (rx_packet_temp == NULL) {
            fbuf = (uint8_t*)frame + FCOM_FRAME_STRUCT_SIZE - frame->size;
            // if (*((uint16_t*)(fbuf + 10)) != FLAN_FLAG_DATA) {
                // printf("Recv: ");
                // for (int i = 0; i < frame->size; i++)
                    // printf("%02x ", fbuf[i]);
                // printf("\n");
            // }
            if (frame->size < 12 || *((uint32_t*)fbuf) != FLAN_PACKET_HEADER) {
                ENTER_CRITICAL_SECTION();
                fcom_release_frame(frame);
                EXIT_CRITICAL_SECTION();
                return;
            }
            rx_packet_length = *((uint16_t*)(fbuf + 8)) + 12 - frame->size;
            rx_packet_temp = frame;
            rx_packet_tail = frame;
        } else {
            rx_packet_tail->next = frame;
            rx_packet_tail = frame;
            rx_packet_length -= frame->size;
        }
        
        if (rx_packet_length <= 0) {
            rx_packet_head = rx_packet_temp;
            rx_packet_temp = NULL;
        }
        
        if (rx_packet_temp)
            flan_rx_pretime = timer_NowTime();
    
    }
}

/* Process FIFO-LAN Packet */
inline void flan_process_packet()
{
    struct flan_socket *sock, *s;
    struct wait_queue *o, *p, *q;
    uint8_t *buf;
    uint16_t src, dst, cmd, len, port;
    int rc, nr;
    
    flan_receive_packet_from_fifocom();

    if (rx_packet_head == NULL)
        return;

    buf = (uint8_t*)rx_packet_head + FCOM_FRAME_STRUCT_SIZE - rx_packet_head->size;
    src = *((uint16_t*)(buf + 4));
    dst = *((uint16_t*)(buf + 6));
    len = *((uint16_t*)(buf + 8));
    cmd = *((uint16_t*)(buf + 10));

    sock = (dst < FLAN_PLT_SOCKET_NR) ? port_lookup_table[dst] : NULL;
    if (sock == NULL) {
        flan_discard_packet();
        return;
    }

    if (sock->state == FLAN_STATE_CONNECTED && sock->remote != src) {
        flan_discard_packet();
        return;
    }

    switch (sock->state) {
    case FLAN_STATE_LISTEN:
        if (cmd == FLAN_FLAG_CONNECT) {
            s = sock->next;
            while (s) {
                if (s->remote == src)
                    break;
                s = s->next;
            }
            if (s == NULL) {
                nr = 0;
                o = NULL;
                p = sock->wait_queue;
                while (p->next) {
                    nr++;
                    if (p->next->remote == src)
                        o = p->next;
                    p = p->next;
                }
                if (o == NULL) {
                    if (nr < sock->connections) {
                        q = flan_request_wait_queue();
                        q->remote = src;
                        p->next = q;
                    } else {
                        sock->remote = src;
                        rc = flan_send_packet(sock, NULL, 0, FLAN_FLAG_NOT_ENOUGH_CONNECTIONS, fcom_send_frame);
                        if (rc < 0)
                            break;
                        flan_discard_packet();
                        break;
                    }
                }
                sock->remote = src;
                rc = flan_send_packet(sock, NULL, 0, FLAN_FLAG_CONNECT_ACK, fcom_send_frame);
                if (rc < 0)
                    break;
            } else {
                rc = flan_send_packet(sock, NULL, 0, FLAN_FLAG_CONNEXTION_EXIST, fcom_send_frame);
                if (rc < 0)
                    break;
            }
        } else if (cmd == FLAN_FLAG_REMOVE_CONNECT) {
            p = sock->wait_queue;
            while (p->next) {
                if (p->next->remote == src) {
                    o = p->next;
                    p->next = o->next;
                    flan_release_wait_queue(o);
                    break;
                }
                p = p->next;
            }
        }
        flan_discard_packet();
        break;
    case FLAN_STATE_WAIT_CONNECT_ACK:
        flan_discard_packet();
        if (cmd == FLAN_FLAG_CONNECT_ACK) {
            sock->pretime = timer_NowTime();
            sock->state = FLAN_STATE_WAIT_NEW_PORT;
        } else if (cmd == FLAN_FLAG_NOT_ENOUGH_CONNECTIONS)
            sock->state = FLAN_STATE_CREATE;
        else if (cmd == FLAN_FLAG_CONNEXTION_EXIST)
            sock->state = FLAN_STATE_CREATE;
        break;
    case FLAN_STATE_WAIT_NEW_PORT:
        if (cmd == FLAN_FLAG_NEW_PORT && len >= 2) {
            sock->state = FLAN_STATE_CONNECTED;
            sock->remote = *((uint16_t*)(buf + 12));
        } else if (cmd == FLAN_FLAG_REMOVE_CONNECT)
            sock->state = FLAN_STATE_CREATE;
        flan_discard_packet();
        break;
    case FLAN_STATE_WAIT_DISCONNECT_ACK:
        flan_discard_packet();
        if (cmd == FLAN_FLAG_DISCONNECT_ACK)
            sock->state = FLAN_STATE_NONE;
        else if (cmd == FLAN_FLAG_DISCONNECT)
            sock->state = FLAN_STATE_NONE;
        if (sock->state == FLAN_STATE_NONE) {
            s = sock->father;
            while (s) {
                if (s->next == sock) {
                    s->next = sock->next;
                    break;
                }
                s = s->next;
            }
        }
        break;
    case FLAN_STATE_CONNECTED:
        if (cmd == FLAN_FLAG_DATA) {
            rx_packet_head->size -= 12;
            if (sock->data_tail)
                sock->data_tail->next = rx_packet_head;
            else
                sock->data_head = rx_packet_head;
            sock->data_tail = rx_packet_tail;
            rx_packet_head = NULL;
            rx_packet_tail = NULL;
        } else if (cmd == FLAN_FLAG_DISCONNECT) {
            rc = flan_send_packet(sock, NULL, 0, FLAN_FLAG_DISCONNECT_ACK, fcom_send_frame);
            if (rc >= 0) {
                flan_discard_packet();
                sock->state = FLAN_STATE_DISCONNECT_READY;
            } 
        }
        break;
    case FLAN_STATE_INTERNAL:
        if (cmd == FLAN_FLAG_DATA) {
            rx_packet_head->size -= 12;
            if (sock->data_tail)
                sock->data_tail->next = rx_packet_head;
            else
                sock->data_head = rx_packet_head;
            sock->data_tail = rx_packet_tail;
            rx_packet_head = NULL;
            rx_packet_tail = NULL;
        }
        break;
    case FLAN_STATE_PING:
        if (cmd == FLAN_FLAG_PING && len >= 2) {
            port = *((uint16_t*)(buf + 12));
            if (port >= FLAN_PLT_SOCKET_NR || port_lookup_table[port] == NULL)
                port |= 0x8000;
            rc = flan_send_packet(sock, &port, sizeof(port), FLAN_FLAG_PING_ACK, fcom_send_frame);
            if (rc >= 0)
                flan_discard_packet();
        } else if (cmd == FLAN_FLAG_PING_ACK && len >= 2) {
            rx_packet_head->size -= 12;
            if (sock->data_tail)
                sock->data_tail->next = rx_packet_head;
            else
                sock->data_head = rx_packet_head;
            sock->data_tail = rx_packet_tail;
            rx_packet_head = NULL;
            rx_packet_tail = NULL;
        } else
            flan_discard_packet();
        break;
    case FLAN_STATE_NONE:
    case FLAN_STATE_CREATE:
    case FLAN_STATE_DISCONNECT_READY:
        flan_discard_packet();
        break;
    }
}

/*****************************************************************************
 *                                                                           *
 *              FIFO-LAN Socket Packet Functions and Variables               *
 *                                                                           *
 *****************************************************************************/

/* Receive Socket Data Packet */
inline int flan_recv_data(struct flan_socket *sock, void *buf, size_t count)
{
    struct fcom_frame *frame;
    int min, size = 0;
    uint8_t *fbuf;
    
    flan_process_packet();

    while (size < count) {
        if (sock->data_head == NULL)
            break;
        frame = sock->data_head;
        
        min = count - size;
        if (min > frame->size)
            min = frame->size;

        fbuf = (uint8_t*)frame + FCOM_FRAME_STRUCT_SIZE - frame->size;
        memcpy((uint8_t*)buf + size, fbuf, min);
        size += min;
        frame->size -= min;
        if (frame->size <= 0) {
            sock->data_head = frame->next;
            if (sock->data_head == NULL)
                sock->data_tail = NULL;
            ENTER_CRITICAL_SECTION();
            fcom_release_frame(frame);
            EXIT_CRITICAL_SECTION();
        }
    }

    return size;
}

/*****************************************************************************
 *                                                                           *
 *                  FIFO-LAN Socket Application Interface                    *
 *                                                                           *
 *****************************************************************************/
/* Create FIFO-LAN Socket */
int flan_socket()
{
    int fd, port;

    fd = flan_request_socket();
    if (fd < 0)
        return -1;
    
    port = flan_register_port(&socket_table[fd], FLAN_ANY_PORT);
    if (port < 0) {
        flan_release_socket(fd);
        return -1;
    }
    socket_table[fd].local = port;

    return fd;
}

/* Close FIFO-LAN Socket */
int flan_close(int fd)
{
    struct flan_socket *sock, *s;
    struct wait_queue *q;
    int rc, ret = 0;
    uint8_t buf[256];

    if (fd >= flan_socket_table_max)
        return -1;
    if (!socket_table[fd].busy)
        return -1;
    
    sock = &socket_table[fd];
    
    do {
        
        flan_recv_data(sock, buf, 256);

        ret = 0;
        
        switch (sock->state) {
        case FLAN_STATE_CREATE:
        case FLAN_STATE_DISCONNECT_READY:
            sock->state = FLAN_STATE_NONE;
            s = sock->father;
            while (s) {
                if (s->next == sock) {
                    s->next = sock->next;
                    break;
                }
                s = s->next;
            }
            break;
        case FLAN_STATE_WAIT_CONNECT_ACK:
        case FLAN_STATE_WAIT_NEW_PORT:
            rc = flan_send_packet(sock, NULL, 0, FLAN_FLAG_REMOVE_CONNECT, fcom_send_frame);
            if (rc >= 0)
                sock->state = FLAN_STATE_NONE;
            break;
        case FLAN_STATE_LISTEN:
            q = sock->wait_queue->next;
            while (q) {
                sock->remote = q->remote;
                rc = flan_send_packet(sock, NULL, 0, FLAN_FLAG_REMOVE_CONNECT, fcom_send_frame);
                if (rc < 0)
                    break;
                sock->wait_queue->next = q->next;
                flan_release_wait_queue(q);
            }
            if (sock->wait_queue->next == NULL)
                sock->state = FLAN_STATE_NONE;
            break;
        case FLAN_STATE_CONNECTED:
            rc = flan_send_packet(sock, NULL, 0, FLAN_FLAG_DISCONNECT, fcom_send_frame);
            if (rc >= 0) {
                sock->state = FLAN_STATE_WAIT_DISCONNECT_ACK;
                sock->pretime = timer_NowTime();
            }
            ret = -1;
            sock->errno = FLAN_EWOULDBLOCK;
            break;
        case FLAN_STATE_WAIT_DISCONNECT_ACK:
            if (timer_NowTime() - sock->pretime > (sock->linger.l_linger * 1000)) {
                sock->errno = FLAN_ETIMEDOUT1;
                sock->state = FLAN_STATE_NONE;
            } else
                sock->errno = FLAN_EWOULDBLOCK;
            ret = -1;
            break;
        case FLAN_STATE_INTERNAL:
        case FLAN_STATE_PING:
            sock->state = FLAN_STATE_NONE;
            break;
        }

        if (sock->state == FLAN_STATE_NONE)
            break;
        if (sock->linger.l_onoff == 0) {
            ret = -1;
            sock->errno = FLAN_EWOULDBLOCK;
            sock->state = FLAN_STATE_NONE;
            break;
        }
        
    } while (1);
    
    if (sock->state == FLAN_STATE_NONE) {
        while (flan_recv_data(sock, buf, 256));
        flan_unregister_port(sock->local);
        flan_release_socket(fd);
    }

    return ret;
}

/* Bind the FIFO-LAN Socket Port */
int flan_bind(int fd, uint16_t local)
{
    int port;
    
    if (fd >= flan_socket_table_max)
        return -1;
    if (!socket_table[fd].busy)
        return -1;
    if (local >= WELL_KNOWN_PORTS) {
        socket_table[fd].errno = FLAN_EINVAL;
        return -1;
    }
    
    port = flan_register_port(&socket_table[fd], local);
    if (port < 0) {
        socket_table[fd].errno = FLAN_EADDRINUSE;
        return -1;
    }
    
    flan_unregister_port(socket_table[fd].local);
    socket_table[fd].local = port;
    
    return 0;
}

/* Setup the maximum number of connections */
int flan_listen(int fd, int connections)
{
    struct wait_queue *queue;
    
    if (fd >= flan_socket_table_max)
        return -1;
    if (!socket_table[fd].busy)
        return -1;
    
    flan_process_packet();
    
    queue = flan_request_wait_queue();
    if (queue == NULL) {
        socket_table[fd].errno = FLAN_ENOMEM;
        return -1;
    }
    
    socket_table[fd].wait_queue = queue;
    socket_table[fd].connections = (connections > 0) ? connections : 1;
    socket_table[fd].state = FLAN_STATE_LISTEN;

    return 0;
}

/* Accept one connection */
int flan_accept(int fd)
{
    struct flan_socket *sock, *s;
    struct wait_queue *queue;
    int rc, new_fd;

    if (fd >= flan_socket_table_max)
        return -1;
    if (socket_table[fd].busy == 0)
        return -1;
    if (socket_table[fd].state != FLAN_STATE_LISTEN) {
        socket_table[fd].errno = FLAN_EWOULDBLOCK;
        return -1;
    }

    do {
        flan_process_packet();
        
        queue = socket_table[fd].wait_queue->next;
        if (socket_table[fd].non_blocking && queue == NULL) {
            socket_table[fd].errno = FLAN_EWOULDBLOCK;
            return -1;
        }
    } while (queue == NULL);

    new_fd = flan_socket();
    if (new_fd < 0)
        return new_fd;

    sock = &socket_table[new_fd];

    sock->remote = queue->remote;
    rc = flan_send_packet(sock, &sock->local, sizeof(sock->local), FLAN_FLAG_NEW_PORT, fcom_send_frame);
    if (rc < 0) {
        flan_close(new_fd);
        sock->errno = FLAN_EWOULDBLOCK;
        return -1;
    }
    
    socket_table[fd].wait_queue->next = queue->next;
    flan_release_wait_queue(queue);
    
    sock->state = FLAN_STATE_CONNECTED;
    
    s = &socket_table[fd];
    sock->father = s;
    while (s->next)
        s = s->next;
    s->next = sock;
    return new_fd;
}

/* Connect to the assigned port of one Server */
int flan_connect(int fd, uint16_t remote)
{
    #define FLAN_WAIT_ACK_TIMEOUT       (1000UL)
    #define FLAN_WAIT_PORT_TIMEOUT      (1000UL)
    #define FLAN_TOTAL_CONNECT_TIMEOUT  (3000UL)
    struct flan_socket *sock;
    uint16_t port;
    int rc;
    unsigned long pretime;

    if (fd >= flan_socket_table_max)
        return -1;
    if (socket_table[fd].busy == 0)
        return -1;
    
    sock = &socket_table[fd];
    
    if (sock->non_blocking == 0)
        pretime = timer_NowTime();

    do {
        flan_process_packet();
        
        switch (sock->state) {
        case FLAN_STATE_CREATE:
            sock->remote = remote;
            rc = flan_send_packet(sock, NULL, 0, FLAN_FLAG_CONNECT, fcom_send_frame);
            if (rc >= 0) {
                sock->state = FLAN_STATE_WAIT_CONNECT_ACK;
                sock->pretime = timer_NowTime();
            }
            break;
        case FLAN_STATE_WAIT_CONNECT_ACK:
            if (timer_NowTime() - sock->pretime > FLAN_WAIT_ACK_TIMEOUT) {
                sock->state = FLAN_STATE_CREATE;
                sock->errno = FLAN_ETIMEDOUT1;
                return -1;
            }
            break;
        case FLAN_STATE_WAIT_NEW_PORT:
            if (timer_NowTime() - sock->pretime > FLAN_WAIT_PORT_TIMEOUT) {
                flan_send_packet(sock, NULL, 0, FLAN_FLAG_REMOVE_CONNECT, fcom_send_frame);
                sock->state = FLAN_STATE_CREATE;
                sock->errno = FLAN_ETIMEDOUT2;
                return -1;
            }
            break;
        case FLAN_STATE_CONNECTED:
            return 0;
        }
        if (sock->non_blocking)
            break;
        if (timer_NowTime() - pretime > FLAN_TOTAL_CONNECT_TIMEOUT) {
            flan_send_packet(sock, NULL, 0, FLAN_FLAG_REMOVE_CONNECT, fcom_send_frame);
            sock->state = FLAN_STATE_CREATE;
            sock->errno = FLAN_ETIMEDOUT;
            return -1;
        }
    } while (1);
    
    sock->errno = FLAN_EWOULDBLOCK;
    return -1;
}

/* Socket Reveice Function */
int flan_recv(int fd, void *buf, size_t count)
{
    int size = 0;
    unsigned long pretime;
    
    if (fd >= flan_socket_table_max)
        return -1;
    if (socket_table[fd].busy == 0)
        return -1;
    
    if (socket_table[fd].non_blocking == 0)
        pretime = timer_NowTime();

    while (size < count) {
        size += flan_recv_data(&socket_table[fd], (uint8_t*)buf + size, count - size);
        if (socket_table[fd].non_blocking)
            break;
        else if (timer_NowTime() - pretime > socket_table[fd].rcvtimeo)
            break;
    }
    
    if (size == 0 && socket_table[fd].state == FLAN_STATE_DISCONNECT_READY) {
        socket_table[fd].errno = FLAN_ECONNABORTED;
        return -1;
    }

    return size;
}

/* Socket Send Inline Function */
inline int flan_send_nowait(int fd, void *buf, size_t count, int (*send)(struct fcom_frame*))
{
    int rc, size = 0;
    unsigned long pretime;
    
    if (fd >= flan_socket_table_max)
        return -1;
    if (socket_table[fd].busy == 0)
        return -1;
    
    if (socket_table[fd].non_blocking == 0)
        pretime = timer_NowTime();

    while (size < count) {
        flan_process_packet();
        
        rc = flan_send_packet(&socket_table[fd], (uint8_t*)buf + size, count - size, FLAN_FLAG_DATA, send);
        if (rc >= 0)
            size += rc;
        if (socket_table[fd].non_blocking)
            break;
        else if (timer_NowTime() - pretime > socket_table[fd].sndtimeo)
            break;
    }
    
    return size;
}

/* Socket Send Function */
int flan_send(int fd, void *buf, size_t count)
{
    return flan_send_nowait(fd, buf, count, fcom_send_frame);
}

/* Socket Send Function First */
int flan_send_first(int fd, void *buf, size_t count)
{
    return flan_send_nowait(fd, buf, count, fcom_send_frame_first);
}

/* Socket I/O Control Function */
int flan_ioctl(int fd, unsigned int cmd, unsigned long arg)
{
    if (fd >= flan_socket_table_max)
        return -1;
    
    switch (cmd) {
    case FLAN_FIONBIO:
        socket_table[fd].non_blocking = *((int*)arg) ? 1 : 0;
        break;
    }

    return 0;
}

/* Set Socket Option */
int flan_setsockopt(int fd, int optname, const void *optval, size_t optlen)
{
    if (fd >= flan_socket_table_max)
        return -1;
    
    switch (optname) {
    case FLAN_SO_SNDTIMEO:
        memcpy(&socket_table[fd].sndtimeo, optval, optlen);
        break;
    case FLAN_SO_RCVTIMEO:
        memcpy(&socket_table[fd].rcvtimeo, optval, optlen);
        break;
    case FLAN_SO_LINGER:
        memcpy(&socket_table[fd].linger, optval, optlen);
        break;
    }

    return 0;
}

/* Get Socket Option */
int flan_getsockopt(int fd, int optname, void *optval, size_t *optlen)
{
    if (fd >= flan_socket_table_max)
        return -1;
    
    switch (optname) {
    case FLAN_SO_SNDTIMEO:
        *optlen = sizeof(socket_table[fd].sndtimeo);
        memcpy(optval, &socket_table[fd].sndtimeo, *optlen);
        break;
    case FLAN_SO_RCVTIMEO:
        *optlen = sizeof(socket_table[fd].rcvtimeo);
        memcpy(optval, &socket_table[fd].rcvtimeo, *optlen);
        break;
    case FLAN_SO_LINGER:
        *optlen = sizeof(socket_table[fd].linger);
        memcpy(optval, &socket_table[fd].linger, *optlen);
        break;
    }

    return 0;
}

/* Get Error Code */
int flan_errno(int fd)
{
    if (fd >= flan_socket_table_max)
        return -1;
    if (socket_table[fd].busy == 0)
        return -1;
    
    return socket_table[fd].errno;
}

/*****************************************************************************
 *                                                                           *
 *                      FIFO-LAN initialize and close                        *
 *                                                                           *
 *****************************************************************************/
/* FIFO-LAN port number for internal communication */
#define FLAN_INTERNAL_PORT  (255)
/* FIFO-LAN port number for ping */
#define FLAN_PING_PORT      (254)
/* FIFO-LAN file descriptor for ping */
#define FLAN_PING_FD        (1)
/* FIFO-LAN initialized counter */
static int flan_initialize = 0;

/* FIFO-LAN initialize */
int flan_initDriver()
{
    int fd, rc, val = 1;

    if (flan_initialize == 0) {
        if (fcom_init() != 0)
            return -1;
        fcom_set_max_pool_size(64 * 1024 * 1024UL);
        fcom_config(FCOM_DATA_NOT_LOSS);
        
        fd = flan_socket();
        if (fd < 0)
            return -1;
        if (fd != FLAN_INTERNAL_FD) {
            flan_close(fd);
            return -1;
        }
        rc = flan_bind(fd, FLAN_INTERNAL_PORT);
        if (rc < 0) {
            flan_close(fd);
            return -1;
        }
        flan_ioctl(fd, FLAN_FIONBIO, (unsigned long)&val);
        socket_table[fd].remote = FLAN_INTERNAL_PORT;
        socket_table[fd].state = FLAN_STATE_INTERNAL;

        fd = flan_socket();
        if (fd < 0) {
            flan_close(FLAN_INTERNAL_FD);
            return -1;
        }
        if (fd != FLAN_PING_FD) {
            flan_close(fd);
            flan_close(FLAN_INTERNAL_FD);
            return -1;
        }
        rc = flan_bind(fd, FLAN_PING_PORT);
        if (rc < 0) {
            flan_close(fd);
            flan_close(FLAN_INTERNAL_FD);
            return -1;
        }
        flan_ioctl(fd, FLAN_FIONBIO, (unsigned long)&val);
        socket_table[fd].remote = FLAN_PING_PORT;
        socket_table[fd].state = FLAN_STATE_PING;
    }
    flan_initialize++;
    
    return 0;
}

/* FIFO-LAN close */
int flan_closeDriver()
{
    #define FLAN_CLOSE_DRIVER_TIMEOUT   (1000)
    unsigned long pretime;
    
    if (flan_initialize == 1) {
        flan_close(FLAN_INTERNAL_FD);
        flan_close(FLAN_PING_FD);
        pretime = timer_NowTime();
        do {
            flan_poll();
            if (timer_NowTime() - pretime > FLAN_CLOSE_DRIVER_TIMEOUT)
                break;
        } while (!fcom_tx_empty());
        fcom_rx_flush();
        fcom_tx_flush();
        fcom_close();
    }
    if (flan_initialize > 0)
        flan_initialize--;
    
    return 0;
}

/* Ping remote port */
int flan_ping(uint16_t port)
{
    #define FLAN_PING_TIMEOUT   (500)
    unsigned long pretime;
    struct flan_socket *sock;
    int rc;
    uint16_t ack_port;
    
    if (flan_initialize == 0)
        return -1;
    
    sock = &socket_table[FLAN_PING_FD];
    
    if (sock->non_blocking == 0)
        pretime = timer_NowTime();
    
    do {
        flan_process_packet();
        
        switch (sock->ping_state) {
        case PING_STATE_RECV:
            if (port == sock->ping_port) {
                rc = flan_recv_data(&socket_table[FLAN_PING_FD], &ack_port, 2);
                if (rc == 2 && (ack_port & 0x7FFF) == port) {
                    if (ack_port & 0x8000)
                        return 0;
                    return 1;
                } else if (timer_NowTime() - sock->pretime > FLAN_PING_TIMEOUT) {
                    sock->ping_state = PING_STATE_SEND;
                    sock->errno = FLAN_ETIMEDOUT1;
                    return -1;
                }
                break;
            } else
                sock->ping_state = PING_STATE_SEND;
        case PING_STATE_SEND:
            rc = flan_send_packet(sock, &port, sizeof(port), FLAN_FLAG_PING, fcom_send_frame);
            if (rc >= 0) {
                sock->ping_state = PING_STATE_RECV;
                sock->ping_port = port;
                sock->ping_status = FLAN_FLAG_NONE;
                sock->pretime = timer_NowTime();
            }
            break;
        }
        
        if (sock->non_blocking)
            break;
        if (timer_NowTime() - pretime > (FLAN_PING_TIMEOUT << 1)) {
            sock->ping_state = PING_STATE_SEND;
            sock->errno = FLAN_ETIMEDOUT;
            return -1;
        }

    } while (1);

    sock->errno = FLAN_EWOULDBLOCK;
    return -1;
}

/* FIFO-LAN get and process packet polling function */
int flan_poll()
{
    flan_process_packet();
    return 0;
}
