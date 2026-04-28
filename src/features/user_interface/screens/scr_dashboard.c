#include <stdint.h>
#include <stdbool.h>
#include "scr_dashboard.h"
#include "../screen_manager.h"
#include "lvgl.h"
#include "../gui_utils.h"

void scr_dashboard_show_fault_banner(bool hasFault) {};
void scr_dashboard_set_rpm(float f) {};
void scr_dashboard_set_sensor(uint32_t idx, float f) {};
void scr_dashboard_set_motor_state(uint32_t state) {};

static lv_obj_t *s_screen;

static void btn_motor_cb(lv_event_t *e)
{
    (void)e;
    screen_manager_goto(SCREEN_MOTOR);
}

// May need to change for each sensors
static void btn_sensors_cb(lv_event_t *e)
{
    (void)e;
    screen_manager_goto(SCREEN_SENSORS);
}

static void btn_alert_cb(lv_event_t *e)
{
    (void)e;
    screen_manager_goto(SCREEN_ALERT);
}


void scr_dashboard_init(void)
{
    // To Do: Modularise colour
    s_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(s_screen, COLOR_BACKGROUND_GREEN, LV_PART_MAIN);

    // Label
    lv_obj_t *label = create_label(s_screen,"Group #30");
    (void) label; // ignore label for now, return value is kept for possible future use

    // Create a container for the navigation bar at the bottom
    lv_obj_t *nav_bar = lv_obj_create(s_screen);
    lv_obj_set_size(nav_bar, LV_HOR_RES, 50);         // Set the navigation bar's height
    lv_obj_align(nav_bar, LV_ALIGN_BOTTOM_MID, 0, 0); // Align it to the bottom of the screen

    // Create the buttons within the navigation bar, spaced evenly
    lv_obj_t *motor_btn = nav_button_init(nav_bar, "Motor", btn_motor_cb, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_t *sensors_btn = nav_button_init(nav_bar, "Sensors", btn_sensors_cb, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t *alerts_btn = nav_button_init(nav_bar, "Alerts", btn_alert_cb, LV_ALIGN_RIGHT_MID, -10, 0);

    // Align the buttons horizontally within the navigation bar
    lv_obj_set_width(motor_btn, 80);
    lv_obj_set_width(sensors_btn, 80);
    lv_obj_set_width(alerts_btn, 80);

    // Add some spacing between buttons
    lv_obj_align(motor_btn, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_align(sensors_btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(alerts_btn, LV_ALIGN_RIGHT_MID, -10, 0);

    // To Do:
    // maybe remove nav for alerts, it should pop up instantly over everything
    // add in other sensors as their own seperate pages
    // add in charts/graph
}
lv_obj_t *scr_dashboard_get(void) { return s_screen; }