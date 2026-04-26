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
#include "features/sensors/i2c_message_struct.h"
#include "drivers/bmi160.h"

extern QueueHandle_t xI2CSendQueue;
extern QueueHandle_t xI2CRecvQueue;

extern SemaphoreHandle_t xOPT3001Semaphore;

/*
 * Write 2-byte value to I2C register
 */
bool I2C_write_reg(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *data, uint16_t len)
{
    // UARTprintf("Indside writei2c\n");

    i2c_send_message_t message;
    message.id = 0;   // writer task id (not used in this implementation)
    message.type = 1; // write
    message.sensor = ui8Addr;
    message.type = I2C_REG_WRITE;
    message.reg = ui8Reg;
    message.len = len;

    for (uint16_t i = 0; i < len; i++)
    {
        message.data[i] = data[i];
    }

    i2c_recv_message_t response;
    response.success = false;

    xQueueSend(xI2CSendQueue, &message, portMAX_DELAY);
    // UARTprintf("wait on semaphore in writei2c\n");
    xQueueReceive(xI2CRecvQueue, &response, pdMS_TO_TICKS(1000));

    return response.success;
}

/*
 * Write 2-byte value to I2C register
 */
bool I2C_write_bytes(uint8_t ui8Addr, uint8_t *data, uint16_t len)
{
    // UARTprintf("Indside writei2c\n");

    i2c_send_message_t message;
    message.id = 0;   // writer task id (not used in this implementation)
    message.type = I2C_RAW_WRITE; // write
    message.sensor = ui8Addr;
    message.len = len;

    for (uint16_t i = 0; i < len; i++)
    {
        message.data[i] = data[i];
    }

    i2c_recv_message_t response;
    response.success = false;

    xQueueSend(xI2CSendQueue, &message, portMAX_DELAY);
    // UARTprintf("wait on semaphore in writei2c\n");
    xQueueReceive(xI2CRecvQueue, &response, pdMS_TO_TICKS(1000));

    return response.success;
}

int8_t I2C_write_BMI160(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *data, uint16_t len)
{
    return I2C_write_reg(ui8Addr, ui8Reg, data, len)
               ? BMI160_OK
               : BMI160_E_COM_FAIL;
}

/*
 * Read 2-byte value from I2C register
 */
bool I2C_read_reg(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *data, uint16_t len)
{
    // UARTprintf("Indside readi2c\n");

    i2c_send_message_t message;
    message.id = 0;   // writer task id (not used in this implementation)
    message.type = I2C_REG_READ; // read
    message.sensor = ui8Addr;
    message.reg = ui8Reg;
    message.len = len;

    i2c_recv_message_t response;
    response.success = false;

    xQueueSend(xI2CSendQueue, &message, portMAX_DELAY);
    // UARTprintf("wait on semaphore in writei2c\n");
    xQueueReceive(xI2CRecvQueue, &response, pdMS_TO_TICKS(1000));

    for (uint16_t i = 0; i < len; i++)
    {
        data[i] = response.data[i];
    }

    return response.success;
}



/*
 * Read 2-byte value from I2C register
 */
bool I2C_read_bytes(uint8_t ui8Addr, uint8_t *data, uint16_t len)
{
    // UARTprintf("Indside readi2c\n");

    i2c_send_message_t message;
    message.id = 0;   // writer task id (not used in this implementation)
    message.type = I2C_RAW_READ; // read
    message.sensor = ui8Addr;
    message.len = len;

    i2c_recv_message_t response;
    response.success = false;

    xQueueSend(xI2CSendQueue, &message, portMAX_DELAY);
    // UARTprintf("wait on semaphore in writei2c\n");
    xQueueReceive(xI2CRecvQueue, &response, pdMS_TO_TICKS(1000));

    for (uint16_t i = 0; i < len; i++)
    {
        data[i] = response.data[i];
    }

    return response.success;
}

int8_t I2C_read_BMI160(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *data, uint16_t len)
{
    bool result = I2C_read_reg(ui8Addr, ui8Reg, data, len);
    if (!result)
    {
        // UARTprintf("BMI bad");
    }
    return result
        ? BMI160_OK
        : BMI160_E_COM_FAIL;
}