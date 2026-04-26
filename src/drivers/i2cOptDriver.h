/**************************************************************************************************
*  Filename:       i2cOptDriver.h
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
extern int8_t I2C_write_BMI160(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *Data, uint16_t len);
extern bool I2C_read_reg(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *Data, uint16_t len);
extern int8_t I2C_read_BMI160(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *Data, uint16_t len);



#endif /* _I2COPTDRIVER_H_ */
