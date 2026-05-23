#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


float getPower(void);
void PowerInit(void);
float getCurrent(void);
