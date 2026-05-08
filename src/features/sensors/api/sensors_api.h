#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

void addRotation(void);

void Sensor_Init(void);

uint16_t Sensor_GetLux(void);
void Sensor_UpdateLux(uint16_t lux);

uint16_t Sensor_GetAccel(void);
void Sensor_UpdateAccel(uint16_t accel);

uint16_t Sensor_getTemp(void);
void Sensor_UpdateTemp(uint16_t temp);

uint16_t getHumidity(void);
void Sensor_UpdateHumidity(uint16_t humidity);

uint16_t Sensor_GetSpeed(void);
void Sensor_UpdateSpeed(uint16_t rpm);

uint16_t Sensor_GetPower(void);
void Sensor_UpdatePower(uint16_t power);

uint16_t Sensor_GetDistance(void);
void Sensor_UpdateDistance(uint16_t dist);
