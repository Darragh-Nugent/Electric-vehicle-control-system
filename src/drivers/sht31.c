#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

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
#include "drivers/i2cOptDriver.h"
#include "sht31.h"

#include "motorlib.h"
#include "features/priorities.h"

#define SHT31_I2C_ADDRESS 0x00

static sht31_dev config;

bool sht31_write_command(uint16_t command)
{
    uint8_t command_reversed[2];
    command_reversed[0] = command >> 8;
    command_reversed[1] = command & 0xFF;
    return config.write(SHT31_ADDR, command_reversed, 2);
}

void sht31_reset(void)
{
    sht31_write_command(SHT31_SOFTRESET);
    config.delay(10);
}

bool sht31_init(sht31_dev new_config)
{
    config.write = new_config.write;
    config.read = new_config.read;
    config.delay = new_config.delay;
    sht31_reset();
}

uint8_t sht31_crc8(const uint8_t* data, int len) {
    const uint8_t POLYNOMIAL = 0x31;
    uint8_t crc = 0xFF;

    for (int j = len; j; --j) {
        crc ^= *data++;

        for (int i = 8; i; --i) {
            crc = (crc & 0x80)
                  ? (crc << 1) ^ POLYNOMIAL
                  : (crc << 1);
        }
    }
    return crc;
}


bool sht31_getTempHum(float *temp, float *humidity) {
    uint8_t readbuffer[6];

    sht31_write_command(SHT31_MEAS_HIGHREP);

    config.delay(50);
    bool result = config.read(SHT31_ADDR, *readbuffer, 6);
    if (!result) 
    {
        return false;
    }

    uint16_t ST, SRH;
    ST = readbuffer[0];
    ST <<= 8;
    ST |= readbuffer[1];

    if (readbuffer[2] != sht31_crc8(readbuffer, 2)) {
        return false;
    }

    SRH = readbuffer[3];
    SRH <<= 8;
    SRH |= readbuffer[4];

    if (readbuffer[5] != sht31_crc8(readbuffer + 3, 2)) {
        return false;
    }

    double stemp = ST;
    stemp *= 175;
    stemp /= 0xffff;
    stemp = -45 + stemp;
    *temp = stemp;

    double shum = SRH;
    shum *= 100;
    shum /= 0xFFFF;

    *humidity = shum;

    return true;
}
