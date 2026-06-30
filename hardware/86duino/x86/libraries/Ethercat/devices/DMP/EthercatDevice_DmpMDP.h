#ifndef ___WZ_ETHERCAT_DEVICE_DMP_MDP_H
#define ___WZ_ETHERCAT_DEVICE_DMP_MDP_H

#include "EthercatDevice_DmpCommon.h"
#include "modules/EthercatModule_AnalogInput.h"
#include "modules/EthercatModule_AnalogOutput.h"
#include "modules/EthercatModule_DigitalInput.h"
#include "modules/EthercatModule_DigitalOutput.h"
#include "modules/EthercatModule_SerialInterface.h"
#include <vector>

struct AiChannelMapping {
    EthercatModule_AnalogInput *module;
    int ch;
};

struct AoChannelMapping {
    EthercatModule_AnalogOutput *module;
    int ch;
};

struct DiChannelMapping {
    EthercatModule_DigitalInput *module;
    int ch;
};

struct DoChannelMapping {
    EthercatModule_DigitalOutput *module;
    int ch;
};

#define LOW     (0x00)
#define HIGH    (0x01)

class EthercatDevice_DmpMDP_Generic : public _EthercatDevice_DmpCommonDriver
{
public:
    EthercatDevice_DmpMDP_Generic();
    virtual ~EthercatDevice_DmpMDP_Generic();

    int attach(uint16_t slave_id, EthercatMaster *master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int attach(uint16_t slave_id, EthercatMaster &master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int detach();
    int update();
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
    EthercatModule_AnalogOutput *getAoModule(int module_index);
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
    EthercatModule_AnalogInput *getAiModule(int module_index);
    /*
        Map a global AI channel index to its physical module and local channel.
     */
    AiChannelMapping getAiChannelMapping(int ch);

    /*********************************************
     *               Digital Output              *
     *********************************************/
    /*
        Set the output state of a specific digital pin. 
        The 'value' parameter typically accepts 0 (LOW) or 1 (HIGH).
    */
    int digitalWrite(int ch, uint8_t value);
    /*
        Set the output states of all digital output pins simultaneously 
        using a bitmask. Each bit in 'value' represents a specific channel.
    */
    int digitalWriteAll(uint32_t value);
    /*
        Get the total number of DO channels across all modules.
    */
    int getDoChannelCount() const;
    /*
        Get the number of Digital Output modules currently attached to the device.
    */
    int getDoModuleCount() const;
    /*
        Retrieve a specific DO module by its index.
    */
    EthercatModule_DigitalOutput *getDoModule(int module_index);
    /*
        Map a global DO channel index to its physical module and local channel.
     */
    DoChannelMapping getDoChannelMapping(int ch);

    /*********************************************
     *               Digital Input               *
     *********************************************/
    /*
        Read the logical state of a specific digital input pin.
        Returns 0 (LOW) or 1 (HIGH).
    */
    int digitalRead(int ch);
    /*
        Read the states of all digital input pins at once.
        The return value is a bitmask where each bit represents the state of a channel.
    */
    uint32_t digitalReadAll();
    /*
        Get the total number of DI channels across all modules.
    */
    int getDiChannelCount() const;
    /*
        Get the number of Digital Input modules currently attached to the device.
    */
    int getDiModuleCount() const;
    /*
        Retrieve a specific DI module by its index.
    */
    EthercatModule_DigitalInput *getDiModule(int module_index);
    /*
        Map a global DI channel index to its physical module and local channel.
     */
    DiChannelMapping getDiChannelMapping(int ch);

    /*********************************************
     *              Serial Interface             *
     *********************************************/
    /*
        Configure the baud rate for the specified UART port.
        Returns 0 on success, or an error code.
    */
    int uartSetBaud(int dev, uint32_t baud);
    /*
        Configure the UART frame format (data bits, stop bits, parity).
        Returns 0 on success, or an error code.
    */
    int uartSetFormat(int dev, uint8_t format);
    /*
        Configure the flow control mode (e.g., ECAT_UART_NO_CONTROL, ECAT_UART_RTS_CTS).
        Returns 0 on success, or an error code.
    */
    int uartSetFlowControl(int dev, int control);
    /*
        Clear both the hardware TX and RX FIFOs.
        Returns 0 on success, or an error code.
    */
    int uartClearFIFO(int dev);
    /*
        Check if the specified UART port is in RS485 mode.
        Returns 1 if in RS485 mode, 0 if not, or an error code.
    */
    int uartIs485(int dev);
    /* Get the current state of the RTS (Request to Send) signal. */
    int uartGetRTS(int dev);
    /* Get the current state of the CTS (Clear to Send) signal. */
    int uartGetCTS(int dev);
    /* Get the current state of the DTR (Data Terminal Ready) signal. */
    int uartGetDTR(int dev);
    /* Get the current state of the DSR (Data Set Ready) signal. */
    int uartGetDSR(int dev);
    /*
        Set the RTS signal state. 
        Value 0: Inactive; 1-255: Active.
    */
    int uartSetRTS(int dev, uint8_t value);
    /*
        Set the DTR signal state. 
        Value 0: Inactive; 1-255: Active.
    */
    int uartSetDTR(int dev, uint8_t value);
    /*
        Transmit multiple bytes to the UART port.
        Returns the number of bytes sent, or an error code.
    */
    int uartSend(int dev, void *buf, size_t size);
    /*
        Write a single byte to the UART port.
        Returns 1 if successful, or an error code.
    */
    int uartWrite(int dev, uint8_t value);
    /*
        Receive multiple bytes from the UART port.
        Returns the number of bytes received, or an error code.
    */
    int uartReceive(int dev, void *buf, size_t size);
    /*
        Read a single byte from the UART port.
        Returns the byte value (0-255), or an error code.
    */
    int uartRead(int dev);
    /* Clear the software Transmit (TX) Queue. */
    int uartClearTxQueue(int dev);
    /* Clear the software Receive (RX) Queue. */
    int uartClearRxQueue(int dev);
    /* Get the number of bytes currently in the software TX Queue. */
    int uartQueryTxQueue(int dev);
    /* Get the number of bytes currently available in the software RX Queue. */
    int uartQueryRxQueue(int dev);
    /* Check if the software TX Queue is empty (Returns 1 if empty). */
    int uartTxQueueEmpty(int dev);
    /* Check if the software RX Queue is empty (Returns 1 if empty). */
    int uartRxQueueEmpty(int dev);
    /* Check if the software TX Queue is full (Returns 1 if full). */
    int uartTxQueueFull(int dev);
    /* Check if the software RX Queue is full (Returns 1 if full). */
    int uartRxQueueFull(int dev);
    /*
        Get the number of Serial Interface modules currently attached to the device.
    */
    int getUartModuleCount() const;
    /*
        Retrieve a specific Serial Interface module by its index.
    */
    EthercatModule_SerialInterface *getUartModule(int module_index);

private:
    std::vector<AiChannelMapping> _ai_lookup_table;
    std::vector<AoChannelMapping> _ao_lookup_table;
    std::vector<DiChannelMapping> _di_lookup_table;
    std::vector<DoChannelMapping> _do_lookup_table;
    std::vector<EthercatModule_AnalogInput *> _ai_modules;
    std::vector<EthercatModule_AnalogOutput *> _ao_modules;
    std::vector<EthercatModule_DigitalInput *> _di_modules;
    std::vector<EthercatModule_DigitalOutput *> _do_modules;
    std::vector<EthercatModule_SerialInterface *> _uart_modules;
};

#endif
