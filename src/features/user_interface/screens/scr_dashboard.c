#include <stdint.h>
#include <stdbool.h>
#include "scr_dashboard.h"
#include "../screen_manager.h"
#include "lvgl.h"

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

// Nav Button
static lv_obj_t *prv_nav_button_init(lv_obj_t *parent, const char *label, lv_event_cb_t cb,
                                     lv_align_t align, int32_t x_ofs, int32_t y_ofs)
{
    lv_obj_t *button = lv_button_create(parent);
    lv_obj_set_size(button, 90, 40);
    lv_obj_add_event_cb(button, cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *lbl = lv_label_create(button);
    lv_label_set_text(lbl, label);
    lv_obj_center(lbl);

    return button;
}
void scr_dashboard_init(void)
{
    // To Do: Modularise colour
    lv_color_t green = {124, 218, 124};
    s_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(s_screen, green, LV_PART_MAIN);

    // Label
    lv_obj_t *title_label = lv_label_create(s_screen);
    lv_label_set_text(title_label, "Group #30");
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_30, 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 8);

    // Create a container for the navigation bar at the bottom
    lv_obj_t *nav_bar = lv_obj_create(s_screen);
    lv_obj_set_size(nav_bar, LV_HOR_RES, 50);         // Set the navigation bar's height
    lv_obj_align(nav_bar, LV_ALIGN_BOTTOM_MID, 0, 0); // Align it to the bottom of the screen

    // Create the buttons within the navigation bar, spaced evenly
    lv_obj_t *motor_btn = prv_nav_button_init(nav_bar, "Motor", btn_motor_cb, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_t *sensors_btn = prv_nav_button_init(nav_bar, "Sensors", btn_sensors_cb, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t *alerts_btn = prv_nav_button_init(nav_bar, "Alerts", btn_alert_cb, LV_ALIGN_RIGHT_MID, -10, 0);

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