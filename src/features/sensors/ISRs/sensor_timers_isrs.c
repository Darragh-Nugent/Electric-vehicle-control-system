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
#include "features/sensors/core/sensor_events.h"

extern EventGroupHandle_t xSensorEvents;

void xOPT3001TimerHandler(void)
{
    // UARTprintf("Opt timer\n");
    TimerIntClear(TIMER6_BASE, TIMER_TIMA_TIMEOUT); // Clear the timer interrupt.
    xEventGroupSetBitsFromISR(xSensorEvents, LIGHT_SENSOR_EVENT, NULL);
}

// void xBMI160TimerHandler(void)
// {
//     // UARTprintf("acceleration timer\n");
//     TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT); // Clear the timer interrupt.
//     xEventGroupSetBits(xSensorEvents, ACCEL_SENSOR_EVENT);
// }

void xSHT31TimerHandler(void)
{
    TimerIntClear(TIMER2_BASE, TIMER_TIMA_TIMEOUT); // Clear the timer interrupt.
    xEventGroupSetBitsFromISR(xSensorEvents, TEMP_SENSOR_EVENT, NULL);
}

void xSpeedTimerHandler(void)
{
    TimerIntClear(TIMER3_BASE, TIMER_TIMA_TIMEOUT); // Clear the timer interrupt.
    xEventGroupSetBitsFromISR(xSensorEvents, SPEED_SENSOR_EVENT | ACCEL_SENSOR_EVENT, NULL);
}

void xPowerTimerHandler(void)
{
    TimerIntClear(TIMER4_BASE, TIMER_TIMA_TIMEOUT); // Clear the timer interrupt.
    xEventGroupSetBitsFromISR(xSensorEvents, POWER_SENSOR_EVENT, NULL);
}

void xDistTimerHandler(void)
{
    TimerIntClear(TIMER5_BASE, TIMER_TIMA_TIMEOUT); // Clear the timer interrupt.
    xEventGroupSetBitsFromISR(xSensorEvents, DIST_SENSOR_EVENT, NULL);
}