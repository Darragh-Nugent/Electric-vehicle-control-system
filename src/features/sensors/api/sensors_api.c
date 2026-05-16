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

static SemaphoreHandle_t xLuxMutex;
static SemaphoreHandle_t xAccelMutex;
static SemaphoreHandle_t xTempMutex;
static SemaphoreHandle_t xHumidityMutex;
static SemaphoreHandle_t xSpeedMutex;
static SemaphoreHandle_t xPowerMutex;
static SemaphoreHandle_t xDistMutex;

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

void Sensor_UpdateDistance(uint16_t value)
{
    Sensor_Update(&sensor.distance, value);
}

sensor_sample_t Sensor_GetDistance(void)
{
    return Sensor_Get(&sensor.distance);
}