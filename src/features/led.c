#include "stdint.h"
#include "stdbool.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/gpio.h"
void enableHeadLights(void)
{
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0 | GPIO_PIN_1);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4 | GPIO_PIN_0);
};
void disableHeadLights(void)
{
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, 0x0);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4 | GPIO_PIN_0, 0x0);
};