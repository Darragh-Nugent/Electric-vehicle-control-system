#ifndef TASK_PRIORITIES_H
#define TASK_PRIORITIES_H

#include "FreeRTOS.h"
#include "task.h"

#define MOTOR_CONTROL_PRIORITY         (configMAX_PRIORITIES - 1)
#define ACCELERATION_SENSOR_PRIORITY   (tskIDLE_PRIORITY + 1)
#define DISTANCE_SENSOR_PRIORITY       (tskIDLE_PRIORITY + 1)
#define ENV_SENSOR_PRIORITY            (tskIDLE_PRIORITY + 1)
#define GUI_PRIORITY                   (tskIDLE_PRIORITY + 1)
#define LIGHT_SENSOR_PRIORITY          (tskIDLE_PRIORITY + 1)
#define POWER_SENSOR_PRIORITY          (tskIDLE_PRIORITY + 1)
#define SPEED_SENSOR_PRIORITY          (tskIDLE_PRIORITY + 1)

#endif
