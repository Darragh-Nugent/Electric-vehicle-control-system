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
#include "utils///uartstdio.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "drivers/opt3001.h"
#include "driverlib/i2c.h"
#include "driverlib/interrupt.h"
#include "drivers/i2cDriver.h"

#include "motorlib.h"
#include "features/priorities.h"

/*-----------------------------------------------------------*/

#define LIGHT_SENSOR_ADDRESS 0x47
#define ACCEL_SENSOR_ADDRESS 0x69
#define TEMP_SENSOR_ADDRESS 0x44

/*-----------------------------------------------------------*/

static void I2C_Reset(void);

bool I2C_write_bytes_internal(uint32_t base, uint8_t ui8Addr, uint8_t *data, uint16_t len);
bool I2C_write_reg_internal(uint32_t base, uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len);
bool I2C_write_single_internal(uint32_t base, uint8_t ui8Addr, uint8_t data);

bool I2C_read_bytes_internal(uint32_t base, uint8_t ui8Addr, uint8_t *data, uint16_t len);
bool I2C_read_reg_internal(uint32_t base, uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *data, uint16_t len);

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
// void xI2C0Handler(void)
// {
//     BaseType_t xI2CTaskWoken;
//     uint32_t ui32I2CStatus;

//     /* Read interrupt status */
//     ui32I2CStatus = I2CMasterIntStatusEx(I2C0_BASE, true);
//     /* Clear interrupt */
//     I2CMasterIntClearEx(I2C0_BASE, ui32I2CStatus);

//     /* Initialize as pdFALSE for FreeRTOS ISR handling */
//     xI2CTaskWoken = pdFALSE;

//     /* Check which interrupt was called */
//     if ((ui32I2CStatus & I2C_MASTER_INT_TIMEOUT) == I2C_MASTER_INT_TIMEOUT)
//     {
//         errorFlag = true;
//     }

//     xSemaphoreGiveFromISR(xI2CSemaphore, &xI2CTaskWoken);

//     /* Yield if required */
//     portYIELD_FROM_ISR(xI2CTaskWoken);
// }

void xI2C2Handler(void)
{
    BaseType_t xI2CTaskWoken = pdFALSE;

    // Clear all pending interrupts
    uint32_t ui32I2CStatus = I2CMasterIntStatusEx(I2C2_BASE, true);
    I2CMasterIntClearEx(I2C2_BASE, ui32I2CStatus);

    xSemaphoreGiveFromISR(xI2CSemaphore, &xI2CTaskWoken);
    portYIELD_FROM_ISR(xI2CTaskWoken);
}

void vI2CManagerTask(void *pvParameters)
{
    i2c_send_message_t message;
    i2c_recv_message_t response;

    I2C_Reset();
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_I2C2));
    ////uartprintf("!! I2C Manager: peripheral ready, starting\n");

    while (1)
    {
        //uartprintf("!! I2C Manager: waiting for message\n");
        xQueueReceive(xI2CSendQueue, &message, portMAX_DELAY);
        //uartprintf("!! I2C Manager: got message type=%d sensor=0x%02x\n", message.type, message.sensor);

        // Initialise response
        response.id = message.id;
        response.sensor = message.sensor;
        response.success = true;

        // uint32_t I2C_Base = message.sensor == TEMP_SENSOR_ADDRESS ? I2C2_BASE : I2C0_BASE;
        uint32_t I2C_Base = I2C2_BASE;

        switch (message.type)
        {
        case I2C_REG_READ:
            if (!I2C_read_reg_internal(I2C_Base, message.sensor, message.reg, message.data, message.len))
            {
                //uartprintf("Bad read\n");
                response.success = false;
                break;
            }

            for (uint16_t i = 0; i < message.len; i++)
            {
                response.data[i] = message.data[i];
            }
            break;

        case I2C_REG_WRITE:
            if (!I2C_write_reg_internal(I2C_Base, message.sensor, message.reg, message.data, message.len))
            {
                response.success = false;
            }

            break;

        case I2C_RAW_READ:
            if (!I2C_read_bytes_internal(I2C_Base, message.sensor, message.data, message.len))
            {
                response.success = false;
                break;
            }

            for (uint16_t i = 0; i < message.len; i++)
            {
                response.data[i] = message.data[i];
            }
            break;

        case I2C_RAW_WRITE:
            if (!I2C_write_bytes_internal(I2C_Base, message.sensor, message.data, message.len))
            {
                response.success = false;
            }
            break;

        default:
            response.success = false;
            break;
        }

        //uartprintf("!! I2C Manager: sending response success=%d\n", response.success);
        if (xQueueSend(xI2CRecvQueue, &response, pdMS_TO_TICKS(200)) != pdTRUE)
        {
            //uartprintf("!! I2C Manager: response queue full!\n");
        }
        //uartprintf("!! I2C Manager: response sent\n");
    }
}

static void I2C_Reset(void)
{
    //uartprintf("!! I2C_Reset: starting\n");

    IntDisable(INT_I2C2);
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C2);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C2); // <-- missing!

    uint32_t timeout = 10000;
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_I2C2))
    {
        if (--timeout == 0)
        {
            //uartprintf("!! I2C_Reset: peripheral never became ready, bailing\n");
            return; // bail rather than spin forever
        }
    }

    //uartprintf("!! I2C_Reset: peripheral ready\n");

    //
    // Temporarily switch pins to GPIO
    //
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_5); // SCL
    GPIOPinTypeGPIOInput(GPIO_PORTN_BASE, GPIO_PIN_4);  // SDA

    //
    // Ensure SCL starts high
    //
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_5, GPIO_PIN_5);

    vTaskDelay(pdMS_TO_TICKS(100));

    //
    // If SDA stuck low, clock out slave
    //
    if (!(GPIOPinRead(GPIO_PORTN_BASE, GPIO_PIN_4) & GPIO_PIN_4))
    {
        for (int i = 0; i < 9; i++)
        {
            GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_5, 0);
            vTaskDelay(pdMS_TO_TICKS(100));

            GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_5, GPIO_PIN_5);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }

    //
    // Restore I2C pin muxing
    //
    GPIOPinConfigure(GPIO_PN5_I2C2SCL);
    GPIOPinConfigure(GPIO_PN4_I2C2SDA);

    GPIOPinTypeI2CSCL(GPIO_PORTN_BASE, GPIO_PIN_5);
    GPIOPinTypeI2C(GPIO_PORTN_BASE, GPIO_PIN_4);

    //
    // Reinitialize I2C
    //
    I2CMasterInitExpClk(I2C2_BASE, g_ui32SysClock, false);

    //
    // Clear pending interrupts
    //
    I2CMasterIntClearEx(I2C2_BASE,
                        I2C_MASTER_INT_DATA |
                            I2C_MASTER_INT_TIMEOUT);

    //
    // Re-enable interrupts
    //
    I2CMasterIntEnableEx(I2C2_BASE,
                         I2C_MASTER_INT_DATA |
                             I2C_MASTER_INT_TIMEOUT);

    IntEnable(INT_I2C2);

    //
    // Clear software state
    //
    errorFlag = false;

    while (xSemaphoreTake(xI2CSemaphore, 0) == pdTRUE)
        ;
}

bool I2C_write_reg_internal(uint32_t base, uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    // //uartprintf("write reg\n");

    if (errorFlag)
    {
        I2C_Reset();
    }

    errorFlag = false;

    while (xSemaphoreTake(xI2CSemaphore, 0) == pdTRUE)
        ;

    I2CMasterSlaveAddrSet(base, addr, false);

    // Send register address as the first byte with BURST_SEND_START
    I2CMasterDataPut(base, reg);
    I2CMasterControl(base, I2C_MASTER_CMD_BURST_SEND_START);

    //uartprintf("!! waiting for BURST_SEND_START ack, bus busy=%d\n", I2CMasterBusy(base));

    if (xSemaphoreTake(xI2CSemaphore, pdMS_TO_TICKS(100)) != pdTRUE || errorFlag)
    {
        //uartprintf("!! BUSRT_SEND timed out errorFlag = %d, MasterErr=0x%08x\n",errorFlag, I2CMasterErr(base));
        I2C_Reset();
        return false;
    }

    // Send data bytes, finishing on the last one
    for (uint16_t i = 0; i < len; i++)
    {
        I2CMasterDataPut(base, data[i]);

        if (i == len - 1)
        {
            I2CMasterControl(base, I2C_MASTER_CMD_BURST_SEND_FINISH);
            //uartprintf("!! waiting for BUSRT_SEND_FINISH ack, bus busy=%d\n", I2CMasterBusy(base));
        }
        else
        {
            I2CMasterControl(base, I2C_MASTER_CMD_BURST_SEND_CONT);
            //uartprintf("!! waiting for BURST_SEND_CONT ack, bus busy=%d\n", I2CMasterBusy(base));
        }

        if (xSemaphoreTake(xI2CSemaphore, pdMS_TO_TICKS(100)) != pdTRUE || errorFlag)
        {
            //uartprintf("!! timed out, MasterErr=0x%08x\n", I2CMasterErr(base));
            I2C_Reset();
            return false;
        }

        //uartprintf("!! Wrote data = %02x\n", data[i]);
    }

    return true;
}

bool I2C_write_bytes_internal(uint32_t base, uint8_t addr, uint8_t *data, uint16_t len)
{
    // //uartprintf("write bytes\n");

    if (errorFlag)
    {
        I2C_Reset();
    }

    errorFlag = false;

    while (xSemaphoreTake(xI2CSemaphore, 0) == pdTRUE)
        ;

    I2CMasterSlaveAddrSet(base, addr, false);

    for (uint16_t i = 0; i < len; i++)
    {
        I2CMasterDataPut(base, data[i]);

        if (i == 0)
        {
            I2CMasterControl(base, I2C_MASTER_CMD_BURST_SEND_START);
        }
        else if (i == len - 1)
        {
            I2CMasterControl(base, I2C_MASTER_CMD_BURST_SEND_FINISH);
        }
        else
        {
            I2CMasterControl(base, I2C_MASTER_CMD_BURST_SEND_CONT);
        }

        if (xSemaphoreTake(xI2CSemaphore, pdMS_TO_TICKS(100)) != pdTRUE || errorFlag)
        {
            I2C_Reset();
            return false;
        }
    }

    return true;
}

bool I2C_read_reg_internal(uint32_t base, uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *data, uint16_t len)
{
    // //uartprintf("Read reg\n");

    if (errorFlag)
    {
        I2C_Reset();
    }

    // //uartprintf("len = %d\n", len);
    errorFlag = false;

    while (xSemaphoreTake(xI2CSemaphore, 0) == pdTRUE)
        ;

    // Set slave address (write mode)
    I2CMasterSlaveAddrSet(base, ui8Addr, false);

    // Send register address
    I2CMasterDataPut(base, ui8Reg);
    I2CMasterControl(base, I2C_MASTER_CMD_SINGLE_SEND);
    // //uartprintf("Sent reg\n");
    //uartprintf("!! waiting for SINGLE_SEND ack, bus busy=%d\n", I2CMasterBusy(base));

    if (xSemaphoreTake(xI2CSemaphore, pdMS_TO_TICKS(100)) != pdTRUE || errorFlag)
    {
        //uartprintf("!! SINGLE_SEND timed out, MasterErr=0x%08x\n", I2CMasterErr(base));
        I2C_Reset();
        return false;
    }

    // Set slave address (read mode)
    I2CMasterSlaveAddrSet(base, ui8Addr, true);

    for (uint16_t i = 0; i < len; i++)
    {
        if (errorFlag)
        {
            I2C_Reset();
        }

        if (len == 1)
        {
            // Single byte read — use dedicated single receive command
            I2CMasterControl(base, I2C_MASTER_CMD_SINGLE_RECEIVE);
            //uartprintf("!! waiting for SINGLE_RECEIVE ack, bus busy=%d\n", I2CMasterBusy(base));
        }
        else if (i == 0)
        {
            // Read MSB
            I2CMasterControl(base, I2C_MASTER_CMD_BURST_RECEIVE_START);
            //uartprintf("!! waiting for BURST_RECIEVE_START ack, bus busy=%d\n", I2CMasterBusy(base));
        }
        else if (i == len - 1)
        {
            I2CMasterControl(base, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
            //uartprintf("!! waiting for BURST_RECEIVE_FINISH ack, bus busy=%d\n", I2CMasterBusy(base));
        }
        else
        {
            // Read LSB
            I2CMasterControl(base, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
            //uartprintf("!! waiting for BURST_RECEIVE_CONT ack, bus busy=%d\n", I2CMasterBusy(base));
        }

        // //uartprintf("%d sem start\n", i + 1);
        if (xSemaphoreTake(xI2CSemaphore, pdMS_TO_TICKS(100)) != pdTRUE || errorFlag)
        {
            //uartprintf("!! timed out, MasterErr=0x%08x\n", I2CMasterErr(base));
            I2C_Reset();
            return false;
        }
        // //uartprintf("%d sem end\n", i + 1);

        data[i] = I2CMasterDataGet(base);
        //uartprintf("!! Received data = %02x\n", data[i]);
    }

    return true;
}

/*
 * Read 2-byte value from I2C register
 */
bool I2C_read_bytes_internal(uint32_t base, uint8_t ui8Addr, uint8_t *data, uint16_t len)
{
    // //uartprintf("Read bytes\n");

    if (errorFlag)
    {
        I2C_Reset();
    }

    // //uartprintf("len = %d\n", len);
    errorFlag = false;

    while (xSemaphoreTake(xI2CSemaphore, 0) == pdTRUE)
        ;

    // Set slave address (read mode)
    I2CMasterSlaveAddrSet(base, ui8Addr, true);

    for (uint16_t i = 0; i < len; i++)
    {
        if (len == 1)
        {
            // Single byte read — use dedicated single receive command
            I2CMasterControl(base, I2C_MASTER_CMD_SINGLE_RECEIVE);
        }
        else if (i == 0)
        {
            // Read MSB
            I2CMasterControl(base, I2C_MASTER_CMD_BURST_RECEIVE_START);
        }
        else if (i == len - 1)
        {
            I2CMasterControl(base, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
        }
        else
        {
            I2CMasterControl(base, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
        }

        // //uartprintf("%d sem start\n", i + 1);
        if (xSemaphoreTake(xI2CSemaphore, pdMS_TO_TICKS(100)) != pdTRUE || errorFlag)
        {
            I2C_Reset();
            return false;
        }
        // //uartprintf("%d sem end\n", i + 1);

        data[i] = I2CMasterDataGet(base);
    }

    return true;
}

bool I2C_write_single_internal(uint32_t base, uint8_t ui8Addr, uint8_t data)
{
    if (errorFlag)
    {
        I2C_Reset();
    }

    errorFlag = false;

    while (xSemaphoreTake(xI2CSemaphore, 0) == pdTRUE)
        ;

    // Set slave address (write mode)
    I2CMasterSlaveAddrSet(base, ui8Addr, false);

    // Send register address
    I2CMasterDataPut(base, data);
    I2CMasterControl(base, I2C_MASTER_CMD_SINGLE_SEND);
    //uartprintf("Sent reg\n");
    if (xSemaphoreTake(xI2CSemaphore, pdMS_TO_TICKS(100)) != pdTRUE || errorFlag)
    {
        I2C_Reset();
        return false;
    }

    return true;
}