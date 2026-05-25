#include "lvgl.h"
#include "../screen_manager.h"
#include "scr_graph.h"
#include <stdint.h>
#include <stdbool.h>
#include "../gui_utils.h"
#include "features/sensors/api/sensors_api.h"

static int32_t scaleYMin = 0;
static int32_t scaleYMax = 100;
static lv_obj_t *s_screen;
static lv_obj_t *label;
static uint16_t period;
static uint16_t prevPeriod;
static lv_obj_t *scale;
graph_t *default_graph;

static void btn_home_cb(lv_event_t *e)
{
    (void)e;
    screen_manager_goto(SCREEN_SENSORS);
}

static int32_t get_lux(void)
{
    // Sensor_GetLux();
    return (int32_t)Sensor_GetLux().value;
}

static int32_t get_power(void)
{
    // Sensor_GetPower();
    return (int32_t)Sensor_GetPower().value;
}

static int32_t get_distance(void)
{
    // Sensor_GetDistance();
    return (int32_t) Sensor_GetDistance().value;
}

static int32_t get_acceleration(void)
{
    // Sensor_GetAcceleration();
    return (int32_t) Sensor_GetAccel().value;
}


void shared_graph_timer_cb(lv_timer_t *t)
{
    graph_t *graph = (graph_t *)lv_timer_get_user_data(t);

    // Read current active screen
    ScreenId_t screen = screen_manager_active();

    // Call the appropriate sensor callback
    int32_t value = 0;
    switch (screen)
    {
    case SCREEN_DIST_SENSOR:
        value = get_distance();
        prevPeriod = period;
        period = SENSOR_DISTANCE_PERIOD;
        lv_label_set_text(label, "Distance (mm)");
        break;
    case SCREEN_ACCEL_SENSOR:
        value = get_acceleration();
        prevPeriod = period;
        period = SENSOR_ACCELERATION_PERIOD;
        lv_label_set_text(label, "Accel (m/s/s)");
        break;
    case SCREEN_PWR_SENSOR:
        value = get_power();
        prevPeriod = period;
        period = SENSOR_POWER_PERIOD;
        lv_label_set_text(label, "Power (W)");
        break;
    case SCREEN_LIGHT_SENSOR:
        value = get_lux();
        prevPeriod = period;
        period = SENSOR_LIGHT_PERIOD;
        lv_label_set_text(label, "Lux (lx)");
        break;
    default:
        return;
    }

    // Update graph
    lv_chart_series_t *ser = lv_chart_get_series_next(graph->chart, NULL);
    lv_chart_set_next_value(graph->chart, ser, value);

    // Change Y scale axis if input is out of bounds
    if (graph->scale != NULL)
    {
        if (value > graph->scaleYMax)
        {
            int32_t difference = value - graph->scaleYMax;

            // Move Y min just below the smallest value currently, and move y Max just above
            int32_t yMin = LV_MAX(0, graph->scaleYMin + difference - graph->overheadGap);
            int32_t yMax = value + graph->overheadGap;

            lv_scale_set_range(graph->scale, yMin, yMax);
            lv_chart_set_range(graph->chart, LV_CHART_AXIS_PRIMARY_Y, yMin, yMax);

            graph->scaleYMin = yMin;
            graph->scaleYMax = yMax;
        }
        else if (value < graph->scaleYMin)
        {

            int32_t difference = graph->scaleYMin - value;
            int32_t yMin = LV_MAX(0, (graph->scaleYMin - difference - graph->overheadGap));
            int32_t yMax = graph->scaleYMax - difference + graph->overheadGap;
            lv_scale_set_range(graph->scale, yMin, yMax);
            lv_chart_set_range(graph->chart, LV_CHART_AXIS_PRIMARY_Y, yMin, yMax);
            graph->scaleYMin = yMin;
            graph->scaleYMax = yMax;
        }
    }
    uint16_t p = lv_chart_get_point_count(graph->chart);
    uint16_t s = lv_chart_get_x_start_point(graph->chart, ser);
    int32_t *a = lv_chart_get_y_array(graph->chart, ser);

    a[(s + 1) % p] = LV_CHART_POINT_NONE;
    a[(s + 2) % p] = LV_CHART_POINT_NONE;
    a[(s + 3) % p] = LV_CHART_POINT_NONE;

    // Rescale X
    if (prevPeriod != period)
    {
        uint32_t total_time_s = (period * CHART_POINT_COUNT)/ 1000;
        lv_scale_set_range(scale, 0, total_time_s); // total time covered by chart
        lv_scale_set_total_tick_count(scale, total_time_s + 1);
        lv_scale_set_major_tick_every(scale, 5);
    }

    lv_chart_refresh(graph->chart);
    lv_timer_set_period(graph->timer, period);
}

void scr_graph_init(void)
{
    s_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(s_screen, COLOR_BACKGROUND_GREEN, LV_PART_MAIN);

    label = create_label(s_screen, "Graph: --");
    (void)label; // ignore label for now, return value is kept for possible future use

    lv_obj_t *prev_button = create_icon_button(s_screen, LV_SYMBOL_PREV, btn_home_cb, LV_ALIGN_TOP_LEFT, 8, 8);
    lv_obj_set_width(prev_button, 40);

    default_graph = create_graph(s_screen, scaleYMin, scaleYMax, DEFAULT_OVERHEAD_GAP, DEFAULT_PERIOD, NULL, CHART_POINT_COUNT);
    default_graph->timer = lv_timer_create(shared_graph_timer_cb, DEFAULT_PERIOD, default_graph);
    scale = create_time_scale(s_screen, default_graph->chart, period, CHART_POINT_COUNT);
}

lv_obj_t *scr_graph_sensor_get(void)
{
    return s_screen;
};
