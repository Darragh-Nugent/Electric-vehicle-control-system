#include "driverlib/pin_map.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "drivers/rtos_hw_drivers.h"
#include "utils/uartstdio.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "drivers/opt3001.h"
#include "driverlib/i2c.h"
#include "driverlib/interrupt.h"
#include "drivers/sht31.h"
#include "drivers/i2cDriver.h"
#include "driverlib/timer.h"

#include "motorlib.h"
#include "features/priorities.h"
#include "features/sensors/sensor_events.h"

static void prvSHT31Delay(uint32_t period)
{
    vTaskDelay(pdMS_TO_TICKS(period));
}

void SensorSHT31Init(void) 
{
    sht31_dev sht31dev;
    sht31dev.read = I2C_read_bytes;
    sht31dev.write = I2C_write_bytes;
    sht31dev.delay = prvSHT31Delay;
    sht31_init(sht31dev);
}

bool SensorSHT31GetTemHum(float *temp, float *humidity)
{
    return sht31_getTempHum(temp, humidity);
}
