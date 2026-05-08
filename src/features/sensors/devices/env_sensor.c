#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"

#include "utils/uartstdio.h"
#include "drivers/sht31.h"
#include "drivers/i2cDriver.h"

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
