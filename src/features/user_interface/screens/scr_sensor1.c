#include "lvgl.h"
#include "../screen_manager.h"
#include "scr_motor.h"
#include <stdint.h>
#include <stdbool.h>
#include "../gui_utils.h"
static lv_obj_t *s_screen;

void scr_sensor1_get_x(float x) {};

void scr_sensor1_set_x(float x) {};

static void btn_home_cb(lv_event_t *e)
{
    (void)e;
    screen_manager_goto(SCREEN_SENSORS);
}

void scr_sensor1_init(void)
{
    s_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(s_screen, COLOR_BACKGROUND_GREEN, LV_PART_MAIN);
    // To DO:
    // Add relevant buttons and diagnostics for motor

    lv_obj_t *label = create_label(s_screen, "Sensor 1");
    (void)label; // ignore label for now, return value is kept for possible future use

    lv_obj_t *prev_button = create_icon_button(s_screen, LV_SYMBOL_PREV, btn_home_cb, LV_ALIGN_TOP_LEFT, 8, 8);
    (void) prev_button;
}

lv_obj_t *scr_sensor1_get(void)
{
    return s_screen;
};
