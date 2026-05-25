#include <stdint.h>

typedef struct
{
    uint32_t counts_per_rotation;
    float sample_time;
} speed_sensor_dev_t;

void Speed_Sensor_Init(speed_sensor_dev_t new_dev);

float Speed_Sensor_GetRPM(uint32_t counts);