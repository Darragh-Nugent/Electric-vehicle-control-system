#include <stdint.h>

#define I2C_MAX_LEN 32

typedef struct i2c_send_message_t
{
    uint8_t id;  /* writer task id */
    uint8_t type;  /* message type: read or write */
    uint8_t sensor;  /* sensor address */
    uint8_t reg;  /* register address */
    uint16_t len;
    uint8_t data[I2C_MAX_LEN]; /* message value */
} i2c_send_message_t;

typedef struct i2c_recv_message_t
{
    uint8_t id;  /* writer task id */
    uint8_t sensor;  /* sensor address */
    bool success;
    uint16_t len;
    uint8_t data[I2C_MAX_LEN]; /* message value */
} i2c_recv_message_t;