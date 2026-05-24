#include "lvgl.h"
#include "../screen_manager.h"
#include "scr_humidity_sensor.h"
#include <stdint.h>
#include <stdbool.h>
#include "../gui_utils.h"

static lv_obj_t *s_screen;
roundScale_t *humidity_graph;

static void btn_home_cb(lv_event_t *e)
{
    (void)e;
    screen_manager_goto(SCREEN_SENSORS);
}

static int32_t get_humidity(void)
{
    // Sensor_GetHumidity();
    return lv_rand(-1, 2);
}

void scr_humidity_sensor_init(void)
{
    s_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(s_screen, COLOR_BACKGROUND_GREEN, LV_PART_MAIN);

    // To DO:
    // Add relevant buttons and diagnostics for motor

    // Label

    lv_obj_t *label = create_label(s_screen, "HUMIDITY (%)");
    (void)label; // ignore label for now, return value is kept for possible future use

    lv_obj_t *prev_button = create_icon_button(s_screen, LV_SYMBOL_PREV, btn_home_cb, LV_ALIGN_TOP_LEFT, 8, 8);
    lv_obj_set_width(prev_button, 40);

    humidity_graph = create_speedometer(s_screen, get_humidity, SENSOR_HUMIDITY_SCALE_RADIUS,
                                        SENSOR_HUMIDITY_NEEDLE_LENGTH,
                                        0,
                                        SENSOR_HUMIDITY_PERIOD,
                                        100, 21, 2);
    lv_obj_align(humidity_graph->scale, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_align(humidity_graph->needle, LV_ALIGN_BOTTOM_MID, 0, 0);
}

lv_obj_t *scr_humidity_sensor_get(void)
{
    return s_screen;
};
