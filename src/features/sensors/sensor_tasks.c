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
#include "driverlib/adc.h"

#include "motorlib.h"
#include "features/priorities.h"
#include "sensor_events.h"
#include "sensors_api.h"

/*-----------------------------------------------------------*/

extern void vI2CManagerTask(void *pvParameters);
extern void vSensorManagerTask(void *pvParameters);

/*-----------------------------------------------------------*/

extern void xI2C0Handler(void);
extern void xI2C2Handler(void);

extern void xOPT3001TimerHandler(void);
extern void xSHT31TimerHandler(void);
extern void xBMI160TimerHandler(void);
extern void xSpeedTimerHandler(void);
extern void xPowerTimerHandler(void);

/*-----------------------------------------------------------*/

static void prvI2CInit(void);
static void prvTimerInit(void);
static void prvButtonInit(void);

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
    prvButtonInit();
    Sensor_Init();

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
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2); // Enable the Timer 2 Module.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3); // Enable the Timer 3 Module.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER4); // Enable the Timer 4 Module.

    // Configure the interrupt time
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, g_ui32SysClock / 2); // set to ~ 2Hz

    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER1_BASE, TIMER_A, g_ui32SysClock / 100); // set to ~ 100Hz

    TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER2_BASE, TIMER_A, g_ui32SysClock); // set to ~ 1Hz

    TimerConfigure(TIMER3_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER3_BASE, TIMER_A, g_ui32SysClock / 100); // set to ~ 100Hz

    TimerConfigure(TIMER4_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER4_BASE, TIMER_A, g_ui32SysClock / 150); // set to ~ 150Hz

    // Regester and enable the interrupts
    TimerIntRegister(TIMER0_BASE, TIMER_A, xOPT3001TimerHandler);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER0_BASE, TIMER_A);

    TimerIntRegister(TIMER1_BASE, TIMER_A, xBMI160TimerHandler);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER1_BASE, TIMER_A);

    TimerIntRegister(TIMER2_BASE, TIMER_A, xSHT31TimerHandler);
    TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER2_BASE, TIMER_A);

    TimerIntRegister(TIMER3_BASE, TIMER_A, xSpeedTimerHandler);
    TimerIntEnable(TIMER3_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER3_BASE, TIMER_A);

    TimerIntRegister(TIMER4_BASE, TIMER_A, xPowerTimerHandler);
    TimerIntEnable(TIMER4_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER4_BASE, TIMER_A);

    // Enable Master Interrupts
    IntMasterEnable();
}

static void prvADCInit(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0))
        ;
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC1))
        ;
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE))
        ;
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD))
        ;

    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);
    GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_7);

    // Step 0: PE3 (AIN0)
    ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH0);

    // Step 1: PD7 (AIN4), end + interrupt
    ADCSequenceStepConfigure(ADC0_BASE, 0, 1,
                             ADC_CTL_CH4 | ADC_CTL_END | ADC_CTL_IE);

    ADCSequenceEnable(ADC0_BASE, 0);
    ADCIntEnable(ADC0_BASE, 0);
}

static void prvButtonInit(void)
{
    /* Initialize the LaunchPad Buttons. */
    ButtonsInit();

    /* Configure both switches to trigger an interrupt on a falling edge. */
    GPIOIntTypeSet(BUTTONS_GPIO_BASE, ALL_BUTTONS, GPIO_FALLING_EDGE);

    /* Enable the interrupt for LaunchPad GPIO Port in the GPIO peripheral. */
    GPIOIntEnable(BUTTONS_GPIO_BASE, ALL_BUTTONS);

    /* Enable the Port F interrupt in the NVIC. */
    IntEnable(INT_GPIOJ);

    /* Enable global interrupts in the NVIC. */
    IntMasterEnable();
}