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
#include "drivers/i2cOptDriver.h"
#include "driverlib/timer.h"

#include "motorlib.h"
#include "features/priorities.h"
#include "features/sensors/sensor_events.h"

#define PREV_NUM 8


extern uint32_t g_ui32SysClock;

extern void xSHT31Handler(void);

void prvSensorSHT31TimerInit(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0); // Enable the Timer 0 Module.

    TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER2_BASE, TIMER_A, g_ui32SysClock);    // set to ~ 1Hz

    TimerIntRegister(TIMER2_BASE, TIMER_A, xSHT31Handler); // set the timer interrupt vector
    TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);         // Enable the timer interrupt
    TimerEnable(TIMER2_BASE, TIMER_A);                       // Enable the timers.
    IntMasterEnable();                                       // Enable Master Interrupts
}

uint16_t getFilteredValue(uint16_t newValue)
{
    static uint16_t prev[PREV_NUM] = { 0 };

    uint16_t sum = 0;
    // Shift the existing values to the left
    for (int i = 0; i < PREV_NUM - 1; i++)
    {
        prev[i] = prev[i + 1];
        sum += prev[i];
    }
    // UARTprintf("After summation in moving average");
    // Add the new value to the end of the array
    prev[PREV_NUM - 1] = newValue;

    return (sum + prev[PREV_NUM - 1]) / PREV_NUM;
}

static void prvSHT31Delay(uint32_t period)
{
    vTaskDelay(pdMS_TO_TICKS(period));
}

extern void prvSensorSHT31Init(void) 
{
    sht31_dev sht31dev;
    sht31dev.read = I2C_read_bytes;
    sht31dev.write = I2C_write_bytes;
    sht31dev.delay = prvSHT31Delay;
    sht31_init(sht31dev);
}