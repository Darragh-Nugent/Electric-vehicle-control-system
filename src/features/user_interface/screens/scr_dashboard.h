#include "lvgl.h"
#include <stdint.h>


void scr_dashboard_init(void);
void scr_dashboard_show_fault_banner(bool);
void scr_dashboard_set_rpm(float);
void scr_dashboard_set_sensor(uint32_t,float);
void scr_dashboard_set_motor_state(uint32_t);
