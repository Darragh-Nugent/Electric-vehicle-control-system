#include "motor_control.h"

static float referenceSpeedRPM = 0.0f;
static float integralError = 0.0f;
static float dutyCommand = 0.0f;

void initMotorControl(void)
{
    referenceSpeedRPM = 0.0f;
    integralError = 0.0f;
    dutyCommand = 0.0f;
}

uint16_t motorRampUpdate(uint16_t desiredSpeedRPM, bool estopActive, float dtSeconds)
{
    float targetSpeed = (float)desiredSpeedRPM;

    if(estopActive)
    {
        targetSpeed = 0.0f;
    }

    float rampLimitRPMPerSecond;

    if(estopActive)
    {
        rampLimitRPMPerSecond = MOTOR_ESTOP_DECEL_RPM_PER_S;
    }
    else if(targetSpeed < referenceSpeedRPM)
    {
        rampLimitRPMPerSecond = MOTOR_DECEL_LIMIT_RPM_PER_S;
    }
    else
    {
        rampLimitRPMPerSecond = MOTOR_ACCEL_LIMIT_RPM_PER_S;
    }


    float maxStep = rampLimitRPMPerSecond * dtSeconds;

    if(referenceSpeedRPM < targetSpeed)
    {
        referenceSpeedRPM += maxStep;
        if(referenceSpeedRPM > targetSpeed)
        {
            referenceSpeedRPM = targetSpeed;
        }
    }
    else if(referenceSpeedRPM > targetSpeed)
    {
        referenceSpeedRPM -= maxStep;
        if(referenceSpeedRPM < targetSpeed)
        {
            referenceSpeedRPM = targetSpeed;
        }
    }
    return (uint16_t)referenceSpeedRPM; 

}

uint16_t motorControlGetReferenceSpeed(void)
{
    return (uint16_t)referenceSpeedRPM;
}

void motorControlResetReferenceSpeed(void)
{
    referenceSpeedRPM = 0.0f;
}


uint16_t motorPIUpdate(uint16_t referenceSpeedRPM, uint16_t actualSpeedRPM, float dtSeconds)
{
    float error = (float)referenceSpeedRPM - (float)actualSpeedRPM;

    integralError += error * dtSeconds;

    float controlOutput = MOTOR_KP * error + MOTOR_KI * integralError;

    dutyCommand += controlOutput;

    if(dutyCommand > MOTOR_DUTY_MAX)
    {
        dutyCommand = MOTOR_DUTY_MAX;
        integralError -= error * dtSeconds; 
    }
    else if(dutyCommand < MOTOR_DUTY_MIN)
    {
        dutyCommand = MOTOR_DUTY_MIN;
        integralError -= error * dtSeconds;
    }

    return (uint16_t)dutyCommand;
}

void motorPIReset(void)
{
    integralError = 0.0f;
    dutyCommand = 0.0f;
}