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
#include "drivers/i2cOptDriver.h"

#include "motorlib.h"
#include "features/priorities.h"
#include "i2c_message_struct.h"

/*-----------------------------------------------------------*/

bool writeI2CInternal(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *data);
bool readI2CInternal(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *data);

extern uint32_t g_ui32SysClock;

/*
 * Time stamp global variable.
 */
volatile uint32_t g_ui32TimeStamp;

extern QueueHandle_t xI2CSendQueue;
extern QueueHandle_t xI2CReceiveQueue;

extern SemaphoreHandle_t xI2CSemaphore;
extern SemaphoreHandle_t xOPT3001Semaphore;

/*
 * Global variable to log the last GPIO button pressed.
 */
volatile static uint32_t g_pui32ButtonPressed = NULL;

volatile bool errorFlag = false;

/*
 * Handles interrupts from I2C0. Checks for timeout and gives xI2CSemaphore to unblock
 */
void xI2CHandler(void)
{
    BaseType_t xI2CTaskWoken;
    uint32_t ui32I2CStatus;

    /* Read interrupt status */
    ui32I2CStatus = I2CMasterIntStatusEx(I2C0_BASE, true);
    /* Clear interrupt */
    I2CMasterIntClearEx(I2C0_BASE, ui32I2CStatus);

    /* Initialize as pdFALSE for FreeRTOS ISR handling */
    xI2CTaskWoken = pdFALSE;

    /* Check which interrupt was called */
    if ((ui32I2CStatus & I2C_MASTER_INT_TIMEOUT) == I2C_MASTER_INT_TIMEOUT)
    {
        errorFlag = true;
    }

    xSemaphoreGiveFromISR(xI2CSemaphore, &xI2CTaskWoken);

    /* Yield if required */
    portYIELD_FROM_ISR(xI2CTaskWoken);
}

void vI2CManagerTask(void *pvParameters)
{
    i2c_send_message_t message;
    i2c_recv_message_t response;

    while (1)
    {
        xQueueReceive(xI2CSendQueue, &message, portMAX_DELAY);
        // UARTprintf("Received message from task %d: sensor %d, reg %d, val %d\n", message.id, message.sensor, message.reg, message.data);

        uint8_t data[2];
        bool success = false;

        if (message.type == 0) // read
        {
            if (!readI2CInternal(message.sensor, message.reg, message.data))
            {
            }
        }
        else if (message.type == 1) // write
        {
            if (!writeI2CInternal(message.sensor, message.reg, message.data))
            {
            }
        }

        switch (message.id)
        {
        case 0:
            xSemaphoreGive(xOPT3001Semaphore);
            break;
        
        default:
            break;
        }
    }
}

bool writeI2CInternal(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *data)
{
    errorFlag = false;
    while (xSemaphoreTake(xI2CSemaphore, 0) == pdTRUE)
        ;

    // UARTprintf("Inside writeI2C\n");

    // Set slave address (write mode)
    I2CMasterSlaveAddrSet(I2C0_BASE, ui8Addr, false);

    // Send register address
    I2CMasterDataPut(I2C0_BASE, ui8Reg);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);

    if (xSemaphoreTake(xI2CSemaphore, portMAX_DELAY) != pdTRUE || errorFlag)
    {
        return false;
    }
    // UARTprintf("Sent reg address\n");

    // Send MSB
    I2CMasterDataPut(I2C0_BASE, data[0]);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);

    if (xSemaphoreTake(xI2CSemaphore, portMAX_DELAY) != pdTRUE || errorFlag)
    {
        return false;
    }
    // UARTprintf("Sent MSB\n");

    // Send LSB
    I2CMasterDataPut(I2C0_BASE, data[1]);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);

    if (xSemaphoreTake(xI2CSemaphore, portMAX_DELAY) != pdTRUE || errorFlag)
    {
        return false;
    }
    // UARTprintf("Sent LSB\n");
    return true;
}

/*
 * Read 2-byte value from I2C register
 */
bool readI2CInternal(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *data)
{
    xSemaphoreTake(xI2CSemaphore, 0);
    errorFlag = false;

    // Set slave address (write mode)
    I2CMasterSlaveAddrSet(I2C0_BASE, ui8Addr, false);

    // Send register address
    I2CMasterDataPut(I2C0_BASE, ui8Reg);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);

    if (xSemaphoreTake(xI2CSemaphore, portMAX_DELAY) != pdTRUE || errorFlag)
    {
        return false;
    }
    // Set slave address (read mode)
    I2CMasterSlaveAddrSet(I2C0_BASE, ui8Addr, true);

    // Read MSB
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);

    if (xSemaphoreTake(xI2CSemaphore, portMAX_DELAY) != pdTRUE || errorFlag)
    {
        return false;
    }
    data[0] = I2CMasterDataGet(I2C0_BASE);

    // Read LSB
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);

    if (xSemaphoreTake(xI2CSemaphore, portMAX_DELAY) != pdTRUE || errorFlag)
    {
        return false;
    }

    data[1] = I2CMasterDataGet(I2C0_BASE);
    return true;
}