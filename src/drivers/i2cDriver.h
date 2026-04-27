/**************************************************************************************************
*  Filename:       i2cDriver.h
*  By:             Jesse Haviland
*  Created:        1 February 2019
*  Revised:        2 February 2019
*  Revision:       1.0
*
*  Description:    i2c Driver for use with the Texas Instruments OP3001 Optical Sensor
*************************************************************************************************/



#ifndef _I2COPTDRIVER_H_
#define _I2COPTDRIVER_H_



// ----------------------- Includes -----------------------
#include <stdbool.h>
#include <stdint.h>



// ----------------------- Exported prototypes -----------------------
extern bool I2C_write_reg(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *Data, uint16_t len);
bool I2C_write_bytes(uint8_t ui8Addr, uint8_t *data, uint16_t len);
extern int8_t I2C_write_BMI160(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *Data, uint16_t len);
extern bool I2C_read_reg(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *Data, uint16_t len);
bool I2C_read_bytes(uint8_t ui8Addr, uint8_t *data, uint16_t len);
extern int8_t I2C_read_BMI160(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *Data, uint16_t len);

// ----------------------- Message structs -----------------------

#define I2C_MAX_LEN 32

typedef enum
{
    I2C_REG_READ,
    I2C_REG_WRITE,
    I2C_RAW_WRITE,
    I2C_RAW_READ,
} i2c_type_t;

    typedef struct i2c_send_message_t
{
    uint8_t id;     /* writer task id */
    i2c_type_t type;    /* wether the transmission uses a register or not*/
    uint8_t sensor; /* sensor address */
    uint8_t reg;    /* register address */
    uint16_t len;
    uint8_t data[I2C_MAX_LEN]; /* message value */
} i2c_send_message_t;

typedef struct i2c_recv_message_t
{
    uint8_t id;     /* writer task id */
    uint8_t sensor; /* sensor address */
    bool success;
    uint16_t len;
    uint8_t data[I2C_MAX_LEN]; /* message value */
} i2c_recv_message_t;



#endif /* _I2COPTDRIVER_H_ */
