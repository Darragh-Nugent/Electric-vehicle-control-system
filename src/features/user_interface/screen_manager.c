// ui/screen_manager.c
#include "screen_manager.h"
#include "screens/scr_dashboard.h"
#include "screens/scr_settings.h"
#include "screens/scr_motor.h"
#include "screens/scr_sensors.h"
#include "screens/scr_alerts.h"
#include "screens/scr_speed_sensor.h"
#include "screens/scr_temp_sensor.h"
#include "screens/scr_power_sensor.h"
#include "screens/scr_light_sensor.h"
#include "screens/scr_accel_sensor.h"
#include "screens/scr_dist_sensor.h"
#include "screens/scr_humidity_sensor.h"
#include "screens/scr_status.h"
#include "utils/uartstdio.h"

static lv_obj_t *s_screens[SCREEN_COUNT];
static ScreenId_t s_active = SCREEN_DASHBOARD;

// Animation direction — slide left when going "forward", right when going "back"
static lv_scr_load_anim_t prv_anim_for(ScreenId_t from, ScreenId_t to)
{
    return (to > from) ? LV_SCR_LOAD_ANIM_MOVE_LEFT
                       : LV_SCR_LOAD_ANIM_MOVE_RIGHT;
}

void screen_manager_init(void)
{
    scr_dashboard_init();
    scr_settings_init();
    scr_motor_init();
    scr_sensors_init();
    scr_speed_sensor_init();
    scr_temp_sensor_init();
    scr_pwr_sensor_init();
    scr_light_sensor_init();
    scr_accel_sensor_init();
    scr_dist_sensor_init();
    scr_humidity_sensor_init();
    scr_status_init();

    s_screens[SCREEN_DASHBOARD] = scr_dashboard_get();
    s_screens[SCREEN_SETTINGS] = scr_settings_get();
    s_screens[SCREEN_MOTOR] = scr_motor_get();
    s_screens[SCREEN_SENSORS] = scr_sensors_get();
    s_screens[SCREEN_SPEED_SENSOR] = scr_speed_sensor_get();
    s_screens[SCREEN_TEMP_SENSOR] = scr_temp_sensor_get();
    s_screens[SCREEN_PWR_SENSOR] = scr_pwr_sensor_get();
    s_screens[SCREEN_LIGHT_SENSOR] = scr_light_sensor_get();
    s_screens[SCREEN_ACCEL_SENSOR] = scr_accel_sensor_get();
    s_screens[SCREEN_DIST_SENSOR] = scr_dist_sensor_get();
    s_screens[SCREEN_HUMIDITY_SENSOR] = scr_humidity_sensor_get();
   s_screens[SCREEN_STATUS] = scr_status_get();

    // Load the default screen immediately (no animation on first load)
    lv_screen_load(s_screens[SCREEN_DASHBOARD]);
    s_active = SCREEN_DASHBOARD;
}

void screen_manager_goto(ScreenId_t id)
{
    if (id == s_active || id >= SCREEN_COUNT)
        return;

    lv_scr_load_anim_t anim = prv_anim_for(s_active, id);

    lv_screen_load_anim(
        s_screens[id],
        anim,
        350,  // animation duration ms
        0,    // delay ms before starting
        false // do NOT delete the old screen after transition - BAD if done!!
    );

    s_active = id;
}

ScreenId_t screen_manager_active(void)
{
    return s_active;
}