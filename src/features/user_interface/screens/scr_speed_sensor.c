#include "lvgl.h"
#include "../screen_manager.h"
#include "scr_motor.h"
#include <stdint.h>
#include <stdbool.h>
#include "../../data.h"
#include "../gui_utils.h"
#define CHART_POINT_COUNT 80
#define TIME_LABEL_WIDTH 6
static lv_obj_t *s_screen;
graph_t *speed_graph;
static int32_t scaleYMin = SENSOR_SPEED_YMIN;
static int32_t scaleYMax = SENSOR_SPEED_YMAX;
void scr_sensor1_get_x(float x) {};

void scr_sensor1_set_x(float x) {};

static void btn_home_cb(lv_event_t *e)
{
    (void)e;
    screen_manager_goto(SCREEN_SENSORS);
}

// Obtained from https://lvgl.io/docs/open/9.2/examples
// More specifically: https://github.com/lvgl/lvgl/blob/f718ef47d9c4f729e130b4cec00aff9f3d629335/examples/widgets/chart/lv_example_chart_8.c

static int32_t get_speed(void)
{
    // Sensor_GetSpeed();
    return lv_rand(0, 3000);
}

static lv_obj_t *create_time_scale(lv_obj_t *parent, lv_obj_t *chart, uint32_t period, uint16_t points)
{
    // Create the scale
    lv_obj_t *scale = lv_scale_create(parent);
    lv_obj_set_size(scale, CHART_WIDTH, 40); // width matches chart, height for labels
    lv_obj_align_to(scale,chart, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    lv_scale_set_mode(scale, LV_SCALE_MODE_HORIZONTAL_BOTTOM);
    uint32_t total_time_s = (period * points) / 1000;
    lv_scale_set_range(scale, 0, total_time_s); // total time covered by chart
    lv_scale_set_total_tick_count(scale, total_time_s + 1);
    lv_scale_set_major_tick_every(scale, 1);


    return scale;
}

void scr_speed_sensor_init(void)
{
    s_screen = lv_obj_create(NULL);
    setBackgroundColour(s_screen);
    lv_obj_remove_flag(s_screen, LV_OBJ_FLAG_SCROLLABLE);


    lv_obj_t *label = create_label(s_screen, "Speed (RPM)");
    (void)label; // ignore label for now, return value is kept for possible future use

    lv_obj_t *prev_button = create_icon_button(s_screen, LV_SYMBOL_PREV, btn_home_cb, LV_ALIGN_TOP_LEFT, 8, 8);
    lv_obj_set_width(prev_button, 40);

    speed_graph = create_graph(s_screen, scaleYMin, scaleYMax, SENSOR_SPEED_OVERHEADGAP, SENSOR_SPEED_PERIOD, get_speed, CHART_SPEED_POINT_COUNT);
    lv_obj_t *time_scale = create_time_scale(s_screen, speed_graph->chart, SENSOR_SPEED_PERIOD, CHART_SPEED_POINT_COUNT);
    (void) time_scale;
}

lv_obj_t *scr_speed_sensor_get(void)
{
    return s_screen;
};
