#include "lvgl.h"
#include <stdint.h>


void scr_motor_init(void);

lv_obj_t *scr_motor_get(void);

void scr_motor_set_rpm(float rpm);

void scr_motor_set_current(float amps);

void scr_motor_set_state(uint8_t state);
