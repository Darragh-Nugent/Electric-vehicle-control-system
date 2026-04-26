/* Standard includes. */
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Hardware includes. */
#include "driverlib/pin_map.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_ints.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "drivers/rtos_hw_drivers.h"
#include "utils/uartstdio.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driver_lib/udma.h" //

#include "features/user_interface/gui_task.h"

// Motor lib
#include <motorlib.h>
#include "grlib.h"
#include "drivers/Kentec320x240x16_ssd2119_spi.h"
#include "drivers/touch.h"
#include "widget.h"
/*-----------------------------------------------------------*/

/* The system clock frequency. */
uint32_t g_ui32SysClock;

/* Set up the hardware ready to run this demo. */
static void prvSetupHardware(void);

/* This function sets up UART0 to be used for a console to display information
 * as the example is running. */
static void prvConfigureUART(void);

extern void vCreateMotorTask(void);
extern void vCreateSensorTasks(void);
extern void vCreateGuiTask(void);
extern void vCreateDemoTask(void);
static void prvConfigureHallInts(void);

/*-----------------------------------------------------------*/

int main(void)
{
    prvSetupHardware();

    vCreateMotorTask();
    vCreateSensorTasks();
    vCreateDemoTask();

    vTaskStartScheduler();

    for (;;)
        ;
}
/*-----------------------------------------------------------*/
static void prvConfigureUART(void)
{
    /* Enable GPIO port A which is used for UART0 pins.
     * TODO: change this to whichever GPIO port you are using. */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    /* Configure the pin muxing for UART0 functions on port A0 and A1.
     * This step is not necessary if your part does not support pin muxing.
     * TODO: change this to select the port/pin you are using. */
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);

    /* Enable UART0 so that we can configure the clock. */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    /* Use the internal 16MHz oscillator as the UART clock source. */
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    /* Select the alternate (UART) function for these pins.
     * TODO: change this to select the port/pin you are using. */
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    /* Initialize the UART for console I/O. */
    UARTStdioConfig(0, 9600, 16000000);
}
/*-----------------------------------------------------------*/
static void prvSetupHardware(void)
{
    /* Run from the PLL at configCPU_CLOCK_HZ MHz. */
    g_ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                             SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |
                                             SYSCTL_CFG_VCO_240),
                                            configCPU_CLOCK_HZ);

    /* Configure device pins. */
    PinoutSet(false, false);

    /* Configure UART0 to send messages to terminal. */
    prvConfigureUART();

    /* Set-up interrupts for hall sensors */
    prvConfigureHallInts();
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook(void)
{
    /* vApplicationMallocFailedHook() will only be called if
    configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
    function that will get called if a call to pvPortMalloc() fails.
    pvPortMalloc() is called internally by the kernel whenever a task, queue,
    timer or semaphore is created.  It is also called by various parts of the
    demo application.  If heap_1.c or heap_2.c are used, then the size of the
    heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
    FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
    to query the size of free heap space that remains (although it does not
    provide information on how the remaining heap might be fragmented). */
    IntMasterDisable();
    for (;;)
        ;
}
/*-----------------------------------------------------------*/
static void prvConfigureHallInts(void)
{

    /* Configure GPIO ports to trigger an interrupt on rising/falling or both edges. */

    /* Enable the interrupt for LaunchPad GPIO Port in the GPIO peripheral. */

    /* Enable the Ports interrupt in the NVIC. */

    /* Enable global interrupts in the NVIC. */
    IntMasterEnable();
}

/*-----------------------------------------------------------*/

void vApplicationIdleHook(void)
{
    /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
    to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
    task.  It is essential that code added to this hook function never attempts
    to block in any way (for example, call xQueueReceive() with a block time
    specified, or call vTaskDelay()).  If the application makes use of the
    vTaskDelete() API function (as this demo application does) then it is also
    important that vApplicationIdleHook() is permitted to return to its calling
    function, because it is the responsibility of the idle task to clean up
    memory allocated by the kernel to any task that has since been deleted. */
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
    (void)pcTaskName;
    (void)pxTask;

    /* Run time stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected. */
    IntMasterDisable();
    for (;;)
        ;
}
/*-----------------------------------------------------------*/

void *malloc(size_t xSize)
{
    /* There should not be a heap defined, so trap any attempts to call
    malloc. */
    IntMasterDisable();
    for (;;)
        ;
}
/*-----------------------------------------------------------*/
