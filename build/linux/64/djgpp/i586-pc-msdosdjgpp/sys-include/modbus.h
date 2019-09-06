/*
  modbus.h - Modbus Library for libmodbus-dmp
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

#ifndef __MODBUS_H
#define __MODBUS_H

#include <stdint.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef OFF
#define OFF 0
#endif

#ifndef ON
#define ON 1
#endif

/* The full version, in string form (suited for string concatenation)
 */
#define LIBMODBUS_VERSION_STRING "3.1.4"

/* Internal use */
#define MSG_LENGTH_UNDEFINED                (-1)

/* Modbus function codes */
#define MODBUS_FC_READ_COILS                (0x01)
#define MODBUS_FC_READ_DISCRETE_INPUTS      (0x02)
#define MODBUS_FC_READ_HOLDING_REGISTERS    (0x03)
#define MODBUS_FC_READ_INPUT_REGISTERS      (0x04)
#define MODBUS_FC_WRITE_SINGLE_COIL         (0x05)
#define MODBUS_FC_WRITE_SINGLE_REGISTER     (0x06)
#define MODBUS_FC_READ_EXCEPTION_STATUS     (0x07)
#define MODBUS_FC_WRITE_MULTIPLE_COILS      (0x0F)
#define MODBUS_FC_WRITE_MULTIPLE_REGISTERS  (0x10)
#define MODBUS_FC_REPORT_SLAVE_ID           (0x11)
#define MODBUS_FC_MASK_WRITE_REGISTER       (0x16)
#define MODBUS_FC_WRITE_AND_READ_REGISTERS  (0x17)
#define MODBUS_FC_SYNC_READ_REGISTERS       (0x65)
#define MODBUS_FC_SYNC_WRITE_REGISTERS      (0x66)

#define MODBUS_BROADCAST_ADDRESS            (0)

/* Modbus_Application_Protocol_V1_1b.pdf (chapter 6 section 1 page 12)
 * Quantity of Coils to read (2 bytes): 1 to 2000 (0x7D0)
 * (chapter 6 section 11 page 29)
 * Quantity of Coils to write (2 bytes): 1 to 1968 (0x7B0)
 */
#define MODBUS_MAX_READ_BITS                (2000)
#define MODBUS_MAX_WRITE_BITS               (1968)

/* Modbus_Application_Protocol_V1_1b.pdf (chapter 6 section 3 page 15)
 * Quantity of Registers to read (2 bytes): 1 to 125 (0x7D)
 * (chapter 6 section 12 page 31)
 * Quantity of Registers to write (2 bytes) 1 to 123 (0x7B)
 * (chapter 6 section 17 page 38)
 * Quantity of Registers to write in R/W registers (2 bytes) 1 to 121 (0x79)
 */
#define MODBUS_MAX_READ_REGISTERS           (125)
#define MODBUS_MAX_WRITE_REGISTERS          (123)
#define MODBUS_MAX_WR_WRITE_REGISTERS       (121)
#define MODBUS_MAX_WR_READ_REGISTERS        (125)

/* The size of the MODBUS PDU is limited by the size constraint inherited from
 * the first MODBUS implementation on Serial Line network (max. RS485 ADU = 256
 * bytes). Therefore, MODBUS PDU for serial line communication = 256 - Server
 * address (1 byte) - CRC (2 bytes) = 253 bytes.
 */
#define MODBUS_MAX_PDU_LENGTH               (253)

/* Consequently:
 * - RTU MODBUS ADU = 253 bytes + Server address (1 byte) + CRC (2 bytes) = 256
 *   bytes.
 * - TCP MODBUS ADU = 253 bytes + MBAP (7 bytes) = 260 bytes.
 * so the maximum of both backend in 260 bytes. This size can used to allocate
 * an array of bytes to store responses and it will be compatible with the two
 * backends.
 */
#define MODBUS_MAX_ADU_LENGTH               (260)

/* Max between RTU , ASCII, and TCP max adu length (so TCP) */
#define MAX_MESSAGE_LENGTH  (513)

typedef struct _modbus modbus_t;
typedef struct _modbus_mapping_private modbus_mapping_private_t;

typedef enum
{
    MODBUS_ERROR_RECOVERY_NONE          = 0,
    MODBUS_ERROR_RECOVERY_LINK          = (1<<1),
    MODBUS_ERROR_RECOVERY_PROTOCOL      = (1<<2)
} modbus_error_recovery_mode;

typedef struct {
    modbus_mapping_private_t *priv;
    int nb_bits;
    int start_bits;
    int nb_input_bits;
    int start_input_bits;
    int nb_input_registers;
    int start_input_registers;
    int nb_registers;
    int start_registers;
    uint16_t *tab_bits;
    uint16_t *tab_input_bits;
    uint16_t *tab_input_registers;
    uint16_t *tab_registers;
} modbus_mapping_t;

/*
 *  ---------- Request     Indication ----------
 *  | Client | ---------------------->| Server |
 *  ---------- Confirmation  Response ----------
 */
typedef enum {
    /* Request message on the server side */
    MSG_INDICATION,
    /* Request message on the client side */
    MSG_CONFIRMATION
} msg_type_t;

/* Random number to avoid errno conflicts */
#define MODBUS_ENOBASE 112345678

/* Protocol exceptions */
enum {
    MODBUS_EXCEPTION_ILLEGAL_FUNCTION = 0x01,
    MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS,
    MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE,
    MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE,
    MODBUS_EXCEPTION_ACKNOWLEDGE,
    MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY,
    MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE,
    MODBUS_EXCEPTION_MEMORY_PARITY,
    MODBUS_EXCEPTION_NOT_DEFINED,
    MODBUS_EXCEPTION_GATEWAY_PATH,
    MODBUS_EXCEPTION_GATEWAY_TARGET,
    MODBUS_EXCEPTION_MAX
};

#define EMBXILFUN  (MODBUS_ENOBASE + MODBUS_EXCEPTION_ILLEGAL_FUNCTION)
#define EMBXILADD  (MODBUS_ENOBASE + MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS)
#define EMBXILVAL  (MODBUS_ENOBASE + MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE)
#define EMBXSFAIL  (MODBUS_ENOBASE + MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE)
#define EMBXACK    (MODBUS_ENOBASE + MODBUS_EXCEPTION_ACKNOWLEDGE)
#define EMBXSBUSY  (MODBUS_ENOBASE + MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY)
#define EMBXNACK   (MODBUS_ENOBASE + MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE)
#define EMBXMEMPAR (MODBUS_ENOBASE + MODBUS_EXCEPTION_MEMORY_PARITY)
#define EMBXGPATH  (MODBUS_ENOBASE + MODBUS_EXCEPTION_GATEWAY_PATH)
#define EMBXGTAR   (MODBUS_ENOBASE + MODBUS_EXCEPTION_GATEWAY_TARGET)

/* Native libmodbus error codes */
#define EMBBADCRC  (EMBXGTAR + 1)
#define EMBBADDATA (EMBXGTAR + 2)
#define EMBBADEXC  (EMBXGTAR + 3)
#define EMBUNKEXC  (EMBXGTAR + 4)
#define EMBMDATA   (EMBXGTAR + 5)
#define EMBBADSLAVE (EMBXGTAR + 6)

int modbus_set_slave(modbus_t *ctx, int slave);
int modbus_set_error_recovery(modbus_t *ctx, modbus_error_recovery_mode error_recovery);
int modbus_set_socket(modbus_t *ctx, int s);
int modbus_get_socket(modbus_t *ctx);

int modbus_get_response_timeout(modbus_t *ctx, uint32_t *to_sec, uint32_t *to_usec);
int modbus_set_response_timeout(modbus_t *ctx, uint32_t to_sec, uint32_t to_usec);

int modbus_get_byte_timeout(modbus_t *ctx, uint32_t *to_sec, uint32_t *to_usec);
int modbus_set_byte_timeout(modbus_t *ctx, uint32_t to_sec, uint32_t to_usec);

int modbus_get_header_length(modbus_t *ctx);

int modbus_connect(modbus_t *ctx);
void modbus_close(modbus_t *ctx);
int modbus_connect_assign(modbus_t *ctx, void *port);

void modbus_free(modbus_t *ctx);

int modbus_flush(modbus_t *ctx);
int modbus_set_debug(modbus_t *ctx, int flag);

const char *modbus_strerror(int errnum);

int modbus_send_raw_request(modbus_t *ctx, uint8_t *raw_req, int raw_req_length);
int modbus_send_raw_response(modbus_t *ctx, uint8_t *raw_rsp, int raw_rsp_length, uint8_t *req, int req_length);

int modbus_receive_available(modbus_t *ctx);
int modbus_receive_reset(modbus_t *ctx);
int modbus_receive(modbus_t *ctx, uint8_t *req);
int modbus_receive_nowait(modbus_t *ctx, uint8_t *req);

int modbus_receive_confirmation(modbus_t *ctx, uint8_t *rsp);


/* Read Coils */
int modbus_read_bits(modbus_t *ctx, int addr, int nb, uint8_t *dest);
int modbus_read_bits_nowait(modbus_t *ctx, int addr, int nb);
int modbus_read_bits_completed(modbus_t *ctx, uint8_t *dest);

/* Read Discrete Inputs */
int modbus_read_input_bits(modbus_t *ctx, int addr, int nb, uint8_t *dest);
int modbus_read_input_bits_nowait(modbus_t *ctx, int addr, int nb);
int modbus_read_input_bits_completed(modbus_t *ctx, uint8_t *dest);

/* Read Holding Registers */
int modbus_read_registers(modbus_t *ctx, int addr, int nb, uint16_t *dest);
int modbus_read_registers_nowait(modbus_t *ctx, int addr, int nb);
int modbus_read_registers_completed(modbus_t *ctx, uint16_t *dest);

/* Read Input Registers */
int modbus_read_input_registers(modbus_t *ctx, int addr, int nb, uint16_t *dest);
int modbus_read_input_registers_nowait(modbus_t *ctx, int addr, int nb);
int modbus_read_input_registers_completed(modbus_t *ctx, uint16_t *dest);

/* Write Single Coil */
int modbus_write_bit(modbus_t *ctx, int coil_addr, int status);
int modbus_write_bit_nowait(modbus_t *ctx, int coil_addr, int status);
int modbus_write_bit_completed(modbus_t *ctx);

/* Write Single Register */
int modbus_write_register(modbus_t *ctx, int reg_addr, int value);
int modbus_write_register_nowait(modbus_t *ctx, int reg_addr, int value);
int modbus_write_register_completed(modbus_t *ctx);

/* Write Multiple Coils */
int modbus_write_bits(modbus_t *ctx, int addr, int nb, const uint8_t *data);
int modbus_write_bits_nowait(modbus_t *ctx, int addr, int nb, const uint8_t *data);
int modbus_write_bits_completed(modbus_t *ctx);

/* Write Multiple Registers */
int modbus_write_registers(modbus_t *ctx, int addr, int nb, const uint16_t *data);
int modbus_write_registers_nowait(modbus_t *ctx, int addr, int nb, const uint16_t *data);
int modbus_write_registers_completed(modbus_t *ctx);

/* Mask Write Register */
int modbus_mask_write_register(modbus_t *ctx, int addr, uint16_t and_mask, uint16_t or_mask);
int modbus_mask_write_register_nowait(modbus_t *ctx, int addr, uint16_t and_mask, uint16_t or_mask);
int modbus_mask_write_register_completed(modbus_t *ctx);

/* Read/Write Multiple Registers */
int modbus_write_and_read_registers(modbus_t *ctx, int write_addr, int write_nb,
                                    const uint16_t *src, int read_addr, int read_nb,
                                    uint16_t *dest);
int modbus_write_and_read_registers_nowait(modbus_t *ctx, int write_addr, int write_nb,
                                           const uint16_t *src, int read_addr, int read_nb);
int modbus_write_and_read_registers_completed(modbus_t *ctx, uint16_t *dest);

/* Report Server ID */
int modbus_report_slave_id(modbus_t *ctx, int max_dest, uint8_t *dest);
int modbus_report_slave_id_nowait(modbus_t *ctx);
int modbus_report_slave_id_completed(modbus_t *ctx, int max_dest, uint8_t *dest);

/* Sync Write Multiple Registers */
int modbus_sync_write_registers(modbus_t *ctx, int addr, int nb,
                                uint8_t *id_table, int id_num, const uint16_t *data);
int modbus_sync_write_registers_nowait(modbus_t *ctx, int addr, int nb,
                                       uint8_t *id_table, int id_num, const uint16_t *data);
int modbus_sync_write_registers_completed(modbus_t *ctx);

/* Sync Read Holding Registers */
int modbus_sync_read_registers(modbus_t *ctx, int addr, int nb,
                               uint8_t *id_table, int id_num, uint16_t *dest);
int modbus_sync_read_registers_nowait(modbus_t *ctx, int addr, int nb,
                                      uint8_t *id_table, int id_num);
int modbus_sync_read_registers_completed(modbus_t *ctx, uint16_t *dest);

modbus_mapping_t *modbus_mapping_create(void);
uint16_t *modbus_mapping_bits(modbus_mapping_t *mb_mapping,
                    unsigned int start_bits, unsigned int nb_bits);
uint16_t *modbus_mapping_input_bits(modbus_mapping_t *mb_mapping,
                    unsigned int start_input_bits, unsigned int nb_input_bits);
uint16_t *modbus_mapping_registers(modbus_mapping_t *mb_mapping,
                    unsigned int start_registers, unsigned int nb_registers);
uint16_t *modbus_mapping_input_registers(modbus_mapping_t *mb_mapping,
                    unsigned int start_input_registers, unsigned int nb_input_registers);
uint16_t *modbus_mapping_bits_assign(modbus_mapping_t *mb_mapping,
                    uint16_t *bits, unsigned int start_bits, unsigned int nb_bits);
uint16_t *modbus_mapping_input_bits_assign(modbus_mapping_t *mb_mapping,
                    uint16_t *input_bits, unsigned int start_input_bits, unsigned int nb_input_bits);
uint16_t *modbus_mapping_registers_assign(modbus_mapping_t *mb_mapping,
                    uint16_t *registers, unsigned int start_registers, unsigned int nb_registers);
uint16_t *modbus_mapping_input_registers_assign(modbus_mapping_t *mb_mapping,
                    uint16_t *input_registers, unsigned int start_input_registers, unsigned int nb_input_registers);
modbus_mapping_t *modbus_mapping_new_start_address(
    unsigned int start_bits, unsigned int nb_bits,
    unsigned int start_input_bits, unsigned int nb_input_bits,
    unsigned int start_registers, unsigned int nb_registers,
    unsigned int start_input_registers, unsigned int nb_input_registers);
modbus_mapping_t *modbus_mapping_new(int nb_bits, int nb_input_bits,
                                     int nb_registers, int nb_input_registers);
void modbus_mapping_free(modbus_mapping_t *mb_mapping);

int modbus_reply(modbus_t *ctx, const uint8_t *req,
                 int req_length, modbus_mapping_t *mb_mapping);
int modbus_reply_exception(modbus_t *ctx, const uint8_t *req, unsigned int exception_code);

int modbus_set_callback(modbus_t *ctx, int function, int (*callback)(int, int, int, void*), void *data);

int modbus_skip_slave(modbus_t *ctx, int skip);
int modbus_query_slave(modbus_t *ctx, uint8_t *req, int req_length);
int modbus_shrink_data_unit(modbus_t *ctx, uint8_t *req, int req_length, uint8_t *buf);
int modbus_ignore_confirmation(modbus_t *ctx);
int _modbus_receive_msg_nowait(modbus_t *ctx, uint8_t *msg, msg_type_t msg_type);

/**
 * UTILS FUNCTIONS
 **/

#define MODBUS_GET_HIGH_BYTE(data) (((data) >> 8) & 0xFF)
#define MODBUS_GET_LOW_BYTE(data) ((data) & 0xFF)
#define MODBUS_GET_INT64_FROM_INT16(tab_int16, index) \
    (((int64_t)tab_int16[(index)    ] << 48) + \
     ((int64_t)tab_int16[(index) + 1] << 32) + \
     ((int64_t)tab_int16[(index) + 2] << 16) + \
      (int64_t)tab_int16[(index) + 3])
#define MODBUS_GET_INT32_FROM_INT16(tab_int16, index) ((tab_int16[(index)] << 16) + tab_int16[(index) + 1])
#define MODBUS_GET_INT16_FROM_INT8(tab_int8, index) ((tab_int8[(index)] << 8) + tab_int8[(index) + 1])
#define MODBUS_SET_INT16_TO_INT8(tab_int8, index, value) \
    do { \
        tab_int8[(index)] = (value) >> 8;  \
        tab_int8[(index) + 1] = (value) & 0xFF; \
    } while (0)
#define MODBUS_SET_INT32_TO_INT16(tab_int16, index, value) \
    do { \
        tab_int16[(index)    ] = (value) >> 16; \
        tab_int16[(index) + 1] = (value); \
    } while (0)
#define MODBUS_SET_INT64_TO_INT16(tab_int16, index, value) \
    do { \
        tab_int16[(index)    ] = (value) >> 48; \
        tab_int16[(index) + 1] = (value) >> 32; \
        tab_int16[(index) + 2] = (value) >> 16; \
        tab_int16[(index) + 3] = (value); \
    } while (0)

void modbus_set_bits_from_byte(uint16_t *dest, int idx, const uint8_t value);
void modbus_set_bits_from_bytes(uint16_t *dest, int idx, unsigned int nb_bits,
                            const uint8_t *tab_byte);
uint8_t modbus_get_byte_from_bits(const uint8_t *src, int idx, unsigned int nb_bits);
float modbus_get_float(const uint16_t *src);
float modbus_get_float_abcd(const uint16_t *src);
float modbus_get_float_dcba(const uint16_t *src);
float modbus_get_float_badc(const uint16_t *src);
float modbus_get_float_cdab(const uint16_t *src);

void modbus_set_float(float f, uint16_t *dest);
void modbus_set_float_abcd(float f, uint16_t *dest);
void modbus_set_float_dcba(float f, uint16_t *dest);
void modbus_set_float_badc(float f, uint16_t *dest);
void modbus_set_float_cdab(float f, uint16_t *dest);

#include "modbus-rtu.h"
#include "modbus-ascii.h"
#include "modbus-tcp.h"

#endif
