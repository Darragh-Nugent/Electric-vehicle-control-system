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

#include "vl53l0x_platform.h"
#include "vl53l0x_def.h"
#include "vl53l0x_platform_log.h"
#include "vl53l0x_i2c_platform.h"
#include "vl53l0x_api.h"

static VL53L0X_Dev_t sensor;

void SensorVL53L0xInit(void)
{
    uint8_t VhvSettings;
    uint8_t PhaseCal;
    uint32_t refSpadCount;
    uint8_t isApertureSpads;

    sensor.I2cDevAddr = 0x29;
    sensor.comms_type = 1;
    sensor.comms_speed_khz = 100;

    VL53L0X_Error result;

    result = VL53L0X_DataInit(&sensor);
    if (result)
        goto fail;

    UARTprintf("Data init\n");

    result = VL53L0X_StaticInit(&sensor);
    if (result)
        goto fail;

    UARTprintf("static init\n");

    result = VL53L0X_PerformRefCalibration(&sensor, &VhvSettings, &PhaseCal);
    if (result)
        goto fail;

    UARTprintf("ref calibrate\n");

    result = VL53L0X_PerformRefSpadManagement(&sensor, &refSpadCount, &isApertureSpads);
    if (result)
        goto fail;
    UARTprintf("ref mesaure\n");

    result = VL53L0X_SetDeviceMode(&sensor, VL53L0X_DEVICEMODE_CONTINUOUS_RANGING);
    if (result)
        goto fail;
    UARTprintf("device mode\n");

    result = VL53L0X_StartMeasurement(&sensor);
    if (result)
        goto fail;

    UARTprintf("measurment start\n");

    return;

fail:
    UARTprintf("VL53L0X init failed: %d\n", result);
}

bool getDistance(uint16_t *dist)
{
    uint8_t dataReady;

    if (VL53L0X_GetMeasurementDataReady(&sensor, &dataReady) != VL53L0X_ERROR_NONE || dataReady == 0)
    {
        return false;
    }

    VL53L0X_RangingMeasurementData_t data;
    VL53L0X_GetRangingMeasurementData(&sensor, &data);
    VL53L0X_ClearInterruptMask(&sensor, 0);

    *dist = data.RangeMilliMeter;
    return true;
}
