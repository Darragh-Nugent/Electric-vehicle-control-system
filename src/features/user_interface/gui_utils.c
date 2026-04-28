#include "lvgl.h"
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
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, 8, 8);
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
