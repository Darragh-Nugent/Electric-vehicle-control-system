#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float ref_voltage;
    float adc_max_counts;
    float gain;
    float resistance;
    float motor_voltage;
    uint32_t offsetA;
    uint32_t offsetB;
} power_sensor_dev_t;

void Power_Sensor_Init(power_sensor_dev_t new_dev);
float Power_Sensor_GetCurrent(uint32_t adcA, uint32_t adcB);
float Power_Sensor_GetPower(uint32_t adcA, uint32_t adcB);