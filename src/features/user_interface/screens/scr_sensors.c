#include <stdint.h>
#include <stdint.h>
#include "lvgl.h"
#include "../screen_manager.h"
#include "scr_sensors.h"
#include "../gui_utils.h"

void scr_sensors_set_value(uint32_t sensor, float value) {};

static lv_obj_t *s_screen;

// To Do : modularise this
static void btn_home_cb(lv_event_t *e)
{
    screen_manager_goto(SCREEN_DASHBOARD);
}

static void btn_sensor1_cb(lv_event_t *e)
{
    screen_manager_goto(SCREEN_SENSOR1);
}

static void btn_sensor2_cb(lv_event_t *e)
{
    screen_manager_goto(SCREEN_SENSOR2);
}

static void btn_sensor3_cb(lv_event_t *e)
{
    screen_manager_goto(SCREEN_SENSOR3);
}

void scr_sensors_init(void)
{
    s_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(s_screen, COLOR_BACKGROUND_GREEN, LV_PART_MAIN);

    // To DO:
    // Add similar but for other sensors
    // Modularise this as its copy and pasted design for each sensor

    // Label
    lv_obj_t *title = create_label(s_screen, "Sensors");
    (void)title;
    
    // Nav for sensors
    // Create a container for the navigation bar at the bottom
    lv_obj_t *nav_bar = lv_obj_create(s_screen);
    lv_obj_set_size(nav_bar, LV_HOR_RES, 50);         // Set the navigation bar's height
    lv_obj_align(nav_bar, LV_ALIGN_BOTTOM_MID, 0, 0); // Align it to the bottom of the screen

    // Create the buttons within the navigation bar, spaced evenly
    lv_obj_t *motor_btn = nav_button_init(nav_bar, "Sensor 1", btn_sensor1_cb, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_t *sensors_btn = nav_button_init(nav_bar, "Sensor 2", btn_sensor2_cb, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t *alerts_btn = nav_button_init(nav_bar, "Sensor 3", btn_sensor3_cb, LV_ALIGN_RIGHT_MID, -10, 0);

    // Align the buttons horizontally within the navigation bar
    lv_obj_set_width(motor_btn, 80);
    lv_obj_set_width(sensors_btn, 80);
    lv_obj_set_width(alerts_btn, 80);

    // Add some spacing between buttons
    lv_obj_align(motor_btn, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_align(sensors_btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(alerts_btn, LV_ALIGN_RIGHT_MID, -10, 0);

    // Home
    lv_obj_t *home_button = create_icon_button(s_screen, LV_SYMBOL_HOME, btn_home_cb, LV_ALIGN_TOP_LEFT, 8, 8);
    (void) home_button;
}

lv_obj_t *scr_sensors_get(void)
{
    return s_screen;
};
