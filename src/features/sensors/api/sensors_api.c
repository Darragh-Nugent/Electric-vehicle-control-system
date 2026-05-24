#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "sensors_api.h"

typedef struct
{
    sensor_sample_t sample;
    SemaphoreHandle_t mutex;
} sensor_point_t;

typedef struct
{
    sensor_point_t lux;
    sensor_point_t abs_accel;
    sensor_point_t temp;
    sensor_point_t humidity;
    sensor_point_t rpm;
    sensor_point_t power;
    sensor_point_t distance;
} sensors_t;

static sensors_t sensor;
static sensor_point_t accelThreshold;
static sensor_point_t distThreshold;
static sensor_point_t powerThreshold;

void Sensor_Init(void)
{
    sensor.lux.sample = (sensor_sample_t){0};
    sensor.abs_accel.sample = (sensor_sample_t){0};
    sensor.temp.sample = (sensor_sample_t){0};
    sensor.humidity.sample = (sensor_sample_t){0};
    sensor.rpm.sample = (sensor_sample_t){0};
    sensor.power.sample = (sensor_sample_t){0};
    sensor.distance.sample = (sensor_sample_t){0};

    sensor.lux.mutex = xSemaphoreCreateMutex();
    sensor.abs_accel.mutex = xSemaphoreCreateMutex();
    sensor.temp.mutex = xSemaphoreCreateMutex();
    sensor.humidity.mutex = xSemaphoreCreateMutex();
    sensor.rpm.mutex = xSemaphoreCreateMutex();
    sensor.power.mutex = xSemaphoreCreateMutex();
    sensor.distance.mutex = xSemaphoreCreateMutex();

    // Initially set thresholds to max value
    powerThreshold.sample.value = 0xFFFF;
    distThreshold.sample.value = 0xFFFF;
    accelThreshold.sample.value = 0xFFFF;


    powerThreshold.mutex = xSemaphoreCreateMutex();
    distThreshold.mutex = xSemaphoreCreateMutex();
    accelThreshold.mutex = xSemaphoreCreateMutex();


}

static void Sensor_Update(sensor_point_t* point, uint16_t value)
{
    xSemaphoreTake(point->mutex, portMAX_DELAY);

    point->sample.value = value;
    point->sample.seq++;

    xSemaphoreGive(point->mutex);
}

static sensor_sample_t Sensor_Get(sensor_point_t* point)
{
    sensor_sample_t temp;

    xSemaphoreTake(point->mutex, portMAX_DELAY);

    temp = point->sample;

    xSemaphoreGive(point->mutex);

    return temp;
}


void Sensor_UpdateLux(uint16_t value)
{
    Sensor_Update(&sensor.lux, value);
}

sensor_sample_t Sensor_GetLux(void)
{
    return Sensor_Get(&sensor.lux);
}

void Sensor_UpdateAccel(uint16_t value)
{
    Sensor_Update(&sensor.abs_accel, value);
}

sensor_sample_t Sensor_GetAccel(void)
{
    return Sensor_Get(&sensor.abs_accel);
}

void Sensor_UpdateThresholdAccel(uint16_t value)
{
    Sensor_Update(&accelThreshold, value);
}

sensor_sample_t Sensor_GetThresholdAccel(void)
{
    return Sensor_Get(&accelThreshold);
}

void Sensor_UpdateTemp(uint16_t value)
{
    Sensor_Update(&sensor.temp, value);
}

sensor_sample_t Sensor_GetTemp(void)
{
    return Sensor_Get(&sensor.temp);
}

void Sensor_UpdateHumidity(uint16_t value)
{
    Sensor_Update(&sensor.humidity, value);
}

sensor_sample_t Sensor_GetHumidity(void)
{
    return Sensor_Get(&sensor.humidity);
}

void Sensor_UpdateSpeed(uint16_t value)
{
    Sensor_Update(&sensor.rpm, value);
}

sensor_sample_t Sensor_GetSpeed(void)
{
    return Sensor_Get(&sensor.rpm);
}

void Sensor_UpdatePower(uint16_t value)
{
    Sensor_Update(&sensor.power, value);
}

sensor_sample_t Sensor_GetPower(void)
{
    return Sensor_Get(&sensor.power);
}

void Sensor_UpdateThresholdPower(uint16_t value)
{
    Sensor_Update(&powerThreshold, value);
}

sensor_sample_t Sensor_GetThresholdPower(void)
{
    return Sensor_Get(&powerThreshold);
}

void Sensor_UpdateDistance(uint16_t value)
{
    Sensor_Update(&sensor.distance, value);
}

sensor_sample_t Sensor_GetDistance(void)
{
    return Sensor_Get(&sensor.distance);
}

void Sensor_UpdateThresholdDistance(uint16_t value)
{
    Sensor_Update(&distThreshold, value);
}

sensor_sample_t Sensor_GetThresholdDistance(void)
{
    return Sensor_Get(&distThreshold);
}