#include <stdint.h>
#include <stdbool.h>

void SensorSHT31Init(void);
bool SensorSHT31GetTemHum(float *temp, float *humidity);