#include "lvgl.h"
#include "../data.h"
typedef void (*dropdown_cb_t)(const char *text);

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
    lv_obj_center(chart);
     lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, yMin, yMax);
    lv_chart_set_point_count(chart, 80);
    // lv_chart_series_t *ser = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
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
graph_t *create_graph(lv_obj_t * s_screen, int32_t yMin, int32_t yMax, 
    int16_t overhead, int16_t period, graph_data_cb_t cb){
    graph_t * graph = lv_malloc(sizeof(graph_t));
    graph->chart = create_chart(s_screen, yMin, yMax, overhead);
    graph->scale = create_scale(s_screen, graph->chart, yMax);
    graph->scaleYMax = yMax;
    graph->scaleYMin = yMin;
    graph->overheadGap = overhead;
    graph->get_value_cb = cb; // Function cb reference to get value for specific sensor
    graph->timer = lv_timer_create(add_graph_data_cb, period, graph);

    return graph;
}