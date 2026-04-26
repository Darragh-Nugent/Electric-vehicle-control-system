#include "driverlib/pin_map.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "drivers/rtos_hw_drivers.h"
#include "utils/uartstdio.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "drivers/opt3001.h"
#include "driverlib/i2c.h"
#include "driverlib/interrupt.h"
#include "drivers/bmi160.h"
#include "drivers/i2cOptDriver.h"

#include "motorlib.h"
#include "features/priorities.h"
#include "features/sensors/sensor_events.h"

/*-----------------------------------------------------------*/

/*
 * Current clock period
 */
extern uint32_t g_ui32SysClock;

/*
 * Time stamp global variable.
 */
volatile uint32_t g_ui32TimeStamp;
/*
 * Global variable to log the last GPIO button pressed.
 */
volatile static uint32_t g_pui32ButtonPressed = NULL;

/*
 * The binary semaphore used by the switch ISR & task.
 */
extern SemaphoreHandle_t xButtonSemaphore;

extern EventGroupHandle_t xSensorEvents;

/*-----------------------------------------------------------*/

/*
 * The configuration struct for the acceleration sensor
 */
struct bmi160_dev bmi160dev;

/*
 * The struct for holding the acceleration data
 */
struct bmi160_sensor_data bmi160_accel;

/*-----------------------------------------------------------*/

extern void xI2CHandler(void);

/*
 * Functions for the light sensor
 */
extern void prvSensorOPT3001Init(void);
uint16_t getFilteredLight(uint16_t newValue);

/*-----------------------------------------------------------*/

/*
 * Functions for the acceleration sensor
 */
extern void prvSensorBmi160Init(struct bmi160_dev *bmi160dev);
extern int16_t getAbsoluteAccel(struct bmi160_sensor_data bmi160_accel);
extern double getFilteredAccel(int16_t rawAccel);

/*
 * Called by main() to do example specific hardware configurations and to
 * create the Process Switch task.
 */
void vCreateOPTTask(void);

/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

void vLightSensorTask(void *pvParameters)
{
    bool success = false;
    uint16_t rawData = 0;
    float convertedLux = 0;

    // UARTprintf("Starting Light Sensor Task\n");

    // // Test that sensor is set up correctly
    // UARTprintf("Testing OPT3001 Sensor:\n");
    // success = sensorOpt3001Test();

    // Initialise sensor
    bool result = sensorOpt3001Init();
    if (!result)
    {
        UARTprintf("Sensor not init\n");
    }

    prvSensorBmi160Init(&bmi160dev);

    UARTprintf("Sensor start\n");
    // If the test fails, retry the full init + test sequence rather than
    // retesting a sensor that was never successfully enabled.
    while (!success)
    {
        SysCtlDelay(g_ui32SysClock);
        UARTprintf("Test Failed, Trying again\n");
        success = sensorOpt3001Test();
    }

    UARTprintf("All Tests Passed!\n\n");

    // uint32_t id = 0;

    uint32_t events;

    // Loop Forever
    while (1)
    {
        events = xEventGroupWaitBits(xSensorEvents, ALL_SENSOR_EVENTS, pdTRUE, pdFALSE, portMAX_DELAY);

        if (events & LIGHT_SENSOR_EVENT)
        {
            // Read and convert OPT values
            success = sensorOpt3001Read(&rawData);

            if (success)
            {
                sensorOpt3001Convert(rawData, &convertedLux);
                uint16_t lux = (uint16_t)convertedLux;
                uint16_t filteredLux = getFilteredLight(lux);
                // UARTprintf("Lux: %5d\n", lux_int);
                UARTprintf("%u,%d\n", lux, filteredLux);
            }
        }
        if (events & ACCEL_SENSOR_EVENT)
        {
            int8_t result = bmi160_get_sensor_data(BMI160_ACCEL_SEL, &bmi160_accel, NULL, &bmi160dev);
            int16_t absoluteAccel = getAbsoluteAccel(bmi160_accel);
            double filteredAccel = getFilteredAccel(absoluteAccel);
            // UARTprintf("%d,%d\n", absoluteAccel, (int)(filteredAccel));

            //     if (filteredAccel > 6000)
            //     {
            //         Motor_EStop();
            //     }
            // }
        }
    }
}
