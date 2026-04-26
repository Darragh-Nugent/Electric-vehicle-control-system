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
#include "drivers/i2cOptDriver.h"
#include "driverlib/timer.h"

#include "motorlib.h"
#include "features/priorities.h"

extern uint32_t g_ui32SysClock;

extern void xBMI160Handler(void);

void prvSensorBMI160TimerInit(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1); // Enable the Timer 1 Module.

    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER1_BASE, TIMER_A, g_ui32SysClock / 100); // set to ~ 100Hz

    TimerIntRegister(TIMER1_BASE, TIMER_A, xBMI160Handler); // set the timer interrupt vector
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);        // Enable the timer interrupt
    TimerEnable(TIMER1_BASE, TIMER_A);                      // Enable the timers.
    IntMasterEnable();                                      // Enable Master Interrupts
}

static void prvBmi160Delay(uint32_t period)
{
    vTaskDelay(pdMS_TO_TICKS(period));
}

void prvSensorBmi160Init(struct bmi160_dev *bmi160dev)
{
    bmi160dev->write = writeI2CBMI160;
    bmi160dev->read = readI2CBMI160;
    bmi160dev->delay_ms = prvBmi160Delay;
    bmi160dev->id = 0x69; // or 0x69 — check your SDO pin
    bmi160dev->intf = BMI160_I2C_INTF;

    bmi160_init(bmi160dev);
    UARTprintf("Chip ID: 0x%02x\n", bmi160dev->chip_id); // should print 0xD1

    bmi160dev->accel_cfg.odr = BMI160_ACCEL_ODR_1600HZ;
    bmi160dev->accel_cfg.range = BMI160_ACCEL_RANGE_2G;
    bmi160dev->accel_cfg.bw = BMI160_ACCEL_BW_NORMAL_AVG4;
    bmi160dev->accel_cfg.power = BMI160_ACCEL_NORMAL_MODE;

    // Configure gyro — required for self test
    bmi160dev->gyro_cfg.odr = BMI160_GYRO_ODR_3200HZ;
    bmi160dev->gyro_cfg.range = BMI160_GYRO_RANGE_2000_DPS;
    bmi160dev->gyro_cfg.bw = BMI160_GYRO_BW_NORMAL_MODE;
    bmi160dev->gyro_cfg.power = BMI160_GYRO_NORMAL_MODE;

    int8_t result = bmi160_set_sens_conf(bmi160dev);
    if (result != 0)
    {
        UARTprintf("BMI160 config failing with error %d\n", result);
    }
    bmi160dev->delay_ms(100); // allow sensors to ramp up

    int8_t bmi160Test = -1;
    while (bmi160Test != 0)
    {
        SysCtlDelay(g_ui32SysClock);
        UARTprintf("BMI160 self test failed. Code = %d\n", bmi160Test);
        bmi160Test = bmi160_perform_self_test(BMI160_ACCEL_ONLY, bmi160dev);
    }

    bmi160dev->accel_cfg.odr = BMI160_ACCEL_ODR_1600HZ;
    bmi160dev->accel_cfg.range = BMI160_ACCEL_RANGE_2G;
    bmi160dev->accel_cfg.bw = BMI160_ACCEL_BW_NORMAL_AVG4;
    bmi160dev->accel_cfg.power = BMI160_ACCEL_NORMAL_MODE;

    // Configure gyro — required for self test
    bmi160dev->gyro_cfg.odr = BMI160_GYRO_ODR_3200HZ;
    bmi160dev->gyro_cfg.range = BMI160_GYRO_RANGE_2000_DPS;
    bmi160dev->gyro_cfg.bw = BMI160_GYRO_BW_NORMAL_MODE;
    bmi160dev->gyro_cfg.power = BMI160_GYRO_NORMAL_MODE;

    bmi160_set_sens_conf(bmi160dev);

    bmi160dev->delay_ms(100); // allow sensors to ramp up
}