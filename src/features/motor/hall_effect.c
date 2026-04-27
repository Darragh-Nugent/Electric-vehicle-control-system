#include "driverlib/pin_map.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "drivers/rtos_hw_drivers.h"
#include "utils/uartstdio.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/interrupt.h"

#include "motorlib.h"
#include "features/priorities.h"
#include "motor_api.h"

extern SemaphoreHandle_t motorUpToSpeedSemaphore;

volatile bool speed_semaphore_given = false; // bytes are atomic on Cortex-M4 processors.

void hallSensorHandler(void)
{
    GPIOIntClear(GPIO_PORTM_BASE, GPIO_PIN_3);
    GPIOIntClear(GPIO_PORTH_BASE, GPIO_PIN_2);
    GPIOIntClear(GPIO_PORTN_BASE, GPIO_PIN_2);

    bool hall_a = GPIOPinRead(GPIO_PORTM_BASE, GPIO_PIN_3);
    bool hall_b = GPIOPinRead(GPIO_PORTH_BASE, GPIO_PIN_2);
    bool hall_c = GPIOPinRead(GPIO_PORTN_BASE, GPIO_PIN_2);

    updateMotor(hall_a, hall_b, hall_c);

    // speed measuring code here

    if (!speed_semaphore_given) // TODO: also check if the speed threshold has been met.
    {
        speed_semaphore_given = true;
        xSemaphoreGiveFromISR(motorUpToSpeedSemaphore, NULL);
    }
}

void hallSensorGPIOConfig(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);

    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM)) {}
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOH)) {}
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)) {}

    GPIOPinTypeGPIOInput(GPIO_PORTM_BASE, GPIO_PIN_3);
    GPIOPinTypeGPIOInput(GPIO_PORTH_BASE, GPIO_PIN_2);
    GPIOPinTypeGPIOInput(GPIO_PORTN_BASE, GPIO_PIN_2);

    GPIOIntTypeSet(GPIO_PORTM_BASE, GPIO_PIN_3, GPIO_BOTH_EDGES);
    GPIOIntTypeSet(GPIO_PORTH_BASE, GPIO_PIN_2, GPIO_BOTH_EDGES);
    GPIOIntTypeSet(GPIO_PORTN_BASE, GPIO_PIN_2, GPIO_BOTH_EDGES);
}

void hallSensorIntEnable(void)
{
    GPIOIntEnable(GPIO_PORTM_BASE, GPIO_PIN_3);
    GPIOIntEnable(GPIO_PORTH_BASE, GPIO_PIN_2);
    GPIOIntEnable(GPIO_PORTN_BASE, GPIO_PIN_2);

    IntEnable(INT_GPIOM);
    IntEnable(INT_GPIOH);
    IntEnable(INT_GPION);
}

void hallSensorIntDisable(void)
{
    GPIOIntDisable(GPIO_PORTM_BASE, GPIO_PIN_3);
    GPIOIntDisable(GPIO_PORTH_BASE, GPIO_PIN_2);
    GPIOIntDisable(GPIO_PORTN_BASE, GPIO_PIN_2);

    IntDisable(INT_GPIOM);
    IntDisable(INT_GPIOH);
    IntDisable(INT_GPION);
}
