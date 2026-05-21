#include "lvgl.h"
#include "../data.h"

void setBackgroundColour(lv_obj_t *parent);
lv_obj_t *nav_button_init(lv_obj_t *parent, const char *label, lv_event_cb_t cb,
                          lv_align_t align, int32_t x_ofs, int32_t y_ofs);

lv_obj_t *create_label(lv_obj_t *parent, const char *label);

lv_obj_t *create_icon_button(lv_obj_t *parent,
                             const char *icon,
                             lv_event_cb_t cb,
                             lv_align_t align,
                             int32_t x,
                             int32_t y);

lv_obj_t *create_dropdown(lv_obj_t *parent,
                          const char *options,
                          void (*on_change)(const char *));

lv_obj_t *ui_create_numeric_keyboard(lv_obj_t *parent);

void ui_attach_keyboard(lv_obj_t *text_area, lv_obj_t *keyboard);

graph_t * create_graph(lv_obj_t * s_screen, int32_t yMin, int32_t yMax, int16_t overhead, int16_t period, graph_data_cb_t cb);
roundScale_t * create_speedometer(lv_obj_t *parent, scale_data_cb_t cb,int16_t radius, int16_t needle_length, int32_t cur_value, int16_t period);