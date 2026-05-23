#define MODE_COUNT 8
typedef enum
{
    NONE,
    LIGHT,
    SPEED,
    POWER,
    ACCEL,
    TEMP,
    HUMIDITY,
    DIST
} uart_mode_t;