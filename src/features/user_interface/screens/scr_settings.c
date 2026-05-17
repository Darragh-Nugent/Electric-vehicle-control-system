#include "lvgl.h"
#include "../screen_manager.h"
#include "scr_motor.h"
#include <stdint.h>
#include <stdbool.h>
#include "../gui_utils.h"
static lv_obj_t *s_screen;
static lv_obj_t *spinbox;

static void btn_home_cb(lv_event_t *e)
{
    (void)e;
    screen_manager_goto(SCREEN_DASHBOARD);
}

static void lv_spinbox_increment_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_SHORT_CLICKED || code  == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_increment(spinbox);
    }
}

static void lv_spinbox_decrement_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_decrement(spinbox);
    }
}

static lv_obj_t *create_threshold_card(
    lv_obj_t *parent,
    const char *title,
    const char *unit)
{
    lv_obj_t *card = lv_obj_create(parent);

    lv_obj_set_size(card, 250, 120);

    lv_obj_set_style_radius(card, 12, 0);

    lv_obj_set_style_pad_all(card, 10, 0);

    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_align(card, LV_ALIGN_LEFT_MID, 8, 0);

    lv_obj_t *title_label = lv_label_create(card);

    lv_label_set_text(title_label, title);

    lv_obj_align(title_label,
        LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t *current_label =
        lv_label_create(card);

    lv_label_set_text(current_label,
        "Current: 0");

    lv_obj_align(current_label,
        LV_ALIGN_TOP_LEFT, 0, 30);

    spinbox =
        lv_spinbox_create(card);

    lv_spinbox_set_range(spinbox, 0, 2500); // should be defines
    lv_spinbox_set_digit_count(spinbox, 4);
    lv_spinbox_set_dec_point_pos(spinbox, 3);
    lv_spinbox_step_prev(spinbox);
    lv_obj_set_width(spinbox, 100);
    lv_obj_center(spinbox);
    int32_t h = lv_obj_get_height(spinbox);
    lv_obj_align(spinbox,LV_ALIGN_BOTTOM_LEFT,h,0); 


    lv_obj_t * btn = lv_button_create(s_screen);
    lv_obj_set_size(btn, h, h);
    lv_obj_align_to(btn, spinbox, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_set_style_bg_image_src(btn, LV_SYMBOL_PLUS, 0);
    lv_obj_add_event_cb(btn, lv_spinbox_increment_event_cb, LV_EVENT_ALL,  NULL);

    btn = lv_button_create(s_screen);
    lv_obj_set_size(btn, h, h);
    lv_obj_align_to(btn, spinbox, LV_ALIGN_OUT_LEFT_MID, -5, 0);
    lv_obj_set_style_bg_image_src(btn, LV_SYMBOL_MINUS, 0);
    lv_obj_add_event_cb(btn, lv_spinbox_decrement_event_cb, LV_EVENT_ALL, NULL);

    return card;
}

void scr_settings_init(void)
{
    s_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(s_screen, COLOR_BACKGROUND_GREEN, LV_PART_MAIN);
    // lv_obj_clear_flag(s_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t* power = create_threshold_card(s_screen, "Power", "P");
    lv_obj_t* accel = create_threshold_card(s_screen, "Acceleration", "m/s/s");
    lv_obj_t* dist = create_threshold_card(s_screen, "Distance", "m");
    lv_obj_align_to(accel, power, LV_ALIGN_OUT_BOTTOM_MID, 0,8);
    lv_obj_align_to(dist, accel, LV_ALIGN_OUT_BOTTOM_MID, 0,8);
    // Label

    lv_obj_t *label = create_label(s_screen, "Thresholds");
    lv_obj_align(label, LV_ALIGN_TOP_RIGHT,-30,8); // ignore label for now, return value is kept for possible future use

    lv_obj_t *home_button = create_icon_button(s_screen, LV_SYMBOL_HOME, btn_home_cb, LV_ALIGN_TOP_LEFT, 8, 8);
    (void)home_button;
}

lv_obj_t *scr_settings_get(void)
{
    return s_screen;
};
