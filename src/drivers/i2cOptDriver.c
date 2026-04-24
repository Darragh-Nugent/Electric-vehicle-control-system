/**************************************************************************************************
 * Filename: i2cOptDriver.c
 * By: Jesse Haviland
 * Created: 1 February 2019
 * Revised: 23 March 2019
 * Revision: 2.0
 *
 * Description: i2c Driver for use with opt3001.c and the TI OP3001 Optical Sensor
 **************************************************************************************************/

// ----------------------- Includes -----------------------
#include "i2cOptDriver.h"
#include "inc/hw_memmap.h"
#include "driverlib/i2c.h"
#include "utils/uartstdio.h"
#include "driverlib/sysctl.h"
#include "FreeRTOS.h"
#include "semphr.h"

extern SemaphoreHandle_t xI2CSemaphore;
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

/*
 * Write 2-byte value to I2C register
 */
bool writeI2C(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *data)
{
    errorFlag = false;
    while (xSemaphoreTake(xI2CSemaphore, 0) == pdTRUE);

    UARTprintf("Inside writeI2C\n");

    // Set slave address (write mode)
    I2CMasterSlaveAddrSet(I2C0_BASE, ui8Addr, false);

    // Send register address
    I2CMasterDataPut(I2C0_BASE, ui8Reg);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);

    if (xSemaphoreTake(xI2CSemaphore, portMAX_DELAY) != pdTRUE || errorFlag)
    {
        return false;
    }
    UARTprintf("Sent reg address\n");

    // Send MSB
    I2CMasterDataPut(I2C0_BASE, data[0]);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);

    if (xSemaphoreTake(xI2CSemaphore, portMAX_DELAY) != pdTRUE || errorFlag)
    {
        return false;
    }
    UARTprintf("Sent MSB\n");

    // Send LSB
    I2CMasterDataPut(I2C0_BASE, data[1]);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);

    if (xSemaphoreTake(xI2CSemaphore, portMAX_DELAY) != pdTRUE || errorFlag)
    {
        return false;
    }
    UARTprintf("Sent LSB\n");
    return true;
}

/*
 * Read 2-byte value from I2C register
 */
bool readI2C(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *data)
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