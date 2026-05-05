#ifndef __LSM6DSO_H
#define __LSM6DSO_H

#ifdef __cplusplus
extern "C" {
#endif

void *lsm6dso_init(int acc_addr, int gyro_addr, int magn_addr);
bool  lsm6dso_getraw(void *h, int *raw);
void  lsm6dso_raw2value(void *h, int *src, double *dst);
void  lsm6dso_close(void *h);

#ifdef __cplusplus
}
#endif

#endif