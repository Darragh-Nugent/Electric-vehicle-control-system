#include "lvgl.h"
#include "../screen_manager.h"
#include "scr_motor.h"
#include <stdint.h>
#include <stdbool.h>
#include "../gui_utils.h"

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
    lv_obj_set_style_bg_color(s_screen, COLOR_BACKGROUND_GREEN, LV_PART_MAIN);

    // To DO:
    // Add relevant buttons and diagnostics for motor

    lv_obj_t *home_button = create_icon_button(s_screen,LV_SYMBOL_HOME, btn_home_cb, LV_ALIGN_TOP_LEFT, 8,8);
    (void) home_button;
    lv_obj_t *label = create_label(s_screen, "Motor");
    (void)label; // ignore label for now, return value is kept for possible future use
}

lv_obj_t *scr_motor_get(void)
{
    return s_screen;
};
