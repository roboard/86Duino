#ifndef ___WZ_ETHERCAT_DEVICE_DMP_SENSOR_H
#define ___WZ_ETHERCAT_DEVICE_DMP_SENSOR_H

#include "EthercatDevice_DmpCommon.h"
#include <stdint.h>

class EthercatDevice_QS11RKIOGEDB;
class EthercatDevice_DmpSensor_Generic : public _EthercatDevice_DmpCommonDriver {

public:
    EthercatDevice_DmpSensor_Generic(uint32_t product = 0);

    int attach(uint16_t slave_id, EthercatMaster *master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int attach(uint16_t slave_id, EthercatMaster &master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int detach();

    /* ADS1260 */
    double getTemperatureFromADS1260();
    double getLocalTemperatureFromADS1260();

    /* ADS1118 */
    double getTemperatureFromADS1118(int ch);
    double getLocalTemperatureFromADS1118();

    /* BEM280 */
    double getHumidityFromBEM280();
    double getPressureFromBEM280();
    double getTemperatureFromBEM280();

    /* ADXL345 */
    int16_t getAccelerationFromADXL345(int axis);

    /* TMP117 */
    double getTemperatureFromTMP117();

    /* HDC2022 */
    double getTemperatureFromHDC2022();
    double getHumidityFromHDC2022();

    /* ADC8689 */
    int analogRead();

    /* DAC8771 */
    int analogWrite(uint16_t value);

};

class EthercatDevice_QS11RKIOGEDB : public EthercatDevice_DmpSensor_Generic {
public:
    EthercatDevice_QS11RKIOGEDB();
};

#endif
