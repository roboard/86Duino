/*
  modbus-private.h - Modbus Library for libmodbus-dmp
  Copyright (c) 2018 Johnson Hung <Dyhung@dmp.com.tw>. All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  
  (libmodbus-dmp is derived from libmodbus: http://libmodbus.org/ )
*/

#ifndef __MODBUS_PRIVATE_H
#define __MODBUS_PRIVATE_H

#include "modbus.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>

#if defined(DJGPP)

#include <termios.h>
#include <netinet/in.h>
#include <stdbool.h>

#include "io.h"
#include "com.h"

#include <sws_cfg.h>
#include <sws_sock.h>
#define ECONNRESET      SWS_ECONNRESET
#define ECONNREFUSED    SWS_ECONNREFUSED
#define ETIMEDOUT       SWS_ETIMEDOUT
#define ENOPROTOOPT     SWS_ENOPROTOOPT

#elif defined(WIN32)

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define SHUT_RDWR SD_BOTH

typedef int ssize_t;

#else
    
#include <arpa/inet.h>

#include <linux/serial.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include <asm/termios.h>

#define closesocket close

#endif

/* It's not really the minimal length (the real one is report slave ID
 * in RTU (4 bytes)) but it's a convenient size to use in RTU or TCP
 * communications to read many values or write a single one.
 * Maximum between :
 * - HEADER_LENGTH_TCP (7) + function (1) + address (2) + number (2)
 * - HEADER_LENGTH_RTU (1) + function (1) + address (2) + number (2) + CRC (2)
 */
#define _MIN_REQ_LENGTH     (12)

#define _REPORT_SLAVE_ID    (180)
#define _RESPONSE_TIMEOUT   (500000)
#define _BYTE_TIMEOUT       (500000)

#define MODBUS_CB_FUNCTION_NUM              (11)
#define MODBUS_CB_READ_COILS                (0x00)
#define MODBUS_CB_READ_DISCRETE_INPUTS      (0x01)
#define MODBUS_CB_READ_HOLDING_REGISTERS    (0x02)
#define MODBUS_CB_READ_INPUT_REGISTERS      (0x03)
#define MODBUS_CB_WRITE_SINGLE_COIL         (0x04)
#define MODBUS_CB_WRITE_SINGLE_REGISTER     (0x05)
#define MODBUS_CB_WRITE_MULTIPLE_COILS      (0x06)
#define MODBUS_CB_WRITE_MULTIPLE_REGISTERS  (0x07)
#define MODBUS_CB_MASK_WRITE_REGISTER       (0x08)
#define MODBUS_CB_SYNC_READ_REGISTERS       (0x09)
#define MODBUS_CB_SYNC_WRITE_REGISTERS      (0x0A)

typedef enum {
    _MODBUS_BACKEND_TYPE_RTU=0,
    _MODBUS_BACKEND_TYPE_TCP,
    _MODBUS_BACKEND_TYPE_ASCII
} modbus_backend_type_t;

/* 3 steps are used to parse the query */
typedef enum {
    _STEP_FUNCTION,
    _STEP_META,
    _STEP_DATA
} _step_t;

/* This structure reduces the number of params in functions and so
 * optimizes the speed of execution (~ 37%). */
typedef struct _sft {
    int slave;
    int function;
    int t_id;
} sft_t;

typedef struct _modbus_backend {
    unsigned int backend_type;
    unsigned int header_length;
    unsigned int checksum_length;
    unsigned int max_adu_length;
    int (*set_slave) (modbus_t *ctx, int slave);
    int (*build_request_basis) (modbus_t *ctx, int function, int addr,
                                int nb, uint8_t *req);
    int (*build_response_basis) (sft_t *sft, uint8_t *rsp);
    int (*prepare_response_tid) (const uint8_t *req, int *req_length);
    int (*send_msg_pre) (uint8_t *req, int req_length);
    ssize_t (*send) (modbus_t *ctx, const uint8_t *req, int req_length);
    int (*receive) (modbus_t *ctx, uint8_t *req);
    ssize_t (*recv) (modbus_t *ctx, uint8_t *rsp, int rsp_length);
    int (*check_integrity) (modbus_t *ctx, uint8_t *msg,
                            const int msg_length);
    int (*pre_check_confirmation) (modbus_t *ctx, const uint8_t *req,
                                   const uint8_t *rsp, int rsp_length);
    int (*connect) (modbus_t *ctx);
    int (*connect_assign) (modbus_t *ctx, void *port);
    void (*close) (modbus_t *ctx);
    int (*flush) (modbus_t *ctx);
    void (*free) (modbus_t *ctx);
    int (*available) (modbus_t *ctx);
} modbus_backend_t;

typedef struct {
    int (*func)(int, int, int, void*);
    void *data;
} modbus_callback_t;

typedef struct _modbus {
    /* Slave address */
    int slave;
    /* Socket or file descriptor */
    int s;
    int debug;
    int error_recovery;
    struct timeval response_timeout;
    struct timeval byte_timeout;
    const modbus_backend_t *backend;
    void *backend_data;
    
    _step_t step;
    uint8_t req[MAX_MESSAGE_LENGTH];
    int length_to_read;
    int msg_length;
    int msg_length_backup;
    int nb;
    
    int id_table[256];
    int id_num;
    int id_count;
    int previous_id;
    int mapping_address;
    
    int rsp_length;
    uint8_t rsp[MAX_MESSAGE_LENGTH];
    
    /* To handle many slaves on the same link */
    int confirmation_to_ignore;
    
    int sync_read_state;
    
    uint8_t backup_msg[MAX_MESSAGE_LENGTH];
    int backup_num;
    int backup_count;
    
    int skip_check_slave;
    
    modbus_callback_t callback[MODBUS_CB_FUNCTION_NUM];
    
} modbus_t;

typedef struct _modbus_mapping_private {
    int bits_alloc;
    int input_bits_alloc;
    int input_registers_alloc;
    int registers_alloc;
} modbus_mapping_private_t;

void _modbus_init_common(modbus_t *ctx);
int _modbus_receive_msg_nowait(modbus_t *ctx, uint8_t *msg, msg_type_t msg_type);

#endif
