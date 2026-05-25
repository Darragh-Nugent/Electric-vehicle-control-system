#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "power_sensor_driver.h"

static power_sensor_dev_t dev;

void Power_Sensor_Init(power_sensor_dev_t new_dev)
{
    dev = new_dev;
}

float Power_Sensor_GetCurrent(uint32_t adcA, uint32_t adcB)
{
    float converted_voltage[2];
    float current[3];

    converted_voltage[0] = (float)adcA * (dev.ref_voltage / dev.adc_max_counts);
    converted_voltage[1] = (float)adcB * (dev.ref_voltage / dev.adc_max_counts);

    current[0] = (dev.ref_voltage / 2.0f - converted_voltage[0]) 
        / (dev.gain * dev.resistance);
    current[1] = (dev.ref_voltage / 2.0f - converted_voltage[1]) 
        / (dev.gain * dev.resistance);

    // Sum of currents in motor will always add to 0
    current[2] = -(current[0] + current[1]);

    // Find average using a denominator of 2 as one will always be 0
    return (fabs(current[0]) + fabs(current[1]) + fabs(current[2])) / 2;
}

float Power_Sensor_GetPower(uint32_t adcA, uint32_t adcB)
{
    float total_current = Power_Sensor_GetCurrent(adcA, adcB);
    return total_current * dev.motor_voltage;
}
