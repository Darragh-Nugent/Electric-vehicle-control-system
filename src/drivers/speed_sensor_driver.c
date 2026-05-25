#include "speed_sensor_driver.h"

static speed_sensor_dev_t dev;

void Speed_Sensor_Init(speed_sensor_dev_t new_dev)
{
    dev = new_dev;
}

float Speed_Sensor_GetRPM(uint32_t counts)
{
    float rotations =
        (float)counts / dev.counts_per_rotation;

    return (rotations / dev.sample_time) * 60.0f;
}