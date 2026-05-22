#include "lvgl.h"
#include "math.h"
#include "../data.h"
#include "./screen_manager.h"
#include "utils/uartstdio.h"
#include "gui_utils.h"

typedef void (*dropdown_cb_t)(const char *text);

void setBackgroundColour(lv_obj_t *parent)
{
    lv_obj_set_style_bg_color(
        parent,
        COLOR_BACKGROUND_GREEN,
        LV_PART_MAIN);

    lv_obj_set_style_bg_grad_color(
        parent,
        lv_palette_main(LV_PALETTE_LIME),
        LV_PART_MAIN);

    lv_obj_set_style_bg_grad_dir(
        parent,
        LV_GRAD_DIR_VER,
        LV_PART_MAIN);

    lv_obj_set_style_bg_opa(
        parent,
        LV_OPA_COVER,
        LV_PART_MAIN);
}

// Nav Button
lv_obj_t *nav_button_init(lv_obj_t *parent, const char *label, lv_event_cb_t cb,
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

// Label
lv_obj_t *create_label(lv_obj_t *parent, const char *label)
{
    lv_obj_t *title_label = lv_label_create(parent);
    lv_label_set_text(title_label, label);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_30, 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 8);

    return title_label;
}

lv_obj_t *create_icon_button(lv_obj_t *parent, const char *icon, lv_event_cb_t cb, lv_align_t align, int32_t x, int32_t y)
{

    lv_obj_t *button = lv_button_create(parent);
    lv_obj_set_size(button, 80, 36);
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, x, y);
    lv_obj_add_event_cb(button, cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *home = lv_label_create(button);
    lv_label_set_text(home, icon); // LVGL built-in icon
    lv_obj_center(home);

    return button;
}

static void dropdown_event_cb(lv_event_t *e)
{
    // We only care about when the drop down value has changed, otherwise we ignore it
    if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED)
        return;

    lv_obj_t *obj = lv_event_get_target(e);

    // Retrieve the user data (user_data = user_cb)
    // Treat this void pointer as a pointer to a function that takes a const char* and returns void
    dropdown_cb_t cb = (dropdown_cb_t)lv_event_get_user_data(e);

    // If no cb provided do nothing (e.g the function pointer when creating the dd)
    if (!cb)
        return;

    char buf[32];
    lv_dropdown_get_selected_str(obj, buf, sizeof(buf));

    // Calls the user provided cb with the selected item
    cb(buf);
}

lv_obj_t *create_dropdown(lv_obj_t *parent, const char *options, void (*on_change)(const char *))
{
    lv_obj_t *dd = lv_dropdown_create(parent);

    lv_dropdown_set_options(dd, options);

    lv_obj_add_event_cb(dd, dropdown_event_cb, LV_EVENT_VALUE_CHANGED, on_change);

    return dd;
}

static void kb_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    lv_obj_t *ta = lv_event_get_target(e);
    lv_obj_t *kb = (lv_obj_t *)lv_event_get_user_data(e);

    if (code == LV_EVENT_FOCUSED)
    {
        lv_keyboard_set_textarea(kb, ta);
        lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }

    if (code == LV_EVENT_DEFOCUSED)
    {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }

    if (code == LV_EVENT_CANCEL)
    {
        if (kb)
        {
            lv_keyboard_set_textarea(kb, NULL);
            lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

lv_obj_t *ui_create_numeric_keyboard(lv_obj_t *parent)
{
    lv_obj_t *kb = lv_keyboard_create(parent);

    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUMBER);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);

    return kb;
}

void ui_attach_keyboard(lv_obj_t *text_area, lv_obj_t *keyboard)
{
    lv_obj_add_event_cb(keyboard, kb_event_cb, LV_EVENT_FOCUSED, keyboard);
}

/**
 * This Section covers the graph creation for the speed, power, light, acceleration and distance sensors
 */

// Callback function to add the graph
void add_graph_data_cb(lv_timer_t *t)
{
    graph_t *graph = (graph_t *)lv_timer_get_user_data(t);
    lv_chart_series_t *ser = lv_chart_get_series_next(graph->chart, NULL);
    int32_t value = graph->get_value_cb();
    // queue receive with 0 delay
    lv_chart_set_next_value(graph->chart, ser, value);

    // Change scale axis if input is out of bounds
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

    lv_chart_refresh(graph->chart);
}

// Create the chart
static lv_obj_t *create_chart(lv_obj_t *s_screen, int32_t yMin, int32_t yMax, int16_t overhead)
{
    /*Create a stacked_area_chart.obj*/
    lv_obj_t *chart = lv_chart_create(s_screen);
    lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_CIRCULAR);
    lv_obj_set_style_size(chart, 0, 0, LV_PART_INDICATOR);
    lv_obj_set_size(chart, 220, 130);
    lv_obj_align(chart, LV_ALIGN_BOTTOM_MID, 0, -50);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, yMin, yMax);
    lv_chart_set_point_count(chart, 80);
    lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
    // /*Prefill with data*/
    // uint32_t i;
    // for (i = 0; i < 80; i++)
    // {
    //     lv_chart_set_next_value(chart, ser, lv_rand(10, 90));
    // }

    return chart;
}

// Create the scale
static lv_obj_t *create_scale(lv_obj_t *s_screen, lv_obj_t *chart, int32_t yMax)
{
    lv_obj_t *scale = lv_scale_create(s_screen);

    lv_scale_set_mode(scale, LV_SCALE_MODE_VERTICAL_LEFT);
    lv_obj_set_size(scale, 40, 130);
    lv_scale_set_range(scale, 0, yMax);
    lv_scale_set_total_tick_count(scale, 6);
    lv_scale_set_major_tick_every(scale, 1);

    lv_obj_align_to(scale, chart, LV_ALIGN_OUT_LEFT_MID, -5, 0);
    return scale;
}

// Create the graph
graph_t *create_graph(lv_obj_t *s_screen, int32_t yMin, int32_t yMax,
                      int16_t overhead, int16_t period, graph_data_cb_t cb)
{
    graph_t *graph = lv_malloc(sizeof(graph_t));
    graph->chart = create_chart(s_screen, yMin, yMax, overhead);
    graph->scale = create_scale(s_screen, graph->chart, yMax);
    graph->scaleYMax = yMax;
    graph->scaleYMin = yMin;
    graph->overheadGap = overhead;
    graph->get_value_cb = cb; // Function cb reference to get value for specific sensor
    // If user specifies a callback function
    if (cb) graph->timer = lv_timer_create(add_graph_data_cb, period, graph);
    else graph->timer = NULL;

    return graph;
}

graph_t *reset_graph(graph_t *graph, int32_t yMin, int32_t yMax, graph_data_cb_t cb)
{

    lv_chart_set_range(graph->chart, LV_CHART_AXIS_PRIMARY_Y, yMin, yMax);
    lv_chart_series_t *ser = lv_chart_get_series_next(graph->chart, NULL);
    if (ser)
    {
        lv_chart_set_all_value(graph->chart, ser, LV_CHART_POINT_NONE);
        lv_chart_refresh(graph->chart);
    }

    graph->get_value_cb = cb;
    return graph;
}
/**
 * This section covers the rounded scale used for the motor and humidity sensors
 *
 */

// Convert a value (0–100) into an angle in degrees
static float value_to_angle(int32_t value)
{
    return 135 + (270 * value) / 100; // start 135°, range 270°
}

// Update the needle points based on a value
static void update_needle_points(roundScale_t *scale, int32_t value)
{
    float angle = value_to_angle(value);
    float rad = angle * (M_PI / 180.0f);

    // Unsure why the inital x,y needle coords rely on scale when the parent of the needle is the screen, so position should be relative to the screen
    scale->needle_points[0].x = scale->radius;
    scale->needle_points[0].y = scale->radius;
    scale->needle_points[1].x = scale->radius + cos(rad) * scale->needle_length;
    scale->needle_points[1].y = scale->radius + sin(rad) * scale->needle_length;

    lv_line_set_points(scale->needle, scale->needle_points, 2);
}

// Animation callback for LVGL
static void needle_anim_cb(void *obj, int32_t value)
{
    roundScale_t *scale = (roundScale_t *)obj;
    update_needle_points(scale, value);
    lv_obj_invalidate(scale->needle); // only redraw the needle
}

static void needle_update_timer_cb(lv_timer_t *t)
{
    roundScale_t *scale = (roundScale_t *)lv_timer_get_user_data(t);
    int32_t sensor_value = scale->get_value_cb();
    int16_t value = scale->cur_value + sensor_value;
    int32_t tot_sensor_value = value;
    if (tot_sensor_value < 0)
        tot_sensor_value = 0;
    else if (tot_sensor_value > 50 && tot_sensor_value < 100)
        tot_sensor_value -= lv_rand(-1, 2);
    else if (tot_sensor_value > 100)
        tot_sensor_value = 100;
    // Animate needle from last value to sensor_value over 100 ms
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, scale);
    lv_anim_set_values(&a, scale->cur_value, tot_sensor_value);
    lv_anim_set_time(&a, 100);
    lv_anim_set_exec_cb(&a, needle_anim_cb);
    lv_anim_start(&a);

    scale->cur_value = tot_sensor_value;
}

// Modified and obtained from https://lvgl.io/docs/open/widgets/scale
roundScale_t *create_speedometer(lv_obj_t *parent, scale_data_cb_t cb, int16_t radius, int16_t needle_length, int32_t cur_value, int16_t period)
{
    roundScale_t *round_scale = lv_malloc(sizeof(roundScale_t));

    round_scale->cur_value = cur_value;
    round_scale->radius = radius;
    round_scale->needle_length = needle_length;
    round_scale->get_value_cb = cb;

    // Create scale background
    round_scale->scale = lv_scale_create(parent);
    lv_obj_set_size(round_scale->scale, radius * 2, radius * 2);
    lv_scale_set_mode(round_scale->scale, LV_SCALE_MODE_ROUND_INNER);
    lv_obj_set_style_bg_opa(round_scale->scale, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(round_scale->scale, lv_palette_lighten(LV_PALETTE_GREY, 5), 0);
    lv_obj_set_style_radius(round_scale->scale, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(round_scale->scale, true, 0);
    lv_obj_center(round_scale->scale);
    lv_obj_align(round_scale->scale, LV_ALIGN_LEFT_MID, 25, 0);

    lv_scale_set_label_show(round_scale->scale, true);
    lv_scale_set_total_tick_count(round_scale->scale, 21);
    lv_scale_set_major_tick_every(round_scale->scale, 2);
    lv_obj_set_style_length(round_scale->scale, 5, LV_PART_ITEMS);
    lv_obj_set_style_length(round_scale->scale, 10, LV_PART_INDICATOR);
    lv_scale_set_range(round_scale->scale, 0, 100);
    lv_scale_set_angle_range(round_scale->scale, 270);
    lv_scale_set_rotation(round_scale->scale, 135);

    round_scale->needle = lv_line_create(parent);
    lv_obj_set_size(round_scale->needle, radius * 2, radius * 2);
    lv_obj_center(round_scale->needle);
    lv_obj_align(round_scale->needle, LV_ALIGN_LEFT_MID, 25, 0);

    lv_obj_set_style_line_width(round_scale->needle, 4, LV_PART_MAIN);
    lv_obj_set_style_line_color(round_scale->needle, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
    lv_obj_set_style_line_rounded(round_scale->needle, true, LV_PART_MAIN);

    // Initialize needle at 0
    update_needle_points(round_scale, cur_value);

    round_scale->timer = lv_timer_create(needle_update_timer_cb, period, round_scale);
    return round_scale;
}

lv_obj_t *create_card(lv_obj_t *parent,
                      const char *title,
                      lv_coord_t x,
                      lv_coord_t y,
                      lv_color_t color,
                      lv_obj_t **value_label)
{
    lv_obj_t *card = lv_obj_create(parent);

    lv_obj_set_size(card, 140, 75);
    lv_obj_set_pos(card, x, y);

    lv_obj_set_style_radius(card, 18, 0);

    lv_obj_set_style_bg_color(card, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_90, 0);

    lv_obj_set_style_border_width(card, 2, 0);
    lv_obj_set_style_border_color(card, color, 0);

    lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF);

    /* Title */
    lv_obj_t *lbl_title = lv_label_create(card);
    lv_label_set_text(lbl_title, title);

    lv_obj_set_style_text_color(lbl_title,
                                lv_color_hex(0xCBD5E1),
                                0);

    lv_obj_align(lbl_title, LV_ALIGN_TOP_LEFT, 0, 0);

    /* Value */
    if (value_label != NULL)
    {
        *value_label = lv_label_create(card);

        lv_obj_set_style_text_font(*value_label,
                                   &lv_font_montserrat_22,
                                   0);

        lv_obj_set_style_text_color(*value_label,
                                    lv_color_white(),
                                    0);

        lv_label_set_text(*value_label, "--");

        lv_obj_align(*value_label,
                     LV_ALIGN_CENTER,
                     0,
                     10);
    }

    return card;
}