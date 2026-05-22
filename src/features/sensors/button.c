#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"

#include "features/motor/states.h"
#include "features/motor/motor_api.h"

/* Hardware includes. */
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "drivers/rtos_hw_drivers.h"

#include "driverlib/timer.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/uart.h"
#include "driverlib/i2c.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "uart_mode.h"

extern uart_mode_t uart_mode;

extern volatile bool motorEStopRequested;
extern SemaphoreHandle_t faultAcknowledgedSemaphore;
extern volatile motor_state_t motor_state;

void xButtonsHandler(void)
{
    // UARTprintf("Button\n");
    static uint32_t g_ui32TimeStamp = 0;   
    BaseType_t xOPTTaskWoken;
    uint32_t ui32Status;

    /* Initialize the xOPTTaskWoken as pdFALSE.  This is required as the
     * FreeRTOS interrupt safe API will change it if needed should a
     * context switch be required. */
    xOPTTaskWoken = pdFALSE;

    /* Read the buttons interrupt status to find the cause of the interrupt. */
    ui32Status = GPIOIntStatus(BUTTONS_GPIO_BASE, true);

    /* Clear the interrupt. */
    GPIOIntClear(BUTTONS_GPIO_BASE, ui32Status);

    /* Debounce the input with 200ms filter */
    if ((xTaskGetTickCount() - g_ui32TimeStamp) > pdMS_TO_TICKS(100))
    {
        /* Log which button was pressed to trigger the ISR. */
        if ((ui32Status & USR_SW1) == USR_SW1)
        {
            uart_mode = (uart_mode + 1) % MODE_COUNT;
        }
        else if ((ui32Status & USR_SW2) == USR_SW2)
        {
            if (motor_state == MOTOR_STATE_FAULT)
                {
                    // motorAcknowledgeFault();
                    xSemaphoreGiveFromISR(faultAcknowledgedSemaphore, &xOPTTaskWoken);
                }
                else
                {
                    motorRequestEStop();
                }
        }

        UARTprintf("Button: %d\n", uart_mode);

        /* This FreeRTOS API call will handle the context switch if it is
         * required or have no effect if that is not needed. */
        portYIELD_FROM_ISR(xOPTTaskWoken);
    }

    /* Update the time stamp. */
    g_ui32TimeStamp = xTaskGetTickCount();
}