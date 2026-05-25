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
#include "drivers/power_sensor_driver.h"

#define REF_VOLTS 3.3f
#define ADC_MAX_COUNTS 4096.0f
#define GAIN 10
#define SHUNT_RESISTANCE 0.007f
#define VOLTS 24.0f

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
    power_sensor_dev_t dev;
    dev.ref_voltage = REF_VOLTS;
    dev.adc_max_counts = ADC_MAX_COUNTS;
    dev.gain = GAIN;
    dev.resistance = SHUNT_RESISTANCE;
    dev.motor_voltage = VOLTS;
    Power_Sensor_Init(dev);
}

// this is just the getPower without the voltage scaling. 
// TODO: remove duplicate code. Use getCurrent always and scale by voltage when needed (it's a constant). 
float getCurrent(void)
{
    ADCProcessorTrigger(ADC1_BASE, 0);
    xSemaphoreTake(xPowerSemaphore, portMAX_DELAY);
    uint32_t local_adc_values[2];

    taskENTER_CRITICAL();
    local_adc_values[0] = adc_values[0];
    local_adc_values[1] = adc_values[1];
    taskEXIT_CRITICAL();

    return Power_Sensor_GetCurrent(local_adc_values[0], local_adc_values[1]);
}

float getPower(void)
{
    ADCProcessorTrigger(ADC1_BASE, 0);
    xSemaphoreTake(xPowerSemaphore, portMAX_DELAY);
    uint32_t local_adc_values[2];

    taskENTER_CRITICAL();
    local_adc_values[0] = adc_values[0];
    local_adc_values[1] = adc_values[1];
    taskEXIT_CRITICAL();

    return Power_Sensor_GetPower(local_adc_values[0], local_adc_values[1]);
}