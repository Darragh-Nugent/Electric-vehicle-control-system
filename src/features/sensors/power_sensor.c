#include "driverlib/pin_map.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

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
#include "driverlib/adc.h"

#define VOLTS 24

SemaphoreHandle_t xPowerSemaphore;

uint32_t current_values[2];

void xPowerHandler(void)
{
    BaseType_t xTaskWoken = pdFALSE;
    ADCSequenceDataGet(ADC0_BASE, 0, current_values);
    ADCIntClear(ADC0_BASE, 0);
    xSemaphoreGiveFromISR(xPowerSemaphore, xTaskWoken);
    portYIELD_FROM_ISR(xTaskWoken);
}

void PowerInit()
{
    xPowerSemaphore = xSemaphoreCreateBinary();
}

float getPower(void)
{
    ADCProcessorTrigger(ADC0_BASE, 0);
    xSemaphoreTake(xPowerSemaphore, portMAX_DELAY);
    uint32_t local_current[3];

    taskENTER_CRITICAL();
    local_current[0] = current_values[0];
    local_current[1] = current_values[1];
    taskEXIT_CRITICAL();

    local_current[3] = local_current[0] + local_current[1] / 2;

    return (local_current[0] + local_current[1] + local_current[2]) * VOLTS;
}