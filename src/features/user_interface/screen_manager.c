// ui/screen_manager.c
#include "screen_manager.h"
#include "screens/scr_dashboard.h"
#include "screens/scr_motor.h"
#include "screens/scr_sensors.h"
#include "screens/scr_alerts.h"
#include "screens/scr_sensor1.h"
#include "screens/scr_sensor2.h"
#include "screens/scr_sensor3.h"
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
    scr_motor_init();
    scr_sensors_init();
    scr_sensor1_init();
    scr_sensor2_init();
    scr_sensor3_init();

    s_screens[SCREEN_DASHBOARD] = scr_dashboard_get();
    s_screens[SCREEN_MOTOR] = scr_motor_get();
    s_screens[SCREEN_SENSORS] = scr_sensors_get();
    s_screens[SCREEN_SENSOR1] = scr_sensor1_get();
    s_screens[SCREEN_SENSOR2] = scr_sensor2_get();
    s_screens[SCREEN_SENSOR3] = scr_sensor3_get();

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
        200,  // animation duration ms
        0,    // delay ms before starting
        false // do NOT delete the old screen after transition - BAD if done!!
    );

    s_active = id;
}

ScreenId_t screen_manager_active(void)
{
    return s_active;
}