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
#include "driverlib/i2c.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"

#include "motorlib.h"
#include "features/priorities.h"
#include "../sensor_events.h"

extern EventGroupHandle_t xSensorEvents;

void xOPT3001TimerHandler(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT); // Clear the timer interrupt.
    xEventGroupSetBits(xSensorEvents, LIGHT_SENSOR_EVENT);
}

void xBMI160TimerHandler(void)
{
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT); // Clear the timer interrupt.
    xEventGroupSetBits(xSensorEvents, ACCEL_SENSOR_EVENT);
}

void xSHT31TimerHandler(void)
{
    TimerIntClear(TIMER2_BASE, TIMER_TIMA_TIMEOUT); // Clear the timer interrupt.
    xEventGroupSetBits(xSensorEvents, TEMP_SENSOR_EVENT);
}

void xSpeedTimerHandler(void)
{
    TimerIntClear(TIMER3_BASE, TIMER_TIMA_TIMEOUT); // Clear the timer interrupt.
    xEventGroupSetBits(xSensorEvents, SPEED_SENSOR_EVENT);
}

void xPowerTimerHandler(void)
{
    TimerIntClear(TIMER4_BASE, TIMER_TIMA_TIMEOUT); // Clear the timer interrupt.
    xEventGroupSetBits(xSensorEvents, POWER_SENSOR_EVENT);
}