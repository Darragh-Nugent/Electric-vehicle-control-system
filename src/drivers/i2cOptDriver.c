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

extern QueueHandle_t xI2CSendQueue;
extern QueueHandle_t xI2CRecvQueue;

extern SemaphoreHandle_t xOPT3001Semaphore;

/*
 * Write 2-byte value to I2C register
 */
bool writeI2C(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *data)
{
    // UARTprintf("Indside writei2c\n");

    i2c_send_message_t message;
    message.id = 0;   // writer task id (not used in this implementation)
    message.type = 1; // write
    message.sensor = ui8Addr;
    message.reg = ui8Reg;
    message.data[0] = data[0];
    message.data[1] = data[1];

    i2c_recv_message_t response;
    response.success = false;

    xQueueSend(xI2CSendQueue, &message, portMAX_DELAY);
    // UARTprintf("wait on semaphore in writei2c\n");
    xQueueReceive(xI2CRecvQueue, &response, pdMS_TO_TICKS(1000));
    data[0] = response.data[0];
    data[1] = response.data[1];

    return response.success;
}

/*
 * Read 2-byte value from I2C register
 */
bool readI2C(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *data)
{
    // UARTprintf("Indside readi2c\n");

    i2c_send_message_t message;
    message.id = 0;   // writer task id (not used in this implementation)
    message.type = 0; // read
    message.sensor = ui8Addr;
    message.reg = ui8Reg;
    message.data[0] = data[0];
    message.data[1] = data[1];

    i2c_recv_message_t response;
    response.success = false;

    xQueueSend(xI2CSendQueue, &message, portMAX_DELAY);
    // UARTprintf("wait on semaphore in writei2c\n");
    xQueueReceive(xI2CRecvQueue, &response, pdMS_TO_TICKS(1000));
    data[0] = response.data[0];
    data[1] = response.data[1];

    return response.success;
}