#include "lvgl.h"
#include "../screen_manager.h"
#include "scr_motor.h"
#include <stdint.h>
#include <stdbool.h>
#include "../gui_utils.h"
static lv_obj_t *s_screen;

void scr_sensor1_get_x(float x) {};

void scr_sensor1_set_x(float x) {};

static void btn_home_cb(lv_event_t *e)
{
    (void)e;
    screen_manager_goto(SCREEN_SENSORS);
}

// Obtained from https://lvgl.io/docs/open/9.2/examples
// More specifically: https://github.com/lvgl/lvgl/blob/f718ef47d9c4f729e130b4cec00aff9f3d629335/examples/widgets/chart/lv_example_chart_8.c
static void add_data(lv_timer_t * t)
{
    lv_obj_t * chart = lv_timer_get_user_data(t);
    lv_chart_series_t * ser = lv_chart_get_series_next(chart, NULL);

    lv_chart_set_next_value(chart, ser, lv_rand(10, 90));

    uint16_t p = lv_chart_get_point_count(chart);
    uint16_t s = lv_chart_get_x_start_point(chart, ser);
    int32_t * a = lv_chart_get_y_array(chart, ser);

    a[(s + 1) % p] = LV_CHART_POINT_NONE;
    a[(s + 2) % p] = LV_CHART_POINT_NONE;
    a[(s + 2) % p] = LV_CHART_POINT_NONE;

    lv_chart_refresh(chart);
}
lv_obj_t* lv_chart(lv_obj_t *s_screen)
{
    /*Create a stacked_area_chart.obj*/
    lv_obj_t * chart = lv_chart_create(s_screen);
    lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_CIRCULAR);
    lv_obj_set_style_size(chart, 0, 0, LV_PART_INDICATOR);
    lv_obj_set_size(chart, 220, 130);
    lv_obj_center(chart);

    lv_chart_set_point_count(chart, 80);
    lv_chart_series_t * ser = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
    /*Prefill with data*/
    uint32_t i;
    for(i = 0; i < 80; i++) {
        lv_chart_set_next_value(chart, ser, lv_rand(10, 90));
    }

    lv_timer_create(add_data, 50, chart);
    return chart;

}

lv_obj_t* lv_scale(lv_obj_t *s_screen, lv_obj_t *chart)
{
    lv_obj_t *scale = lv_scale_create(s_screen);

    lv_scale_set_mode(scale, LV_SCALE_MODE_VERTICAL_LEFT);
    lv_obj_set_size(scale, 40, 160);
    lv_scale_set_range(scale, 0, 2500);
    lv_scale_set_total_tick_count(scale, 6);
    lv_scale_set_major_tick_every(scale, 1);

    lv_obj_align_to(scale, chart, LV_ALIGN_OUT_LEFT_MID, -5, 0);
    return scale;

}

void scr_sensor1_init(void)
{
    s_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(s_screen, COLOR_BACKGROUND_GREEN, LV_PART_MAIN);
    // To DO:
    // Add relevant buttons and diagnostics for motor

    lv_obj_t *label = create_label(s_screen, "Sensor 1");
    (void)label; // ignore label for now, return value is kept for possible future use

    lv_obj_t *prev_button = create_icon_button(s_screen, LV_SYMBOL_PREV, btn_home_cb, LV_ALIGN_TOP_LEFT, 8, 8);
    (void) prev_button;

    lv_obj_t *chart = lv_chart(s_screen);
    lv_obj_t *scale = lv_scale(s_screen, chart);

}

lv_obj_t *scr_sensor1_get(void)
{
    return s_screen;
};
