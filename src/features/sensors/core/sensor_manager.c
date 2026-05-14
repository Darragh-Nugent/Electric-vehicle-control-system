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

#define MAX_VALID_RPM 6000
#define MAX_INVALID_SPEED_COUNT 10

/*-----------------------------------------------------------*/

extern EventGroupHandle_t xSensorEvents;

/*-----------------------------------------------------------*/

extern void xI2CHandler(void);

/*
 * Functions for the light sensor
 */
extern void prvSensorOPT3001TimerInit(void);

/*-----------------------------------------------------------*/

/*
 * Uart enum
 */
uart_mode_t uart_mode = NONE;
uart_mode_t local_uart_mode = NONE;

/*-----------------------------------------------------------*/

void vSensorManagerTask(void *pvParameters)
{
    // UARTprintf("Sensor Manager start\n"); ///////////////
    // Initialise light sensor
    SensorOPT3001Init();

    // Intialise acceleration sensor
    SensorBmi160Init();

    // Intialise env sensor
    SensorSHT31Init();

    // Initialise the power sensor
    PowerInit();

    // Initialise the distance sensor
    // UARTprintf("Dist init start\n"); ///////////////
    SensorVL53L0xInit();
    // UARTprintf("Dist init success\n"); ///////////////

    // UARTprintf("All Tests Passed!\n\n"); ///////////////

    // Create filters for sensors
    moving_avg_t lightFilter = {{0}, 0, 0};
    moving_avg_t tempFilter = {{0}, 0, 0};
    moving_avg_t humidityFilter = {{0}, 0, 0};
    exp_filter_t accelFilter = {0.25, 0};
    exp_filter_t speedFilter = {0.3, 0};
    exp_filter_t powerFilter = {0.25, 0};
    exp_filter_t distFilter = {0.25, 0};

    float lastValidSpeed = 0.0f;
    uint8_t invalidSpeedCount = 0;

    uint32_t events;

    // Loop Forever
    while (1)
    {
        events = xEventGroupWaitBits(xSensorEvents, ALL_SENSOR_EVENTS, pdTRUE, pdFALSE, portMAX_DELAY);

        taskENTER_CRITICAL();
        local_uart_mode = uart_mode;
        taskEXIT_CRITICAL();

        if (events & LIGHT_SENSOR_EVENT)
        {
            float lux;
            // Read and convert OPT values
            if (getLux(&lux))
            {
                float filteredLux = filterMovingAverage(&lightFilter, lux);
                // UARTprintf("%d,%d\n", (int)lux, (int)filteredLux);
                Sensor_UpdateLux(filteredLux);
                if (local_uart_mode == LIGHT)
                {
                    UARTprintf("%d,%d\n", (int)lux, (int)filteredLux);
                }
            }
        }
        if (events & ACCEL_SENSOR_EVENT)
        {
            uint16_t absoluteAccel;
            if (getAbsoluteAccel(&absoluteAccel))
            {
                float filteredAccel = filterExponential(&accelFilter, (float)absoluteAccel);
                Sensor_UpdateAccel(filteredAccel);
                if (local_uart_mode == ACCEL)
                {
                    UARTprintf("%d,%d\n", absoluteAccel, (int)(filteredAccel));
                }

                //     if (filteredAccel > 6000)
                //     {
                //         Motor_EStop();
                //     }
                // }
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

        if (events & SPEED_SENSOR_EVENT)
        {
            float speed;
            speed = getRPM();
            float filteredSpeed = filterExponential(&speedFilter, speed);
            
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
                    filteredSpeed = 0.0f;
                }
            }

            else
            {
                invalidSpeedCount = 0;
                lastValidSpeed = filteredSpeed;
            }

            Sensor_UpdateSpeed((uint16_t)filteredSpeed);

            // UARTprintf("RAW:%d,FILT:%d\n", (int)speed, (int)filteredSpeed);

            if (local_uart_mode == SPEED)
            {
                UARTprintf("%d,%d\n", (int)speed, (int)filteredSpeed);
            }
        }

        // if (events & POWER_SENSOR_EVENT)
        // {
        //     uint32_t power = getPower();
        //     float filteredPower = filterExponential(&powerFilter, (float)power);

        //     Sensor_UpdatePower(filteredPower);
        //     // UARTprintf("%d,%d\n", power, (int)filteredPower);
        //     if (local_uart_mode == POWER)
        //     {
        //         UARTprintf("%d,%d\n", power, (int)filteredPower);
        //     }
        // }

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
            }
        }
    }
}



