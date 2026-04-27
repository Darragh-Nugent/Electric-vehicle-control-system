#pragma once
#include "lvgl.h"

typedef enum {
    SCREEN_DASHBOARD = 0,
    SCREEN_MOTOR,
    SCREEN_SENSORS, // Need to add more if required or mayeb just display them all on a page?
    SCREEN_SENSOR1,
    SCREEN_SENSOR2,
    SCREEN_SENSOR3,
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