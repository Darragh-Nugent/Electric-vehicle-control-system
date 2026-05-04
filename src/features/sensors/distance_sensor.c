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

    sensor.I2cDevAddr = 0x52;
    sensor.comms_type = 1;
    sensor.comms_speed_khz = 400;

    VL53L0X_Error result = VL53L0X_DataInit(&sensor);
    result = VL53L0X_StaticInit(&sensor);
    result = VL53L0X_PerformRefCalibration(&sensor, &VhvSettings, &PhaseCal);
    result = VL53L0X_PerformRefSpadManagement(&sensor, &refSpadCount, &isApertureSpads);
    result = VL53L0X_SetDeviceMode(&sensor, VL53L0X_DEVICEMODE_SINGLE_RANGING);

    if (result != VL53L0X_ERROR_NONE)
    {
        UARTprintf("Distance sensor intitialisation failed\n");
    }
}

uint16_t getDistance(void)
{
    VL53L0X_RangingMeasurementData_t data;

    VL53L0X_Error result = VL53L0X_PerformSingleRangingMeasurement(&sensor, &data);

    if (result != VL53L0X_ERROR_NONE)
    {
        UARTprintf("Distance sensor communication failed\n");
        return 0xFFFF;
    }

    if (data.RangeStatus != 0)
    {
        return 0xFFFF;
    }

    return data.RangeMilliMeter;
}
