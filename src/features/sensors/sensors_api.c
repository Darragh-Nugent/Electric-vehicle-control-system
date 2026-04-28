#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "sensors_api.h"

typedef struct
{
    uint16_t lux;
    uint16_t abs_accel;
    uint16_t temp;
    uint16_t humidity;
    uint16_t rmp;
    uint16_t power;
    uint16_t distance;
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
    xLuxMutex = xSemaphoreCreateMutex();
    xAccelMutex = xSemaphoreCreateMutex();
    xTempMutex = xSemaphoreCreateMutex();
    xHumidityMutex = xSemaphoreCreateMutex();
    xSpeedMutex = xSemaphoreCreateMutex();
    xPowerMutex = xSemaphoreCreateMutex();
    xDistMutex = xSemaphoreCreateMutex();
}

uint16_t Sensor_GetLux(void)
{
    uint16_t value;
    xSemaphoreTake(xLuxMutex, portMAX_DELAY);
    value = sensor.lux;
    xSemaphoreGive(xLuxMutex);

    return value;
}

void Sensor_UpdateLux(uint16_t lux)
{
    xSemaphoreTake(xLuxMutex, portMAX_DELAY);
    sensor.lux = lux;
    xSemaphoreGive(xLuxMutex);
}

uint16_t Sensor_GetAccel(void)
{
    uint16_t value;
    xSemaphoreTake(xAccelMutex, portMAX_DELAY);
    value = sensor.abs_accel;
    xSemaphoreGive(xAccelMutex);

    return value;
}

void Sensor_UpdateAccel(uint16_t accel)
{
    xSemaphoreTake(xAccelMutex, portMAX_DELAY);
    sensor.abs_accel = accel;
    xSemaphoreGive(xAccelMutex);
}

uint16_t Sensor_GetTemp(void)
{
    uint16_t value;
    xSemaphoreTake(xTempMutex, portMAX_DELAY);
    value = sensor.temp;
    xSemaphoreGive(xTempMutex);

    return value;
}

void Sensor_UpdateTemp(uint16_t temp)
{
    xSemaphoreTake(xTempMutex, portMAX_DELAY);
    sensor.temp = temp;
    xSemaphoreGive(xTempMutex);
}

uint16_t Sensor_GetHumidity(void)
{
    uint16_t value;
    xSemaphoreTake(xHumidityMutex, portMAX_DELAY);
    value = sensor.humidity;
    xSemaphoreGive(xHumidityMutex);

    return value;
}

void Sensor_UpdateHumidity(uint16_t humidity)
{
    xSemaphoreTake(xHumidityMutex, portMAX_DELAY);
    sensor.humidity = humidity;
    xSemaphoreGive(xHumidityMutex);
}

uint16_t Sensor_GetSpeed(void)
{
    uint16_t value;
    xSemaphoreTake(xSpeedMutex, portMAX_DELAY);
    value = sensor.rmp;
    xSemaphoreGive(xSpeedMutex);

    return value;
}

void Sensor_UpdateSpeed(uint16_t rpm)
{
    xSemaphoreTake(xSpeedMutex, portMAX_DELAY);
    sensor.rmp = rpm;
    xSemaphoreGive(xSpeedMutex);
}

uint16_t Sensor_GetPower(void)
{
    uint16_t value;
    xSemaphoreTake(xPowerMutex, portMAX_DELAY);
    value = sensor.power;
    xSemaphoreGive(xPowerMutex);

    return value;
}

void Sensor_UpdatePower(uint16_t power)
{
    xSemaphoreTake(xPowerMutex, portMAX_DELAY);
    sensor.power = power;
    xSemaphoreGive(xPowerMutex);
}

uint16_t Sensor_GetDist(void)
{
    uint16_t value;
    xSemaphoreTake(xDistMutex, portMAX_DELAY);
    value = sensor.distance;
    xSemaphoreGive(xDistMutex);

    return value;
}

void Sensor_UpdateDist(uint16_t dist)
{
    xSemaphoreTake(xDistMutex, portMAX_DELAY);
    sensor.distance = dist;
    xSemaphoreGive(xDistMutex);
}