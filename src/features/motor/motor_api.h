#ifndef MOTOR_API_H
#define MOTOR_API_H

#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "semphr.h"

#include "states.h"

extern SemaphoreHandle_t motorStateMutex;
extern SemaphoreHandle_t motorSetSpeedMutex;
extern SemaphoreHandle_t motorStartSemaphore;

// State transitions

void motorInit(void);
void motorStart(void);
void motorRunning(void);
void motorEStop(void);
void motorFaultLatched(void);

// State accessors

motor_state_t motorGetState(void);
bool motorSetState(motor_state_t state);

// Speed accessors

void motorSetSpeed(uint16_t rpm);
uint16_t motorGetSpeed(void);

#endif