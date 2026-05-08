#include "lvgl.h"
#include "../screen_manager.h"
#include "scr_motor.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "utils/uartstdio.h"
#include "../gui_utils.h"
#include "../../data.h"
#include "features/motor/motor_api.h"
#include "features/motor/states.h"

// LV_IMAGE_DECLARE(img_hand);
static lv_obj_t *s_screen;
static lv_obj_t *rpm_input;

lv_obj_t *needle_line;
lv_obj_t *needle_img;
int32_t speed = 0;
void scr_motor_set_rpm(float rpm) {};

void scr_motor_set_current(float amps) {};

void scr_motor_set_state(uint8_t state) {};

static void btn_home_cb(lv_event_t *e)
{
    (void)e;
    screen_manager_goto(SCREEN_DASHBOARD);
}

// Submit button cb
static void submit_rpm_cb(lv_event_t *e)
{
    const char *text = lv_textarea_get_text(rpm_input);

    int rpm = atoi(text);
    UARTprintf("RPM: %i\n", rpm);
    LV_LOG_USER("RPM set: %d", rpm);

    bool res = ui_push_f(UI_MSG_MOTOR_RPM, (float)rpm);
    if (!res)
    {
        UARTprintf("Hmm, I'll see if i remember to fix this later");
    }
}

// Text Area cb
static void ta_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target_obj(e);
    lv_obj_t *kb = (lv_obj_t *)lv_event_get_user_data(e);
    if (code == LV_EVENT_FOCUSED)
    {
        lv_keyboard_set_textarea(kb, ta);
        lv_obj_remove_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }

    if (code == LV_EVENT_DEFOCUSED)
    {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
}

// Dropdown Button cb
static void on_state_changed(const char *state)
{
    LV_LOG_USER("Motor state: %s", state);
    bool res = false;
    if (lv_strcmp(state, "IDLE") == 0)
    {
        UARTprintf("IDLE\n");
        res = ui_push_u(UI_MSG_MOTOR_IDLE,MOTOR_STATE_IDLE);
    }
    else if (lv_strcmp(state, "STARTING") == 0)
    {
        UARTprintf("STARTING\n");
        res = ui_push_u(UI_MSG_MOTOR_STARTING,MOTOR_STATE_STARTING);
    }
    else if (lv_strcmp(state, "EXPLODE") == 0)
    {
        UARTprintf("KABOOOM\n");
    }

    if (!res)
    {
        UARTprintf("Let's hope this is never printed");
    }
}

// Obtained from https://lvgl.io/docs/open/9.2/examples
static void set_needle_line_value(lv_timer_t *t)
{
    lv_obj_t *scale = lv_timer_get_user_data(t);

    int32_t value = lv_rand(-1, 2);
    speed += value;
    if (speed < 0)
        speed = 0;
    if (speed > 40)
        speed = 40;
    // if(xQueueReceive(tempQueue, &temp, 0) == pdPASS)
    // {
    //     lv_bar_set_value(bar, temp, LV_ANIM_ON);
    // }
    // OR a api call that handles queues internally

    lv_scale_set_line_needle_value(scale, needle_line, 60, speed);
}

lv_obj_t * lv_speedometer(lv_obj_t *s_screen)
{
    lv_obj_t *scale_line = lv_scale_create(s_screen);

    lv_obj_set_size(scale_line, 150, 150);
    lv_scale_set_mode(scale_line, LV_SCALE_MODE_ROUND_INNER);
    lv_obj_set_style_bg_opa(scale_line, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(scale_line, lv_palette_lighten(LV_PALETTE_GREY, 5), 0);
    lv_obj_set_style_radius(scale_line, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(scale_line, true, 0);
    lv_obj_align(scale_line, LV_ALIGN_CENTER, LV_PCT(2), 0);

    lv_scale_set_label_show(scale_line, true);

    lv_scale_set_total_tick_count(scale_line, 41);
    lv_scale_set_major_tick_every(scale_line, 5);

    lv_obj_set_style_length(scale_line, 5, LV_PART_ITEMS);
    lv_obj_set_style_length(scale_line, 10, LV_PART_INDICATOR);
    lv_scale_set_range(scale_line, 0, 40);

    lv_scale_set_angle_range(scale_line, 270);
    lv_scale_set_rotation(scale_line, 135);

    needle_line = lv_line_create(scale_line);
    lv_obj_set_style_line_width(needle_line, 6, LV_PART_MAIN);
    lv_obj_set_style_line_rounded(needle_line, true, LV_PART_MAIN);

    lv_timer_create(set_needle_line_value, 50, scale_line);

    return scale_line;

}

void scr_motor_init(void)
{
    s_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(s_screen, COLOR_BACKGROUND_GREEN, LV_PART_MAIN);

    lv_obj_t * scale_line = lv_speedometer(s_screen);
    // To DO:
    // Add relevant buttons and diagnostics for motor

    lv_obj_t *home_button = create_icon_button(s_screen, LV_SYMBOL_HOME, btn_home_cb, LV_ALIGN_TOP_LEFT, 8, 8);
    (void)home_button;
    lv_obj_t *label = create_label(s_screen, "Motor");
    (void)label; // ignore label for now, return value is kept for possible future use

    // NAV bar
    lv_obj_t *nav_bar = lv_obj_create(s_screen);
    lv_obj_set_size(nav_bar, LV_HOR_RES, 50);         // Set the navigation bar's height
    lv_obj_align(nav_bar, LV_ALIGN_BOTTOM_MID, 0, 0); // Align it to the bottom of the screen

    lv_obj_set_flex_flow(nav_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(nav_bar,
                          LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);

    // Keyboard
    lv_obj_t *kb = ui_create_numeric_keyboard(s_screen);

    // RPM - Text Area
    rpm_input = lv_textarea_create(nav_bar);
    lv_textarea_set_placeholder_text(rpm_input, "RPM");
    lv_obj_add_event_cb(rpm_input, ta_event_cb, LV_EVENT_ALL, kb);
    lv_obj_set_size(rpm_input, 80, 30);
    lv_obj_align(rpm_input, LV_ALIGN_LEFT_MID, 10, 0);

    // Submit button
    lv_obj_t *btn = lv_button_create(nav_bar);
    lv_obj_add_event_cb(btn, submit_rpm_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_size(btn, 40, 15);
    lv_label_set_text(btn, "SET");

    // Drop Down
    lv_obj_t *mode_dd = create_dropdown(
        nav_bar,
        "IDLE\nSTARTING\nEXPLODE",
        on_state_changed);

    lv_obj_set_size(mode_dd, 120, 30);
    lv_obj_align(mode_dd, LV_ALIGN_RIGHT_MID, -10, 0);
}

lv_obj_t *scr_motor_get(void)
{
    return s_screen;
};
