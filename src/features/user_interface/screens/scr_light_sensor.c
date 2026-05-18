#include "lvgl.h"
#include "../screen_manager.h"
#include "scr_light_sensor.h"
#include <stdint.h>
#include <stdbool.h>
#include "../gui_utils.h"

#define OVERHEAD_GAP 200
#define PERIOD 200

static int32_t scaleYMin = 0;
static int32_t scaleYMax = 100;
static lv_obj_t *s_screen;
static graph_t *lux_graph;


static void btn_home_cb(lv_event_t *e)
{
    (void)e;
    screen_manager_goto(SCREEN_SENSORS);
}

static int32_t get_lux(void){
    // Sensor_GetLux();
    return lv_rand(0,800);
}

void scr_light_sensor_init(void)
{
    s_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(s_screen, COLOR_BACKGROUND_GREEN, LV_PART_MAIN);

    // To DO:
    // Add relevant buttons and diagnostics for motor

    // Label

    lv_obj_t *label = create_label(s_screen, "Lux");
    (void)label; // ignore label for now, return value is kept for possible future use

    lv_obj_t *prev_button = create_icon_button(s_screen, LV_SYMBOL_PREV, btn_home_cb, LV_ALIGN_TOP_LEFT, 8, 8);
    (void)prev_button;

    lux_graph = create_graph(s_screen,scaleYMin,scaleYMax,OVERHEAD_GAP,PERIOD,get_lux);
    
}

lv_obj_t *scr_light_sensor_get(void)
{
    return s_screen;
};
