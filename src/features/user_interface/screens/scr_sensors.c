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

static void btn_speed_sensor_cb(lv_event_t *e)
{
    screen_manager_goto(SCREEN_SPEED_SENSOR);
}

static void btn_temp_sensor_cb(lv_event_t *e)
{
    screen_manager_goto(SCREEN_TEMP_SENSOR);
}

static void btn_light_sensor_cb(lv_event_t *e)
{
    screen_manager_goto(SCREEN_LIGHT_SENSOR);
}

static void btn_pwr_sensor_cb(lv_event_t *e)
{
    screen_manager_goto(SCREEN_PWR_SENSOR);
}

static void btn_accel_sensor_cb(lv_event_t *e)
{
    screen_manager_goto(SCREEN_ACCEL_SENSOR);
}

static void btn_dist_sensor_cb(lv_event_t *e)
{
    screen_manager_goto(SCREEN_DIST_SENSOR);
}

static void btn_humidity_sensor_cb(lv_event_t *e){
    screen_manager_goto(SCREEN_HUMIDITY_SENSOR);
}



void scr_sensors_init(void)
{
    s_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(s_screen, LV_OBJ_FLAG_SCROLLABLE);
    setBackgroundColour(s_screen);

    // To DO:
    // Add similar but for other sensors
    // Modularise this as its copy and pasted design for each sensor

    // Label
    lv_obj_t *title = create_label(s_screen, "Sensors");
    (void)title;
    
    // Nav for sensors
    // Create a container for the navigation bar at the bottom
    lv_obj_t *nav_bar = lv_obj_create(s_screen);
    lv_obj_set_size(nav_bar, LV_HOR_RES, 180);         // Set the navigation bar's height
    lv_obj_align(nav_bar, LV_ALIGN_TOP_MID, 0, 50); // Align it to the bottom of the screen

    // Create the buttons within the navigation bar, spaced evenly
    lv_obj_t *speed_btn = nav_button_init(nav_bar, "Speed", btn_speed_sensor_cb, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_t *power_btn = nav_button_init(nav_bar, "Power", btn_pwr_sensor_cb, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t *lux_btn = nav_button_init(nav_bar, "Light", btn_light_sensor_cb, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_t *accel_btn = nav_button_init(nav_bar, "Accel", btn_accel_sensor_cb, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_t *distance_btn = nav_button_init(nav_bar, "Distance", btn_dist_sensor_cb, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_t *temp_btn = nav_button_init(nav_bar, "Temp", btn_temp_sensor_cb, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_t *humidity_btn = nav_button_init(nav_bar, "Humidty", btn_humidity_sensor_cb, LV_ALIGN_RIGHT_MID,0,0);


    // Align the buttons horizontally within the navigation bar
    lv_obj_set_width(speed_btn, 80);
    lv_obj_set_width(power_btn, 80);
    lv_obj_set_width(lux_btn, 80);
    lv_obj_set_width(accel_btn, 80);
    lv_obj_set_width(distance_btn, 80);
    lv_obj_set_width(temp_btn,80);
    lv_obj_set_width(humidity_btn,80);

    // Add some spacing between buttons
    lv_obj_set_flex_flow(nav_bar, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(nav_bar, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);


    // Home
    lv_obj_t *home_button = create_icon_button(s_screen, LV_SYMBOL_HOME, btn_home_cb, LV_ALIGN_TOP_LEFT, 8, 8);
    lv_obj_set_width(home_button,40);
}

lv_obj_t *scr_sensors_get(void)
{
    return s_screen;
};
