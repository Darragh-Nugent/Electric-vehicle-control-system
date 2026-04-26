#include "lvgl.h"
#include "../screen_manager.h"
#include "scr_motor.h"
#include <stdint.h>
#include <stdbool.h>

static lv_obj_t *s_screen;

void scr_motor_set_rpm(float rpm) {};

void scr_motor_set_current(float amps) {};

void scr_motor_set_state(uint8_t state) {};

static void btn_home_cb(lv_event_t *e)
{
    (void)e;
    screen_manager_goto(SCREEN_DASHBOARD);
}

void scr_motor_init(void)
{
    s_screen = lv_obj_create(NULL);

    // To DO:
    // Add relevant buttons and diagnostics for motor 

    lv_obj_t *button = lv_button_create(s_screen);
    lv_obj_set_size(button, 80, 36);
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, 8, 8);
    lv_obj_add_event_cb(button, btn_home_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *label = lv_label_create(button);
    lv_label_set_text(label, LV_SYMBOL_HOME); // LVGL built-in home icon
    lv_obj_center(label);


}

lv_obj_t *scr_motor_get(void)
{
    return s_screen;
};
