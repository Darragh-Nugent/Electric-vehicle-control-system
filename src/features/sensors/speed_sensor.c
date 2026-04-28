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

#define INT_PER_ROTATION 3
#define SPEED_SAMPLING_TIME 100

extern uint32_t g_ui32SysClock;

static float partial_rotation = 0;

// Adds an additional turn of the hall effect sensors
void addRotation(void)
{
    partial_rotation++;
}

float getRPM(void)
{
    float local_partial_rotation;

    taskENTER_CRITICAL();
    local_partial_rotation = partial_rotation;
    partial_rotation = 0;
    taskEXIT_CRITICAL();

    float distance = partial_rotation / 3;
    float time = (1  / SPEED_SAMPLING_TIME) * 60;

    partial_rotation = 0;

    return distance / time;
}