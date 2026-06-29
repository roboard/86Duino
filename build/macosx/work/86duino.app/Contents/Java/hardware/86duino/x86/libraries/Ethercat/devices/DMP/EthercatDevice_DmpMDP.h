#ifndef ___WZ_ETHERCAT_DEVICE_DMP_MDP_H
#define ___WZ_ETHERCAT_DEVICE_DMP_MDP_H

#include "EthercatDevice_DmpCommon.h"
#include "modules/EthercatModule_AnalogInput.h"
#include "modules/EthercatModule_AnalogOutput.h"
#include <vector>

struct AiChannelMapping {
    EthercatModule_AnalogInput* module;
    int ch;
};

struct AoChannelMapping {
    EthercatModule_AnalogOutput* module;
    int ch;
};

class EthercatDevice_DmpMDP_Generic : public _EthercatDevice_DmpCommonDriver
{
public:
    EthercatDevice_DmpMDP_Generic();
    virtual ~EthercatDevice_DmpMDP_Generic();

    int attach(uint16_t slave_id, EthercatMaster *master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int attach(uint16_t slave_id, EthercatMaster &master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int detach();
    int remapping() override;
    int unmapping() override;

    /*********************************************
     *               Analog Output               *
     *********************************************/
    /*
        This function is an analog output function. The parameter 'value'
        is an int32_t type with a range from -2^31 to 2^31, which mapping
        range is as follows:

        - Voltage Mode: Maps to a voltage range of -64V to 64V.
        - Current Mode: Maps to a current range of -16A to 16A.
    */
    int analogWrite(int ch, int32_t value);
    /*
        This function is used for voltage output mode, and the parameter
        'voltage' is a double type, representing the desired output voltage
        value in volts (V).
    */
    int voltageWrite(int ch, double voltage);
    /*
        This function is used for current output mode, and the parameter
        'current' is a double type, representing the desired output current
        value in amperes (A).
    */
    int currentWrite(int ch, double current);
    /*
        Get the total number of AO channels across all modules.
     */
    int getAoChannelCount() const;
    /*
        Get the number of Analog Output modules currently attached to the device.
     */
    int getAoModuleCount() const;
    /*
        Retrieve a specific AO module by its index.
     */
    EthercatModule_AnalogOutput* getAoModule(int module_index);
    /*
        Map a global AO channel index to its physical module and local channel.
     */
    AoChannelMapping getAoChannelMapping(int ch);

    /*********************************************
     *                Analog Input               *
     *********************************************/
    /*
        This function maps an analog voltage input ranging from -64V to 64V to
        an integer range of -2^31 to 2^31.
    */
    int analogRead(int ch);
    /*
        The function returns the voltage value as a double, eliminating the
        necessity for any mapping process.
    */
    double voltageRead(int ch);
    /*
        Get the total number of AI channels across all modules.
     */
    int getAiChannelCount() const;
    /*
        Get the number of Analog Input modules currently attached to the device.
     */
    int getAiModuleCount() const;
    /*
        Retrieve a specific AI module by its index.
     */
    EthercatModule_AnalogInput* getAiModule(int module_index);
    /*
        Map a global AI channel index to its physical module and local channel.
     */
    AiChannelMapping getAiChannelMapping(int ch);

private:
    std::vector<AiChannelMapping> _ai_lookup_table;
    std::vector<AoChannelMapping> _ao_lookup_table;
    std::vector<EthercatModule_AnalogInput*> _ai_modules;
    std::vector<EthercatModule_AnalogOutput*> _ao_modules;
};

#endif
