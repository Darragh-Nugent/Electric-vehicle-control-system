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
#include "i2c_message_struct.h"

/*-----------------------------------------------------------*/

extern uint32_t g_ui32SysClock;

extern QueueHandle_t xI2CQueue;

/*
 * Time stamp global variable.
 */
volatile uint32_t g_ui32TimeStamp;
/*
 * Global variable to log the last GPIO button pressed.
 */
volatile static uint32_t g_pui32ButtonPressed = NULL;

void vI2CManagerTask(void *pvParameters)
{
    i2c_message_t message;

    while (1)
    {
        xQueueReceive(xI2CQueue, &message, portMAX_DELAY);
    }
}