#include "driverlib/pin_map.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

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

#include "motorlib.h"
#include "features/priorities.h"

/*-----------------------------------------------------------*/

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


extern void xI2CHandler(void);

/*
 * The tasks as described in the comments at the top of this file.
 */
static void prvSensorOPT3001Init(void);

/*
 * Called by main() to do example specific hardware configurations and to
 * create the Process Switch task.
 */
void vCreateOPTTask(void);

/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

uint16_t findMovingAverage(uint16_t newValue, uint16_t *prevLux)
{
    uint16_t sum = 0;
    // Shift the existing values to the left
    for (int i = 0; i < 7; i++)
    {
        prevLux[i] = prevLux[i + 1];
        sum += prevLux[i];
    }
    // UARTprintf("After summation in moving average");
    // Add the new value to the end of the array
    prevLux[7] = newValue;

    return (sum + prevLux[7]) / 8;
}

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
    sensorOpt3001Init();

    UARTprintf("OPT3001 Example\n");
    // If the test fails, retry the full init + test sequence rather than
    // retesting a sensor that was never successfully enabled.
    while (!success)
    {
        SysCtlDelay(g_ui32SysClock);
        UARTprintf("Test Failed, Trying again\n");
        success = sensorOpt3001Test();
    }

    UARTprintf("All Tests Passed!\n\n");

    uint32_t id = 0;

    uint16_t prevLux[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    // Loop Forever
    while (1)
    {
        SysCtlDelay(g_ui32SysClock / 100);

        // Read and convert OPT values
        success = sensorOpt3001Read(&rawData);

        if (success)
        {
            sensorOpt3001Convert(rawData, &convertedLux);
            int lux_int = (int)convertedLux;
            UARTprintf("Lux: %5d\n", lux_int);

            //  UARTprintf("Before semaphore take in opttask");
            // xSemaphoreTake(xUARTModeSemaphore, portMAX_DELAY);
            // Construct Text
            // if (UARTMode == NORMAL)
            // {
            //     id = 0;

            //     UARTprintf("Lux: %5d\n", lux_int);
            //     UARTprintf("Time Taken:  %u\n", timeTakenToReadI2C);
            // }
            // else if (UARTMode == PLOTTING)
            // {
            //     //UARTprintf("Calculating moving average");
            //     uint16_t movingAverage = findMovingAverage(lux_int, prevLux);
            //     UARTprintf("%u,%u,%d\n", id, lux_int, movingAverage);
            //     id++;
            // }

            // xSemaphoreGive(xUARTModeSemaphore);
        }
    }
}

