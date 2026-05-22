#include "lvgl.h"
#include "../screen_manager.h"
#include "scr_motor.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "utils/uartstdio.h"
#include "../gui_utils.h"
#include "../../data.h"
#include "features/motor/states.h"
#include "features/motor/motor_api.h"
#include <math.h>


// LV_IMAGE_DECLARE(img_hand);
static lv_obj_t *s_screen;
static lv_obj_t *rpm_input;
static lv_obj_t *motor_state_label;
static lv_obj_t *led;
static roundScale_t *speedometer;
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

    uint16_t rpm = atoi(text);
    UARTprintf("RPM: %i\n", rpm);
    LV_LOG_USER("RPM set: %d", rpm);

    bool res = ui_push_u(UI_MSG_MOTOR_RPM, rpm);
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

// Dropdown Button cb     "IDLE\nRUN\nBREAK\nEXPLODE",
static void on_state_changed(const char *state)
{
    bool res = false;
    if (lv_strcmp(state, "IDLE") == 0)
    {
        UARTprintf("IDLE\n");
        res = ui_push_u(UI_MSG_MOTOR_IDLE, MOTOR_STATE_IDLE);
    }
    else if (lv_strcmp(state, "RUN") == 0)
    {
        UARTprintf("RUNNING\n");
        res = ui_push_u(UI_MSG_MOTOR_RUNNING, MOTOR_STATE_RUNNING);
    }
    else if (lv_strcmp(state, "BREAK") == 0)
    {
        UARTprintf("BREAK\n");
        res = ui_push_u(UI_MSG_MOTOR_BREAKING, MOTOR_STATE_BRAKING);
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

void motor_state_update_cb(lv_timer_t *timer)
{
    (void)timer;
    static motor_state_t prev = MOTOR_STATE_IDLE;
    motor_state_t cur = motorGetState(); // MOTOR_STATE_RUNNING;//
    if (prev != cur)
    {
        // Update state label
        lv_label_set_text(motor_state_label, motor_state_names[cur]);
        lv_obj_invalidate(motor_state_label);
        prev = cur;

        // Update LED
        switch (cur)
        {
        case MOTOR_STATE_IDLE:
            lv_led_set_color(led, lv_palette_main(LV_PALETTE_ORANGE));
            UARTprintf("MOTOR_STATE_IDLE\n");
            break;
        case MOTOR_STATE_STARTING:
            lv_led_set_color(led, lv_palette_main(LV_PALETTE_ORANGE));
            UARTprintf("MOTOR_STATE_STARTING\n");
            break;
        case MOTOR_STATE_RUNNING:
            lv_led_set_color(led, lv_palette_main(LV_PALETTE_LIGHT_GREEN));
            UARTprintf("MOTOR_STATE_RUNNING\n");
            break;
        case MOTOR_STATE_BRAKING: // doesnt mention in slides
            lv_led_set_color(led, lv_palette_main(LV_PALETTE_ORANGE));
            UARTprintf("MOTOR_STATE_BRAKING\n");
            break;
        case MOTOR_STATE_FAULT: // does this also include estop
            lv_led_set_color(led, lv_palette_main(LV_PALETTE_RED));
            UARTprintf("MOTOR_STATE_FAULT\n");
            break;
        }
        lv_obj_align_to(led, motor_state_label, LV_ALIGN_LEFT_MID, -30, 0);
    }
}

static int32_t get_speed(void)
{
    // Sensor_GetSpeed or something
    return (int32_t)lv_rand(-1, 2);
}

void scr_motor_init(void)
{
    s_screen = lv_obj_create(NULL);
    setBackgroundColour(s_screen);
    lv_obj_remove_flag(s_screen, LV_OBJ_FLAG_SCROLLABLE);
    speedometer = create_speedometer(s_screen, get_speed, SPEEDO_SCALE_RADIUS, SPEEDO_NEEDLE_LENGTH, 0, SPEEDO_PERIOD);
    // To DO:
    // Add relevant buttons and diagnostics for motor

    lv_obj_t *home_button = create_icon_button(s_screen, LV_SYMBOL_HOME, btn_home_cb, LV_ALIGN_TOP_LEFT, 8, 8);
    lv_obj_t *label = create_label(s_screen, "Speedometer");
    lv_obj_set_width(home_button, 40);
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

    lv_obj_remove_flag(nav_bar, LV_OBJ_FLAG_SCROLLABLE);
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
    lv_obj_t *btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "SET");
    lv_obj_center(btn_label);

    UARTprintf("scr_motor Drop down\n");
    // Drop Down
    lv_obj_t *mode_dd = create_dropdown(
        nav_bar,
        "IDLE\nRUN\nBREAK\nEXPLODE",
        on_state_changed);

    lv_obj_set_size(mode_dd, 120, 30);
    lv_obj_align(mode_dd, LV_ALIGN_RIGHT_MID, -10, 0);
    motor_state_label = lv_label_create(s_screen);
    lv_label_set_text(motor_state_label, "IDLE");
    lv_obj_align(motor_state_label, LV_ALIGN_RIGHT_MID, -25, 0);
    lv_timer_create(motor_state_update_cb, 200, NULL);

    led = lv_led_create(s_screen);
    lv_obj_align_to(led, motor_state_label, LV_ALIGN_LEFT_MID, -30, 0);
    lv_led_set_color(led, lv_palette_main(LV_PALETTE_ORANGE));
    lv_led_on(led);
}

lv_obj_t *scr_motor_get(void)
{
    return s_screen;
};
