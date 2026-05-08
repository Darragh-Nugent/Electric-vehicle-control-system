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
#include "drivers/bmi160.h"
#include "drivers/i2cDriver.h"
#include "driverlib/timer.h"

#include "motorlib.h"
#include "features/priorities.h"

/*
 * The configuration struct for the acceleration sensor
 */
struct bmi160_dev bmi160dev;

/*
 * The struct for holding the acceleration data
 */
struct bmi160_sensor_data bmi160_accel;


static int8_t prvBmi160WriteI2C(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *data, uint16_t len)
{
    return I2C_write_reg(ui8Addr, ui8Reg, data, len)
               ? BMI160_OK
               : BMI160_E_COM_FAIL;
}

static int8_t prvBmi160ReadI2C(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *data, uint16_t len)
{
    return I2C_read_reg(ui8Addr, ui8Reg, data, len)
               ? BMI160_OK
               : BMI160_E_COM_FAIL;
}

static void prvBmi160Delay(uint32_t period)
{
    vTaskDelay(pdMS_TO_TICKS(period));
}

void SensorBmi160Init(void)
{
    bmi160dev.write = prvBmi160WriteI2C;
    bmi160dev.read = prvBmi160ReadI2C;
    bmi160dev.delay_ms = prvBmi160Delay;
    bmi160dev.id = 0x69;
    bmi160dev.intf = BMI160_I2C_INTF;

    bmi160_init(&bmi160dev);
    UARTprintf("Chip ID: 0x%02x\n", bmi160dev.chip_id); // should print 0xD1

    bmi160dev.accel_cfg.odr = BMI160_ACCEL_ODR_1600HZ;
    bmi160dev.accel_cfg.range = BMI160_ACCEL_RANGE_8G;
    bmi160dev.accel_cfg.bw = BMI160_ACCEL_BW_NORMAL_AVG4;
    bmi160dev.accel_cfg.power = BMI160_ACCEL_NORMAL_MODE;

    // Configure gyro — required for self test
    bmi160dev.gyro_cfg.odr = BMI160_GYRO_ODR_3200HZ;
    bmi160dev.gyro_cfg.range = BMI160_GYRO_RANGE_2000_DPS;
    bmi160dev.gyro_cfg.bw = BMI160_GYRO_BW_NORMAL_MODE;
    bmi160dev.gyro_cfg.power = BMI160_GYRO_NORMAL_MODE;

    int8_t result = bmi160_set_sens_conf(&bmi160dev);
    if (result != 0)
    {
        UARTprintf("BMI160 config failing with error %d\n", result);
    }
    bmi160dev.delay_ms(100); // allow sensors to ramp up

    int8_t bmi160Test = -1;
    bmi160Test = bmi160_perform_self_test(BMI160_ACCEL_ONLY, &bmi160dev);
    while (bmi160Test != 0)
    {
        UARTprintf("BMI160 self test failed. Code = %d\n", bmi160Test);
        vTaskDelay(pdMS_TO_TICKS(100));
        bmi160Test = bmi160_perform_self_test(BMI160_ACCEL_ONLY, &bmi160dev);
    }

    // Reconfigure sensor as bmi160_perform_self_test resets it for some reason
    bmi160dev.accel_cfg.odr = BMI160_ACCEL_ODR_100HZ;
    bmi160dev.accel_cfg.range = BMI160_ACCEL_RANGE_8G;
    bmi160dev.accel_cfg.bw = BMI160_ACCEL_BW_NORMAL_AVG4;
    bmi160dev.accel_cfg.power = BMI160_ACCEL_NORMAL_MODE;

    bmi160_set_sens_conf(&bmi160dev);

    bmi160dev.delay_ms(100); // allow sensors to ramp up
}

bool getAbsoluteAccel(uint16_t *absAccel)
{
    int8_t result = bmi160_get_sensor_data(BMI160_ACCEL_SEL, &bmi160_accel, NULL, &bmi160dev);
    if (result != 0)
    {
        return false;
    }

    *absAccel = abs(bmi160_accel.x) + abs(bmi160_accel.y) + abs(bmi160_accel.z);
    return true;
}