#include "driverlib/pin_map.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"

#include "utils/uartstdio.h"

#include "features/priorities.h"
#include "features/sensors/uart_mode.h"
#include "sensor_events.h"
#include "sensor_filters.h"
#include "features/sensors/devices/light_sensor.h"
#include "features/sensors/devices/acceleration_sensor.h"
#include "features/sensors/devices/env_sensor.h"
#include "features/sensors/devices/speed_sensor.h"
#include "features/sensors/devices/power_sensor.h"
#include "features/sensors/devices/distance_sensor.h"
#include "features/sensors/api/sensors_api.h"
#include "features/motor/motor_api.h"
#include "utils/muart.h"

#define MAX_VALID_RPM 5500
#define MAX_INVALID_SPEED_COUNT 10

/*-----------------------------------------------------------*/

extern EventGroupHandle_t xSensorEvents;

/*-----------------------------------------------------------*/

extern void xI2CHandler(void);

/*
 * Functions for the light sensor
 */
extern void prvSensorOPT3001TimerInit(void);

extern void motorRequestEStop(void);

/*-----------------------------------------------------------*/

/*
 * Uart enum
 */
volatile uart_mode_t uart_mode = NONE;

/*-----------------------------------------------------------*/

void vSensorManagerTask(void *pvParameters)
{
    UARTprintf("Sensor Manager start\n"); ///////////////
    // Initialise light sensor
    SensorOPT3001Init();

    // Intialise acceleration sensor
    SensorBmi160Init();

    // Intialise env sensor
    SensorSHT31Init();

    // // Initialise the power sensor
    PowerInit();

    // Initialise the distance sensor
    UARTprintf("Dist init start\n"); ///////////////
    SensorVL53L0xInit();
    // UARTprintf("Dist init success\n"); ///////////////

    UARTprintf("All Tests Passed!\n\n"); ///////////////

    // Create filters for sensors
    moving_avg_t lightFilter = {{0}, 0, 0};
    moving_avg_t tempFilter = {{0}, 0, 0};
    moving_avg_t humidityFilter = {{0}, 0, 0};
    exp_filter_t accelFilter = {0.25, 0};
    exp_filter_t powerFilter = {0.25, 0};
    exp_filter_t distFilter = {0.4, 0};

    uart_mode_t local_uart_mode = NONE;

    uint32_t events;

    xEventGroupClearBits(xSensorEvents, ALL_SENSOR_EVENTS);
    // Loop Forever
    while (1)
    {
        // events = xEventGroupWaitBits(xSensorEvents, ALL_SENSOR_EVENTS, pdTRUE, pdFALSE, portMAX_DELAY);
        events = xEventGroupWaitBits(xSensorEvents, SENSOR_MANAGER_EVENTS, pdTRUE, pdFALSE, portMAX_DELAY);
        taskENTER_CRITICAL();
        local_uart_mode = uart_mode;
        taskEXIT_CRITICAL();

        if (events & POWER_SENSOR_EVENT)
        {
            float power = getPower();
            float filteredPower = filterExponential(&powerFilter, power);
            Sensor_UpdatePower(filteredPower);
            if (local_uart_mode == POWER)
            {
                UARTprintf("%d,%d\n", (int)power, (int)filteredPower);
            }

            if (filteredPower > Sensor_GetThresholdPower().value)
            {
                // Motor_EStop();
            }
        }

        if (events & LIGHT_SENSOR_EVENT)
        {
            // UARTprintf("Lux entered\n");
            float lux;
            // Read and convert OPT values
            if (getLux(&lux))
            {
                // UARTprintf("got Lux\n");
                float filteredLux = filterMovingAverage(&lightFilter, lux);
                // MUARTprintf("%d,%d\n", (int)lux, (int)filteredLux);
                Sensor_UpdateLux(filteredLux);
                if (local_uart_mode == LIGHT)
                {
                    UARTprintf("%d,%d\n", (int)lux, (int)filteredLux);
                }
            }
            else
            {
                UARTprintf("Lux failed\n");
            }
        }

        if (events & ACCEL_SENSOR_EVENT)
        {
            uint16_t absoluteAccel;
            // UARTprintf(">> accel: requesting read\n");
            if (getAbsoluteAccel(&absoluteAccel))
            {
                // UARTprintf(">> accel: read returned ok=\n");
                float filteredAccel = filterExponential(&accelFilter, (float)absoluteAccel);
                Sensor_UpdateAccel(filteredAccel);
                if (local_uart_mode == ACCEL)
                {
                    UARTprintf("%d,%d\n", absoluteAccel, (int)(filteredAccel));
                }

                if (filteredAccel > Sensor_GetThresholdAccel().value)
                {
                    // Motor_EStop();
                }
            }
            else
            {
                UARTprintf(">> accel: read returned not ok\n");
            }
        }

        if (events & TEMP_SENSOR_EVENT)
        {
            float temp;
            float humidity;
            if (SensorSHT31GetTemHum(&temp, &humidity))
            {
                float filteredTemp = filterMovingAverage(&tempFilter, temp);
                float filteredHumidity = filterMovingAverage(&humidityFilter, humidity);

                Sensor_UpdateTemp(filteredTemp);
                Sensor_UpdateHumidity(filteredHumidity);
                if (local_uart_mode == TEMP)
                {
                    UARTprintf("%d,%d\n", (int)(temp), (int)(filteredTemp));
                }
                else if (local_uart_mode == HUMIDITY)
                {
                    UARTprintf("%d,%d\n", (int)(humidity), (int)(filteredHumidity));
                }
            }
        }

        if (events & DIST_SENSOR_EVENT)
        {
            uint16_t distance;
            if (getDistance(&distance))
            {
                float filteredDistance = filterExponential(&distFilter, distance);
                Sensor_UpdateDistance(filteredDistance);
                if (local_uart_mode == DIST)
                {
                    UARTprintf("%d,%d\n", (int)distance, (int)filteredDistance);
                }

                if (filteredDistance > Sensor_GetThresholdDistance().value)
                {
                    // Motor_EStop();
                }
            }
        }
    }
}

void vSpeedSensorTask(void *pvParameters)
{
    uart_mode_t local_uart_mode = NONE;

    exp_filter_t speedFilter = {0.3f, 0};
    float lastValidSpeed = 0.0f;
    uint8_t invalidSpeedCount = 0;

    const TickType_t speedPeriod = pdMS_TO_TICKS(10);
    TickType_t lastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        vTaskDelayUntil(&lastWakeTime, speedPeriod);

        taskENTER_CRITICAL();
        local_uart_mode = uart_mode;
        taskEXIT_CRITICAL();

        float rawSpeed = getRPM();
        float filteredSpeed = filterExponential(&speedFilter, rawSpeed);
        // UARTprintf("%d,%d\n", (int)rawSpeed, (int)filteredSpeed);

        if (filteredSpeed < 0.0f)
        {
            filteredSpeed = 0.0f;
        }

        if (filteredSpeed > MAX_VALID_RPM)
        {
            invalidSpeedCount++;

            if (invalidSpeedCount <= MAX_INVALID_SPEED_COUNT)
            {
                filteredSpeed = lastValidSpeed;
            }
            else
            {
                filteredSpeed = lastValidSpeed;
                // MotorRequestEStop();
            }
        }
        else
        {
            invalidSpeedCount = 0;
            lastValidSpeed = filteredSpeed;
        }

        Sensor_UpdateSpeed((uint16_t)filteredSpeed);

        if (local_uart_mode == SPEED)
        {
            UARTprintf("%d,%d\n", (int)rawSpeed, (int)filteredSpeed);
        }
    }
}

void vPowerSensorTask(void *pvParameters)
{
    uart_mode_t local_uart_mode = NONE;

    exp_filter_t powerFilter = {0.25, 0};

    // Initialise the power sensor
    PowerInit();

    const TickType_t period = pdMS_TO_TICKS(5);
    TickType_t lastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        vTaskDelayUntil(&lastWakeTime, period);

        taskENTER_CRITICAL();
        local_uart_mode = uart_mode;
        taskEXIT_CRITICAL();

        float power = getPower();
        float filteredPower = filterExponential(&powerFilter, power);
        Sensor_UpdatePower(filteredPower);
        if (local_uart_mode == POWER)
        {
            UARTprintf("%d,%d\n", (int)power, (int)filteredPower);
        }
    }
}