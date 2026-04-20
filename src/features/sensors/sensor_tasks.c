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

extern void vAccelerationSensorTask(void *pvParameters);
extern void vDistanceSensorTask(void *pvParameters);
extern void vEnvSensorTask(void *pvParameters);
extern void vLightSensorTask(void *pvParameters);
extern void vPowerSensorTask(void *pvParameters);
extern void vSpeedSensorTask(void *pvParameters);

void vCreateSensorTasks(void);

void vCreateSensorTasks(void)
{
    xTaskCreate(
        vAccelerationSensorTask,
        "AccelSensorTask",
        256,
        NULL,
        ACCELERATION_SENSOR_PRIORITY,
        NULL
    );

    xTaskCreate(
        vDistanceSensorTask,
        "DistanceSensorTask",
        256,
        NULL,
        DISTANCE_SENSOR_PRIORITY,
        NULL
    );

    xTaskCreate(
        vEnvSensorTask,
        "EnvSensorTask",
        256,
        NULL,
        ENV_SENSOR_PRIORITY,
        NULL
    );

    xTaskCreate(
        vLightSensorTask,
        "LightSensorTask",
        256,
        NULL,
        LIGHT_SENSOR_PRIORITY,
        NULL
    );

    xTaskCreate(
        vPowerSensorTask,
        "PowerSensorTask",
        256,
        NULL,
        POWER_SENSOR_PRIORITY,
        NULL
    );

    xTaskCreate(
        vSpeedSensorTask,
        "SpeedSensorTask",
        256,
        NULL,
        SPEED_SENSOR_PRIORITY,
        NULL
    );
}