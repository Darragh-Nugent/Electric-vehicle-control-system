#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "utils/uartstdio.h"

#define INT_PER_ROTATION 24 // got this number by counting the number of interrupts in full rotation! dont change pls
#define SPEED_SAMPLING_TIME 0.01f

extern uint32_t g_ui32SysClock;


static volatile uint32_t partial_rotation = 0;

// Adds an additional turn of the hall effect sensors
void addRotation(void)
{
    partial_rotation++;
}

float getRPM(void)
{
    static uint32_t prev_time = 0;
    // uint32_t current_time = xTaskGetTickCount(); //<-------------************************************************** */

    // float time_ms = (current_time - prev_time)* portTICK_PERIOD_MS;
    // prev_time = current_time;

    // if (time_ms < 0.001f) return 0.0f;

    uint32_t local_partial_rotation;
    taskENTER_CRITICAL();
    local_partial_rotation = partial_rotation;
    partial_rotation = 0;
    taskEXIT_CRITICAL();

    float distance = (float)local_partial_rotation / INT_PER_ROTATION;
    // float time_sec = time_ms / 1000.0f;



    return (distance / SPEED_SAMPLING_TIME) * 60.0f;
}