#include "lsm6dso.h"
#include "LSM6DSOSensor.h"
#include "Arduino.h"
#include "spi.h"

static int getRegVal(void* l, unsigned char type, unsigned char *pVal) {
    int i = -1, error = 0;
    int16_t Value[3];

    if (type == 0) {
        if (((LSM6DSOSensor*) l)->Get_X_AxesRaw(Value) == LSM6DSO_ERROR) error++;
    } else if (type == 1) {
        if (((LSM6DSOSensor*) l)->Get_G_AxesRaw(Value) == LSM6DSO_ERROR) error++;
    } else
        error = 1;

    if (error == 0) {
        uint8_t* d = (uint8_t*)Value;
        for (i=0; i<6; i++)
            pVal[i] = d[i];
    }

    return i;
}

void *lsm6dso_init(int acc_addr, int gyro_addr, int magn_addr)
{
    LSM6DSOSensor* lsm6dso = new LSM6DSOSensor(&SPI1, 99); // SPICS is not used. Because it use SPICS1 inside.

    lsm6dso->Enable_X();
    lsm6dso->Enable_G();

    return lsm6dso;
}

void  lsm6dso_close(void *h)
{
    if (h) delete (LSM6DSOSensor*)h;
}

bool lsm6dso_getraw(void *h, int *raw)
{
    LSM6DSOSensor *l = (LSM6DSOSensor *)h;
    unsigned char temp[12];
    short word[6];
    int error = 0;
    
    if (getRegVal(l, 0, &temp[0]) != 6) {
        error++;
    } else if (getRegVal(l, 1, &temp[6]) != 6) {
        error++;
    }
    
    if (error == 0) {
        
        word[0] = (short)temp[0]  | ((short)temp[1]  << 8);
        word[1] = (short)temp[2]  | ((short)temp[3]  << 8);
        word[2] = (short)temp[4]  | ((short)temp[5]  << 8);
        raw[0] = (int)word[0];
        raw[1] = (int)word[1];
        raw[2] = (int)word[2];
    
        word[3] = (short)temp[6]  | ((short)temp[7]  << 8);
        word[4] = (short)temp[8]  | ((short)temp[9]  << 8);
        word[5] = (short)temp[10] | ((short)temp[11] << 8);
        raw[3] = (int)word[3];
        raw[4] = (int)word[4];
        raw[5] = (int)word[5];
        
        return true;
        
    }
    
    raw[0] = 0;
    raw[1] = 0;
    raw[2] = 0;
    raw[3] = 0;
    raw[4] = 0;
    raw[5] = 0;
    
    return false;
}

void lsm6dso_raw2value(void *h, int *src, double *dst)
{
    LSM6DSOSensor *l = (LSM6DSOSensor *)h;
    int32_t Acceleration[3];
    int32_t AngularRate[3];
    
    if (l->Get_X_Axes(Acceleration) == LSM6DSO_OK) {
        dst[0] = (double)Acceleration[0];
        dst[1] = (double)Acceleration[1];
        dst[2] = (double)Acceleration[2];
    }

    if (l->Get_G_Axes(AngularRate) == LSM6DSO_OK) {
        dst[3] = (double)AngularRate[0];
        dst[4] = (double)AngularRate[1];
        dst[5] = (double)AngularRate[2];
    }
}
