#include "motor_control.h"
#include "features/sensors/devices/power_sensor.h"
#include "utils/uartstdio.h"

static float referenceSpeedRPM = 0.0f;
static float integralError = 0.0f;

#define K_w   0.0506f
#define K_i   0.4033f
#define K_int -1.0081f

#define INTEGRAL_MAX    800.0f
#define INTEGRAL_MIN   -800.0f

void initMotorControl(void)
{
    referenceSpeedRPM = 0.0f;
    integralError = 0.0f;
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

void motorControlSetReferenceSpeed(uint16_t rpm)
{
    referenceSpeedRPM = (float)rpm;
}

void motorControlResetReferenceSpeed(void)
{
    referenceSpeedRPM = 0.0f;
}


uint16_t motorLQRUpdate(uint16_t referenceSpeedRPM_in, uint16_t actualSpeedRPM, float dtSeconds)
{
    
    if (referenceSpeedRPM_in ==0) // to byypass min duty if estopping
    {
        integralError = 0.0f;
        return 0;
    }
    
    float omega = actualSpeedRPM * (2.0f * 3.1415926535f / 60.0f);
    float omegaRef = referenceSpeedRPM_in * (2.0f * 3.1415926535f / 60.0f);

    float error = omegaRef - omega;

    integralError += error * dtSeconds;

    if (integralError > INTEGRAL_MAX) integralError = INTEGRAL_MAX;
    if (integralError < INTEGRAL_MIN) integralError = INTEGRAL_MIN;

    float current = getCurrent();

    float u = - (K_w * omega) - (K_i * current) - (K_int * integralError);

    if (u > MOTOR_DUTY_MAX) 
    {
        u = MOTOR_DUTY_MAX;
        integralError -= error * dtSeconds;
    } 
    else if (u < MOTOR_DUTY_MIN) 
    {
        u = MOTOR_DUTY_MIN;
        integralError -= error * dtSeconds;
    }

    return (uint16_t)(u + 0.5f);
}

void motorControllerReset(void)
{
    integralError = 0.0f;
}

void motorControllerInit(void)
{
    integralError = 0.0f;
}


void motorSerialPlotOutput(uint16_t desiredSpeed, uint16_t referenceSpeed, uint16_t actualSpeed, uint16_t duty)
{
    UARTprintf("%u,%u,%u,%u\n", desiredSpeed, referenceSpeed, actualSpeed, duty);
}