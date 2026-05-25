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

void xPowerHandler(void)
{
    BaseType_t xTaskWoken = pdFALSE;
    ADCIntClear(ADC1_BASE, 1);

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
    xSemaphoreTake(xPowerSemaphore, 0);

    uint32_t local_adc_values[2];

    ADCProcessorTrigger(ADC1_BASE, 1);
    xSemaphoreTake(xPowerSemaphore, portMAX_DELAY);
    ADCSequenceDataGet(ADC1_BASE, 1, local_adc_values);

    return Power_Sensor_GetCurrent(local_adc_values[0], local_adc_values[1]);
}

float getPower(void)
{
    xSemaphoreTake(xPowerSemaphore, 0);
    
    uint32_t local_adc_values[2];

    ADCProcessorTrigger(ADC1_BASE, 1);
    xSemaphoreTake(xPowerSemaphore, portMAX_DELAY);
    ADCSequenceDataGet(ADC1_BASE, 1, local_adc_values);    

    return Power_Sensor_GetPower(local_adc_values[0], local_adc_values[1]);
}

void getCurrentAndPower(float* current, float* power)
{
    uint32_t local_adc_values[2];

    ADCProcessorTrigger(ADC1_BASE, 1);
    xSemaphoreTake(xPowerSemaphore, portMAX_DELAY);
    ADCSequenceDataGet(ADC1_BASE, 1, local_adc_values);

    *current = Power_Sensor_GetCurrent(local_adc_values[0], local_adc_values[1]);
    *power = *current * VOLTS;
}