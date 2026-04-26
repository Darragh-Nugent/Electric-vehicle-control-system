#include <stdint.h>
#include <stdbool.h>
#include "scr_dashboard.h"
#include "../screen_manager.h"
#include "lvgl.h"

void scr_dashboard_show_fault_banner(bool hasFault) {};
void scr_dashboard_set_rpm(float f) {};
void scr_dashboard_set_sensor(uint32_t idx,float f) {};
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
    s_screen = lv_obj_create(NULL);

    prv_nav_button_init(s_screen, "Motor", btn_motor_cb, LV_ALIGN_BOTTOM_LEFT, -8, -8);
    prv_nav_button_init(s_screen, "Sensors", btn_sensors_cb, LV_ALIGN_BOTTOM_LEFT, -8, -8);
    prv_nav_button_init(s_screen, "Alerts", btn_alert_cb, LV_ALIGN_BOTTOM_LEFT, -8, -8);

    // To Do:
    // maybe remove nav for alerts, it should pop up instantly over everything
    // add in other sensors as their own seperate pages
    // add in charts/graph


}

lv_obj_t *scr_dashboard_get(void){return s_screen;}