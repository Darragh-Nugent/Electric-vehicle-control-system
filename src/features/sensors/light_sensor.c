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
#include "drivers/bmi160.h"
#include "drivers/i2cOptDriver.h"
#include "driverlib/timer.h"

#include "motorlib.h"
#include "features/priorities.h"
#include "features/sensors/sensor_events.h"

#define LIGHT_PREV_NUM 8


extern uint32_t g_ui32SysClock;

extern void xOPT3001Handler(void);

void prvSensorOPT3001Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0); // Enable the Timer 0 Module.

    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, g_ui32SysClock / 2);    // set to ~ 2Hz

    TimerIntRegister(TIMER0_BASE, TIMER_A, xOPT3001Handler); // set the timer interrupt vector
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);         // Enable the timer interrupt
    TimerEnable(TIMER0_BASE, TIMER_A);                       // Enable the timers.
    IntMasterEnable();                                       // Enable Master Interrupts
}

uint16_t getFilteredLight(uint16_t newValue)
{
    static uint16_t prevLux[LIGHT_PREV_NUM] = { 0 };

    uint16_t sum = 0;
    // Shift the existing values to the left
    for (int i = 0; i < LIGHT_PREV_NUM - 1; i++)
    {
        prevLux[i] = prevLux[i + 1];
        sum += prevLux[i];
    }
    // UARTprintf("After summation in moving average");
    // Add the new value to the end of the array
    prevLux[LIGHT_PREV_NUM - 1] = newValue;

    return (sum + prevLux[LIGHT_PREV_NUM - 1]) / LIGHT_PREV_NUM;
}