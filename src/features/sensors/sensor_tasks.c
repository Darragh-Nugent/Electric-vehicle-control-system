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
#include "features/sensors/i2c_message_struct.h"
#include "sensor_events.h"

extern void vI2CManagerTask(void *pvParameters);
extern void vAccelerationSensorTask(void *pvParameters);
extern void vDistanceSensorTask(void *pvParameters);
extern void vEnvSensorTask(void *pvParameters);
extern void vLightSensorTask(void *pvParameters);
extern void vPowerSensorTask(void *pvParameters);
extern void vSpeedSensorTask(void *pvParameters);

extern void xI2CHandler(void);
extern void xOPT3001Handler(void);

static void prvSensorOPT3001Init(void);
static void prvI2CInit(void);

void vCreateSensorTasks(void);

extern uint32_t g_ui32SysClock;

SemaphoreHandle_t xButtonSemaphore = NULL;
SemaphoreHandle_t xI2CSemaphore = NULL;
SemaphoreHandle_t xOPT3001Semaphore = NULL;

QueueHandle_t xI2CSendQueue;
QueueHandle_t xI2CRecvQueue;

EventGroupHandle_t xSensorEvents;

void vCreateSensorTasks(void)
{
    xButtonSemaphore = xSemaphoreCreateBinary();
    xI2CSemaphore = xSemaphoreCreateBinary();
    xOPT3001Semaphore = xSemaphoreCreateBinary();

    xI2CSendQueue = xQueueCreate(10, sizeof(i2c_send_message_t));
    xI2CRecvQueue = xQueueCreate(10, sizeof(i2c_recv_message_t));

    xSensorEvents = xEventGroupCreate();

    prvI2CInit();
    prvSensorOPT3001Init();
    // vTaskDelay(pdMS_TO_TICKS(1000)); // delay to ensure sensor is initialised before testing

    xTaskCreate(
        vI2CManagerTask,
        "I2CManagerTask",
        256,
        NULL,
        LIGHT_SENSOR_PRIORITY,
        NULL);

    // xTaskCreate(
    //     vAccelerationSensorTask,
    //     "AccelSensorTask",
    //     256,
    //     NULL,
    //     ACCELERATION_SENSOR_PRIORITY,
    //     NULL);

    // xTaskCreate(
    //     vDistanceSensorTask,
    //     "DistanceSensorTask",
    //     256,
    //     NULL,
    //     DISTANCE_SENSOR_PRIORITY,
    //     NULL);

    // xTaskCreate(
    //     vEnvSensorTask,
    //     "EnvSensorTask",
    //     256,
    //     NULL,
    //     ENV_SENSOR_PRIORITY,
    //     NULL);

    xTaskCreate(
        vLightSensorTask,
        "LightSensorTask",
        256,
        NULL,
        LIGHT_SENSOR_PRIORITY,
        NULL);

    // xTaskCreate(
    //     vPowerSensorTask,
    //     "PowerSensorTask",
    //     256,
    //     NULL,
    //     POWER_SENSOR_PRIORITY,
    //     NULL);

    // xTaskCreate(
    //     vSpeedSensorTask,
    //     "SpeedSensorTask",
    //     256,
    //     NULL,
    //     SPEED_SENSOR_PRIORITY,
    //     NULL);
}

static void prvI2CInit(void)
{
    //
    // The I2C0 peripheral must be enabled before use.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    // Wait until both peripherals are fully clocked before touching their
    // registers. Skipping this causes intermittent failures on TM4C129x.
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_I2C0))
    {
    }
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB))
    {
    }

    //
    // Configure the pin muxing for I2C0 functions on port B2 and B3.
    // This step is not necessary if your part does not support pin muxing.
    //
    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);

    //
    // Select the I2C function for these pins.  This function will also
    // configure the GPIO pins pins for I2C operation, setting them to
    // open-drain operation with weak pull-ups.  Consult the data sheet
    // to see which functions are allocated per pin.
    //
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

    I2CMasterInitExpClk(I2C0_BASE, g_ui32SysClock, false);
    I2CIntRegister(I2C0_BASE, xI2CHandler);

    // Enable i2c interrupt sources
    I2CMasterIntEnableEx(I2C0_BASE, I2C_MASTER_INT_DATA | I2C_MASTER_INT_TIMEOUT);

    IntEnable(INT_I2C0); // should be in opt_task (thats what the semaphore example had)

    I2CMasterTimeoutSet(I2C0_BASE, g_ui32SysClock / 10);
    IntMasterEnable();
}

static void prvSensorOPT3001Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0); // Enable the Timer 0 Module.

    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, g_ui32SysClock / 2);    // set to ~ 2Hz

    TimerIntRegister(TIMER0_BASE, TIMER_A, xOPT3001Handler); // set the timer interrupt vector
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);         // Enable the timer interrupt
    TimerEnable(TIMER0_BASE, TIMER_A);                       // Enable the timers.
    IntMasterEnable();                                       // Enable Master Interrupts
}