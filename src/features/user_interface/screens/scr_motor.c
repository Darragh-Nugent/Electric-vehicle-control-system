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

// LV_IMAGE_DECLARE(img_hand);
static lv_obj_t *s_screen;
static lv_obj_t *rpm_input;
static lv_obj_t *motor_state_label;
static lv_obj_t *led;
static lv_obj_t *needle_line;
static lv_obj_t *scale_line;
static int32_t speed = 0;
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
    else if (lv_strcmp(state, "BREAK") == 0){
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

// Obtained from https://lvgl.io/docs/open/9.2/examples
#include <math.h>

#define SCALE_RADIUS 70
#define NEEDLE_LENGTH 60

static lv_point_precise_t needle_points[2];

// Convert a value (0–100) into an angle in degrees
static float value_to_angle(int32_t value)
{
    return 135 + (270 * value) / 100; // start 135°, range 270°
}

// Update the needle points based on a value
static void update_needle_points(int32_t value)
{
    float angle = value_to_angle(value);
    float rad = angle * (M_PI / 180.0f);

    // Unsure why the inital x,y needle coords rely on scale when the parent of the needle is the screen, so position should be relative to the screen
    needle_points[0].x = SCALE_RADIUS;
    needle_points[0].y = SCALE_RADIUS;
    needle_points[1].x = SCALE_RADIUS + cos(rad)*NEEDLE_LENGTH;
    needle_points[1].y = SCALE_RADIUS + sin(rad)*NEEDLE_LENGTH;

    lv_line_set_points(needle_line, needle_points, 2);
}

// Animation callback for LVGL
static void needle_anim_cb(void * obj, int32_t value)
{
    update_needle_points(value);
    lv_obj_invalidate((lv_obj_t *)obj); // only redraw the needle
}

static void needle_update_timer_cb(lv_timer_t *timer)
{
    (void)timer;

    int8_t value = speed + lv_rand(-1,2);
    int32_t sensor_value = value;
    if (sensor_value < 0) sensor_value = 0;
    else if (sensor_value > 50 && sensor_value < 100) sensor_value -= lv_rand(-1,2);
    else if (sensor_value > 100) sensor_value = 100;
    // Animate needle from last value to sensor_value over 100 ms
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, needle_line);
    lv_anim_set_values(&a, speed, sensor_value);
    lv_anim_set_time(&a, 100);
    lv_anim_set_exec_cb(&a, needle_anim_cb);
    lv_anim_start(&a);

    speed = sensor_value;
}

// Modified and obtained from https://lvgl.io/docs/open/widgets/scale
lv_obj_t * lv_speedometer(lv_obj_t *parent)
{
    s_screen = parent;

    // Create scale background
    scale_line = lv_scale_create(s_screen);
    lv_obj_set_size(scale_line, SCALE_RADIUS*2, SCALE_RADIUS*2);
    lv_scale_set_mode(scale_line, LV_SCALE_MODE_ROUND_INNER);
    lv_obj_set_style_bg_opa(scale_line, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(scale_line, lv_palette_lighten(LV_PALETTE_GREY, 5), 0);
    lv_obj_set_style_radius(scale_line, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(scale_line, true, 0);
    lv_obj_center(scale_line);
    lv_obj_align(scale_line,LV_ALIGN_LEFT_MID,0,0);

    lv_scale_set_label_show(scale_line, true);
    lv_scale_set_total_tick_count(scale_line, 21);   
    lv_scale_set_major_tick_every(scale_line, 2);   
    lv_obj_set_style_length(scale_line, 5, LV_PART_ITEMS);    
    lv_obj_set_style_length(scale_line, 10, LV_PART_INDICATOR);
    lv_scale_set_range(scale_line, 0, 100);
    lv_scale_set_angle_range(scale_line, 270);
    lv_scale_set_rotation(scale_line, 135);

    needle_line = lv_line_create(s_screen);
    lv_obj_set_size(needle_line, SCALE_RADIUS*2, SCALE_RADIUS*2);
    lv_obj_center(needle_line);
    lv_obj_align(needle_line,LV_ALIGN_LEFT_MID,0,0);

    lv_obj_set_style_line_width(needle_line, 4, LV_PART_MAIN);
    lv_obj_set_style_line_color(needle_line, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
    lv_obj_set_style_line_rounded(needle_line, true, LV_PART_MAIN);

    // Initialize needle at 0
    update_needle_points(speed);

    return scale_line;
}

void motor_state_update_cb(lv_timer_t *timer){
    (void) timer;
     const char* motor_state_names[] = {
        "Idle",
        "Starting",
        "Running",
        "Breaking",
        "Fault"
    };    
    static motor_state_t prev = MOTOR_STATE_IDLE;
    motor_state_t cur =  motorGetState(); //MOTOR_STATE_RUNNING;//
    if (prev != cur)
    {
        // Update state label
        lv_label_set_text(motor_state_label, motor_state_names[cur]);
        lv_obj_invalidate(motor_state_label);
        prev = cur;

        // Update LED
        switch(cur){
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
        lv_obj_align_to(led,motor_state_label, LV_ALIGN_LEFT_MID, -30, 0);
        
    }
}

void scr_motor_init(void)
{
    s_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(s_screen, COLOR_BACKGROUND_GREEN, LV_PART_MAIN);

    lv_speedometer(s_screen);
    if (scale_line) lv_timer_create(needle_update_timer_cb, 50, scale_line);
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
    lv_obj_align(motor_state_label,LV_ALIGN_RIGHT_MID,0,0);    
    lv_timer_create(motor_state_update_cb, 200, scale_line);

    led  = lv_led_create(s_screen);
    lv_obj_align_to(led,motor_state_label, LV_ALIGN_LEFT_MID, -30, 0);
    lv_led_set_color(led, lv_palette_main(LV_PALETTE_ORANGE));
    lv_led_on(led);


}

lv_obj_t *scr_motor_get(void)
{
    return s_screen;
};
