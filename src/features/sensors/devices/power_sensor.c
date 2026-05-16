#include "driverlib/pin_map.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "utils/uartstdio.h"
#include "driverlib/adc.h"
#include "utils/muart.h"

#define REF_VOLTS 3.3f
#define ADC_MAX_COUNTS 4096.0f
#define GAIN 10
#define SHUNT_RESISTANCE 0.007f
#define VOLTS 24

SemaphoreHandle_t xPowerSemaphore;

uint32_t adc_values[2];

void xPowerHandler(void)
{
    BaseType_t xTaskWoken = pdFALSE;
    ADCIntClear(ADC1_BASE, 0);

    ADCSequenceDataGet(ADC1_BASE, 0, adc_values);

    xSemaphoreGiveFromISR(xPowerSemaphore, &xTaskWoken);
    portYIELD_FROM_ISR(xTaskWoken);
}

void PowerInit(void)
{
    xPowerSemaphore = xSemaphoreCreateBinary();
}

float getPower(void)
{
    ADCProcessorTrigger(ADC1_BASE, 0);
    xSemaphoreTake(xPowerSemaphore, portMAX_DELAY);
    uint32_t local_adc_values[2];
    float converted_voltage[2];
    float current[3];

    taskENTER_CRITICAL();
    local_adc_values[0] = adc_values[0];
    local_adc_values[1] = adc_values[1];
    taskEXIT_CRITICAL();

    converted_voltage[0] = (float)local_adc_values[0] * (REF_VOLTS / ADC_MAX_COUNTS);
    converted_voltage[1] = (float)local_adc_values[1] * (REF_VOLTS / ADC_MAX_COUNTS);

    current[0] = (REF_VOLTS / 2.0f - converted_voltage[0]) / (GAIN * SHUNT_RESISTANCE);
    current[1] = (REF_VOLTS / 2.0f - converted_voltage[1]) / (GAIN * SHUNT_RESISTANCE);

    current[2] = (current[0] + current[1]) / 2.0f;

    return (current[0] + current[1] + current[2]) * VOLTS;
}