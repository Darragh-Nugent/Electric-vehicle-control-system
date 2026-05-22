#include "lvgl.h"
#include "../screen_manager.h"
#include "scr_accel_sensor.h"
#include <stdint.h>
#include <stdbool.h>
#include "../gui_utils.h"

#define OVERHEAD_GAP 5
#define PERIOD 50

static int32_t scaleYMin = 0;
static int32_t scaleYMax = 100;
static lv_obj_t *s_screen;
graph_t *accel_graph;


static void btn_home_cb(lv_event_t *e)
{
    (void)e;
    screen_manager_goto(SCREEN_SENSORS);
}

static int32_t get_acceleration(void){
    // Sensor_GetAcceleration();
    return lv_rand(0,100);
}

void scr_accel_sensor_init(void)
{
    s_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(s_screen, COLOR_BACKGROUND_GREEN, LV_PART_MAIN);

    // To DO:
    // Add relevant buttons and diagnostics for motor

    // Label

    lv_obj_t *label = create_label(s_screen, "ACCEL (m/s/s)");
    (void)label; // ignore label for now, return value is kept for possible future use

    lv_obj_t *prev_button = create_icon_button(s_screen, LV_SYMBOL_PREV, btn_home_cb, LV_ALIGN_TOP_LEFT, 8, 8);
    lv_obj_set_width(prev_button,40);

    accel_graph = create_graph(s_screen,scaleYMin,scaleYMax,OVERHEAD_GAP,PERIOD,get_acceleration);
    
}

lv_obj_t *scr_accel_sensor_get(void)
{
    return s_screen;
};
