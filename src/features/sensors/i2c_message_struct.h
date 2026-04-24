#include <stdint.h>

typedef struct i2c_message_t
{
    uint8_t id;  /* writer task id */
    uint8_t type;  /* message type: read or write */
    uint8_t sensor;  /* sensor address */
    uint8_t reg;  /* register address */
    uint16_t val; /* message value */
} i2c_message_t;