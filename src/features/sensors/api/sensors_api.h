#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct 
{
    uint16_t value;
    uint32_t seq;
} sensor_sample_t;


void addRotation(void);

void Sensor_Init(void);

sensor_sample_t Sensor_GetLux(void);
void Sensor_UpdateLux(uint16_t lux);

sensor_sample_t Sensor_GetAccel(void);
void Sensor_UpdateAccel(uint16_t accel);

sensor_sample_t Sensor_GetTemp(void);
void Sensor_UpdateTemp(uint16_t temp);

sensor_sample_t Sensor_GetHumidity(void);
void Sensor_UpdateHumidity(uint16_t humidity);

sensor_sample_t Sensor_GetSpeed(void);
void Sensor_UpdateSpeed(uint16_t rpm);

sensor_sample_t Sensor_GetPower(void);
void Sensor_UpdatePower(uint16_t power);

sensor_sample_t Sensor_GetDistance(void);
void Sensor_UpdateDistance(uint16_t dist);
