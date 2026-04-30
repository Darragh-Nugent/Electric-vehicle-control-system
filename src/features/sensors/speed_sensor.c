#include "driverlib/pin_map.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "drivers/rtos_hw_drivers.h"
#include "utils/uartstdio.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "motorlib.h"
#include "features/priorities.h"

#define INT_PER_ROTATION 6
#define SPEED_SAMPLING_TIME 100

extern uint32_t g_ui32SysClock;

static uint32_t partial_rotation = 0;

// Adds an additional turn of the hall effect sensors
void addRotation(void)
{
    partial_rotation++;
}

float getRPM(void)
{
    static uint32_t prev_time = 0;
    uint32_t current_time = pdTICKS_TO_MS(xTaskGetTickCount());

    uint32_t time_ms = current_time - prev_time;
    prev_time = current_time;

    uint32_t local_partial_rotation;
    taskENTER_CRITICAL();
    local_partial_rotation = partial_rotation;
    partial_rotation = 0;
    taskEXIT_CRITICAL();

    float distance = (float)local_partial_rotation / INT_PER_ROTATION;
    float time_sec = time_ms / 1000.0f;


    return (distance / time_sec) * 60.0f;
}