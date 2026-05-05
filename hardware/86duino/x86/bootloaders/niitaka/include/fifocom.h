#ifndef __FIFOCOM_H
#define __FIFOCOM_H

#include <stdint.h>

int fcom_init();
void fcom_close();
void fcom_set_rx_timtout_ms(uint32_t timeout);
void fcom_set_tx_timtout_ms(uint32_t timeout);
void fcom_set_max_pool_size(uint32_t size);
void fcom_config(int config);
#define FCOM_NORMAL         (0) // If user does not receive until buffer is fulled,
                                // it will discard following data.
#define FCOM_DATA_NOT_LOSS  (1) // If master and slave system use write blocking function to send 
                                // large data more than Software RX Buffer size, they will wait to
                                // send data for each other.

bool fcom_rx_not_empty();
bool fcom_tx_empty();

int fcom_recv_nowait(uint8_t *buf, uint32_t len);
int fcom_recv(uint8_t *buf, uint32_t len);
int fcom_read_byte(uint8_t *pval);
int fcom_read_word(uint16_t *pval);
int fcom_read_dword(uint32_t *pval);

int fcom_send_nowait(uint8_t *buf, uint32_t len);
int fcom_send(uint8_t *buf, uint32_t len);
int fcom_write_byte(uint8_t val);
int fcom_write_word(uint16_t val);
int fcom_write_dword(uint32_t val);

#endif
