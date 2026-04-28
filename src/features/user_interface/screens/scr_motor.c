#include "lvgl.h"
#include "../screen_manager.h"
#include "scr_motor.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "utils/uartstdio.h"
#include "../gui_utils.h"

static lv_obj_t *s_screen;
static lv_obj_t *rpm_input;
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

    if (lv_strcmp(state, "IDLE") == 0)
    {
        UARTprintf("IDLE\n");
    }
    else if (lv_strcmp(state, "RUNNING") == 0)
    {
        UARTprintf("RUNNING\n");
    }
    else if (lv_strcmp(state, "EXPLODE") == 0)
    {
        UARTprintf("KABOOOM\n");
    }
}

void scr_motor_init(void)
{
    s_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(s_screen, COLOR_BACKGROUND_GREEN, LV_PART_MAIN);

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

    // Drop Down
    lv_obj_t *mode_dd = create_dropdown(
        nav_bar,
        "IDLE\nRUNNING\nEXPLODE",
        on_state_changed);

    lv_obj_set_size(mode_dd, 120, 30);
    lv_obj_align(mode_dd, LV_ALIGN_RIGHT_MID, -10, 0);
}

lv_obj_t *scr_motor_get(void)
{
    return s_screen;
};
