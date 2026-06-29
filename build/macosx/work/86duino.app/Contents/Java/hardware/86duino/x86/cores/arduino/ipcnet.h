#ifndef __IPCNET_H
#define __IPCNET_H

#include <stdlib.h>
#include <stdint.h>
#include "ipcfifo.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IPCNET_REAL_TIME_VER
// #define IPCNET_DPC_ENABLE

#ifdef IPCFIFO_POLLING_MODE_ENABLE
    #define IPCNET_POLLING_MODE_ENABLE
#endif

/* System Channel Socket. */
#define IPCNET_SYSTEM_CH    (1)

/* IPC-NET Socket Error Code */
#define IPCNET_EWOULDBLOCK  (502) // Operation would block
#define IPCNET_ETIMEDOUT    (503) // Timed out
#define IPCNET_ETIMEDOUT1   (504) // Timed out 1
#define IPCNET_ETIMEDOUT2   (505) // Timed out 2
#define IPCNET_ENOMEM       (506) // Not enough space/cannot allocate memory
#define IPCNET_EINVAL       (507) // Invalid argument
#define IPCNET_EADDRINUSE   (508) // Address already in use
#define IPCNET_ECONNABORTED (509) // Connection aborted

/* IPC-NET Application Interface */
int ipcnet_initialize();
int ipcnet_shutdown();
int ipcnet_ping(uint16_t port);
int ipcnet_wait_flush(uint32_t timeoutMs);

/* Polling function when using polling mode */
int ipcnet_process();
#ifdef IPCFIFO_POLLING_MODE_ENABLE
    /* Set Polling function ipcnet_process() deadline (microseconds) */
    inline void ipcnet_set_deadline(unsigned long deadline) {
        ipcfifo_set_deadline(deadline);
    }
    /* Clear Polling function ipcnet_process() deadline (microseconds) */
    inline void ipcnet_clear_deadline() {
        ipcfifo_clear_deadline();
    }
#endif

/* IPC-NET System Event Application Interface: Event ID: 0 ~ 55 */
int ipcnet_send_system_event(uint16_t EventID, uint8_t *msg, uint16_t msg_size); /* msg_size = 0 ~ 46 */
int ipcnet_register_system_eventCB(uint16_t EventID, void *data, void (*callback)(void*, uint8_t*, uint16_t), int flags);
#define IPCNET_EVENT_DISABLE_INTR   (0)
#define IPCNET_EVENT_ENABLE_INTR    (1)

/* IPC-NET Socket Application Interface */
int ipcnet_socket();
int ipcnet_close(int fd);
int ipcnet_connect(int fd, uint16_t port);
int ipcnet_bind(int fd, uint16_t port);
int ipcnet_listen(int fd, int connections);
int ipcnet_accept(int fd);
int ipcnet_recv(int fd, void *buf, size_t count);
int ipcnet_send(int fd, void *buf, size_t count);
int ipcnet_errno(int fd);
int ipcnet_send_socket_event(int fd, void *buf, size_t count); /* count = 0 ~ 42 */
int ipcnet_register_socket_eventCB(int fd, void (*callback)(int, int, void*, size_t));
int ipcnet_SETTINGS_connect_timeout_ms(int fd, uint32_t wait_ack_timeout, uint32_t wait_port_timeout, uint32_t timeout);

/* IPC-NET I/O Control command */
#define IPCNET_FIONBIO      (0)
int ipcnet_ioctl(int fd, unsigned int cmd, unsigned long arg);

/* IPC-NET Set/Get Socket Option */
#define IPCNET_SO_SNDTIMEO  (0)
#define IPCNET_SO_RCVTIMEO  (1)
#define IPCNET_SO_LINGER    (2)
struct ipcnet_linger {
    int l_onoff;
    int l_linger;
};
int ipcnet_setsockopt(int s, int optname, const void *optval, size_t optlen);
int ipcnet_getsockopt(int s, int optname, void *optval, size_t *optlen);

/* IPC-NET Check Send Event Empty. */
inline bool ipcnet_check_event_empty() {
    return ipcfifo_first_frame_empty();
}

#ifdef __cplusplus
}
#endif

#endif
