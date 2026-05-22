
#include "lvgl.h"
#include "../screen_manager.h"
#include "scr_status.h"
#include "../gui_utils.h"
#include "./features/motor/states.h"
#include "./features/motor/motor_api.h"
// #include "../sensors.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint16_t value;
    uint32_t seq;
} sensor_sample_t;

void addRotation(void);
void Sensor_Init(void);

sensor_sample_t getFaultOrWarning(void) { return (sensor_sample_t){.value = 400, .seq = 1}; };
extern sensorThresholds_t g_thresholds;

static lv_obj_t *s_screen;

/* Value labels */
static lv_obj_t *lbl_state;
static lv_obj_t *lbl_fault;
static lv_obj_t *lbl_dist_thresh;
static lv_obj_t *lbl_power_thresh;
static lv_obj_t *lbl_accel_thresh;

static void status_cb(lv_timer_t *t)
{
    (void)t;

    motor_state_t state = motorGetState();
    sensor_sample_t fault = getFaultOrWarning();

    lv_label_set_text(lbl_state, motor_state_names[state]);
    lv_label_set_text_fmt(lbl_fault, "%s: %u", "test", fault.value);
    lv_label_set_text_fmt(lbl_dist_thresh, "Min Distance: %u mm", g_thresholds.TH_DIST);
    lv_label_set_text_fmt(lbl_power_thresh, "Max Power: %u W", g_thresholds.TH_POWER);
    lv_label_set_text_fmt(lbl_accel_thresh, "Max Acceleration: %u m/s^2", g_thresholds.TH_ACCEL);
}

/* --------------------------------------------------------- */
/* Button Handler                                            */
/* --------------------------------------------------------- */

static void btn_home_cb(lv_event_t *e)
{
    (void)e;
    screen_manager_goto(SCREEN_DASHBOARD);
}

/* --------------------------------------------------------- */
/* Screen Init                                               */
/* --------------------------------------------------------- */

void scr_status_init(void)
{
    s_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(s_screen, LV_OBJ_FLAG_SCROLLABLE);

    /* Background */
    setBackgroundColour(s_screen);

    /* Title */
    lv_obj_t *title = lv_label_create(s_screen);
    lv_label_set_text(title, "System Status");

    lv_obj_set_style_text_font(title, &lv_font_montserrat_22, 0);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 12);

    /* Cards */
    create_card(s_screen, "Motor State", 10, 50,
                lv_palette_main(LV_PALETTE_RED), &lbl_state);

    create_card(s_screen, "Warning/Fault", 170, 50,
                lv_palette_main(LV_PALETTE_BLUE), &lbl_fault);

    lv_obj_t *safetyCond = create_card(s_screen, "Active Safety Conditions", 90, 155,
                                       lv_palette_main(LV_PALETTE_YELLOW), NULL);

    lv_obj_align(safetyCond, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_size(safetyCond, 300, 100);

    lbl_dist_thresh = lv_label_create(safetyCond);
    lv_obj_set_style_text_color(lbl_dist_thresh, lv_color_white(), 0);
    lv_obj_align(lbl_dist_thresh, LV_ALIGN_TOP_LEFT, 10, 20);

    lbl_power_thresh = lv_label_create(safetyCond);
    lv_obj_set_style_text_color(lbl_power_thresh, lv_color_white(), 0);
    lv_obj_align(lbl_power_thresh, LV_ALIGN_TOP_LEFT, 10, 40);

    lbl_accel_thresh = lv_label_create(safetyCond);
    lv_obj_set_style_text_color(lbl_accel_thresh, lv_color_white(), 0);
    lv_obj_align(lbl_accel_thresh, LV_ALIGN_TOP_LEFT, 10, 60);
    /* Home Button */
    lv_obj_t *home_button =
        create_icon_button(s_screen,
                           LV_SYMBOL_HOME,
                           btn_home_cb,
                           LV_ALIGN_TOP_LEFT,
                           8, 8);
    lv_obj_set_width(home_button, 40);
    /* Update Timer */
    lv_timer_create(status_cb, 250, NULL);
    status_cb(NULL);
}

lv_obj_t *scr_status_get(void)
{
    return s_screen;
}