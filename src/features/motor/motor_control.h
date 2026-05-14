#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <stdint.h>
#include <stdbool.h>

#define MOTOR_ACCEL_LIMIT_RPM_PER_S 500
#define MOTOR_DECEL_LIMIT_RPM_PER_S 500
#define MOTOR_ESTOP_DECEL_RPM_PER_S 1000

#define MOTOR_DUTY_MIN 5
#define MOTOR_DUTY_START 13
#define MOTOR_DUTY_MAX 30 //45

#define MOTOR_KP 0.0005f
#define MOTOR_KI 0.0001f

void initMotorControl(void);

uint16_t motorRampUpdate(uint16_t desiredSpeedRPM,  bool estopActive, float dtSeconds);
uint16_t motorControlGetReferenceSpeed(void);
void motorControlSetReferenceSpeed(uint16_t rpm);
void motorControlResetReferenceSpeed(void);


uint16_t motorPIUpdate(uint16_t referenceSpeedRPM, uint16_t actualSpeedRPM, float dtSeconds);
void motorPIReset(void);
void motorPIInit(uint16_t startDuty);

void motorSerialPlotOutput(uint16_t desiredSpeed, uint16_t referenceSpeed, uint16_t actualSpeed, uint16_t duty);

#endif