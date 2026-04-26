#ifndef _SHT31_H_
#define _SHT31_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define SHT31_ADDR    0x44
#define SHT31_MEAS_HIGHREP_STRETCH 0x2C06
#define SHT31_MEAS_MEDREP_STRETCH  0x2C0D
#define SHT31_MEAS_LOWREP_STRETCH  0x2C10
#define SHT31_MEAS_HIGHREP         0x2400
#define SHT31_MEAS_MEDREP          0x240B
#define SHT31_MEAS_LOWREP          0x2416
#define SHT31_READSTATUS           0xF32D
#define SHT31_CLEARSTATUS          0x3041
#define SHT31_SOFTRESET            0x30A2
#define SHT31_HEATEREN             0x306D
#define SHT31_HEATERDIS            0x3066

typedef bool (*sht31_read_fptr_t)(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
typedef bool (*sht31_write_fptr_t)(uint8_t dev_addr, uint8_t reg_addr, uint8_t *read_data, uint16_t len);
typedef void (*sht31_delay_fptr_t)(uint32_t period);


typedef struct  
{
  sht31_read_fptr_t write;

  sht31_write_fptr_t read;

  sht31_delay_fptr_t delay;
} sht31_dev;

#endif
