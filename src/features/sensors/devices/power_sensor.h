#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

void PowerInit(void);
float getPower(void);
float getCurrent(void);
void getCurrentAndPower(float* current, float* power);
