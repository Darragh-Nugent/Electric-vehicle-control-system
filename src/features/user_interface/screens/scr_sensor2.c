#include "lvgl.h"
#include "../screen_manager.h"
#include "scr_motor.h"
#include <stdint.h>
#include <stdbool.h>

static lv_obj_t *s_screen;

void scr_sensor2_get_x(float x) {};

void scr_sensor2_set_x(float x) {};

static void btn_home_cb(lv_event_t *e)
{
    (void)e;
    screen_manager_goto(SCREEN_SENSORS);
}

void scr_sensor2_init(void)
{
    lv_color_t green = {124, 218, 124};
    s_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(s_screen, green, LV_PART_MAIN);

    // To DO:
    // Add relevant buttons and diagnostics for motor

    // Label
    lv_obj_t *title_label = lv_label_create(s_screen);
    lv_label_set_text(title_label, "Sensor 2");
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 8);

    lv_obj_t *button = lv_button_create(s_screen);
    lv_obj_set_size(button, 80, 36);
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, 8, 8);
    lv_obj_add_event_cb(button, btn_home_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *label = lv_label_create(button);
    lv_label_set_text(label, LV_SYMBOL_PREV); // LVGL built-in home icon
    lv_obj_center(label);
}

lv_obj_t *scr_sensor2_get(void)
{
    return s_screen;
};
