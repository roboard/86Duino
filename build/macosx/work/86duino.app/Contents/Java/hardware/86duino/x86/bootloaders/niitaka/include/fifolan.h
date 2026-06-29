#ifndef __FIFOLAN_H
#define __FIFOLAN_H

#include <stdlib.h>
#include <stdint.h>

#define FLAN_INTERNAL_FD    (0)

/* FIFO-LAN Socket Error Code */
#define FLAN_EWOULDBLOCK    (502) // Operation would block
#define FLAN_ETIMEDOUT      (503) // Timed out
#define FLAN_ETIMEDOUT1     (504) // Timed out 1
#define FLAN_ETIMEDOUT2     (505) // Timed out 2
#define FLAN_ENOMEM         (506) // Not enough space/cannot allocate memory
#define FLAN_EINVAL         (507) // Invalid argument
#define FLAN_EADDRINUSE     (508) // Address already in use
#define FLAN_ECONNABORTED   (509) // Connection aborted

/* FIFO-LAN Socket Application Interface */
int flan_initDriver();
int flan_closeDriver();
int flan_ping(uint16_t port);
int flan_socket();
int flan_close(int fd);
int flan_connect(int fd, uint16_t port);
int flan_bind(int fd, uint16_t port);
int flan_listen(int fd, int connections);
int flan_accept(int fd);
int flan_recv(int fd, void *buf, size_t count);
int flan_send(int fd, void *buf, size_t count);
int flan_send_first(int fd, void *buf, size_t count);
int flan_errno(int fd);
int flan_poll();

/* FIFO-LAN I/O Control command */
#define FLAN_FIONBIO        (0)
int flan_ioctl(int fd, unsigned int cmd, unsigned long arg);

/* FIFO-LAN Set/Get Socket Option */
#define FLAN_SO_SNDTIMEO    (0)
#define FLAN_SO_RCVTIMEO    (1)
#define FLAN_SO_LINGER      (3)
struct flan_linger {
    int l_onoff;
    int l_linger;
};
int flan_setsockopt(int s, int optname, const void *optval, size_t optlen);
int flan_getsockopt(int s, int optname, void *optval, size_t *optlen);

#endif
