#include "myeva.h"

#include "utility/tinyxml2.h"

using namespace tinyxml2;

#define UNSUPPORT        (0)
#define SLAVE_DIQ        (1)
#define SLAVE_HID        (2)
#define SLAVE_LCD        (3)
#define SLAVE_OPF        (4)
#define SLAVE_MOTOR      (5)
#define SLAVE_MONITOR    (6)
#define SLAVE_SENSOR     (7)
#define SLAVE_AIQ        (8)

// DD List Function SubType
#define FS_COUNTER   (0)
#define FS_VOLTAGE   (1)
#define FS_ANALOG    (2)

#define CIA402_TYPE  (10)
#define _3AXIS_TYPE  (11)

extern char* userInternalResourcePath;
char eniPath[256];
char evaPath[256];

typedef struct _CiA402Data {
    unsigned long cia402_PPR[3];
    int cia402_homingMode[3];
    double cia402_homing_v[3];
    double cia402_homing_acc[3];
    double cia402_max_v[3];
    double cia402_max_acc[3];
    double cia402_max_dec[3];
    double cia402_default_v[3];
    double cia402_default_acc[3];
} CiA402Data;

typedef struct {

    char       *ObjectName;
    uint32_t    VendorId;
    uint32_t    ProductCode;
    uint16_t    AliasId;
    uint16_t    SlaveId;
    bool        haveMCU;
    double      Is;
    double      Ip;
    uint32_t    MaxSerial;
    uint32_t    MaxLcm;
    uint32_t    MaxMpg;
    uint32_t    MaxKeypad;
    uint32_t    MaxBuzzerPin;
    uint32_t    MaxLcd;
    uint32_t    MaxMicrophone;
    uint32_t    MaxSpeaker;
    uint32_t    MaxCounterPin;
    uint32_t    MaxVoltagePin;
    uint32_t    MaxMachine;
    uint32_t    MaxServo;
    uint32_t    MaxEncoder;
    uint32_t    MaxEMSensor;
    uint32_t    MaxForceSensor;
    uint32_t    MaxDO;
    uint32_t    MaxDI;
    uint32_t    MaxAO;
    uint32_t    MaxAI;
    int         Type;
    int        *MappingSerial;
    int        *MappingLcm;
    int        *MappingMpg;
    int        *MappingKeypad;
    int        *MappingBuzzerPin;
    int        *MappingLcd;
    int        *MappingMicrophone;
    int        *MappingSpeaker;
    int        *MappingCounterPin;
    int        *MappingVoltagePin;
    int        *MappingMachine;
    int        *MappingServo;
    int        *MappingEncoder;
    int        *MappingEMSensor;
    int        *MappingForceSensor;
    int        *MappingDO;
    int        *MappingDI;
    int        *MappingAO;
    int        *MappingAI;
    void       *PrivateObj;
    void       *PrivateData;

} _MyEcatSlaveSetting;

typedef struct myecat_setting {

    char *ObjectName;
    char *ENIName;
    bool Redundant;
    bool IgnoreOverride;
    bool SyncSafeOP;
    bool AutoRestart;
    int  DcMode;
    uint64_t CycleTime;
    uint32_t MaxDigitalPins;
    uint32_t MaxLcds;
    uint32_t MaxSerials;
    uint32_t MaxLcms;
    uint32_t MaxMpgs;
    uint32_t MaxBuzzerPins;
    uint32_t MaxKeypads;
    uint32_t MaxIntegerDataPins;
    uint32_t MaxMicrophones;
    uint32_t MaxSpeakers;
    uint32_t MaxDecimalDataPins;
    uint32_t MaxMachines;
    uint32_t MaxEncoders;
    uint32_t MaxServos;
    uint32_t MaxEMSensors;
    uint32_t MaxForceSensors;
    uint16_t SlaveCount;
    _MyEcatSlaveSetting *Slave;

} _MyEcatSetting;

static inline int getBoolAttribute(XMLElement *root, const char *token, void *pValue)
{
    return (root->QueryBoolAttribute(token, (bool*)pValue) > 0) ? 0 : 1;
}

static inline int getIntAttribute(XMLElement *root, const char *token, void *pValue)
{
    return (root->QueryIntAttribute(token, (int*)pValue) > 0) ? 0 : 1;
}


static inline int getHexDecValue(XMLElement *root, void *pValue, size_t size)
{
    int64_t scan;
    int rc;

    if (size > sizeof(scan))
        return -1;

    rc = sscanf(root->GetText(), "%lld", (int64_t *)&scan);
    if (rc <= 0) {
        rc = sscanf(root->GetText(), "#x%llX", (int64_t *)&scan);
        if (rc <= 0)
            return -1;
    }

    if (sizeof(scan) < sizeof(uint32_t) && (scan & (0xFFFFFFFFULL << (8 * size))))
        return -1;

    memcpy(pValue, &scan, size);
    
    return 1;
}

static inline int getHexDecValueElement(XMLElement *root, const char *token, void *pValue, size_t size)
{
    XMLElement *element;

    element = root->FirstChildElement(token);
    if (!element)
        return 0;

    return getHexDecValue(element, pValue, size);
}

static inline int getDoubleValue(XMLElement* root, void* pValue, size_t size)
{
    double scan;
    int rc;

    if (size > sizeof(scan))
        return -1;

    rc = sscanf(root->GetText(), "%lf", (double*)&scan);
    if (rc <= 0)
        return -1;

    memcpy(pValue, &scan, size);

    return 1;
}

static inline int getStringElement(XMLElement *root, const char *token, char **pString)
{
    XMLElement *element;
    int len;

    element = root->FirstChildElement(token);
    if (!element || element->GetText() == NULL || (len = strlen(element->GetText())) == 0)
        return 0;

    pString[0] = (char*)malloc(len + 1);
    if (pString[0] == NULL)
        return -1;

    if (element)
        strncpy(pString[0], element->GetText(), len);
    pString[0][len] = '\0';
    
    return 1;
}

static inline void __release_slave(_MyEcatSlaveSetting *slave)
{
    if (slave->Type == SLAVE_DIQ) {
        if (slave->MappingDO) free(slave->MappingDO);
        if (slave->MappingDI) free(slave->MappingDI);
    } else if (slave->Type == SLAVE_HID) {
        if (slave->MappingSerial) free(slave->MappingSerial);
        if (slave->MappingLcm) free(slave->MappingLcm);
        if (slave->MappingMpg) free(slave->MappingMpg);
        if (slave->MappingKeypad) free(slave->MappingKeypad);
        if (slave->MappingBuzzerPin) free(slave->MappingBuzzerPin);
    }  else if (slave->Type == SLAVE_MONITOR) {
        if (slave->MappingMicrophone) free(slave->MappingMicrophone);
        if (slave->MappingSpeaker) free(slave->MappingSpeaker);
        if (slave->MappingCounterPin) free(slave->MappingCounterPin);
        if (slave->MappingVoltagePin) free(slave->MappingVoltagePin);
    } else if (slave->Type == SLAVE_LCD) {
        if (slave->MappingLcd) free(slave->MappingLcd);
    } else if (slave->Type == SLAVE_MOTOR) {
        if (slave->MappingMachine) free(slave->MappingMachine);
        if (slave->MappingServo) free(slave->MappingServo);
        if (slave->MappingEncoder) free(slave->MappingEncoder);
    } else if (slave->Type == SLAVE_SENSOR) {
        if (slave->MappingEMSensor) free(slave->MappingEMSensor);
        if (slave->MappingForceSensor) free(slave->MappingForceSensor);
    } else if (slave->Type == SLAVE_AIQ) {
        if (slave->MappingAO) free(slave->MappingAO);
        if (slave->MappingAI) free(slave->MappingAI);
    }
    
    if (slave->ObjectName)
        free(slave->ObjectName);
    
    slave->ObjectName = NULL;
    slave->MappingDO = NULL;
    slave->MappingDI = NULL;
}

static inline void __release_master(_MyEcatSetting *master)
{
    if (master->ObjectName)
        free(master->ObjectName);
    if (master->ENIName)
        free(master->ENIName);
    master->ENIName = NULL;
}

static inline int __parse_diq_mapping(XMLElement *root, _MyEcatSlaveSetting *setting)
{
    XMLElement *element;
    int i;

    /* Digital Output. */
    setting->MaxDO = 0;
    element = root->FirstChildElement("DigitalOutput");
    while (element) {
        setting->MaxDO++;
        element = element->NextSiblingElement("DigitalOutput");
    }
    setting->MappingDO = (int *)malloc(sizeof(int) * setting->MaxDO);
    if (setting->MappingDO == NULL)
        return -1;
    i = 0;
    element = root->FirstChildElement("DigitalOutput");
    while (element) {
        if (getHexDecValue(element, &setting->MappingDO[i++], sizeof(*setting->MappingDO)) <= 0)
            return -1;
        element = element->NextSiblingElement("DigitalOutput");
    }

    /* Digital Input. */
    setting->MaxDI = 0;
    element = root->FirstChildElement("DigitalInput");
    while (element) {
        setting->MaxDI++;
        element = element->NextSiblingElement("DigitalInput");
    }
    setting->MappingDI = (int *)malloc(sizeof(int) * setting->MaxDI);
    if (setting->MappingDI == NULL)
        return -1;
    i = 0;
    element = root->FirstChildElement("DigitalInput");
    while (element) {
        if (getHexDecValue(element, &setting->MappingDI[i++], sizeof(*setting->MappingDI)) <= 0)
            return -1;
        element = element->NextSiblingElement("DigitalInput");
    }
    return 0;
}

static inline int __parse_lcd_mapping(XMLElement *root, _MyEcatSlaveSetting *setting)
{
    XMLElement *element;
    int i;

    /* Digital Output. */
    setting->MaxLcd = 0;
    element = root->FirstChildElement("Lcd24Number");
    while (element) {
        setting->MaxLcd++;
        element = element->NextSiblingElement("Lcd24Number");
    }
    setting->MappingLcd = (int *)malloc(sizeof(int) * setting->MaxLcd);
    if (setting->MappingLcd == NULL)
        return -1;
    i = 0;
    element = root->FirstChildElement("Lcd24Number");
    while (element) {
        if (getHexDecValue(element, &setting->MappingLcd[i++], sizeof(*setting->MappingLcd)) <= 0)
            return -1;
        element = element->NextSiblingElement("Lcd24Number");
    }

    return 0;
}

static inline int __parse_hid_mapping(XMLElement *root, _MyEcatSlaveSetting *setting)
{
    XMLElement *element;
    int i;

    // Serial
    setting->MaxSerial = 0;
    element = root->FirstChildElement("SerialPortNumber");
    while (element) {
        setting->MaxSerial++;
        element = element->NextSiblingElement("SerialPortNumber");
    }
    setting->MappingSerial = (int *)malloc(sizeof(int) * setting->MaxSerial);
    if (setting->MappingSerial == NULL)
        return -1;
    i = 0;
    element = root->FirstChildElement("SerialPortNumber");
    while (element) {
        if (getHexDecValue(element, &setting->MappingSerial[i++], sizeof(*setting->MappingSerial)) <= 0)
            return -1;
        element = element->NextSiblingElement("SerialPortNumber");
    }
    
    // Lcm
    setting->MaxLcm = 0;
    element = root->FirstChildElement("LcmNumber");
    while (element) {
        setting->MaxLcm++;
        element = element->NextSiblingElement("LcmNumber");
    }
    setting->MappingLcm = (int *)malloc(sizeof(int) * setting->MaxLcm);
    if (setting->MappingLcm == NULL)
        return -1;
    i = 0;
    element = root->FirstChildElement("LcmNumber");
    while (element) {
        if (getHexDecValue(element, &setting->MappingLcm[i++], sizeof(*setting->MappingLcm)) <= 0)
            return -1;
        element = element->NextSiblingElement("LcmNumber");
    }
    
    // Mpg
    setting->MaxMpg = 0;
    element = root->FirstChildElement("MpgNumber");
    while (element) {
        setting->MaxMpg++;
        element = element->NextSiblingElement("MpgNumber");
    }
    setting->MappingMpg = (int *)malloc(sizeof(int) * setting->MaxMpg);
    if (setting->MappingMpg == NULL)
        return -1;
    i = 0;
    element = root->FirstChildElement("MpgNumber");
    while (element) {
        if (getHexDecValue(element, &setting->MappingMpg[i++], sizeof(*setting->MappingMpg)) <= 0)
            return -1;
        element = element->NextSiblingElement("MpgNumber");
    }
    
    // Keypad
    setting->MaxKeypad = 0;
    element = root->FirstChildElement("KeypadNumber");
    while (element) {
        setting->MaxKeypad++;
        element = element->NextSiblingElement("KeypadNumber");
    }
    setting->MappingKeypad = (int *)malloc(sizeof(int) * setting->MaxKeypad);
    if (setting->MappingKeypad == NULL)
        return -1;
    i = 0;
    element = root->FirstChildElement("KeypadNumber");
    while (element) {
        if (getHexDecValue(element, &setting->MappingKeypad[i++], sizeof(*setting->MappingKeypad)) <= 0)
            return -1;
        element = element->NextSiblingElement("KeypadNumber");
    }
    
    // Buzzer
    setting->MaxBuzzerPin = 0;
    element = root->FirstChildElement("BuzzerPinNumber");
    while (element) {
        setting->MaxBuzzerPin++;
        element = element->NextSiblingElement("BuzzerPinNumber");
    }
    setting->MappingBuzzerPin = (int *)malloc(sizeof(int) * setting->MaxBuzzerPin);
    if (setting->MappingBuzzerPin == NULL)
        return -1;
    i = 0;
    element = root->FirstChildElement("BuzzerPinNumber");
    while (element) {
        if (getHexDecValue(element, &setting->MappingBuzzerPin[i++], sizeof(*setting->MappingBuzzerPin)) <= 0)
            return -1;
        element = element->NextSiblingElement("BuzzerPinNumber");
    }

    return 0;
}

static inline int __parse_monitor_mapping(XMLElement *root, _MyEcatSlaveSetting *setting)
{
    XMLElement *element;
    int i;

    // Microphone
    setting->MaxMicrophone = 0;
    element = root->FirstChildElement("MicrophoneNumber");
    while (element) {
        setting->MaxMicrophone++;
        element = element->NextSiblingElement("MicrophoneNumber");
    }
    setting->MappingMicrophone = (int *)malloc(sizeof(int) * setting->MaxMicrophone);
    if (setting->MappingMicrophone == NULL)
        return -1;
    i = 0;
    element = root->FirstChildElement("MicrophoneNumber");
    while (element) {
        if (getHexDecValue(element, &setting->MappingMicrophone[i++], sizeof(*setting->MappingMicrophone)) <= 0)
            return -1;
        element = element->NextSiblingElement("MicrophoneNumber");
    }
    
    // Speaker
    setting->MaxSpeaker = 0;
    element = root->FirstChildElement("SpeakerNumber");
    while (element) {
        setting->MaxSpeaker++;
        element = element->NextSiblingElement("SpeakerNumber");
    }
    setting->MappingSpeaker = (int *)malloc(sizeof(int) * setting->MaxSpeaker);
    if (setting->MappingSpeaker == NULL)
        return -1;
    i = 0;
    element = root->FirstChildElement("SpeakerNumber");
    while (element) {
        if (getHexDecValue(element, &setting->MappingSpeaker[i++], sizeof(*setting->MappingSpeaker)) <= 0)
            return -1;
        element = element->NextSiblingElement("SpeakerNumber");
    }
    
    // Counter Pin
    setting->MaxCounterPin = 0;
    element = root->FirstChildElement("CounterPinNumber");
    while (element) {
        setting->MaxCounterPin++;
        element = element->NextSiblingElement("CounterPinNumber");
    }
    setting->MappingCounterPin = (int *)malloc(sizeof(int) * setting->MaxCounterPin);
    if (setting->MappingCounterPin == NULL)
        return -1;
    i = 0;
    element = root->FirstChildElement("CounterPinNumber");
    while (element) {
        if (getHexDecValue(element, &setting->MappingCounterPin[i++], sizeof(*setting->MappingCounterPin)) <= 0)
            return -1;
        element = element->NextSiblingElement("CounterPinNumber");
    }
    
    // Voltage Pin
    setting->MaxVoltagePin = 0;
    element = root->FirstChildElement("VoltagePinNumber");
    while (element) {
        setting->MaxVoltagePin++;
        element = element->NextSiblingElement("VoltagePinNumber");
    }
    setting->MappingVoltagePin = (int *)malloc(sizeof(int) * setting->MaxVoltagePin);
    if (setting->MappingVoltagePin == NULL)
        return -1;
    i = 0;
    element = root->FirstChildElement("VoltagePinNumber");
    while (element) {
        if (getHexDecValue(element, &setting->MappingVoltagePin[i++], sizeof(*setting->MappingVoltagePin)) <= 0)
            return -1;
        element = element->NextSiblingElement("VoltagePinNumber");
    }
    
    return 0;
}

static inline int __parse_motor_mapping(XMLElement *root, _MyEcatSlaveSetting *setting)
{
    XMLElement *element;
    int i;

    setting->MaxMachine = 0;
    element = root->FirstChildElement("MachineNumber");
    while (element) {
        setting->MaxMachine++;
        element = element->NextSiblingElement("MachineNumber");
    }
    setting->MappingMachine = (int *)malloc(sizeof(int) * setting->MaxMachine);
    if (setting->MappingMachine == NULL)
        return -1;
    i = 0;
    element = root->FirstChildElement("MachineNumber");
    while (element) {
        if (getHexDecValue(element, &setting->MappingMachine[i++], sizeof(*setting->MappingMachine)) <= 0)
            return -1;
        element = element->NextSiblingElement("MachineNumber");
    }
    
    setting->MaxServo = 0;
    element = root->FirstChildElement("ServoNumber");
    while (element) {
        setting->MaxServo++;
        element = element->NextSiblingElement("ServoNumber");
    }
    setting->MappingServo = (int *)malloc(sizeof(int) * setting->MaxServo);
    if (setting->MappingServo == NULL)
        return -1;
    i = 0;
    element = root->FirstChildElement("ServoNumber");
    while (element) {
        if (getHexDecValue(element, &setting->MappingServo[i++], sizeof(*setting->MappingServo)) <= 0)
            return -1;
        element = element->NextSiblingElement("ServoNumber");
    }
    
    setting->MaxEncoder = 0;
    element = root->FirstChildElement("EncoderNumber");
    while (element) {
        setting->MaxEncoder++;
        element = element->NextSiblingElement("EncoderNumber");
    }
    setting->MappingEncoder = (int *)malloc(sizeof(int) * setting->MaxEncoder);
    if (setting->MappingEncoder == NULL)
        return -1;
    i = 0;
    element = root->FirstChildElement("EncoderNumber");
    while (element) {
        if (getHexDecValue(element, &setting->MappingEncoder[i++], sizeof(*setting->MappingEncoder)) <= 0)
            return -1;
        element = element->NextSiblingElement("EncoderNumber");
    }
    
    return 0;
}

static inline int __parse_sensor_mapping(XMLElement *root, _MyEcatSlaveSetting *setting)
{
    XMLElement *element;
    int i;

    /* EM Sensor. */
    setting->MaxEMSensor = 0;
    element = root->FirstChildElement("EMSensorNumber");
    while (element) {
        setting->MaxEMSensor++;
        element = element->NextSiblingElement("EMSensorNumber");
    }
    setting->MappingEMSensor = (int *)malloc(sizeof(int) * setting->MaxEMSensor);
    if (setting->MappingEMSensor == NULL)
        return -1;
    i = 0;
    element = root->FirstChildElement("EMSensorNumber");
    while (element) {
        if (getHexDecValue(element, &setting->MappingEMSensor[i++], sizeof(*setting->MappingEMSensor)) <= 0)
            return -1;
        element = element->NextSiblingElement("EMSensorNumber");
    }
    
    /* Force Sensor. */
    setting->MaxForceSensor = 0;
    element = root->FirstChildElement("ForceSensorNumber");
    while (element) {
        setting->MaxForceSensor++;
        element = element->NextSiblingElement("ForceSensorNumber");
    }
    setting->MappingForceSensor = (int *)malloc(sizeof(int) * setting->MaxForceSensor);
    if (setting->MappingForceSensor == NULL)
        return -1;
    i = 0;
    element = root->FirstChildElement("ForceSensorNumber");
    while (element) {
        if (getHexDecValue(element, &setting->MappingForceSensor[i++], sizeof(*setting->MappingForceSensor)) <= 0)
            return -1;
        element = element->NextSiblingElement("ForceSensorNumber");
    }

    return 0;
}

static inline int __parse_aiq_mapping(XMLElement *root, _MyEcatSlaveSetting *setting)
{
    XMLElement *element;
    int i;

    /* Digital Output. */
    setting->MaxAO = 0;
    element = root->FirstChildElement("AnalogOutput");
    while (element) {
        setting->MaxAO++;
        element = element->NextSiblingElement("AnalogOutput");
    }
    setting->MappingAO = (int *)malloc(sizeof(int) * setting->MaxAO);
    if (setting->MappingAO == NULL)
        return -1;
    i = 0;
    element = root->FirstChildElement("AnalogOutput");
    while (element) {
        if (getHexDecValue(element, &setting->MappingAO[i++], sizeof(*setting->MappingAO)) <= 0)
            return -1;
        element = element->NextSiblingElement("AnalogOutput");
    }

    /* Digital Input. */
    setting->MaxAI = 0;
    element = root->FirstChildElement("AnalogInput");
    while (element) {
        setting->MaxAI++;
        element = element->NextSiblingElement("AnalogInput");
    }
    setting->MappingAI = (int *)malloc(sizeof(int) * setting->MaxAI);
    if (setting->MappingAI == NULL)
        return -1;
    i = 0;
    element = root->FirstChildElement("AnalogInput");
    while (element) {
        if (getHexDecValue(element, &setting->MappingAI[i++], sizeof(*setting->MappingAI)) <= 0)
            return -1;
        element = element->NextSiblingElement("AnalogInput");
    }
    return 0;
}

static inline int __parse_slave(XMLElement *root, _MyEcatSlaveSetting *setting)
{
    XMLElement *element;
    if (getStringElement(root, "ObjectName", &setting->ObjectName) <= 0)
        return -1;
    if (getHexDecValueElement(root, "VendorId", &setting->VendorId, sizeof(setting->VendorId)) <= 0)
        return -1;
    if (getHexDecValueElement(root, "ProductCode", &setting->ProductCode, sizeof(setting->ProductCode)) <= 0)
        return -1;
    if (getHexDecValueElement(root, "AliasAddress", &setting->AliasId, sizeof(setting->AliasId)) <= 0)
        return -1;
    if (getHexDecValueElement(root, "HaveMCU", &setting->haveMCU, sizeof(setting->haveMCU)) <= 0)
        return -1;
    
    getHexDecValueElement(root, "Type", &setting->Type, sizeof(setting->Type));
    
    element = root->FirstChildElement("Mapping");
    if (element == NULL) {
        return -1; // we don't determine it , because it may be one unsopport device
    }
    
    if (setting->Type == SLAVE_HID) {
        if (element && __parse_hid_mapping(element, setting) < 0)
            return -1;
    } else if (setting->Type == SLAVE_LCD) {
        if (element && __parse_lcd_mapping(element, setting) < 0)
            return -1;
    } else if (setting->Type == SLAVE_MONITOR) {
        if (element && __parse_monitor_mapping(element, setting) < 0)
            return -1;
    } else if (setting->Type == SLAVE_MOTOR) {
        if (element && __parse_motor_mapping(element, setting) < 0)
            return -1;
        
        XMLElement* cia402Element = root->FirstChildElement("CiA402Settings");
        
        if (cia402Element != NULL) { // OLD version cfg.eva
            XMLElement* e;
            int data, MaxServos;
            double data2;
            CiA402Data* cia402;

            setting->PrivateData = cia402 = (CiA402Data*)malloc(sizeof(CiA402Data));
            
            MaxServos = setting->MaxServo;
            if (MaxServos > 0) {
                MaxServos--;
                if (getHexDecValueElement(cia402Element, "PPR1", &data, sizeof(int)) > 0)
                    cia402->cia402_PPR[0] = data;
                if (getHexDecValueElement(cia402Element, "HomingMode1", &data, sizeof(int)) > 0)
                    cia402->cia402_homingMode[0] = data;
                e = cia402Element->FirstChildElement("HomingVelocity1");
                if (e && getDoubleValue(e, &data2, sizeof(double)) > 0)
                    cia402->cia402_homing_v[0] = data2;
                e = cia402Element->FirstChildElement("HomingAcc1");
                if (e && getDoubleValue(e, &data2, sizeof(double)) > 0)
                    cia402->cia402_homing_acc[0] = data2;
                e = cia402Element->FirstChildElement("MaxVelocity1");
                if (e && getDoubleValue(e, &data2, sizeof(double)) > 0)
                    cia402->cia402_max_v[0] = data2;
                e = cia402Element->FirstChildElement("MaxAcc1");
                if (e && getDoubleValue(e, &data2, sizeof(double)) > 0)
                    cia402->cia402_max_acc[0] = data2;
                e = cia402Element->FirstChildElement("MaxDec1");
                if (e && getDoubleValue(e, &data2, sizeof(double)) > 0)
                    cia402->cia402_max_dec[0] = data2;
                e = cia402Element->FirstChildElement("DefaultVelocity1");
                if (e && getDoubleValue(e, &data2, sizeof(double)) > 0)
                    cia402->cia402_default_v[0] = data2;
                e = cia402Element->FirstChildElement("DefaultAcc1");
                if (e && getDoubleValue(e, &data2, sizeof(double)) > 0)
                    cia402->cia402_default_acc[0] = data2;
            }
            if (MaxServos > 0) {
                MaxServos--;
                if (getHexDecValueElement(cia402Element, "PPR2", &data, sizeof(int)) > 0)
                    cia402->cia402_PPR[1] = data;
                if (getHexDecValueElement(cia402Element, "HomingMode2", &data, sizeof(int)) > 0)
                    cia402->cia402_homingMode[1] = data;
                e = cia402Element->FirstChildElement("HomingVelocity2");
                if (e && getDoubleValue(e, &data2, sizeof(double)) > 0)
                    cia402->cia402_homing_v[1] = data2;
                e = cia402Element->FirstChildElement("HomingAcc2");
                if (e && getDoubleValue(e, &data2, sizeof(double)) > 0)
                    cia402->cia402_homing_acc[1] = data2;
                e = cia402Element->FirstChildElement("MaxVelocity2");
                if (e && getDoubleValue(e, &data2, sizeof(double)) > 0)
                    cia402->cia402_max_v[1] = data2;
                e = cia402Element->FirstChildElement("MaxAcc2");
                if (e && getDoubleValue(e, &data2, sizeof(double)) > 0)
                    cia402->cia402_max_acc[1] = data2;
                e = cia402Element->FirstChildElement("MaxDec2");
                if (e && getDoubleValue(e, &data2, sizeof(double)) > 0)
                    cia402->cia402_max_dec[1] = data2;
                e = cia402Element->FirstChildElement("DefaultVelocity2");
                if (e && getDoubleValue(e, &data2, sizeof(double)) > 0)
                    cia402->cia402_default_v[1] = data2;
                e = cia402Element->FirstChildElement("DefaultAcc2");
                if (e && getDoubleValue(e, &data2, sizeof(double)) > 0)
                    cia402->cia402_default_acc[1] = data2;
            }
            if (MaxServos > 0) {
                MaxServos--;
                if (getHexDecValueElement(cia402Element, "PPR3", &data, sizeof(int)) > 0)
                    cia402->cia402_PPR[2] = data;
                if (getHexDecValueElement(cia402Element, "HomingMode3", &data, sizeof(int)) > 0)
                    cia402->cia402_homingMode[2] = data;
                e = cia402Element->FirstChildElement("HomingVelocity3");
                if (e && getDoubleValue(e, &data2, sizeof(double)) > 0)
                    cia402->cia402_homing_v[2] = data2;
                e = cia402Element->FirstChildElement("HomingAcc3");
                if (e && getDoubleValue(e, &data2, sizeof(double)) > 0)
                    cia402->cia402_homing_acc[2] = data2;
                e = cia402Element->FirstChildElement("MaxVelocity3");
                if (e && getDoubleValue(e, &data2, sizeof(double)) > 0)
                    cia402->cia402_max_v[2] = data2;
                e = cia402Element->FirstChildElement("MaxAcc3");
                if (e && getDoubleValue(e, &data2, sizeof(double)) > 0)
                    cia402->cia402_max_acc[2] = data2;
                e = cia402Element->FirstChildElement("MaxDec3");
                if (e && getDoubleValue(e, &data2, sizeof(double)) > 0)
                    cia402->cia402_max_dec[2] = data2;
                e = cia402Element->FirstChildElement("DefaultVelocity3");
                if (e && getDoubleValue(e, &data2, sizeof(double)) > 0)
                    cia402->cia402_default_v[2] = data2;
                e = cia402Element->FirstChildElement("DefaultAcc3");
                if (e && getDoubleValue(e, &data2, sizeof(double)) > 0)
                    cia402->cia402_default_acc[2] = data2;
            }
        } else
            setting->PrivateData = NULL;
        
    } else if (setting->Type == SLAVE_SENSOR) {
        if (element && __parse_sensor_mapping(element, setting) < 0)
            return -1;
    } else if (setting->Type == SLAVE_AIQ) {
        if (element && __parse_aiq_mapping(element, setting) < 0)
            return -1;
    } else {
        setting->Type == SLAVE_DIQ;
        if (element && __parse_diq_mapping(element, setting) < 0)
            return -1;
    }
    
    return 0;
}

static inline int __parse_master(XMLElement *root, _MyEcatSetting *setting)
{
    setting->Redundant = false;
    setting->IgnoreOverride = true;
    setting->DcMode = 0;
    setting->SyncSafeOP = false;
    setting->AutoRestart = false;
    if (getBoolAttribute(root, "Redundant", &setting->Redundant) < 0)
        return -1;
    if (getBoolAttribute(root, "IgnoreOverride", &setting->IgnoreOverride) < 0)
        return -1;
    if (getIntAttribute(root, "DcMode", &setting->DcMode) < 0)
        return -1;
    if (getBoolAttribute(root, "SyncSafeOP", &setting->SyncSafeOP) < 0)
        return -1;
    if (getBoolAttribute(root, "AutoRestart", &setting->AutoRestart) < 0)
        return -1;  
    if (getHexDecValueElement(root, "CycleTime", &setting->CycleTime, sizeof(setting->CycleTime)) <= 0)
        return -1;
    if (getHexDecValueElement(root, "MaxDigitalPins", &setting->MaxDigitalPins, sizeof(setting->MaxDigitalPins)) <= 0)
        return -1;
    if (getHexDecValueElement(root, "MaxLcds", &setting->MaxLcds, sizeof(setting->MaxLcds)) <= 0)
        return -1;
    if (getHexDecValueElement(root, "MaxSerials", &setting->MaxSerials, sizeof(setting->MaxSerials)) <= 0)
        return -1;
    if (getHexDecValueElement(root, "MaxLcms", &setting->MaxLcms, sizeof(setting->MaxLcms)) <= 0)
        return -1;
    if (getHexDecValueElement(root, "MaxMpgs", &setting->MaxMpgs, sizeof(setting->MaxMpgs)) <= 0)
        return -1;
    if (getHexDecValueElement(root, "MaxBuzzerPins", &setting->MaxBuzzerPins, sizeof(setting->MaxBuzzerPins)) <= 0)
        return -1;
    if (getHexDecValueElement(root, "MaxKeypads", &setting->MaxKeypads, sizeof(setting->MaxKeypads)) <= 0)
        return -1;
    if (getHexDecValueElement(root, "MaxIntegerDataPins", &setting->MaxIntegerDataPins, sizeof(setting->MaxIntegerDataPins)) <= 0)
        return -1;
    if (getHexDecValueElement(root, "MaxMicrophones", &setting->MaxMicrophones, sizeof(setting->MaxMicrophones)) <= 0)
        return -1;
    if (getHexDecValueElement(root, "MaxSpeakers", &setting->MaxSpeakers, sizeof(setting->MaxSpeakers)) <= 0)
        return -1;
    if (getHexDecValueElement(root, "MaxDecimalDataPins", &setting->MaxDecimalDataPins, sizeof(setting->MaxDecimalDataPins)) <= 0)
        return -1;
    if (getHexDecValueElement(root, "MaxMachines", &setting->MaxMachines, sizeof(setting->MaxMachines)) <= 0)
        return -1;
    if (getHexDecValueElement(root, "MaxServos", &setting->MaxServos, sizeof(setting->MaxServos)) <= 0)
        return -1;
    if (getHexDecValueElement(root, "MaxEncoders", &setting->MaxEncoders, sizeof(setting->MaxEncoders)) <= 0)
        return -1;
    if (getHexDecValueElement(root, "MaxEMSensors", &setting->MaxEMSensors, sizeof(setting->MaxEMSensors)) <= 0)
        return -1;
    if (getHexDecValueElement(root, "MaxForceSensors", &setting->MaxForceSensors, sizeof(setting->MaxForceSensors)) <= 0)
        return -1;
    if (getStringElement(root, "ObjectName", &setting->ObjectName) < 0)
        return -1;
    if (getStringElement(root, "ENIName", &setting->ENIName) < 0)
        return -1;
    return 0;
}

static inline _MyEcatSetting *requestEVA()
{
    _MyEcatSetting *setting;
    XMLDocument xml;
    XMLError error;
    XMLElement *root, *element;
    uint16_t SlaveCount = 0;
    int i = 0;
    
    if (userInternalResourcePath != NULL)
        sprintf(evaPath, "%smyeccfg\\", userInternalResourcePath);
    else
        sprintf(evaPath, "B:\\resource\\myeccfg\\");
    strcat(evaPath, "cfg.eva");
    
    error = xml.LoadFile(evaPath);
    if(error != XML_SUCCESS) {
        printf("ERROR: XMLError = %d\n", error);
        return NULL;
    }
    
    root = xml.RootElement();
    if (strcmp(root->Name(), "EtherCATVirtualArduino")) {
        printf("ERROR: No EtherCATVirtualArduino.\n");
        return NULL;
    }

    element = root->FirstChildElement("Slave");
    while (element) {
        SlaveCount++;
        element = element->NextSiblingElement();
    }
    if (SlaveCount == 0) {
        printf("ERROR: No Slave.\n");
        return NULL;
    }

    element = root->FirstChildElement("Master");
    if (element == NULL) {
        printf("ERROR: No Master.\n");
        return NULL;
    }

    setting = (_MyEcatSetting *)malloc(sizeof(*setting) + SlaveCount * sizeof(_MyEcatSlaveSetting));
    if (setting == NULL) {
        printf("ERROR: Allocate _MyEcatSetting structure failed.\n");
        return NULL;
    }
    memset(setting, 0, sizeof(*setting) + SlaveCount * sizeof(_MyEcatSlaveSetting));
    setting->Slave = (_MyEcatSlaveSetting *)(setting + 1);
    setting->SlaveCount = SlaveCount;

    if (__parse_master(element, setting) < 0) {
        printf("ERROR: Parse Master failed.\n");
        return NULL;
    }

    element = root->FirstChildElement("Slave");
    while (element) {
        if (__parse_slave(element, &setting->Slave[i++]) < 0) {
            /*
            while (i-- >= 0) {
                __release_slave(&setting->Slave[i]);
            }
            */
        }
        element = element->NextSiblingElement();
    }

    return setting;
}

static inline void releaseEVA(_MyEcatSetting *setting)
{
    for (int i = 0; i < setting->SlaveCount; i++)
        __release_slave(&setting->Slave[i]);
    __release_master(setting);
    free(setting);
}

#include "queue.h"
static Queue *EventQueue = NULL;
static bool enableCheckCBError = true;
void ErrorCallback(uint32_t errorcode)
{
    uint8_t buf[16];
    *((uint32_t *)buf) = errorcode | (1 << 31);
    EcatMaster.getErrorCallbackData(&buf[4], 12);
    if (!QueueFull(EventQueue))
        PushBufQueue(EventQueue, buf);
}


void debug(bool d) {
    if (d) {
        enableCheckCBError = true;
    }
    else {
        enableCheckCBError = false;
    }
}

#define ATTACH_ID(alias, idx)   ((alias > 0) ? alias : idx)
#define ATTACH_MODE(alias)      ((alias > 0) ? ECAT_ALIAS_ADDRESS : ECAT_SLAVE_NO)

EthercatDevice_QECR11MP3S Slave0;
#define NUM_SLAVE (1)
EthercatMaster EcatMaster;
MyEVA EVA;

MyVirtualCNC VirtualCNC1;
MyVirtualCNC VirtualCNC2;
MyVirtualCNC VirtualCNC3;
MyVirtualCNC VirtualCNC4;
MyVirtualServo VirtualServo1;
MyVirtualServo VirtualServo2;
MyVirtualServo VirtualServo3;
MyVirtualServo VirtualServo4;
MyVirtualServo VirtualServo5;
MyVirtualServo VirtualServo6;
MyVirtualEncoder VirtualEncoder1;
MyVirtualEncoder VirtualEncoder2;
MyVirtualEncoder VirtualEncoder3;
MyVirtualEncoder VirtualEncoder4;
MyVirtualEncoder VirtualEncoder5;
MyVirtualEncoder VirtualEncoder6;
void _task_handler();
static _HelperTask* _taskRTC = NULL;

typedef struct myecat_portpinmap {
    void* Object;
    int subType;
    int Port;
    uint32_t Pin;
    unsigned Out : 1;
} _MyEcatPortPinMap;
_MyEcatPortPinMap* DIO = NULL;
_MyEcatPortPinMap* BUZZER = NULL;
_MyEcatPortPinMap* INTEGER = NULL;
_MyEcatPortPinMap* DECIMAL = NULL;
_MyEcatSetting* Setting = NULL;

typedef struct deviceName {
    unsigned long pid;
    const char* name;
} DEVICEIFO;

DEVICEIFO devices[] = {
    {0x0086D0D4, "QEC-R11D0FS"},
    {0x0086D305, "QEC-R11D0FH"},
    {0x0086D30A, "QEC-R00D0FH"},
    {0x0086D0D2, "QEC-R11DF0D"},
    {0x0086D306, "QEC-R11DF0H"},
    {0x0086D30E, "QEC-R11DF0S"},
    {0x0086D30B, "QEC-R00DF0H"},
    {0x0086D30D, "QEC-R00DF0S"},
    {0x0086D303, "QEC-R00D0FS"},
    {0x0086D300, "QEC-R00DF0D"},
    {0x0086D0D5, "QEC-R11D88S"},
    {0x0086D301, "QEC-R00D88D"},
    {0x0086D307, "QEC-R11D88D"},
    {0x0086D308, "QEC-R11D88H"},
    {0x0086D309, "QEC-R00D88S"},
    {0x0086D30F, "QEC-R00D88H"},
    {0x0086D304, "QEC-R00DC4D"},
    {0x0086D302, "QEC-R00D4CD"},
    {0x0086D404, "QEC-R11HU1S"},
    {0x0086D405, "QEC-R00HU1S"},
    {0x0086D403, "QEC-R11HU5S"},
    {0x0086D400, "QEC-R00HU5S"},
    {0x0086D401, "QEC-R00HU9S"},
    {0x0086D402, "QEC-R11HU9S"},
    {0x0086D100, "QEC-R00UN01"},
    {0x0086D103, "QEC-R11UN01"},
    {0x0086D0D6, "QEC-R11MP3S"},
    {0x0086D0D9, "QEC-R00MP3S"},
    {0x0086D500, "QEC-R11MC8S"},
    {0x0086D501, "QEC-R11MC1S"},
    {0x0086D502, "QEC-R11MC4S"},
    {0x00860102, "QEC-T120"},
    {0x00860103, "QEC-T600"},
    {0x000085FB, "QEC-R11JT2S"},
    {0x000085FF, "QEC-R11JT3S"},
    {0x0086D320, "QEC-R11DT0L"},
    {0x0086D700, "QEC-R11DT0H"},
    {0x0086D324, "QEC-R11D0TL"},
    {0x0086D800, "QEC-R11D0TH"},
    {0x0086D323, "QEC-R00DT0L"},
    {0x0086D701, "QEC-R00DT0H"},
    {0x0086D327, "QEC-R00D0TL"},
    {0x0086D801, "QEC-R00D0TH"},
    {0x0086D600, "QEC-R11VM8S"},
    {0x0086D601, "QEC-R11VM6S"},
    {0x0086D602, "QEC-R11VM4S"},
    {0x0086D603, "QEC-R11VM2S"},
    {0x0086D604, "QEC-R00VM8S"},
    {0x0086D605, "QEC-R00VM6S"},
    {0x0086D606, "QEC-R00VM4S"},
    {0x0086D607, "QEC-R00VM2S"},
    {0x0086D0E2, "QEC-R11MV3S"},
    {0x0086D0E4, "QEC-R11MV3S"},
    {0x0086D0E5, "QEC-R00MV3S"},
    {0x0086D7FF, "QEC-R11EM1S"},
    {0x00860100, "QEC-Force"},
    {0x0086D880, "QEC-R11A44S"},
    {0x00860104, "QEC-L360"}
};

char UDName[] = "Unknown Device";
const char* getMachineName(int aliasAddress) {
    int _size = sizeof(devices) / sizeof(DEVICEIFO);

    if (aliasAddress == -1) return getBoardName();
    if (Setting == NULL) return UDName;

    for (int i = 0; i < NUM_SLAVE; i++) {
        if (aliasAddress == Setting->Slave[i].AliasId) {
            for (int j = 0; j < _size; j++)
                if (devices[j].pid == Setting->Slave[i].ProductCode) return devices[j].name;
        }
    }

    return UDName;
}

void printErrorMessage(int errorCode) {
    char buf[256];
    EcatMaster.getErrorMessage(errorCode, buf, sizeof(buf));
    Serial.print("Error: ");
    Serial.println(buf);
}

void printEventMessage(int eventCode) {
    char buf[256];
    EcatMaster.getEventMessage(eventCode, buf, sizeof(buf));
    Serial.print("Event: ");
    Serial.println(buf);
}

static double* realCurrentIs = NULL;
static double* realCurrentIp = NULL;
static double* showCurrentIs = NULL;
static double* showCurrentIp = NULL;
static void recalculateCurrent(_MyEcatSetting * setting, int idx, double Ip, double Is) {
    static bool prepareIpRealData = true;
    static bool prepareIsRealData = true;

    if (realCurrentIp == NULL || realCurrentIs == NULL) return;

    realCurrentIs[idx] = Is;
    realCurrentIp[idx] = Ip;

    for (int i = 0; i < NUM_SLAVE; i++) {
        if (realCurrentIs[i] == -1)
            break;

        if (i == (NUM_SLAVE - 1))
            prepareIsRealData = false;
    }

    if (!prepareIsRealData) {
        for (int i = 0, j = 0; i < NUM_SLAVE; ) {
            showCurrentIs[i] = realCurrentIs[i]; // update from real to show array
            for (j = i + 1; (j < NUM_SLAVE) && (!setting->Slave[j].haveMCU); j++);
            if (j < NUM_SLAVE) {
                if (strstr(getMachineName(setting->Slave[i].AliasId), "QEC-R11") != NULL && strstr(getMachineName(setting->Slave[j].AliasId), "QEC-R11") != NULL) // I am red-head LAN and next also is red-head LAN
                    showCurrentIs[i] -= realCurrentIs[j];

                if (showCurrentIs[i] < 0.0) showCurrentIs[i] = 0.0;
            }
            setting->Slave[i].Is = showCurrentIs[i];
            realCurrentIs[i] = -1;
            i = j;
        }
        prepareIsRealData = true;
    }

    if (prepareIpRealData) {
        for (int i = 0; i < NUM_SLAVE; i++) {
            if (realCurrentIp[i] == -1)
                break;

            if (i == (NUM_SLAVE - 1))
                prepareIpRealData = false;
        }
    }

    if (!prepareIpRealData) {
        for (int i = 0, j = 0; i < NUM_SLAVE; ) {
            showCurrentIp[i] = realCurrentIp[i]; // update from real to show array
            for (j = i + 1; (j < NUM_SLAVE) && (!setting->Slave[j].haveMCU); j++);
            if (j < NUM_SLAVE) {
                if (strstr(getMachineName(setting->Slave[i].AliasId), "QEC-R11") != NULL && strstr(getMachineName(setting->Slave[j].AliasId), "QEC-R11") != NULL) // I am red-head LAN and next also is red-head LAN
                    showCurrentIp[i] -= realCurrentIp[j];

                if (showCurrentIp[i] < 0.0) showCurrentIp[i] = 0.0;
            }
            setting->Slave[i].Ip = showCurrentIp[i];
            realCurrentIp[i] = -1;
            i = j;
        }
        prepareIpRealData = true;
    }
}

static bool masterIsRedHead(void) {
    static FILE* _myfp = NULL;
    static bool once = false;

    if (once) {
        if (_myfp != NULL)
            return true; // don't open file again
        else
            return false;
    }

    FILE* fp = fopen("A:/redhead.v86", "r");
    once = true;
    if (fp == NULL) return false; // no this file, it is Black Head
    _myfp = fp;
    fclose(fp);

    return true;
}

double myIS[NUM_SLAVE] = {0.0};
double myIP[NUM_SLAVE] = {0.0};
static double _master_Ip = -1.0, _master_Is = -1.0;
static void refreshIsIp(_MyEcatSetting * setting) {
    double vs, sp, is, ip, temp;

    if (setting == NULL) return;

    if (NUM_SLAVE == 0) {
        _master_Ip = peripheralPowerCurrent(); 
        _master_Is = systemPowerCurrent(); 
        return;
    }

    for (int i = 0; i < NUM_SLAVE; i++) {
        if (!setting->Slave[i].haveMCU) {
            is = 0.0; ip = 0.0;
        } else {
            is = myIS[i];
            ip = myIP[i];
        }
        if (i == 0) {
            if (masterIsRedHead() && strstr(getMachineName(setting->Slave[0].AliasId), "QEC-R11") != NULL) {
                _master_Ip = peripheralPowerCurrent() - ip; 
                _master_Is = systemPowerCurrent() - is; 
                if (_master_Ip < 0) _master_Ip = 0.0; 
                if (_master_Is < 0) _master_Is = 0.0; 
            } else {
                _master_Ip = peripheralPowerCurrent();
                _master_Is = systemPowerCurrent();
            }
        }

        recalculateCurrent(setting, i, ip, is);
    }
}

void (*_myCallback)(void) = NULL;
extern bool isRun();
void _CyclicCallback(void) {
    if (isRun()) {
        if (_myCallback) _myCallback();
    }
}


MyEVA::MyEVA() {}
int findObjectOrderN(void* src, const void** objArray) {
    for (int i = 0; i < NUM_SLAVE; i++) {
        if (src == objArray[i]) {
            return i;
        }
    }
}

void qecm02_stop() {
Slave0.machineSetEmergencyStop();
}

void qecm02_start() {
Slave0.machineClearEmergencyStop();
}

int MyEVA::begin(void (*myCallback)(void))
{
    const char *ObjectName[NUM_SLAVE] = {"Slave0"};
    const void *ObjectPointer[NUM_SLAVE] = {&Slave0};
   _MyEcatSlaveSetting *mapping[NUM_SLAVE] = {NULL};
    _MyEcatSetting * setting;
    _MyEcatPortPinMap *Dio;
    _MyEcatPortPinMap *SerialMapping;
    _MyEcatPortPinMap *MpgMapping;
    _MyEcatPortPinMap *LcmMapping;
    _MyEcatPortPinMap *BuzzerMapping;
    _MyEcatPortPinMap *KeypadMapping;
    _MyEcatPortPinMap *IntegerDataPinMapping;
    _MyEcatPortPinMap *LcdMapping;
    _MyEcatPortPinMap *MicrophoneMapping;
    _MyEcatPortPinMap *SpeakerMapping;
    _MyEcatPortPinMap *DecimalDataPinMapping;
    _MyEcatPortPinMap *MachineMapping;
    _MyEcatPortPinMap *ServoMapping;
    _MyEcatPortPinMap *EncoderMapping;
    _MyEcatPortPinMap *EMSensorMapping;
    _MyEcatPortPinMap *ForceSensorMapping;
    EthercatMasterSettings myEcSetting;
    int rc, i, j, k;

    realCurrentIs = (double*)malloc(sizeof(double) * NUM_SLAVE);
    realCurrentIp = (double*)malloc(sizeof(double) * NUM_SLAVE);
    for (int i = 0; i < NUM_SLAVE; i++) {
        realCurrentIs[i] = -1;
        realCurrentIp[i] = -1;
    }
    showCurrentIs = (double*)malloc(sizeof(double) * NUM_SLAVE);
    showCurrentIp = (double*)malloc(sizeof(double) * NUM_SLAVE);

    setting = requestEVA();
    if (setting == NULL) {
        return EVAERR_NO_CONFIG_FILE;
    }

    for (i = 0; i < NUM_SLAVE; i++) {
        for (j = 0; j < setting->SlaveCount; j++) {
            if (strcmp(setting->Slave[j].ObjectName, ObjectName[i]) == 0) {
                setting->Slave[j].PrivateObj = (void*)ObjectPointer[i];
                mapping[i] = &setting->Slave[j];
                break;
            }
        }
    }

    /* Allocate memory of digital pin mapping. */
    Dio = (_MyEcatPortPinMap*)malloc(setting->MaxDigitalPins * sizeof(*Dio));
    if (Dio == NULL) {
        releaseEVA(setting);
        return EVAERR_NO_ENOUGH_MEMORY;
    }
    memset(Dio, 0, setting->MaxDigitalPins * sizeof(*Dio));
    
    SerialMapping = (_MyEcatPortPinMap*)malloc((setting->MaxSerials + setting->MaxMpgs + setting->MaxLcms + setting->MaxKeypads + setting->MaxIntegerDataPins + setting->MaxLcds + setting->MaxDecimalDataPins + setting->MaxSpeakers + setting->MaxMicrophones + setting->MaxMachines + setting->MaxServos + setting->MaxEncoders + setting->MaxEMSensors + setting->MaxBuzzerPins + setting->MaxForceSensors) * sizeof(*SerialMapping));
    if (SerialMapping == NULL) {
        releaseEVA(setting);
        return EVAERR_NO_ENOUGH_MEMORY;
    }
    memset(SerialMapping, 0, (setting->MaxSerials + setting->MaxMpgs + setting->MaxLcms + setting->MaxKeypads + setting->MaxIntegerDataPins + setting->MaxLcds + setting->MaxDecimalDataPins + setting->MaxSpeakers + setting->MaxMicrophones + setting->MaxMachines + setting->MaxServos + setting->MaxEncoders + setting->MaxEMSensors + setting->MaxBuzzerPins + setting->MaxForceSensors) * sizeof(*SerialMapping));

    MpgMapping = (_MyEcatPortPinMap*)(SerialMapping + setting->MaxSerials);
    LcmMapping = (_MyEcatPortPinMap*)(MpgMapping + setting->MaxMpgs);
    KeypadMapping = (_MyEcatPortPinMap*)(LcmMapping + setting->MaxLcms);
    BuzzerMapping = (_MyEcatPortPinMap*)(KeypadMapping + setting->MaxKeypads);
    IntegerDataPinMapping = (_MyEcatPortPinMap*)(BuzzerMapping + setting->MaxBuzzerPins);
    DecimalDataPinMapping = (_MyEcatPortPinMap*)(IntegerDataPinMapping + setting->MaxIntegerDataPins);
    LcdMapping = (_MyEcatPortPinMap*)(DecimalDataPinMapping + setting->MaxDecimalDataPins);
    SpeakerMapping = (_MyEcatPortPinMap*)(LcdMapping + setting->MaxLcds);
    MicrophoneMapping = (_MyEcatPortPinMap*)(SpeakerMapping + setting->MaxSpeakers);
    MachineMapping = (_MyEcatPortPinMap*)(MicrophoneMapping + setting->MaxMicrophones);
    ServoMapping = (_MyEcatPortPinMap*)(MachineMapping + setting->MaxMachines);
    EncoderMapping = (_MyEcatPortPinMap*)(ServoMapping + setting->MaxServos);
    EMSensorMapping = (_MyEcatPortPinMap*)(EncoderMapping + setting->MaxEncoders);
    ForceSensorMapping = (_MyEcatPortPinMap*)(EMSensorMapping + setting->MaxEMSensors);

    for (i = 0; i < setting->MaxSerials; i++) {
        for (j = 0; j < setting->SlaveCount; j++) {
            if (setting->Slave[j].Type == SLAVE_HID) {
                for (k = 0; k < setting->Slave[j].MaxSerial; k++) {
                    if (i == setting->Slave[j].MappingSerial[k]) {
                        SerialMapping[i].Object = setting->Slave[j].PrivateObj;
                        SerialMapping[i].Port = k;
                    }
                }
            }
        }
    }

    for (i = 0; i < setting->MaxMpgs; i++) {
        for (j = 0; j < setting->SlaveCount; j++) {
            if (setting->Slave[j].Type == SLAVE_HID) {
                for (k = 0; k < setting->Slave[j].MaxMpg; k++) {
                    if (i == setting->Slave[j].MappingMpg[k]) {
                        MpgMapping[i].Object = setting->Slave[j].PrivateObj;
                        MpgMapping[i].Port = 0;
                    }
                }
            }
        }
    }

    for (i = 0; i < setting->MaxLcms; i++) {
        for (j = 0; j < setting->SlaveCount; j++) {
            if (setting->Slave[j].Type == SLAVE_HID) {
                for (k = 0; k < setting->Slave[j].MaxLcm; k++) {
                    if (i == setting->Slave[j].MappingLcm[k]) {
                        LcmMapping[i].Object = setting->Slave[j].PrivateObj;
                        LcmMapping[i].Port = 0;
                    }
                }
            }
        }
    }

    for (i = 0; i < setting->MaxKeypads; i++) {
        for (j = 0; j < setting->SlaveCount; j++) {
            if (setting->Slave[j].Type == SLAVE_HID) {
                for (k = 0; k < setting->Slave[j].MaxKeypad; k++) {
                    if (i == setting->Slave[j].MappingKeypad[k]) {
                        KeypadMapping[i].Object = setting->Slave[j].PrivateObj;
                        KeypadMapping[i].Port = 0;
                    }
                }
            }
        }
    }

    for (i = 0; i < setting->SlaveCount; i++) {
        if (setting->Slave[i].Type == SLAVE_HID) {
            for (j = 0; j < setting->Slave[i].MaxBuzzerPin; j++) {
                k = setting->Slave[i].MappingBuzzerPin[j];
                if (k >= 0 && (uint32_t)k < setting->MaxBuzzerPins) {
                    BuzzerMapping[k].Object = setting->Slave[i].PrivateObj;
                    BuzzerMapping[k].Port = 0;
                }
            }
        }
    }
    for (i = 0; i < setting->SlaveCount; i++) {
        if (setting->Slave[i].Type == SLAVE_MONITOR) {
            for (j = 0; j < setting->Slave[i].MaxCounterPin; j++) {
                k = setting->Slave[i].MappingCounterPin[j];
                if (k >= 0 && (uint32_t)k < setting->MaxIntegerDataPins) {
                    IntegerDataPinMapping[k].Object = setting->Slave[i].PrivateObj;
                    IntegerDataPinMapping[k].subType = FS_COUNTER;
                    IntegerDataPinMapping[k].Port = j;
                }
            }
        }
    }
    for (i = 0; i < setting->MaxLcds; i++) {
        for (j = 0; j < setting->SlaveCount; j++) {
            if (setting->Slave[j].Type == SLAVE_LCD) {
                for (k = 0; k < setting->Slave[j].MaxLcd; k++) {
                    if (i == setting->Slave[j].MappingLcd[k]) {
                        LcdMapping[i].Object = setting->Slave[j].PrivateObj;
                        LcdMapping[i].Port = 0;
                    }
                }
            }
        }
    }
    for (i = 0; i < setting->MaxSpeakers; i++) {
        for (j = 0; j < setting->SlaveCount; j++) {
            if (setting->Slave[j].Type == SLAVE_MONITOR) {
                for (k = 0; k < setting->Slave[j].MaxSpeaker; k++) {
                    if (i == setting->Slave[j].MappingSpeaker[k]) {
                        SpeakerMapping[i].Object = setting->Slave[j].PrivateObj;
                        SpeakerMapping[i].Port = 0;
                    }
                }
            }
        }
    }
    for (i = 0; i < setting->MaxMicrophones; i++) {
        for (j = 0; j < setting->SlaveCount; j++) {
            if (setting->Slave[j].Type == SLAVE_MONITOR) {
                for (k = 0; k < setting->Slave[j].MaxMicrophone; k++) {
                    if (i == setting->Slave[j].MappingMicrophone[k]) {
                        MicrophoneMapping[i].Object = setting->Slave[j].PrivateObj;
                        MicrophoneMapping[i].Port = 0;
                    }
                }
            }
        }
    }
    for (i = 0; i < setting->MaxMachines; i++) {
        for (j = 0; j < setting->SlaveCount; j++) {
            if (setting->Slave[j].Type == SLAVE_MOTOR) {
                for (k = 0; k < setting->Slave[j].MaxMachine; k++) {
                    if (i == setting->Slave[j].MappingMachine[k]) {
                        MachineMapping[i].Object = setting->Slave[j].PrivateObj;
                        MachineMapping[i].Port = 0;
                    }
                }
            }
        }
    }
    for (i = 0; i < setting->MaxServos; i++) {
        for (j = 0; j < setting->SlaveCount; j++) {
            if (setting->Slave[j].Type == SLAVE_MOTOR) {
                for (k = 0; k < setting->Slave[j].MaxServo; k++) {
                    if (i == setting->Slave[j].MappingServo[k]) {
                        ServoMapping[i].Object = setting->Slave[j].PrivateObj;
                        if (setting->Slave[j].MaxServo == 1)
                          ServoMapping[i].subType = CIA402_TYPE;
                        else
                          ServoMapping[i].subType = _3AXIS_TYPE;
                        ServoMapping[i].Port = k;
                    }
                }
            }
        }
    }
    for (i = 0; i < setting->MaxEncoders; i++) {
        for (j = 0; j < setting->SlaveCount; j++) {
            if (setting->Slave[j].Type == SLAVE_MOTOR) {
                for (k = 0; k < setting->Slave[j].MaxEncoder; k++) {
                    if (i == setting->Slave[j].MappingEncoder[k]) {
                        EncoderMapping[i].Object = setting->Slave[j].PrivateObj;
                        EncoderMapping[i].Port = k;
                    }
                }
            }
        }
    }
    for (i = 0; i < setting->SlaveCount; i++) {
        if (setting->Slave[i].Type == SLAVE_MONITOR) {
            for (j = 0; j < setting->Slave[i].MaxVoltagePin; j++) {
                k = setting->Slave[i].MappingVoltagePin[j];
                if (k >= 0 && (uint32_t)k < setting->MaxDecimalDataPins) {
                    DecimalDataPinMapping[k].Object = setting->Slave[i].PrivateObj;
                    DecimalDataPinMapping[k].subType = FS_VOLTAGE;
                    DecimalDataPinMapping[k].Port = j; // here, Port mean Pin, Voltage is pin not port
                    DecimalDataPinMapping[k].Out = 0;
                }
            }
        }
    }
    for (i = 0; i < setting->MaxEMSensors; i++) {
        for (j = 0; j < setting->SlaveCount; j++) {
            if (setting->Slave[j].Type == SLAVE_SENSOR) {
                for (k = 0; k < setting->Slave[j].MaxEMSensor; k++) {
                    if (i == setting->Slave[j].MappingEMSensor[k]) {
                        EMSensorMapping[i].Object = setting->Slave[j].PrivateObj;
                        EMSensorMapping[i].Port = 0;
                    }
                }
            }
        }
    }
    for (i = 0; i < setting->MaxForceSensors; i++) {
        for (j = 0; j < setting->SlaveCount; j++) {
            if (setting->Slave[j].Type == SLAVE_SENSOR) {
                for (k = 0; k < setting->Slave[j].MaxForceSensor; k++) {
                    if (i == setting->Slave[j].MappingForceSensor[k]) {
                        ForceSensorMapping[i].Object = setting->Slave[j].PrivateObj;
                        ForceSensorMapping[i].Port = 0;
                    }
                }
            }
        }
    }

    for (i = 0; i < setting->SlaveCount; i++) {
        if (setting->Slave[i].Type == SLAVE_DIQ) {
            for (j = 0; (uint32_t)j < setting->Slave[i].MaxDO; j++) {
                k = setting->Slave[i].MappingDO[j]; 
                if (k >= 0 && (uint32_t)k < setting->MaxDigitalPins) {
                        Dio[k].Object = setting->Slave[i].PrivateObj; 
                        Dio[k].Pin = j; 
                        Dio[k].Out = 1; 
                }
            }

            for (j = 0; (uint32_t)j < setting->Slave[i].MaxDI; j++) {
                k = setting->Slave[i].MappingDI[j]; 
                if (k >= 0 && (uint32_t)k < setting->MaxDigitalPins) {
                        Dio[k].Object = setting->Slave[i].PrivateObj; 
                        Dio[k].Pin = j; 
                        Dio[k].Out = 0; 
                }
            }
        }
    }

    for (i = 0; i < setting->SlaveCount; i++) {
        if (setting->Slave[i].Type == SLAVE_AIQ) {
            for (j = 0; (uint32_t)j < setting->Slave[i].MaxAO; j++) {
                k = setting->Slave[i].MappingAO[j]; 
                if (k >= 0 && (uint32_t)k < setting->MaxIntegerDataPins) {
                        IntegerDataPinMapping[k].Object = setting->Slave[i].PrivateObj;
                        IntegerDataPinMapping[k].subType = FS_ANALOG;
                        IntegerDataPinMapping[k].Pin = j; 
                        IntegerDataPinMapping[k].Out = 1; 
                }
            }

            for (j = 0; (uint32_t)j < setting->Slave[i].MaxAI; j++) {
                k = setting->Slave[i].MappingAI[j]; 
                if (k >= 0 && (uint32_t)k < setting->MaxIntegerDataPins) {
                        IntegerDataPinMapping[k].Object = setting->Slave[i].PrivateObj;
                        IntegerDataPinMapping[k].subType = FS_ANALOG;
                        IntegerDataPinMapping[k].Pin = j; 
                        IntegerDataPinMapping[k].Out = 0; 
                }
            }
        }
    }

    EcatMaster.readSettings(&myEcSetting);
    myEcSetting.IgnoreBiosOverride = setting->IgnoreOverride;
    myEcSetting.EnableErrorBusReactionSyncUnitToSafeOp = setting->SyncSafeOP;
    myEcSetting.EnableErrorBusReactionSyncUnitToSafeOpAutoRestart = setting->AutoRestart;
    if (setting->DcMode != 0)
        myEcSetting.DcSyncMode = (setting->DcMode == 1) ? ECAT_MASTER_SHIFT : ECAT_BUS_SHIFT;
    EcatMaster.saveSettings(&myEcSetting);

    EthernetPort ethPort = ECAT_ETH_0;
    if (!setting->IgnoreOverride) {
        ethPort = ECAT_ETH_0;
    } else {
        if (setting->Redundant) ethPort = ECAT_ETH_REDUNDANCY;
        else ethPort = ECAT_ETH_0;
    }

    if (setting->ENIName != NULL) {
        if (userInternalResourcePath != NULL)
            sprintf(eniPath, "%smyeccfg\\", userInternalResourcePath); 
        else
            sprintf(eniPath, "B:\\resource\\myeccfg\\"); 
        strcat(eniPath, "eni.xml"); 
        rc = EcatMaster.begin(ethPort, eniPath);
    } else {
        rc = EcatMaster.begin(ethPort, NULL);
    }
    if (rc < 0) {
        Serial.print("Begin - ");
        printErrorMessage(rc);
        free(Dio);
        free(SerialMapping);
        releaseEVA(setting);
        return rc;
    }

    EcatMaster.attachErrorCallback(ErrorCallback);
    
    if ((EventQueue = CreateBufQueue(1024, 16)) == NULL) {
        Serial.println("ERROR: CreateBufQueue(EventQueue) failed.");
        return -1;
    }

    VirtualCNC1.init(MachineMapping[0].Object);
    VirtualCNC2.init(MachineMapping[1].Object);
    VirtualCNC3.init(MachineMapping[2].Object);
    VirtualCNC4.init(MachineMapping[3].Object);
    VirtualEncoder1.init(EncoderMapping[0].Object, EncoderMapping[0].Port);
    VirtualEncoder2.init(EncoderMapping[1].Object, EncoderMapping[1].Port);
    VirtualEncoder3.init(EncoderMapping[2].Object, EncoderMapping[2].Port);
    VirtualEncoder4.init(EncoderMapping[3].Object, EncoderMapping[3].Port);
    VirtualEncoder5.init(EncoderMapping[4].Object, EncoderMapping[4].Port);
    VirtualEncoder6.init(EncoderMapping[5].Object, EncoderMapping[5].Port);
    if (mapping[0]) {
        if (Slave0.attach(ATTACH_ID(mapping[0]->AliasId, 0), EcatMaster, ATTACH_MODE(mapping[0]->AliasId)) != 0) {
            Serial.print("Seq ID:"); Serial.print(0); Serial.print(" Alias address:"); Serial.print(mapping[0]->AliasId); Serial.println("  attach error!");
        }
    }

    if (myCallback) _myCallback = myCallback;
    EcatMaster.attachCyclicCallback(_CyclicCallback);

    rc = EcatMaster.start(setting->CycleTime, ECAT_FREERUN_AUTO); 
    if (rc < 0) {
        Serial.print("Start - ");
        printErrorMessage(rc);
    }

    for (j = 0; j < setting->SlaveCount; j++) {
        if (setting->Slave[j].Type == SLAVE_MONITOR) { // Enable Counter
            ((EthercatDevice_DmpMIC_Generic*)setting->Slave[j].PrivateObj)->counterSetMode(3, 1);
        }
    }

    Slave0.machineServoOn();
    Setting = setting;
    DIO = Dio;
    BUZZER = BuzzerMapping;
    INTEGER = IntegerDataPinMapping;
    DECIMAL = DecimalDataPinMapping;

    myHelper.start(0);
    _taskRTC = myHelper.startLoop(100 * 1024);
    if (_taskRTC == NULL) {
        printf("Error: myHelper init fail");
        return 1;
    }
    _taskRTC->registerFunc(_task_handler);
    _taskRTC->start();

    return 0;
}

void MyEVA::end()
{
    EcatMaster.stop();
    EcatMaster.end();
    free(DIO);
    free(BUZZER);
    free(INTEGER);
    free(DECIMAL);
    releaseEVA(Setting);
    Setting = NULL;
    DIO = NULL;
}

int MyEVA::digitalDataRead(int pin) {
    int ret = -1;
    if (Setting == NULL || pin >= Setting->MaxDigitalPins || DIO[pin].Out == 1 || DIO[pin].Object == NULL)
        return ret;

    ret = ((EthercatDevice_DmpDIQ_Generic*)DIO[pin].Object)->digitalRead(DIO[pin].Pin);

    return ret;
}

int MyEVA::digitalDataWrite(int pin, uint8_t data) {
    int ret = -1;
    if (Setting == NULL || pin >= Setting->MaxDigitalPins || DIO[pin].Out == 0 || DIO[pin].Object == NULL)
        return ret;

    ret = ((EthercatDevice_DmpDIQ_Generic*)DIO[pin].Object)->digitalWrite(DIO[pin].Pin, data);

    return ret;
}

int MyEVA::integerDataRead(int pin) {
    int ret = -1;
    if (Setting == NULL || pin >= Setting->MaxIntegerDataPins || INTEGER[pin].Object == NULL || INTEGER[pin].Out == 1)
        return ret;

    switch (INTEGER[pin].subType) {
    case FS_ANALOG:
        ret = ((EthercatDevice_DmpAIQ_Generic*)INTEGER[pin].Object)->analogRead(INTEGER[pin].Pin);
        break;
    case FS_COUNTER:
        ret = ((EthercatDevice_DmpMIC_Generic*)INTEGER[pin].Object)->counterRead(INTEGER[pin].Port);
        break;
    }

    return ret;
}

int MyEVA::integerDataWrite(int pin, int32_t data) {
    int ret = -1;
    if (Setting == NULL || pin >= Setting->MaxIntegerDataPins || INTEGER[pin].Object == NULL || INTEGER[pin].Out == 0)
        return ret;

    switch (INTEGER[pin].subType) {
    case FS_ANALOG:
        ret = ((EthercatDevice_DmpAIQ_Generic*)INTEGER[pin].Object)->analogWrite(INTEGER[pin].Pin, data);
        break;
    }

    return ret;
}

double MyEVA::decimalDataRead(int pin) {
    int ret = -1;
    if (Setting == NULL || pin >= Setting->MaxDecimalDataPins || DECIMAL[pin].Object == NULL || DECIMAL[pin].Out == 1)
        return ret;

    switch (DECIMAL[pin].subType) {
    case FS_VOLTAGE:
        ret = ((EthercatDevice_DmpVM_Generic*)DECIMAL[pin].Object)->voltageRead(DECIMAL[pin].Port);
        break;
    }

    return ret;
}

int MyEVA::decimalDataWrite(int pin, double data) {
    int ret = -1;
    if (Setting == NULL || pin >= Setting->MaxDecimalDataPins || DECIMAL[pin].Object == NULL || DECIMAL[pin].Out == 0)
        return ret;

    return ret;
}

int MyEVA::integerDataPinMode(int pin, uint32_t range) {
    int ret = -1;
    if (Setting == NULL || pin >= Setting->MaxIntegerDataPins || INTEGER[pin].Object == NULL)
        return ret;

    switch (INTEGER[pin].subType) {
    case FS_ANALOG:
        if (INTEGER[pin].Out == 1)
            ret = ((EthercatDevice_DmpAIQ_Generic*)INTEGER[pin].Object)->aoPinMode(INTEGER[pin].Pin, range);
        else
            ret = ((EthercatDevice_DmpAIQ_Generic*)INTEGER[pin].Object)->aiPinMode(INTEGER[pin].Pin, range);
    break;
    }

    return ret;
}
int MyEVA::digitalWrite(int pin, uint8_t val)
{
    if (Setting == NULL || pin >= Setting->MaxDigitalPins ||
        DIO[pin].Out == 0 ||
        DIO[pin].Object == NULL)
        return -1;

    return digitalDataWrite(pin, val);
}

int MyEVA::digitalRead(int pin)
{
    if (Setting == NULL || pin >= Setting->MaxDigitalPins ||
        DIO[pin].Out == 1 ||
        DIO[pin].Object == NULL)
        return LOW;

    return digitalDataRead(pin);
}

int MyEVA::analogPinMode(int pin, uint32_t range)
{
    if (Setting == NULL || pin >= Setting->MaxIntegerDataPins ||
        INTEGER[pin].Object == NULL || INTEGER[pin].subType != FS_ANALOG)
        return -1;

    return integerDataPinMode(pin, range);
}

int MyEVA::analogWrite(int pin, int32_t val)
{
    if (Setting == NULL || pin >= Setting->MaxIntegerDataPins ||
        INTEGER[pin].Out == 0 ||
        INTEGER[pin].Object == NULL || INTEGER[pin].subType != FS_ANALOG)
        return -1;

    return integerDataWrite(pin, val);
}

int MyEVA::analogRead(int pin)
{
    if (Setting == NULL || pin >= Setting->MaxIntegerDataPins ||
        INTEGER[pin].Out == 1 ||
        INTEGER[pin].Object == NULL || INTEGER[pin].subType != FS_ANALOG)
        return -1;

    return integerDataRead(pin);
}

int MyEVA::tone(int pin, uint32_t freq, uint32_t duration)
{
    if (Setting == NULL || pin >= Setting->MaxBuzzerPins ||
        BUZZER[pin].Object == NULL)
        return -1;

    return ((EthercatDevice_DmpHID_Generic*)BUZZER[pin].Object)->buzzer(freq, duration);
}

int getMySeqIDfromAlias(int aliasId) {
    int index = -1;

    for (int i=0; i<NUM_SLAVE; i++) {
        if (Setting->Slave[i].AliasId == aliasId) {
            index = i;
            break;
        }
    }

    return index;
}

double MyEVA::getUsVoltage(short aliasId)
{
    int index = -1;
    unsigned long tt = 0;

    if (aliasId == -1) return systemPowerVoltage();

    if (Setting == NULL)
        return -1;

    index = getMySeqIDfromAlias(aliasId);

    if (index < 0 || !Setting->Slave[index].haveMCU) return 0.0;

    while (!((_EthercatDevice_DmpCommonDriver*)Setting->Slave[index].PrivateObj)->tryToGetSystemPowerVoltage()) { helperYield(); }

    return ((_EthercatDevice_DmpCommonDriver*)Setting->Slave[index].PrivateObj)->getSystemPowerVoltage();
}

double MyEVA::getUpVoltage(short aliasId)
{
    int index = -1;
    unsigned long tt = 0;

    if (aliasId == -1) return peripheralPowerVoltage();

    if (Setting == NULL)
        return -1;

    index = getMySeqIDfromAlias(aliasId);

    if (index < 0 || !Setting->Slave[index].haveMCU) return 0.0;

    while (!((_EthercatDevice_DmpCommonDriver*)Setting->Slave[index].PrivateObj)->tryToGetPeripheralPowerVoltage()) { helperYield(); }

    return ((_EthercatDevice_DmpCommonDriver*)Setting->Slave[index].PrivateObj)->getPeripheralPowerVoltage();
}

double MyEVA::getIsCurrent(short aliasId)
{
    int index = -1;
    unsigned long tt = 0;

    if (aliasId == -1) {
        refreshIsIp(Setting);
        return _master_Is;
    }

    if (Setting == NULL)
        return -1;

    index = getMySeqIDfromAlias(aliasId);

    if (index < 0 || !Setting->Slave[index].haveMCU) return 0.0;

    while (!((_EthercatDevice_DmpCommonDriver*)Setting->Slave[index].PrivateObj)->tryToGetSystemPowerCurrent()) { helperYield(); }

    myIS[index] = ((_EthercatDevice_DmpCommonDriver*)Setting->Slave[index].PrivateObj)->getSystemPowerCurrent();

    refreshIsIp(Setting);

    return Setting->Slave[index].Is; 
}

double MyEVA::getIpCurrent(short aliasId)
{
    int index = -1;
    unsigned long tt = 0;

    if (aliasId == -1) {
        refreshIsIp(Setting);
        return _master_Ip;
    }

    if (Setting == NULL)
        return -1;

    index = getMySeqIDfromAlias(aliasId);

    if (index < 0 || !Setting->Slave[index].haveMCU) return 0.0;

    while (!((_EthercatDevice_DmpCommonDriver*)Setting->Slave[index].PrivateObj)->tryToGetPeripheralPowerCurrent()) { helperYield(); }

    myIP[index] = ((_EthercatDevice_DmpCommonDriver*)Setting->Slave[index].PrivateObj)->getPeripheralPowerCurrent();

    refreshIsIp(Setting);

    return Setting->Slave[index].Ip;
}

double MyEVA::getTemperature(short aliasId)
{
    int index = -1;
    unsigned long tt = 0;

    if (aliasId == -1) return cpuTemperature();

    if (Setting == NULL)
        return -1;

    index = getMySeqIDfromAlias(aliasId);

    if (index < 0 || !Setting->Slave[index].haveMCU) return 0.0;

    while (!((_EthercatDevice_DmpCommonDriver*)Setting->Slave[index].PrivateObj)->tryToGetSystemTemperature()) { helperYield(); }

    return ((_EthercatDevice_DmpCommonDriver*)Setting->Slave[index].PrivateObj)->getSystemTemperature();
}

int MyEVA::getWorkingHours(short aliasId)
{
    int index = -1;
    unsigned long tt = 0;

    if (aliasId == -1) return 0;

    if (Setting == NULL)
        return -1;

    index = getMySeqIDfromAlias(aliasId);

    if (index < 0 || !Setting->Slave[index].haveMCU) return 0.0;

    while (!((_EthercatDevice_DmpCommonDriver*)Setting->Slave[index].PrivateObj)->tryToGetWorkingHours()) { helperYield(); };

    return ((_EthercatDevice_DmpCommonDriver*)Setting->Slave[index].PrivateObj)->getWorkingHours();
}

int MyEVA::getBootTimes(short aliasId)
{
    int index = -1;
    unsigned long tt = 0;

    if (aliasId == -1) return 0;

    if (Setting == NULL)
        return -1;

    index = getMySeqIDfromAlias(aliasId);

    if (index < 0 || !Setting->Slave[index].haveMCU) return 0.0;

    while (!((_EthercatDevice_DmpCommonDriver*)Setting->Slave[index].PrivateObj)->tryToGetBootTimes()) { helperYield(); };

    return ((_EthercatDevice_DmpCommonDriver*)Setting->Slave[index].PrivateObj)->getBootTimes();
}
void _task_handler()
{
    uint8_t buffer[16];

    ((EthercatDevice_DmpStepper_Generic*)&Slave0)->update();

    if (enableCheckCBError) {
        if (!QueueEmpty(EventQueue) && PopBufQueue(EventQueue, buffer) == true) {
            uint32_t code = ((uint32_t *)buffer)[0] & ~(1 << 31);
            if (((uint32_t *)buffer)[0] & (1 << 31)) {
                printErrorMessage(code);
            } else {
                printEventMessage(code);
            }
        }
    }
}

