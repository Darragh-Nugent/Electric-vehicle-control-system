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
#include "drivers/i2cDriver.h"

#include "motorlib.h"
#include "features/priorities.h"
#include "sensor_events.h"

/*-----------------------------------------------------------*/

extern void vI2CManagerTask(void *pvParameters);
extern void vSensorManagerTask(void *pvParameters);

/*-----------------------------------------------------------*/

extern void xI2C0Handler(void);
extern void xI2C2Handler(void);

extern void xOPT3001Handler(void);
extern void xSHT31Handler(void);
extern void xBMI160Handler(void);

/*-----------------------------------------------------------*/

static void prvI2CInit(void);
static void prvTimerInit(void);

/*-----------------------------------------------------------*/

void vCreateSensorTasks(void);

/*-----------------------------------------------------------*/

extern uint32_t g_ui32SysClock;

SemaphoreHandle_t xButtonSemaphore = NULL;
SemaphoreHandle_t xI2CSemaphore = NULL;
SemaphoreHandle_t xOPT3001Semaphore = NULL;

QueueHandle_t xI2CSendQueue;
QueueHandle_t xI2CRecvQueue;

EventGroupHandle_t xSensorEvents;

/*-----------------------------------------------------------*/

void vCreateSensorTasks(void)
{
    xButtonSemaphore = xSemaphoreCreateBinary();
    xI2CSemaphore = xSemaphoreCreateBinary();
    xOPT3001Semaphore = xSemaphoreCreateBinary();

    xI2CSendQueue = xQueueCreate(10, sizeof(i2c_send_message_t));
    xI2CRecvQueue = xQueueCreate(10, sizeof(i2c_recv_message_t));

    xSensorEvents = xEventGroupCreate();

    prvI2CInit();
    prvTimerInit();

    xTaskCreate(
        vI2CManagerTask,
        "I2CManagerTask",
        256,
        NULL,
        LIGHT_SENSOR_PRIORITY,
        NULL);

    xTaskCreate(
        vSensorManagerTask,
        "LightSensorTask",
        256,
        NULL,
        LIGHT_SENSOR_PRIORITY,
        NULL);
}

/*-----------------------------------------------------------*/

static void prvI2CInit(void)
{
    //
    // Enable I2C0 for Bootsetpack 1
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    //
    // Enable I2C2 for Boosterback 2
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C2);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);

    // Wait until peripherals are fully clocked before touching their
    // registers. Skipping this causes intermittent failures on TM4C129x.
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_I2C0))
    {
    }
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB))
    {
    }
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_I2C2))
    {
    }
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION))
    {
    }

    //
    // Configure the pin muxing for I2C0 and I2C2 functions on port B2 and B3 and port N4 and N5.
    // This step is not necessary if your part does not support pin muxing.
    //
    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);

    GPIOPinConfigure(GPIO_PN5_I2C2SCL);
    GPIOPinConfigure(GPIO_PN4_I2C2SDA);

    //
    // Select the I2C function for these pins.  This function will also
    // configure the GPIO pins pins for I2C operation, setting them to
    // open-drain operation with weak pull-ups.  Consult the data sheet
    // to see which functions are allocated per pin.
    //
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

    GPIOPinTypeI2CSCL(GPIO_PORTN_BASE, GPIO_PIN_5);
    GPIOPinTypeI2C(GPIO_PORTN_BASE, GPIO_PIN_4);

    // Assign interrupt
    I2CMasterInitExpClk(I2C0_BASE, g_ui32SysClock, false);
    I2CIntRegister(I2C0_BASE, xI2C0Handler);

    I2CMasterInitExpClk(I2C2_BASE, g_ui32SysClock, false);
    I2CIntRegister(I2C2_BASE, xI2C2Handler);

    // Enable i2c interrupt sources
    I2CMasterIntEnableEx(I2C0_BASE, I2C_MASTER_INT_DATA | I2C_MASTER_INT_TIMEOUT);
    I2CMasterIntEnableEx(I2C2_BASE, I2C_MASTER_INT_DATA | I2C_MASTER_INT_TIMEOUT);

    IntEnable(INT_I2C0); // should be in opt_task (thats what the semaphore example had)
    IntEnable(INT_I2C2);

    I2CMasterTimeoutSet(I2C0_BASE, g_ui32SysClock / 100);
    I2CMasterTimeoutSet(I2C2_BASE, g_ui32SysClock / 100);

    IntMasterEnable();
}

static void prvTimerInit(void)
{
    // Enable the sensor timers
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0); // Enable the Timer 0 Module.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1); // Enable the Timer 1 Module.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2); // Enable the Timer 0 Module.

    // Configure the interrupt time
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, g_ui32SysClock / 2); // set to ~ 2Hz

    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER1_BASE, TIMER_A, g_ui32SysClock / 100); // set to ~ 100Hz

    TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER2_BASE, TIMER_A, g_ui32SysClock); // set to ~ 1Hz

    // Regester and enable the interrupts
    TimerIntRegister(TIMER0_BASE, TIMER_A, xOPT3001Handler);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER0_BASE, TIMER_A);

    TimerIntRegister(TIMER1_BASE, TIMER_A, xBMI160Handler);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER1_BASE, TIMER_A);

    TimerIntRegister(TIMER2_BASE, TIMER_A, xSHT31Handler);
    TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER2_BASE, TIMER_A);

    // Enable Master Interrupts
    IntMasterEnable();
}