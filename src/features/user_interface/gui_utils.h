#include "lvgl.h"

lv_obj_t *nav_button_init(lv_obj_t *parent, const char *label, lv_event_cb_t cb,
                          lv_align_t align, int32_t x_ofs, int32_t y_ofs);

lv_obj_t *create_label(lv_obj_t *parent, const char *label);

lv_obj_t *create_icon_button(lv_obj_t *parent,
                             const char *icon,
                             lv_event_cb_t cb,
                             lv_align_t align,
                             int32_t x,
                             int32_t y);