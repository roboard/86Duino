#ifndef __WZ_ETHERCAT_MODULE_COMMON_H
#define __WZ_ETHERCAT_MODULE_COMMON_H

enum EcatAiRange {
    ADS8689_RANGE_N22_TO_P22_V = 0, // ADS8689: ±22V (default)
    ADS8689_RANGE_N20_TO_P20_V,     // ADS8689: ±20V
    ADS8689_RANGE_N11_TO_P11_V,     // ADS8689: ±11V
    ADS8689_RANGE_N10_TO_P10_V,     // ADS8689: ±10V
    ADS8689_RANGE_N5_TO_P5_V,       // ADS8689: ±5V
    ADS8689_RANGE_0_TO_22_V,        // ADS8689: 0~22V
    ADS8689_RANGE_0_TO_20_V,        // ADS8689: 0~20V
    ADS8689_RANGE_0_TO_11_V,        // ADS8689: 0~11V
    ADS8689_RANGE_0_TO_10_V         // ADS8689: 0~10V
};

enum EcatAoRange {
    DAC8771_RANGE_0_TO_5_V = 0,     // Voltage output 0 to +5 V (default)
    DAC8771_RANGE_0_TO_10_V,        // Voltage output 0 to +10 V
    DAC8771_RANGE_N5_TO_P5_V,       // Voltage output ±5 V
    DAC8771_RANGE_N10_TO_P10_V,     // Voltage output ±10 V
    DAC8771_RANGE_3p5_TO_23p5_mA,   // Current output 3.5 mA to 23.5 mA
    DAC8771_RANGE_0_TO_20_mA,       // Current output 0 to 20 mA
    DAC8771_RANGE_0_TO_24_mA,       // Current output 0 to 24 mA
    DAC8771_RANGE_N24_TO_P24_mA,    // Current output ±24 mA
    DAC8771_RANGE_0_TO_6_V,         // Voltage output 0 to +6 V
    DAC8771_RANGE_0_TO_12_V,        // Voltage output 0 to +12 V
    DAC8771_RANGE_N6_TO_P6_V,       // Voltage output ±6 V
    DAC8771_RANGE_N12_TO_P12_V,     // Voltage output ±12 V
    DAC8771_RANGE_4_TO_20_mA        // Current output 4 mA to 20 mA
};

enum EcatAoSlewRateClock {
    DAC8771_SR_CLK_258065_HZ = 0,
    DAC8771_SR_CLK_200000_HZ,
    DAC8771_SR_CLK_153845_HZ,
    DAC8771_SR_CLK_131145_HZ,
    DAC8771_SR_CLK_115940_HZ,
    DAC8771_SR_CLK_69565_HZ,
    DAC8771_SR_CLK_37560_HZ,
    DAC8771_SR_CLK_25805_HZ,
    DAC8771_SR_CLK_20150_HZ,
    DAC8771_SR_CLK_16030_HZ,
    DAC8771_SR_CLK_10295_HZ,
    DAC8771_SR_CLK_8280_HZ,
    DAC8771_SR_CLK_6900_HZ,
    DAC8771_SR_CLK_5530_HZ,
    DAC8771_SR_CLK_4240_HZ,
    DAC8771_SR_CLK_3300_HZ
};

enum EcatAoSlewRateStep {
    DAC8771_SR_STEP_1_LSB = 0,
    DAC8771_SR_STEP_2_LSB,
    DAC8771_SR_STEP_4_LSB,
    DAC8771_SR_STEP_8_LSB,
    DAC8771_SR_STEP_16_LSB,
    DAC8771_SR_STEP_32_LSB,
    DAC8771_SR_STEP_64_LSB,
    DAC8771_SR_STEP_128_LSB
};

enum EcatAoShortCircuitAction {
    DAC8771_SC_ACTION_NONE = 0,
    DAC8771_SC_ACTION_REPORT,
    DAC8771_SC_ACTION_DISABLE
};

enum EcatAoOpenCircuitAction {
    DAC8771_OC_ACTION_NONE = 0,
    DAC8771_OC_ACTION_REPORT,
    DAC8771_OC_ACTION_DISABLE
};

enum EcatAoOverTemperatureAction {
    DAC8771_OT_ACTION_NONE = 0,
    DAC8771_OT_ACTION_REPORT,
    DAC8771_OT_ACTION_DISABLE
};

#endif
