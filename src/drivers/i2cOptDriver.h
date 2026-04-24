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
extern bool writeI2C(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *Data);
extern bool readI2C(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *Data);



#endif /* _I2COPTDRIVER_H_ */
