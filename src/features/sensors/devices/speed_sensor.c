#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "utils/uartstdio.h"
#include "drivers/speed_sensor_driver.h"

#define INT_PER_ROTATION 24 // got this number by counting the number of interrupts in full rotation! dont change pls
#define SPEED_SAMPLING_TIME 0.01f

extern uint32_t g_ui32SysClock;


static volatile uint32_t partial_rotation = 0;

// Adds an additional turn of the hall effect sensors
void addRotation(void)
{
    partial_rotation++;
}

void SpeedInit(void)
{
    speed_sensor_dev_t dev;

    dev.counts_per_rotation = INT_PER_ROTATION;
    dev.sample_time = SPEED_SAMPLING_TIME;

    Speed_Sensor_Init(dev);
}

float getRPM(void)
{
    uint32_t counts;

    taskENTER_CRITICAL();
    counts = partial_rotation;
    partial_rotation = 0;
    taskEXIT_CRITICAL();

    return Speed_Sensor_GetRPM(counts);
}