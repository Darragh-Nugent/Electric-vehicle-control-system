#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "states.h"

uint16_t userSetSpeed = 0;

SemaphoreHandle_t motorStateMutex = NULL;
SemaphoreHandle_t motorSetSpeedMutex = NULL;
SemaphoreHandle_t motorStartSemaphore = NULL;
SemaphoreHandle_t motorUpToSpeedSemaphore = NULL;

extern motor_state_t motor_state;
extern void hallSensorIntEnable(void);
extern void kickStartMotor(void);
// Transition state to idle
void motorInit(void)
{
    xSemaphoreTake(motorStateMutex, portMAX_DELAY);
    motor_state = MOTOR_STATE_IDLE;
    xSemaphoreGive(motorStateMutex);
}

// Transition state to running.
void motorRunning(void)
{
    xSemaphoreTake(motorStateMutex, portMAX_DELAY);
    motor_state = MOTOR_STATE_RUNNING;
    xSemaphoreGive(motorStateMutex);
}

// Transition state to starting.
// Enable the hall effect sensor ISR and kick start the motor.
void motorStart(void)
{
    xSemaphoreTake(motorStateMutex, portMAX_DELAY);
    motor_state = MOTOR_STATE_STARTING;
    xSemaphoreGive(motorStateMutex);
    hallSensorIntEnable();
    kickStartMotor();
}

// Transition state to e-stop braking
void motorEStop(void)
{
    xSemaphoreTake(motorStateMutex, portMAX_DELAY);
    motor_state = MOTOR_STATE_BRAKING;
    xSemaphoreGive(motorStateMutex);
}

// Transition state to fault latched.
void motorFaultLatched(void)
{
    xSemaphoreTake(motorStateMutex, portMAX_DELAY);
    motor_state = MOTOR_STATE_FAULT;
    xSemaphoreGive(motorStateMutex);
}

// Set the user preferred motor speed (RPM).
void motorSetSpeed(uint16_t rpm)
{
    xSemaphoreTake(motorSetSpeedMutex, portMAX_DELAY);
    userSetSpeed = rpm;
    xSemaphoreGive(motorSetSpeedMutex);
}

// Get the speed of the motor (RPM).
uint16_t motorGetSpeed(void)
{
    uint16_t speed;
    xSemaphoreTake(motorSetSpeedMutex, portMAX_DELAY);
    speed = userSetSpeed;
    xSemaphoreGive(motorSetSpeedMutex);
    return speed;
}

// Get the state of the motor
motor_state_t motorGetState(void)
{
    motor_state_t state;
    xSemaphoreTake(motorStateMutex, portMAX_DELAY);
    state = motor_state;
    xSemaphoreGive(motorStateMutex);
    return state;
}

// Set the state of the motor.
// Returns true if state update was successful.
bool motorSetState(motor_state_t state)
{
    // TODO: safety checks to ensure motor transitions are valid.
    return false;
}