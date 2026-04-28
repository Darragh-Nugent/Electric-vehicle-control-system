#include "lvgl.h"

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
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, 8, 8);
    lv_obj_add_event_cb(button, cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *home = lv_label_create(button);
    lv_label_set_text(home, icon); // LVGL built-in icon
    lv_obj_center(home);

    return button;
}
