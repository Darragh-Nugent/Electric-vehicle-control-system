#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"

#include "utils/uartstdio.h"
#include "drivers/opt3001.h"
#include "drivers/i2cDriver.h"

extern uint32_t g_ui32SysClock;

bool SensorOPT3001Init(void)
{
    bool result = Opt3001Init();
    if (!result)
    {
        UARTprintf("Sensor not init\n");
    }

    bool success = Opt3001Test();
    // If the test fails, retry the full init + test sequence rather than
    // retesting a sensor that was never successfully enabled.
    while (!success)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
        UARTprintf("OPT3001 Test Failed, Trying again\n");
        success = Opt3001Test();
    }

    return true;
}

bool getLux(float *lux)
{
    bool success;
    uint16_t raw_lux;
    success = Opt3001Read(&raw_lux);

    if (!success) return false;

    Opt3001Convert(raw_lux, lux);
    return true;
}