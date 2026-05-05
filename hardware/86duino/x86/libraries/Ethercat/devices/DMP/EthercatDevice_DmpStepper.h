#ifndef ___WZ_ETHERCAT_DEVICE_DMP_STEPPER_H
#define ___WZ_ETHERCAT_DEVICE_DMP_STEPPER_H

#include "EthercatDevice_DmpCommon.h"
#include "EthercatDevice_CiA402.h"
#include <stdint.h>

class EthercatDevice_QECR11MP3S;
class EthercatDevice_QECR00MP3S;
class EthercatDevice_QECR11MV3S;
class EthercatDevice_QECR00MV3S;
class EthercatDevice_QECR11MV6S;

/* Machine Axes. */
#define ECAT_MACHINE_X_AXIS (0)
#define ECAT_MACHINE_Y_AXIS (1)
#define ECAT_MACHINE_Z_AXIS (2)
#define ECAT_MACHINE_A_AXIS (3)
#define ECAT_MACHINE_B_AXIS (4)
#define ECAT_MACHINE_C_AXIS (5)

/* Encoder List. */
#define ECAT_ENCODER_1      (0x01)
#define ECAT_ENCODER_2      (0x02)
#define ECAT_ENCODER_3      (0x03)
#define ECAT_ENCODER_4      (0x04)
#define ECAT_ENCODER_5      (0x05)
#define ECAT_ENCODER_6      (0x06)
#define ECAT_ENCODER_X      (0x11)
#define ECAT_ENCODER_Y      (0x12)
#define ECAT_ENCODER_Z      (0x13)
#define ECAT_ENCODER_A      (0x14)
#define ECAT_ENCODER_B      (0x15)
#define ECAT_ENCODER_C      (0x16)

class EthercatDevice_DmpStepper_Generic : public _EthercatDevice_DmpCommonDriver {

public:
    EthercatDevice_DmpStepper_Generic(uint32_t product = 0);
    ~EthercatDevice_DmpStepper_Generic();

    int attach(uint16_t slave_id, EthercatMaster *master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int attach(uint16_t slave_id, EthercatMaster &master, EthercatAttachMode mode = ECAT_SLAVE_NO);
    int detach();
    int update();

    #define ECAT_EMERGENCY_STOPPED                  (1)
    #define ECAT_MACHINE_X_AXIS_LIMIT_TOUCHED       (2)
    #define ECAT_MACHINE_Y_AXIS_LIMIT_TOUCHED       (3)
    #define ECAT_MACHINE_Z_AXIS_LIMIT_TOUCHED       (4)
    #define ECAT_MACHINE_A_AXIS_LIMIT_TOUCHED       (5)
    #define ECAT_MACHINE_B_AXIS_LIMIT_TOUCHED       (6)
    #define ECAT_MACHINE_C_AXIS_LIMIT_TOUCHED       (7)
    #define IS_ECAT_ENCODER_1_INDEX_RESET(event)    (((event) & 0x00000300) == 0x00000100)
    #define IS_ECAT_ENCODER_2_INDEX_RESET(event)    (((event) & 0x00000C00) == 0x00000400)
    #define IS_ECAT_ENCODER_3_INDEX_RESET(event)    (((event) & 0x00003000) == 0x00001000)
    #define IS_ECAT_ENCODER_X_INDEX_RESET(event)    (((event) & 0x0000C000) == 0x00004000)
    #define IS_ECAT_ENCODER_Y_INDEX_RESET(event)    (((event) & 0x00030000) == 0x00010000)
    #define IS_ECAT_ENCODER_Z_INDEX_RESET(event)    (((event) & 0x000C0000) == 0x00040000)
    #define IS_ECAT_ENCODER_1_OVERFLOW(event)       (((event) & 0x00000300) == 0x00000200)
    #define IS_ECAT_ENCODER_2_OVERFLOW(event)       (((event) & 0x00000C00) == 0x00000800)
    #define IS_ECAT_ENCODER_3_OVERFLOW(event)       (((event) & 0x00003000) == 0x00002000)
    #define IS_ECAT_ENCODER_X_OVERFLOW(event)       (((event) & 0x0000C000) == 0x00008000)
    #define IS_ECAT_ENCODER_Y_OVERFLOW(event)       (((event) & 0x00030000) == 0x00020000)
    #define IS_ECAT_ENCODER_Z_OVERFLOW(event)       (((event) & 0x000C0000) == 0x00080000)
    #define IS_ECAT_ENCODER_1_UNDERFLOW(event)      (((event) & 0x00000300) == 0x00000300)
    #define IS_ECAT_ENCODER_2_UNDERFLOW(event)      (((event) & 0x00000C00) == 0x00000C00)
    #define IS_ECAT_ENCODER_3_UNDERFLOW(event)      (((event) & 0x00003000) == 0x00003000)
    #define IS_ECAT_ENCODER_X_UNDERFLOW(event)      (((event) & 0x0000C000) == 0x0000C000)
    #define IS_ECAT_ENCODER_Y_UNDERFLOW(event)      (((event) & 0x00030000) == 0x00030000)
    #define IS_ECAT_ENCODER_Z_UNDERFLOW(event)      (((event) & 0x000C0000) == 0x000C0000)
    #define IS_ECAT_ENCODER_4_INDEX_RESET(event)    (((event) & 0x00300000) == 0x00100000)
    #define IS_ECAT_ENCODER_5_INDEX_RESET(event)    (((event) & 0x00C00000) == 0x00400000)
    #define IS_ECAT_ENCODER_6_INDEX_RESET(event)    (((event) & 0x03000000) == 0x01000000)
    #define IS_ECAT_ENCODER_A_INDEX_RESET(event)    (((event) & 0x0C000000) == 0x04000000)
    #define IS_ECAT_ENCODER_B_INDEX_RESET(event)    (((event) & 0x30000000) == 0x10000000)
    #define IS_ECAT_ENCODER_C_INDEX_RESET(event)    (((event) & 0xC0000000) == 0x40000000)
    #define IS_ECAT_ENCODER_4_OVERFLOW(event)       (((event) & 0x00300000) == 0x00200000)
    #define IS_ECAT_ENCODER_5_OVERFLOW(event)       (((event) & 0x00C00000) == 0x00800000)
    #define IS_ECAT_ENCODER_6_OVERFLOW(event)       (((event) & 0x03000000) == 0x02000000)
    #define IS_ECAT_ENCODER_A_OVERFLOW(event)       (((event) & 0x0C000000) == 0x08000000)
    #define IS_ECAT_ENCODER_B_OVERFLOW(event)       (((event) & 0x30000000) == 0x20000000)
    #define IS_ECAT_ENCODER_C_OVERFLOW(event)       (((event) & 0xC0000000) == 0x80000000)
    #define IS_ECAT_ENCODER_4_UNDERFLOW(event)      (((event) & 0x00300000) == 0x00300000)
    #define IS_ECAT_ENCODER_5_UNDERFLOW(event)      (((event) & 0x00C00000) == 0x00C00000)
    #define IS_ECAT_ENCODER_6_UNDERFLOW(event)      (((event) & 0x03000000) == 0x03000000)
    #define IS_ECAT_ENCODER_A_UNDERFLOW(event)      (((event) & 0x0C000000) == 0x0C000000)
    #define IS_ECAT_ENCODER_B_UNDERFLOW(event)      (((event) & 0x30000000) == 0x30000000)
    #define IS_ECAT_ENCODER_C_UNDERFLOW(event)      (((event) & 0xC0000000) == 0xC0000000)
    int attachInterrupt(void (*callback)(int));

    /***************************************
     *          Encoder Interface          *
     ***************************************/
    int encoderWrite(int encoder, int32_t value);
    int encoderDirectionRead(int encoder);
    int32_t encoderRead(int encoder);

    /***************************************
     *          CiA402 Servo Mode          *
     ***************************************/
    EthercatDevice_CiA402 *cia402GetServo(int motor, EthercatDevice_CiA402 *servo = NULL);

    /***************************************
     *   Machine G-Code Controller Mode    *
     ***************************************/
    int machineEnableSoftLimit();
    int machineDisableSoftLimit();
    int machineIsEmergencyStopped();
    int machineSetEmergencyStop();
    int machineClearEmergencyStop();
    int machineIsLimitTouched(int machine_axis);
    int machineIsHomingAttained();
    int machineIsPositionErrorExceeded(int machine_axis);
    int machineIsServoOn();
    int machineIsMoving();
    int machineServoOn();
    int machineServoOff();
    int machineSetHomingSpeed(int machine_axis, double mm_per_min);
    int machineStartHoming();
    int machineGcode(const char *fmt, ...);
    double machineActualPosition(int machine_axis);

    /***************************************
     *          Machine JOG Mode           *
     ***************************************/
    int machineJogMove(int machine_axis, double minimeters);
    int machineJogMoveContinuous(int machine_axis, double mm_per_min);

    /*******************************************************************
     *                                                                 *
     *        Internal EEPROM Parameter Configuration Functions        *
     *                                                                 *
     *******************************************************************/
    #define ECAT_CIA402_SERVO_MODE      (0)
    #define ECAT_GCODE_CONTROLLER_MODE  (1)
    int getDeviceMode();
    int configDeviceMode(uint8_t mode);
    /* -------------------------------------------------- */
    /*                CiA402 Configuration                */
    /* -------------------------------------------------- */
    /* Stepper Motor Resolution. (Unit: steps/revolution) */
    int configCiA402MotorResolution(int motor, uint32_t steps_per_rev);
    /* Encoder Position Feedback Source. */
    int configCiA402MotorPositionFeedbackSource(int motor, int encoder);
    /* Encoder Position Feedback Scale. */
    int configCiA402MotorPositionFeedbackScale(int motor, double scale);
    /* Encoder Position Feedback Offset. */
    int configCiA402MotorPositionFeedbackOffset(int motor, double offset);
    /* Stepper Motor Self-Starting Speed. (Unit: rev/min) */
    int configCiA402MotorSelfStartingSpeed(int motor, double rev_per_min);
    /* Target Velocity Unit. */
    #define ECAT_VELOCITY_UNIT_PPS  (0x00)
    #define ECAT_VELOCITY_UNIT_RPM  (0x01)
    int configCiA402MotorVelocityUnit(int motor, uint8_t velocity_unit);
    /* -------------------------------------------------- */
    /*           Gcode Controller Configuration           */
    /* -------------------------------------------------- */
    #define ECAT_MACHINE_GCODE_MODE (0)
    #define ECAT_MACHINE_JOG_MODE   (1)
    int configMachineMode(int mode);
    /* Machine Axis Mapping:
        0 - X(Motor 1), Y(Motor 2), Z(Motor 3)
        1 - X(Motor 1), Y(Motor 3), Z(Motor 2)
        2 - X(Motor 2), Y(Motor 1), Z(Motor 3)
        3 - X(Motor 2), Y(Motor 3), Z(Motor 1)
        4 - X(Motor 3), Y(Motor 1), Z(Motor 2)
        5 - X(Motor 3), Y(Motor 2), Z(Motor 1) . */
    int configMachineAxisMapping(uint8_t mapping);
    int configMachineAxisSource(int machine_axis, int motor);
    /* Default Feedrate. (Unit: mm/min) */
    int configMachineDefaultFeedrate(double mm_per_min);
    /* Default Homing Speed. (Unit: mm/min) */
    int configMachineDefaultHomingSpeed(int machine_axis, double mm_per_min);
    /* Homing Direction. */
    int configMachineHomingDirection(int machine_axis, bool positive);
    /* Homing Priority. */
    int configMachineHomingPriority(int machine_axis, uint8_t priority);
    /* Maximum Velocity. (Unit: mm/sec) */
    int configMachineMaxVelocity(int machine_axis, double mm_per_sec);
    /* Maximum Acceleration. (Unit: mm/sec^2) */
    int configMachineMaxAcceleration(double mm_per_sec2);
    /* Soft Limit. (Unit: mm) */
    int configMachineSoftLimit(int machine_axis, double min, double max);
    /* Pulses Per Unit. (Unit: pulses/mm) */
    int configMachinePPU(int machine_axis, double pulses_per_mm);
    /* Machine Axis Direction. */
    int configMachineAxisDirection(int machine_axis, int invert);
    /* Encoder Position Feedback Source. */
    int configMachinePositionFeedbackSource(int machine_axis, int encoder);
    /* Encoder Position Feedback Scale. */
    int configMachinePositionFeedbackScale(int machine_axis, double scale);
    /* Encoder Position Feedback Offset. */
    int configMachinePositionFeedbackOffset(int machine_axis, double offset);
    /* Step Loss Compensation Mode. */
    int configMachineStepLossCompensationMode(int machine_axis, uint8_t mode);
    /* Step Loss Compensation Maximum Error. */
    int configMachineStepLossCompensationMaxError(int machine_axis, double max_error);
    /* Stepper Motor Self-Starting Speed. (Unit: rev/min) */
    int configMachineSelfStartingSpeed(int machine_axis, double rev_per_min);
    /* Work offset for G54 to G59 work coordinate systems. (Unit: mm) */
    int configMachineG54WorkOffset(double x_offset, double y_offset, double z_offset);
    int configMachineG55WorkOffset(double x_offset, double y_offset, double z_offset);
    int configMachineG56WorkOffset(double x_offset, double y_offset, double z_offset);
    int configMachineG57WorkOffset(double x_offset, double y_offset, double z_offset);
    int configMachineG58WorkOffset(double x_offset, double y_offset, double z_offset);
    int configMachineG59WorkOffset(double x_offset, double y_offset, double z_offset);
    int configMachineG54WorkOffset(int machine_axis, double offset);
    int configMachineG55WorkOffset(int machine_axis, double offset);
    int configMachineG56WorkOffset(int machine_axis, double offset);
    int configMachineG57WorkOffset(int machine_axis, double offset);
    int configMachineG58WorkOffset(int machine_axis, double offset);
    int configMachineG59WorkOffset(int machine_axis, double offset);
    #define ECAT_JOG_MODE_INCHING       (0)
    #define ECAT_JOG_MODE_CONTINUOUS    (1)
    int configMachineJogMode(int machine_axis, int mode);
    /* -------------------------------------------------- */
    /*               Encoder Configuration                */
    /* -------------------------------------------------- */
    /* Encoder modes. */
    #define ECAT_ENCODER_MODE_STEP_DIR      (0)
    #define ECAT_ENCODER_MODE_CWCCW         (1)
    #define ECAT_ENCODER_MODE_AB_PHASE      (2)
    #define ECAT_ENCODER_MODE_STEP_DIR_x2   (5)
    #define ECAT_ENCODER_MODE_CWCCW_x2      (6)
    #define ECAT_ENCODER_MODE_AB_PHASE_x2   (7)
    int configEncoderMode(int encoder, uint8_t mode);
    /* Digital Filter. (Unit: 10 ns) */
    int configEncoderDigitalFilter(int encoder, uint32_t width);
    /* Encoder Range. */
    int configEncoderRange(int encoder, uint32_t value);
    /* Input Polarity. */
    int configEncoderInputPolarity(int encoder, bool pin_a, bool pin_b, bool pin_z);
    /* Index Reset. */
    int configEncoderIndexReset(int encoder, bool enable);

    /*******************************************************************
     *  The following APIs are only for internal use.                  *
     *******************************************************************/
    /* Get CiA402 Configuration Functions. */
    uint32_t getCiA402MotorResolution(int motor);
    int getCiA402MotorPositionFeedbackSource(int motor);
    double getCiA402MotorPositionFeedbackScale(int motor);
    double getCiA402MotorPositionFeedbackOffset(int motor);
    double getCiA402MotorSelfStartingSpeed(int motor);
    int getCiA402MotorVelocityUnit(int motor);
    /* Get Gcode Controller Configuration Functions. */
    int getMachineMode();
    int getMachineAxisMapping();
    int getMachineAxisSource(int machine_axis);
    int getMachineAxisDirection(int machine_axis);
    int getMachineHomingDirection(int machine_axis);
    int getMachineHomingPriority(int machine_axis);
    double getMachineDefaultFeedrate();
    double getMachineDefaultHomingSpeed(int machine_axis);
    double getMachineMaxVelocity(int machine_axis);
    double getMachineMaxAcceleration();
    double getMachineSoftLimitMax(int machine_axis);
    double getMachineSoftLimitMin(int machine_axis);
    double getMachinePPU(int machine_axis);
    int getMachinePositionFeedbackSource(int machine_axis);
    double getMachinePositionFeedbackScale(int machine_axis);
    double getMachinePositionFeedbackOffset(int machine_axis);
    int getMachineStepLossCompensationMode(int machine_axis);
    double getMachineStepLossCompensationMaxError(int machine_axis);
    double getMachineSelfStartingSpeed(int machine_axis);
    double getMachineG54WorkOffset(int machine_axis);
    double getMachineG55WorkOffset(int machine_axis);
    double getMachineG56WorkOffset(int machine_axis);
    double getMachineG57WorkOffset(int machine_axis);
    double getMachineG58WorkOffset(int machine_axis);
    double getMachineG59WorkOffset(int machine_axis);
    int getMachineJogMode(int machine_axis);
    /* Get Encoder Configuration Functions. */
    int getEncoderMode(int encoder);
    #define ECAT_ENCODER_PIN_A  (0)
    #define ECAT_ENCODER_PIN_B  (1)
    #define ECAT_ENCODER_PIN_Z  (2)
    int getEncoderInputPolarity(int encoder, int pin);
    int getEncoderIndexReset(int encoder);
    uint32_t getEncoderDigitalFilter(int encoder);
    uint32_t getEncoderRange(int encoder);

protected:
    int nDrives;

private:
    void *private_data;
    int _machineAxisSourceEnabled;

};

class EthercatDevice_QECR11MP3S : public EthercatDevice_DmpStepper_Generic {
public:
    EthercatDevice_QECR11MP3S();
};

class EthercatDevice_QECR00MP3S : public EthercatDevice_DmpStepper_Generic {
public:
    EthercatDevice_QECR00MP3S();
};

class EthercatDevice_QECR11MV3S : public EthercatDevice_DmpStepper_Generic {
public:
    EthercatDevice_QECR11MV3S();
};

class EthercatDevice_QECR00MV3S : public EthercatDevice_DmpStepper_Generic {
public:
    EthercatDevice_QECR00MV3S();
};

class EthercatDevice_QECR11MV6S : public EthercatDevice_DmpStepper_Generic {
public:
    EthercatDevice_QECR11MV6S();
};

#endif
