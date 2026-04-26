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

bool writeI2CInternal(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *data, uint16_t len);
bool readI2CInternal(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *data, uint16_t len);

extern uint32_t g_ui32SysClock;

/*
 * Time stamp global variable.
 */
volatile uint32_t g_ui32TimeStamp;

extern QueueHandle_t xI2CSendQueue;
extern QueueHandle_t xI2CRecvQueue;

extern SemaphoreHandle_t xI2CSemaphore;
extern SemaphoreHandle_t xOPT3001Semaphore;

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

        // Initialise response
        response.id = message.id;
        response.sensor = message.sensor;
        response.success = true;

        // UARTprintf("Received message from task %d: sensor %d, reg %d, val %d\n", message.id, message.sensor, message.reg, message.data);

        bool success = false;

        if (message.type == 0) // read
        {
            if (!readI2CInternal(message.sensor, message.reg, message.data, message.len))
            {
                UARTprintf("Bad read\n");
                response.success = false;
            }

            for (uint16_t i = 0; i < message.len; i++)
            {
                response.data[i] = message.data[i];
            }
        }
        else if (message.type == 1) // write
        {
            if (!writeI2CInternal(message.sensor, message.reg, message.data, message.len))
            {
                response.success = false;
            }
        }

        xQueueSend(xI2CRecvQueue, &response, portMAX_DELAY);
    }
}

bool writeI2CInternal(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *data, uint16_t len)
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

    if (xSemaphoreTake(xI2CSemaphore, pdMS_TO_TICKS(10)) != pdTRUE || errorFlag)
    {
        return false;
    }

    for (uint16_t i = 0; i < len; i++)
    {
        if (i == len - 1)
        {
            I2CMasterDataPut(I2C0_BASE, data[i]);
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
        }
        else
        {
            // Write LSB
            I2CMasterDataPut(I2C0_BASE, data[i]);
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
        }

        if (xSemaphoreTake(xI2CSemaphore, pdMS_TO_TICKS(10)) != pdTRUE || errorFlag)
        {
            return false;
        }
    }

    // UARTprintf("Sent LSB\n");
    return true;
}

/*
 * Read 2-byte value from I2C register
 */
bool readI2CInternal(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *data, uint16_t len)
{
    // UARTprintf("len = %d\n", len);
    xSemaphoreTake(xI2CSemaphore, 0);
    errorFlag = false;

    // Set slave address (write mode)
    I2CMasterSlaveAddrSet(I2C0_BASE, ui8Addr, false);

    // Send register address
    I2CMasterDataPut(I2C0_BASE, ui8Reg);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    // UARTprintf("Sent reg\n");
    if (xSemaphoreTake(xI2CSemaphore, pdMS_TO_TICKS(10)) != pdTRUE || errorFlag)
    {
        return false;
    }
    // Set slave address (read mode)
    I2CMasterSlaveAddrSet(I2C0_BASE, ui8Addr, true);

    for (uint16_t i = 0; i < len; i++)
    {
        if (len == 1)
        {
            // Single byte read — use dedicated single receive command
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
        }
        else if (i == 0)
        {
            // Read MSB
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
        }
        else if (i == len - 1)
        {
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
        }
        else
        {
            // Read LSB
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
        }

        // UARTprintf("%d sem start\n", i + 1);
        if (xSemaphoreTake(xI2CSemaphore, pdMS_TO_TICKS(10)) != pdTRUE || errorFlag)
        {
            return false;
        }
        // UARTprintf("%d sem end\n", i + 1);

        data[i] = I2CMasterDataGet(I2C0_BASE);
    }

    return true;
}