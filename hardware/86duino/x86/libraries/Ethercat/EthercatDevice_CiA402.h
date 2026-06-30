#ifndef ___WZ_ETHERCAT_DEVICE_CIA402_H
#define ___WZ_ETHERCAT_DEVICE_CIA402_H

#include "EthercatDevice.h"

enum EthercatCiA402State {

    CIA402_NOT_READY_TO_SWITCH_ON   = 0,
    CIA402_SWITCH_ON_DISABLED       = 1,
    CIA402_READY_TO_SWITCH_ON       = 2,
    CIA402_SWITCHED_ON              = 3,
    CIA402_OPERATION_ENABLED        = 4,
    CIA402_FAULT                    = 5,
    CIA402_FAULT_REACTION_ACTIVE    = 6,
    CIA402_QUICK_STOP_ACTIVE        = 7

};

enum EthercatCiA402Mode {

    CIA402_PP_MODE      = 1,
    CIA402_PV_MODE      = 3,
    CIA402_TQ_MODE      = 4,
    CIA402_HOMING_MODE  = 6,
    CIA402_CSP_MODE     = 8,
    CIA402_CSV_MODE     = 9,
    CIA402_CST_MODE     = 10,

    /************************************************************************
     *  The following macro definition is outdated. It is recommended to    *
     *  avoid using it.                                                     *
     ************************************************************************/
    CIA402_PT_MODE      = CIA402_TQ_MODE

};

enum EthercatCiA402HomingMethod {

    CIA402_HM0  = 0,
    CIA402_HM1  = 1,
    CIA402_HM2  = 2,
    CIA402_HM3  = 3,
    CIA402_HM4  = 4,
    CIA402_HM5  = 5,
    CIA402_HM6  = 6,
    CIA402_HM7  = 7,
    CIA402_HM8  = 8,
    CIA402_HM9  = 9,
    CIA402_HM10 = 10,
    CIA402_HM11 = 11,
    CIA402_HM12 = 12,
    CIA402_HM13 = 13,
    CIA402_HM14 = 14,
    CIA402_HM17 = 17,
    CIA402_HM18 = 18,
    CIA402_HM19 = 19,
    CIA402_HM20 = 20,
    CIA402_HM21 = 21,
    CIA402_HM22 = 22,
    CIA402_HM23 = 23,
    CIA402_HM24 = 24,
    CIA402_HM25 = 25,
    CIA402_HM26 = 26,
    CIA402_HM27 = 27,
    CIA402_HM28 = 28,
    CIA402_HM29 = 29,
    CIA402_HM30 = 30,
    CIA402_HM33 = 33,
    CIA402_HM34 = 34,
    CIA402_HM35 = 35,

    /************************************************************************
     *  The following macro definitions are outdated. It is recommended to  *
     *  avoid using them.                                                   *
     ************************************************************************/
    CIA402_HM0_NONE                             = CIA402_HM0,
    CIA402_HM1_NEG_LIMIT_POS_INDEX              = CIA402_HM1,
    CIA402_HM2_POS_LIMIT_NEG_INDEX              = CIA402_HM2,
    CIA402_HM3_POS_HOME_NEG_INDEX               = CIA402_HM3,
    CIA402_HM4_POS_HOME_POS_INDEX               = CIA402_HM4,
    CIA402_HM5_NEG_HOME_POS_INDEX               = CIA402_HM5,
    CIA402_HM6_NEG_HOME_NEG_INDEX               = CIA402_HM6,
    CIA402_HM7_POS_LIMIT_POS_HOME_NEG_INDEX     = CIA402_HM7,
    CIA402_HM8_POS_LIMIT_POS_HOME_POS_INDEX     = CIA402_HM8,
    CIA402_HM9_POS_LIMIT_NEG_HOME_NEG_INDEX     = CIA402_HM9,
    CIA402_HM10_POS_LIMIT_NEG_HOME_POS_INDEX    = CIA402_HM10,
    CIA402_HM11_NEG_LIMIT_NEG_HOME_POS_INDEX    = CIA402_HM11,
    CIA402_HM12_NEG_LIMIT_NEG_HOME_NEG_INDEX    = CIA402_HM12,
    CIA402_HM13_NEG_LIMIT_POS_HOME_POS_INDEX    = CIA402_HM13,
    CIA402_HM14_NEG_LIMIT_POS_HOME_NEG_INDEX    = CIA402_HM14,
    CIA402_HM17_NEG_LIMIT                       = CIA402_HM17,
    CIA402_HM18_POS_LIMIT                       = CIA402_HM18,
    CIA402_HM19_POS_HOME                        = CIA402_HM19,
    CIA402_HM20_POS_HOME                        = CIA402_HM20,
    CIA402_HM21_NEG_HOME                        = CIA402_HM21,
    CIA402_HM22_NEG_HOME                        = CIA402_HM22,
    CIA402_HM23_POS_LIMIT_POS_HOME              = CIA402_HM23,
    CIA402_HM24_POS_LIMIT_POS_HOME              = CIA402_HM24,
    CIA402_HM25_POS_LIMIT_NEG_HOME              = CIA402_HM25,
    CIA402_HM26_POS_LIMIT_NEG_HOME              = CIA402_HM26,
    CIA402_HM27_NEG_LIMIT_NEG_HOME              = CIA402_HM27,
    CIA402_HM28_NEG_LIMIT_NEG_HOME              = CIA402_HM28,
    CIA402_HM29_NEG_LIMIT_POS_HOME              = CIA402_HM29,
    CIA402_HM30_NEG_LIMIT_POS_HOME              = CIA402_HM30,
    CIA402_HM33_NEG_INDEX                       = CIA402_HM33,
    CIA402_HM34_POS_INDEX                       = CIA402_HM34,
    CIA402_HM35_CURRENT_POS                     = CIA402_HM35

};

class EthercatDevice_CiA402 : public EthercatDevice {
public:
    EthercatDevice_CiA402(uint32_t vendor = 0, uint32_t product = 0);
    ~EthercatDevice_CiA402();

    int attach(uint16_t slave_id, EthercatMaster *master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int attach(uint16_t slave_id, EthercatMaster &master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int attach(uint16_t slave_id, uint8_t drive_id, EthercatMaster *master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int attach(uint16_t slave_id, uint8_t drive_id, EthercatMaster &master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int detach();

    int isStepper();
    int getCiA402Mode();
    int setCiA402Mode(int mode, uint32_t timeout_ms = 1000);
    int getCiA402State();
    int setCiA402State(int state, uint32_t timeout_ms = 1000);
    int enable(uint32_t timeout_ms = 1000);
    int disable(uint32_t timeout_ms = 1000);

    /* Servo Control. */
    int setTargetPosition(int32_t value);
    int setTargetVelocity(int32_t value);
    int setTargetTorque(int16_t value);
    int setProfileAcceleration(uint32_t value);
    int setProfileDeceleration(uint32_t value);
    int setMaxAcceleration(uint32_t value);
    int setMaxDeceleration(uint32_t value);
    int setMaxProfileVelocity(uint32_t value);
    int setMotionProfileType(int16_t value);
    
    /* Position Configuration. */
    int setPositionWindow(uint32_t value);
    int setPositionWindowTime(uint16_t value);
    int setPositionOffset(int32_t value);
    int setSoftwarePositionLimit(int32_t min_value, int32_t max_value);
    int setFollowingErrorWindow(uint32_t value);
    #define CIA402_POLARITY_POSITIVE    (0)
    #define CIA402_POLARITY_NEGATIVE    (1)
    int setPositionPolarity(int polarity);

    /* Velocity Configuration. */
    int setVelocityWindow(uint16_t value);
    int setVelocityWindowTime(uint16_t value);
    int setVelocityThreshold(uint16_t value);
    int setVelocityOffset(int32_t value);
    int setMaxMotorSpeed(uint32_t value);
    int setVelocityPolarity(int polarity);

    /* Torque Configuration. */
    int setTorqueOffset(int16_t value);
    int setMaxTorque(uint16_t value);
    int setPositiveTorqueLimit(uint16_t value);
    int setNegativeTorqueLimit(uint16_t value);

    /* Quick Stop Configuration. */
    int setQuickStopDeceleration(uint32_t value);

    /* Error Behavior Option Code. */
    int setQuickStopOptionCode(int16_t value);
    int setShutdownOptionCode(int16_t value);
    int setDisableOperationOptionCode(int16_t value);
    int setHaltOptionCode(int16_t value);
    int setFaultReactionOptionCode(int16_t value);

    /* Servo Information. */
    uint16_t getErrorCode();
    uint32_t getSupportedDriveModes();
    uint32_t getMotorResolution();

    /* Servo Status. */
    int32_t getPositionActualValue();
    int32_t getVelocityActualValue();
    int16_t getTorqueActualValue();
    int16_t getCurrentActualValue();

    /* Position Details. */
    int32_t getPositionDemandValue();
    int32_t getPositionDemandInternalValue();
    int32_t getPositionActualInternalValue();
    int32_t getAdditionalPositionActualValue(int ordinal = 1);
    int32_t getFollowingErrorActualValue();

    /* Velocity Details. */
    int32_t getVelocityDemandValue();

    /* Torque Details. */
    int16_t getTorqueDemandValue();

    /* Other Functions. */
    uint32_t getDigitalInputs();

    /* Profile Position. */
    int pp_SetVelocity(uint32_t value);
    int pp_SetAcceleration(uint32_t value);
    int pp_SetDeceleration(uint32_t value);
    int pp_SetMotionProfileType(int16_t value);
    #define CIA402_PP_ABSOLUTE   (0)
    #define CIA402_PP_RELATIVE   (1)
    int pp_Run(int32_t position, int relative = CIA402_PP_ABSOLUTE, bool change_immediately = false);
    int pp_IsTargetReached();
    int pp_CheckFollowingError();
    int pp_Halt(uint32_t timeout_ms = 10000);
    int pp_Resume();

    /* Profile Velocity */
    int pv_SetAcceleration(uint32_t value);
    int pv_SetDeceleration(uint32_t value);
    int pv_SetMotionProfileType(int16_t value);
    int pv_Run(int32_t velocity);
    int pv_IsTargetReached();
    int pv_CheckZeroSpeed();
    int pv_CheckMaxSlippageError();
    int pv_Halt(uint32_t timeout_ms = 10000);
    int pv_Resume();

    /* Profile Torque. */
    int tq_SetTorqueSlope(uint32_t value);
    int tq_SetTorqueProfileType(int16_t value);
    int tq_SetMotorRatedCurrent(uint32_t value);
    int tq_SetMotorRatedTorque(uint32_t value);
    int tq_Run(int16_t torque);
    int tq_IsTargetReached();
    int tq_Halt(uint32_t timeout_ms = 10000);
    int tq_Resume();

    /* Homing. */
    int hm_SetHomeOffset(int32_t value);
    int hm_SetHomingMethod(int8_t value);
    int hm_SetHomingSpeeds(uint32_t speed_during_search_switch, uint32_t speed_during_search_zero);
    int hm_SetHomingAcceleration(uint32_t value);
    int hm_Run();
    #define CIA402_HM_RUNNING   (0)
    #define CIA402_HM_ATTAINED  (1)
    #define CIA402_HM_ERROR     (2)
    int hm_IsAttained();
    int hm_Stop(uint32_t timeout_ms = 10000);

    /* Touch Probe. */
    #define CIA402_TPMODE_FIRST_EVENT   (0)
    #define CIA402_TPMODE_CONTINUOUS    (1)
    #define CIA402_TPSRC_TP_INPUT       (1 << 16)
    #define CIA402_TPSRC_ENCODER_Z      (2 << 16)
    #define CIA402_TPSRC_DIGITAL_INPUT1 (1)
    #define CIA402_TPSRC_DIGITAL_INPUT2 (2)
    #define CIA402_TPSRC_DIGITAL_INPUT3 (3)
    #define CIA402_TPSRC_DIGITAL_INPUT4 (4)
    #define CIA402_TPSRC_HW_ENCODER_Z   (5)
    #define CIA402_TPSRC_SW_ENCODER_Z   (6)
    #define CIA402_TPTIMING_NONE        (0)
    #define CIA402_TPTIMING_POS_EDGE    (1)
    #define CIA402_TPTIMING_NEG_EDGE    (2)
    #define CIA402_TPTIMING_BOTH        (3)
    int enableTouchProbe1(int trigger_mode, int trigger_src, int trigger_timing);
    int enableTouchProbe2(int trigger_mode, int trigger_src, int trigger_timing);
    int disableTouchProbe1();
    int disableTouchProbe2();
    #define CIA402_TPVALUE_POS_EDGE     (0)
    #define CIA402_TPVALUE_NEG_EDGE     (1)
    bool isTouchProbe1ValueReady(int value_selector);
    bool isTouchProbe2ValueReady(int value_selector);
    int32_t readTouchProbe1Value(int value_selector);
    int32_t readTouchProbe2Value(int value_selector);

    /* Low-level functions for mode-specific flow control. (Advanced) */
    int setHaltBit(bool halt);
    int isTargetReached();
    int setModeSpecificBit4(bool bit_value);
    int setModeSpecificBit5(bool bit_value);
    int setModeSpecificBit6(bool bit_value);
    int checkModeSpecificBit12();
    int checkModeSpecificBit13();

    /************************************************************************
     *  These functions are outdated. It is recommended that you use the    *
     *  newer functions provided above.                                     *
     ************************************************************************/
    int driveIsStepper();
    int driveGetMode();
    int driveSetMode(EthercatCiA402Mode mode, uint32_t timeout_ms = 1000);
    int driveGetState();
    int driveSetState(EthercatCiA402State state, uint32_t timeout_ms = 1000);
    int driveEnable(uint32_t timeout_ms = 1000);
    int driveDisable(uint32_t timeout_ms = 1000);
    int driveIsTargetReached();
    int driveIsHalt();
    int driveHalt(bool halt = true);

    /* Profile Position. */
    int profilePositionBegin(int32_t pos, int32_t vel = 0, uint32_t acc = 0, uint32_t dec = 0, bool rel = 0);
    int profilePositionIsFollowingError();
    int profilePositionEnd();

    /* Profile Velocity */
    int profileVelocityBegin(int32_t vel, uint32_t acc = 0, uint32_t dec = 0);
    int profileVelocityIsZeroSpeed();
    int profileVelocityIsMaxSlippageError();
    int profileVelocityEnd();

    /* Profile Torque. */
    int profileTorqueBegin(int16_t torque, uint32_t slope = 0);
    int profileTorqueEnd();

    /* Homing. */
    int homingBegin(int8_t method, int32_t offset = 0, uint32_t maxvel = 0, uint32_t zerovel = 0, uint32_t acc = 0);
    int homingIsAttained();
    int homingIsError();
    int homingEnd();

    /* Touch Probe. */
    int touchProbeBegin(uint8_t tp, uint8_t source, bool oneshot = true);
    int touchProbeEnablePositiveEdge(uint8_t tp);
    int touchProbeEnableNegativeEdge(uint8_t tp);
    int touchProbeIsPositiveEdge(uint8_t tp);
    int touchProbeIsNegativeEdge(uint8_t tp);
    int touchProbeReadPositiveValue(uint8_t tp);
    int touchProbeReadNegativeValue(uint8_t tp);
    int touchProbeEnd(uint8_t tp);

    /* Set Output Objects. */
    int driveSetControlword(uint16_t value, bool sdo = false);
    int driveSetQuickStopOptionCode(int16_t value);
    int driveSetShutdownOptionCode(int16_t value);
    int driveSetDisableOperationOptionCode(int16_t value);
    int driveSetHaltOptionCode(int16_t value);
    int driveSetFaultReactionOptionCode(int16_t value);
    int driveSetOperationMode(int8_t value, bool sdo = false);
    int driveSetFollowingErrorWindow(uint32_t value, bool sdo = false);
    int driveSetPositionWindow(uint32_t value, bool sdo = false);
    int driveSetPositionWindowTime(uint16_t value, bool sdo = false);
    int driveSetVelocityWindow(uint16_t value, bool sdo = false);
    int driveSetVelocityWindowTime(uint16_t value, bool sdo = false);
    int driveSetVelocityThreshold(uint16_t value, bool sdo = false);
    int driveSetTargetTorque(int16_t value, bool sdo = false);
    int driveSetMaxTorque(uint16_t value, bool sdo = false);
    int driveSetTargetPosition(int32_t value, bool sdo = false);
    int driveSetHomeOffset(int32_t value, bool sdo = false);
    int driveSetMinSoftwarePositionLimit(int32_t value, bool sdo = false);
    int driveSetMaxSoftwarePositionLimit(int32_t value, bool sdo = false);
    int driveSetPolarity(uint8_t value, bool sdo = false);
    int driveSetMaxProfileVelocity(uint32_t value, bool sdo = false);
    int driveSetMaxMotorSpeed(uint32_t value, bool sdo = false);
    int driveSetProfileVelocity(uint32_t value, bool sdo = false);
    int driveSetProfileAcceleration(uint32_t value, bool sdo = false);
    int driveSetProfileDeceleration(uint32_t value, bool sdo = false);
    int driveSetQuickStopDeceleration(uint32_t value, bool sdo = false);
    int driveSetTorqueSlope(uint32_t value, bool sdo = false);
    int driveSetPositionFactorNumerator(uint32_t value, bool sdo = false);
    int driveSetPositionFactorFeedConstant(uint32_t value, bool sdo = false);
    int driveSetHomingMethod(int8_t value, bool sdo = false);
    int driveSetHomingSpeedDuringSearchForSwitch(uint32_t value, bool sdo = false);
    int driveSetHomingSpeedDuringSearchForZero(uint32_t value, bool sdo = false);
    int driveSetHomingAcceleration(uint32_t value, bool sdo = false);
    int driveSetPositionOffset(int32_t value, bool sdo = false);
    int driveSetVelocityOffset(int32_t value, bool sdo = false);
    int driveSetTorqueOffset(int16_t value, bool sdo = false);
    int driveSetTouchProbeFunction(uint16_t value, bool sdo = false);
    int driveSetTouchProbeSource(uint8_t tp, int16_t value);
    int driveSetMaxAcceleration(uint32_t value, bool sdo = false);
    int driveSetMaxDeceleration(uint32_t value, bool sdo = false);
    int driveSetPositiveTorqueLimit(uint16_t value, bool sdo = false);
    int driveSetNegativeTorqueLimit(uint16_t value, bool sdo = false);
    int driveSetTargetVelocity(int32_t value, bool sdo = false);
    /* Get Output Objects. */
    uint16_t driveGetControlword(bool sdo = false);
    int16_t  driveGetQuickStopOptionCode();
    int16_t  driveGetShutdownOptionCode();
    int16_t  driveGetDisableOperationOptionCode();
    int16_t  driveGetHaltOptionCode();
    int16_t  driveGetFaultReactionOptionCode();
    int8_t   driveGetOperationMode(bool sdo = false);
    uint32_t driveGetFollowingErrorWindow(bool sdo = false);
    uint32_t driveGetPositionWindow(bool sdo = false);
    uint16_t driveGetPositionWindowTime(bool sdo = false);
    uint16_t driveGetVelocityWindow(bool sdo = false);
    uint16_t driveGetVelocityWindowTime(bool sdo = false);
    uint16_t driveGetVelocityThreshold(bool sdo = false);
    int16_t  driveGetTargetTorque(bool sdo = false);
    uint16_t driveGetMaxTorque(bool sdo = false);
    int32_t  driveGetTargetPosition(bool sdo = false);
    int32_t  driveGetHomeOffset(bool sdo = false);
    int32_t  driveGetMinSoftwarePositionLimit(bool sdo = false);
    int32_t  driveGetMaxSoftwarePositionLimit(bool sdo = false);
    uint8_t  driveGetPolarity(bool sdo = false);
    uint32_t driveGetMaxProfileVelocity(bool sdo = false);
    uint32_t driveGetMaxMotorSpeed(bool sdo = false);
    uint32_t driveGetProfileVelocity(bool sdo = false);
    uint32_t driveGetProfileAcceleration(bool sdo = false);
    uint32_t driveGetProfileDeceleration(bool sdo = false);
    uint32_t driveGetQuickStopDeceleration(bool sdo = false);
    uint32_t driveGetTorqueSlope(bool sdo = false);
    uint32_t driveGetPositionFactorNumerator(bool sdo = false);
    uint32_t driveGetPositionFactorFeedConstant(bool sdo = false);
    int8_t   driveGetHomingMethod(bool sdo = false);
    uint32_t driveGetHomingSpeedDuringSearchForSwitch(bool sdo = false);
    uint32_t driveGetHomingSpeedDuringSearchForZero(bool sdo = false);
    uint32_t driveGetHomingAcceleration(bool sdo = false);
    int32_t  driveGetPositionOffset(bool sdo = false);
    int32_t  driveGetVelocityOffset(bool sdo = false);
    int16_t  driveGetTorqueOffset(bool sdo = false);
    uint16_t driveGetTouchProbeFunction(bool sdo = false);
    int16_t  driveGetTouchProbeSource(uint8_t tp);
    uint32_t driveGetMaxAcceleration(bool sdo = false);
    uint32_t driveGetMaxDeceleration(bool sdo = false);
    uint16_t driveGetPositiveTorqueLimit(bool sdo = false);
    uint16_t driveGetNegativeTorqueLimit(bool sdo = false);
    int32_t  driveGetTargetVelocity(bool sdo = false);
    uint32_t driveGetMotorRatedCurrent(bool sdo = false);
    uint32_t driveGetMotorRatedTorque(bool sdo = false);
    int16_t  driveGetMotionProfileType(bool sdo = false);
    /* Get Input Objects. */
    uint16_t driveGetErrorCode(bool sdo = false);
    uint16_t driveGetStatusword(bool sdo = false);
    int8_t   driveGetOperationModeDisplay(bool sdo = false);
    int32_t  driveGetPositionDemandValue(bool sdo = false);
    int32_t  driveGetPositionActualInternalValue(bool sdo = false);
    int32_t  driveGetPositionActualValue(bool sdo = false);
    int32_t  driveGetVelocityDemandValue(bool sdo = false);
    int32_t  driveGetVelocityActualValue(bool sdo = false);
    int16_t  driveGetTorqueDemandValue(bool sdo = false);
    int16_t  driveGetTorqueActualValue(bool sdo = false);
    int16_t  driveGetCurrentActualValue(bool sdo = false);
    uint16_t driveGetTouchProbeStatus(bool sdo = false);
    int32_t  driveGetTouchProbePos1PositiveValue(bool sdo = false);
    int32_t  driveGetTouchProbePos1NegativeValue(bool sdo = false);
    int32_t  driveGetTouchProbePos2PositiveValue(bool sdo = false);
    int32_t  driveGetTouchProbePos2NegativeValue(bool sdo = false);
    int32_t  driveGetFollowingErrorActualValue(bool sdo = false);
    int32_t  driveGetInternalPositionDemandValue(bool sdo = false);
    uint32_t driveGetDigitalInputs(bool sdo = false);
    uint32_t driveGetSupportedDriveModes(bool sdo = false);
    int32_t  driveGetAdditionalActualPosition(uint8_t num = 0, bool sdo = false);
    uint32_t driveGetMotorResolution();

    /************************************************************************
     *  The following APIs are only for internal use.                       *
     ************************************************************************/
    #define CIA402_OBJECT_ACCESS_AUTO       (0)
    #define CIA402_OBJECT_ACCESS_SDO_STRICT (1)
    static void _setCiA402ObjectAccessBehavior(int behavior);
    int remapping() override;
    int unmapping() override;
    int update() override;

private:
    static int _ObjectAccessBehavior;
    void *private_data;

    int getHaltBit();
    int getModeSpecificBit4();
    int setProfileVelocity(uint32_t value);
    int setTorqueSlope(uint32_t value);
    int setTorqueProfileType(int16_t value);
    int setMotorRatedCurrent(uint32_t value);
    int setMotorRatedTorque(uint32_t value);
    int setHomeOffset(int32_t value);
    int setHomingMethod(int8_t value);
    int setHomingSpeedDuringSearchSwitch(uint32_t value);
    int setHomingSpeedDuringSearchZero(uint32_t value);
    int setHomingAcceleration(uint32_t value);
    int setTouchProbeSource(int ordinal, int16_t value);
    int setTouchProbeFunction(uint16_t value);
    int setOperationMode(int8_t value);
    int setControlword(uint16_t value);
    uint16_t getControlword();
    int8_t getOperationModeDisplay();
    uint16_t getStatusword();
    uint16_t getTouchProbeFunction();
    uint16_t getTouchProbeStatus();
    int32_t getTouchProbePosition1PositiveValue();
    int32_t getTouchProbePosition1NegativeValue();
    int32_t getTouchProbePosition2PositiveValue();
    int32_t getTouchProbePosition2NegativeValue();

    int _checkSupportedDriveMode(int mode);
    int _checkWrongStateChange(int state);
    int _setCiA402Mode(int mode, uint32_t timeout_ms);
    int _setCiA402State(int state, uint32_t timeout_ms);
    int _getCiA402State();
    int _setCiA402Command(int command);
    template <typename T> T _getTxPdoObject(uint16_t index, uint8_t subindex, int pdo_offset);
    template <typename T> T _getRxPdoObject(uint16_t index, uint8_t subindex, int pdo_offset);
    template <typename T> int _setRxPdoObject(uint16_t index, uint8_t subindex, int pdo_offset, T value);

    int _Halt(uint32_t timeout_ms = 10000);
    int _Resume();
    int _enableTouchProbe(int tp, int trigger_mode, int trigger_src, int trigger_timing);
    int _disableTouchProbe(int tp);
    bool _isTouchProbeValueReady(int tp, int value_selector);
};

#endif
