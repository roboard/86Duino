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
#define SLAVE_CIQ        (9)
#define SLAVE_ACP        (10)
#define THIRD_SLAVE_DIQ  (11)

// DD List Function SubType
#define FS_COUNTER   (0)
#define FS_VOLTAGE   (1)
#define FS_ANALOG    (2)
#define FS_POWER     (3)

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
    int         EoE;
    char*       EoEMAC;
    char*       EoEIP;
    char*       EoESubnetIP;
    char*       EoEGatewayIP;
    char*       EoEDNSIP;
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
    uint32_t    MaxVP;
    uint32_t    MaxVS;
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
    int        *MappingVP;
    int        *MappingVS;
    void       *PrivateObj;
    void       *PrivateData;

} _MyEcatSlaveSetting;

typedef struct myecat_setting {

    char  *ObjectName;
    char  *ENIName;
    bool  Redundant;
    bool  IgnoreOverride;
    bool  SyncSafeOP;
    bool  AutoRestart;
    int   DcMode;
    int   EoE;
    char* EoEMAC;  
    char* EoEIP;
    char* EoESubnetIP;
    char* EoEGatewayIP;
    char* EoEDNSIP;
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
    } else if (slave->Type == SLAVE_CIQ) {
        if (slave->MappingDO) free(slave->MappingDO);
        if (slave->MappingDI) free(slave->MappingDI);
        if (slave->MappingAO) free(slave->MappingAO);
        if (slave->MappingAI) free(slave->MappingAI);
        if (slave->MappingSerial) free(slave->MappingSerial);
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

static inline int __parse_power_mapping(XMLElement *root, _MyEcatSlaveSetting *setting)
{
    XMLElement *element;
    int i;

    /* VP */
    setting->MaxVP = 0;
    element = root->FirstChildElement("VPNumber");
    while (element) {
        setting->MaxVP++;
        element = element->NextSiblingElement("VPNumber");
    }
    setting->MappingVP = (int *)malloc(sizeof(int) * setting->MaxVP);
    if (setting->MappingVP == NULL)
        return -1;
    i = 0;
    element = root->FirstChildElement("VPNumber");
    while (element) {
        if (getHexDecValue(element, &setting->MappingVP[i++], sizeof(*setting->MappingVP)) <= 0)
            return -1;
        element = element->NextSiblingElement("VPNumber");
    }

    /* VS */
    setting->MaxVS = 0;
    element = root->FirstChildElement("VSNumber");
    while (element) {
        setting->MaxVS++;
        element = element->NextSiblingElement("VSNumber");
    }
    setting->MappingVS = (int *)malloc(sizeof(int) * setting->MaxVS);
    if (setting->MappingVS == NULL)
        return -1;
    i = 0;
    element = root->FirstChildElement("VSNumber");
    while (element) {
        if (getHexDecValue(element, &setting->MappingVS[i++], sizeof(*setting->MappingVS)) <= 0)
            return -1;
        element = element->NextSiblingElement("VSNumber");
    }
    return 0;
}

static inline int __parse_ciq_mapping(XMLElement *root, _MyEcatSlaveSetting *setting)
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
    
    /* Analog Output. */
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

    /* Analog Input. */
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
    
    getHexDecValueElement(root, "EoE", &setting->EoE, sizeof(setting->EoE));
    getStringElement(root, "EoEMAC", &setting->EoEMAC);
    getStringElement(root, "EoEIP", &setting->EoEIP);
    getStringElement(root, "EoESubnetIP", &setting->EoESubnetIP);
    getStringElement(root, "EoEGatewayIP", &setting->EoEGatewayIP);
    getStringElement(root, "EoEDNSIP", &setting->EoEDNSIP);
    
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
            
            if ((setting->VendorId == 0x00000B07 && setting->ProductCode == 0x00001003) ||
                (setting->VendorId == 0x000002BE && setting->ProductCode == 0x000013AF))
                MaxServos = 1; // special case, it only has ONE CiA402 parameter for NPM-AD1441A4
            else
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
    } else if (setting->Type == SLAVE_CIQ) {
        if (element && __parse_ciq_mapping(element, setting) < 0)
            return -1;
    } else if (setting->Type == SLAVE_ACP) {
        if (element && __parse_power_mapping(element, setting) < 0)
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
    
    getHexDecValueElement(root, "EoE", &setting->EoE, sizeof(setting->EoE));
    getStringElement(root, "EoEMAC", &setting->EoEMAC);
    getStringElement(root, "EoEIP", &setting->EoEIP);
    getStringElement(root, "EoESubnetIP", &setting->EoESubnetIP);
    getStringElement(root, "EoEGatewayIP", &setting->EoEGatewayIP);
    getStringElement(root, "EoEDNSIP", &setting->EoEDNSIP);
    
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
