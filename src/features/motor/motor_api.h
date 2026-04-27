#ifndef MOTOR_API_H
#define MOTOR_API_H

#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "semphr.h"

#include "states.h"

extern SemaphoreHandle_t motorStateMutex;
extern SemaphoreHandle_t motorSetSpeedMutex;

// State transitions

void motorInit(void);
void motorStart(void);
void motorRunning(void);
void motorEStop(void);

// State accessors

motor_state_t motorGetState(void);
bool motorSetState(motor_state_t state);

// Speed accessors

void motorSetSpeed(uint16_t rpm);
uint16_t motorGetSpeed(void);

#endif