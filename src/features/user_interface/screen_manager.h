#pragma once
#include "lvgl.h"

#define COLOR_BACKGROUND_GREEN lv_color_make(124, 218, 124)

typedef enum {
    SCREEN_DASHBOARD = 0,
    SCREEN_SETTINGS,
    SCREEN_MOTOR,
    SCREEN_SENSORS, // Need to add more if required or mayeb just display them all on a page?
    SCREEN_SPEED_SENSOR,
    SCREEN_PWR_SENSOR,
    SCREEN_LIGHT_SENSOR,
    SCREEN_ACCEL_SENSOR,
    SCREEN_DIST_SENSOR,
    SCREEN_TEMP_SENSOR,
    SCREEN_STATUS,
    SCREEN_ALERT,
    SCREEN_COUNT   // keep last
} ScreenId_t;

// Called once during UI init — builds all screen objects
void screen_manager_init(void);

// Navigate to any screen with a slide animation.
// safe to call from any lv_event_cb (inside LVGL task).
void screen_manager_goto(ScreenId_t id);

// Returns the currently active screen ID
ScreenId_t screen_manager_active(void);