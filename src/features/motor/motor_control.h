#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <stdint.h>
#include <stdbool.h>

#define MOTOR_ACCEL_LIMIT_RPM_PER_S 500
#define MOTOR_DECEL_LIMIT_RPM_PER_S 500
#define MOTOR_ESTOP_DECEL_RPM_PER_S 1000

#define MOTOR_DUTY_MIN 0
#define MOTOR_DUTY_START 10
#define MOTOR_DUTY_MAX 45

#define MOTOR_KP 0.08f
#define MOTOR_KI 0.02f

void initMotorControl(void);

uint16_t motorRampUpdate(uint16_t desiredSpeedRPM,  bool estopActive, float dtSeconds);
uint16_t motorControlGetReferenceSpeed(void);
void motorControlResetReferenceSpeed(void);


uint16_t motorPIUpdate(uint16_t referenceSpeedRPM, uint16_t actualSpeedRPM, float dtSeconds);
void motorPIReset(void);

#endif