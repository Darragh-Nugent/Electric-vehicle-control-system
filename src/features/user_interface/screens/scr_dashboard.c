#include <stdint.h>
#include <stdbool.h>

void scr_dashboard_init(void){};
void scr_dashboard_show_fault_banner(bool hasFault){};
void scr_dashboard_set_rpm(float f){};
void scr_dashboard_set_sensor(float f){};
void scr_dashboard_set_motor_state(uint32_t state){};