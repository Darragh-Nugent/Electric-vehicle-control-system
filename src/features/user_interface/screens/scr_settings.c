#include "lvgl.h"
#include "../screen_manager.h"
#include "scr_motor.h"
#include <stdint.h>
#include <stdbool.h>
#include "../gui_utils.h"
#include <stdlib.h>
#include "utils/uartstdio.h"
#include "../../data.h"
static lv_obj_t *s_screen;
static lv_obj_t *tabview;
static lv_obj_t *info_tab;
static lv_obj_t *power_tab;
static lv_obj_t *accel_tab;
static lv_obj_t *dist_tab;
// static lv_obj_t *lux_tab;
static lv_obj_t *temp_tab;
static lv_obj_t *humidity_tab;

static lv_obj_t *power_spinbox;
static lv_obj_t *accel_spinbox;
static lv_obj_t *dist_spinbox;
// static lv_obj_t *lux_spinbox;
static lv_obj_t *temp_spinbox;
static lv_obj_t *humidity_spinbox;

#define SPINBOX_SCALE 10.0f

const char *thresholdPrompt = "This is threshold configuration settings for the electric vehicle.\n"
                              "Only the relevant sensors can trigger a fault: \n"
                              "Power\n"
                              "Distance\n"
                              "Acceleration\n"
                              "[Warnings occur when these exceed 80%% of max]";

const char *powerPrompt = "Set the maximum power the motor can recieve in W (Max of 23 W)";

const char *accelPrompt = "Set the maximum acceleration in m/s/s";
const char *distancePrompt = "Set the distance an object can be in m (Max of 10)";

const char *tempPrompt = "Set the maximum detected temperature in degrees celcius";

// const char *luxPrompt = "Set the maximum detected lux value in lx\n"
                        // "[Note wil limit is used to turn on headlights]";

const char *humidityPrompt = "Sets the maximum detected humidity in %";

typedef struct
{
    lv_obj_t *spinbox;
    UiMsgType_t msgType;
} submit_t;

static void btn_home_cb(lv_event_t *e)
{
    (void)e;
    screen_manager_goto(SCREEN_DASHBOARD);
}

lv_obj_t *submit_threshold_btn(lv_obj_t *parent, const char *label, lv_event_cb_t cb,
                               lv_align_t align, int32_t x_ofs, int32_t y_ofs, submit_t *category)
{
    lv_obj_t *button = lv_button_create(parent);
    lv_obj_set_size(button, 90, 40);
    lv_obj_add_event_cb(button, cb, LV_EVENT_CLICKED, category);

    lv_obj_t *lbl = lv_label_create(button);
    lv_label_set_text(lbl, label);
    lv_obj_align(button, align, x_ofs, y_ofs);

    return button;
}

// Kind of coupled and not really set out the best
static void lv_submit_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
        return;

    submit_t *res = (submit_t *)lv_event_get_user_data(e);
    uint16_t value = lv_spinbox_get_value(res->spinbox);
    updateGUIState(res->msgType, value);
}

static void lv_spinbox_increment_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *spinbox = (lv_obj_t *)lv_event_get_user_data(e);
    if (code == LV_EVENT_CLICKED)
    {
        lv_spinbox_increment(spinbox);
    }
}

static void lv_spinbox_decrement_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *spinbox = (lv_obj_t *)lv_event_get_user_data(e);
    if (code == LV_EVENT_CLICKED)
    {
        lv_spinbox_decrement(spinbox);
    }
}

static lv_obj_t *create_threshold_card(
    lv_obj_t *parent,
    const char *title,
    const char *unit,
    const int16_t min,
    const uint16_t max
)
{
    lv_obj_t *card = lv_obj_create(parent);

    lv_obj_set_size(card, 200, 80);

    lv_obj_set_style_radius(card, 12, 0);

    lv_obj_set_style_pad_all(card, 10, 0);

    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_align(card, LV_ALIGN_LEFT_MID, 8, 0);

    lv_obj_t *title_label = lv_label_create(card);

    lv_label_set_text(title_label, title);

    lv_obj_align(title_label,
                 LV_ALIGN_TOP_MID, 0, -5);

    // lv_obj_t *current_label =
    //     lv_label_create(card);

    // lv_label_set_text(current_label,
    //     "Current: 0");

    // lv_obj_align(current_label,
    //     LV_ALIGN_TOP_LEFT, 0, 30);

    lv_obj_t *spinbox = lv_spinbox_create(card);

    lv_spinbox_set_range(spinbox, min, max); // should be defines
    if (max > 10000)  lv_spinbox_set_digit_count(spinbox, 5);
    else if (max > 999) lv_spinbox_set_digit_count(spinbox, 4);
    else if (max > 99 && max < 1000) lv_spinbox_set_digit_count(spinbox, 3);
    else lv_spinbox_set_digit_count(spinbox, 2);
    lv_spinbox_set_dec_point_pos(spinbox, 0);
    lv_spinbox_step_prev(spinbox);
    lv_obj_set_width(spinbox, 100);
    lv_obj_set_height(spinbox, 35);
    lv_obj_center(spinbox);
    int32_t h = lv_obj_get_height(spinbox);
    lv_obj_align(spinbox, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *btn = lv_button_create(card);
    lv_obj_set_size(btn, h, h);
    lv_obj_align_to(btn, spinbox, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_set_style_bg_image_src(btn, LV_SYMBOL_PLUS, 0);
    lv_obj_add_event_cb(btn, lv_spinbox_increment_event_cb, LV_EVENT_CLICKED, spinbox);

    btn = lv_button_create(card);
    lv_obj_set_size(btn, h, h);
    lv_obj_align_to(btn, spinbox, LV_ALIGN_OUT_LEFT_MID, -5, 0);
    lv_obj_set_style_bg_image_src(btn, LV_SYMBOL_MINUS, 0);
    lv_obj_add_event_cb(btn, lv_spinbox_decrement_event_cb, LV_EVENT_CLICKED, spinbox);

    return spinbox;
}

void lv_tab(lv_obj_t *s_screen)
{
    /*Create a Tab view object*/
    uint32_t tab_count = 0;
    uint32_t i = 0;

    tabview = lv_tabview_create(s_screen);
    lv_obj_set_size(tabview, LV_PCT(100), LV_PCT(100));
    lv_tabview_set_tab_bar_position(tabview, LV_DIR_LEFT);
    lv_tabview_set_tab_bar_size(tabview, 80);

    lv_obj_set_style_bg_color(tabview, lv_palette_lighten(LV_PALETTE_RED, 2), 0);

    lv_obj_t *tab_buttons = lv_tabview_get_tab_bar(tabview);
    lv_obj_set_style_bg_color(tab_buttons, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
    lv_obj_set_style_text_color(tab_buttons, lv_palette_lighten(LV_PALETTE_GREY, 5), 0);

    /*Add 5 tabs (the tabs are page (lv_page) and can be scrolled*/
    info_tab = lv_tabview_add_tab(tabview, "Info");
    power_tab = lv_tabview_add_tab(tabview, "Power");
    accel_tab = lv_tabview_add_tab(tabview, "Accel");
    dist_tab = lv_tabview_add_tab(tabview, "Distance");
    // lux_tab = lv_tabview_add_tab(tabview, "Lux");
    temp_tab = lv_tabview_add_tab(tabview, "Temp");
    humidity_tab = lv_tabview_add_tab(tabview, "Humidity");

    tab_count = lv_tabview_get_tab_count(tabview);
    for (i = 0; i < tab_count; i++)
    {
        lv_obj_t *button = lv_obj_get_child(tab_buttons, i);
        lv_obj_set_style_border_side(button, LV_BORDER_SIDE_RIGHT, LV_PART_MAIN | LV_STATE_CHECKED);
    }
    // Style Tabs
    lv_obj_set_style_bg_color(info_tab, lv_palette_lighten(LV_PALETTE_LIGHT_BLUE, 3), 0);
    lv_obj_set_style_bg_opa(info_tab, LV_OPA_COVER, 0);

    lv_obj_set_style_bg_color(dist_tab, lv_palette_lighten(LV_PALETTE_INDIGO, 3), 0);
    lv_obj_set_style_bg_opa(dist_tab, LV_OPA_COVER, 0);

    lv_obj_set_style_bg_color(power_tab, lv_palette_lighten(LV_PALETTE_AMBER, 3), 0);
    lv_obj_set_style_bg_opa(power_tab, LV_OPA_COVER, 0);

    // lv_obj_set_style_bg_color(lux_tab, lv_palette_lighten(LV_PALETTE_YELLOW, 3), 0);
    // lv_obj_set_style_bg_opa(lux_tab, LV_OPA_COVER, 0);

    lv_obj_set_style_bg_color(temp_tab, lv_palette_lighten(LV_PALETTE_CYAN, 3), 0);
    lv_obj_set_style_bg_opa(temp_tab, LV_OPA_COVER, 0);

    lv_obj_set_style_bg_color(humidity_tab, lv_palette_lighten(LV_PALETTE_LIGHT_GREEN, 3), 0);
    lv_obj_set_style_bg_opa(humidity_tab, LV_OPA_COVER, 0);

    lv_obj_remove_flag(lv_tabview_get_content(tabview), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(info_tab, LV_OBJ_FLAG_SCROLLABLE);
}

void formatText(lv_obj_t *text, lv_align_t align, int16_t xOffs, int16_t yOffs)
{
    lv_obj_align(text, align,xOffs,yOffs);
    lv_obj_set_style_text_color(text, lv_color_white(), 0);
    lv_obj_set_width(text, 220);
    lv_obj_set_height(text, 250);
    lv_label_set_long_mode(text, LV_LABEL_LONG_WRAP);
}

void scr_settings_init(void)
{
    s_screen = lv_obj_create(NULL);
    setBackgroundColour(s_screen);
    lv_tab(s_screen);
    lv_obj_clear_flag(s_screen, LV_OBJ_FLAG_SCROLLABLE);

    // Create cards
    power_spinbox = create_threshold_card(power_tab, "Power", "P", 
        SENSOR_POWER_MIN, SENSOR_POWER_MAX);
    accel_spinbox = create_threshold_card(accel_tab, "Acceleration", "m/s/s",
        SENSOR_ACCELERATION_MIN,SENSOR_ACCELERATION_MAX);
    dist_spinbox = create_threshold_card(dist_tab, "Distance", "m"
        ,SENSOR_DISTANCE_MIN,SENSOR_DISTANCE_MAX);
    // lux_spinbox = create_threshold_card(lux_tab, "Lux", "lx");
    temp_spinbox = create_threshold_card(temp_tab, "Temperature", "Celcius",
        SENSOR_TEMP_MIN,SENSOR_TEMP_MAX);
    humidity_spinbox = create_threshold_card(humidity_tab, "Humidity", "%%",
        SENSOR_HUMIDITY_MIN, SENSOR_HUMIDITY_MAX);

    // Initialise relevant data to submit
    static submit_t power_submit_t = {.spinbox = NULL, .msgType = UI_MSG_SENSOR_UPDATE_POWER};
    static submit_t accel_submit_t = {.spinbox = NULL, .msgType = UI_MSG_SENSOR_UPDATE_ACCELERATION};
    static submit_t dist_submit_t = {.spinbox = NULL, .msgType = UI_MSG_SENSOR_UPDATE_DISTANCE};
    // static submit_t lux_submit_t =  {.spinbox = NULL, .msgType = TH_LUX};
    static submit_t temp_submit_t = {.spinbox = NULL, .msgType = UI_MSG_SENSOR_UPDATE_TEMP};
    static submit_t humidity_submit_t = {.spinbox = NULL, .msgType = UI_MSG_SENSOR_UPDATE_HUMIDITY};

    power_submit_t.spinbox = power_spinbox;
    accel_submit_t.spinbox = accel_spinbox;
    dist_submit_t.spinbox = dist_spinbox;
    // lux_submit_t.spinbox = lux_spinbox;
    temp_submit_t.spinbox = temp_spinbox;
    humidity_submit_t.spinbox = humidity_spinbox;

    // Submit button for each tab
    lv_obj_t *sub_pwr_btn = submit_threshold_btn(power_tab, "SUBMIT", lv_submit_cb, LV_ALIGN_BOTTOM_MID, 0, 0, &power_submit_t);
    lv_obj_t *sub_accel_btn = submit_threshold_btn(accel_tab, "SUBMIT", lv_submit_cb, LV_ALIGN_BOTTOM_MID, 0, 0, &accel_submit_t);
    lv_obj_t *sub_dist_btn = submit_threshold_btn(dist_tab, "SUBMIT", lv_submit_cb, LV_ALIGN_BOTTOM_MID, 0, 0, &dist_submit_t);
    // lv_obj_t *sub_lux_btn = submit_threshold_btn(lux_tab,"SUBMIT", lv_submit_cb, LV_ALIGN_BOTTOM_MID, 0,0,&lux_submit_t);
    lv_obj_t *sub_temp_btn = submit_threshold_btn(temp_tab, "SUBMIT", lv_submit_cb, LV_ALIGN_BOTTOM_MID, 0, 0, &temp_submit_t);
    lv_obj_t *sub_humidity_btn = submit_threshold_btn(humidity_tab, "SUBMIT", lv_submit_cb, LV_ALIGN_BOTTOM_MID, 0, 0, &humidity_submit_t);

    // Prevent annoying warnings showing up
    (void)sub_pwr_btn;
    (void)sub_accel_btn;
    (void)sub_dist_btn;
    // (void) sub_lux_btn;
    (void)sub_temp_btn;
    (void)sub_humidity_btn;
    // lv_obj_align_to(accel, power, LV_ALIGN_OUT_BOTTOM_MID, 0,8);
    // lv_obj_align_to(dist, accel, LV_ALIGN_OUT_BOTTOM_MID, 0,8);
    // Label

    lv_obj_t *thresholdLabel = create_label(info_tab, "Thresholds");
    lv_obj_align(thresholdLabel, LV_ALIGN_TOP_RIGHT, 0, 0);

    lv_obj_t *thresholdText = lv_label_create(info_tab);
    formatText(thresholdText, LV_ALIGN_CENTER,0,60);
    lv_label_set_text(thresholdText, thresholdPrompt);
    lv_obj_remove_flag(thresholdText, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *powerText = lv_label_create(power_tab);
    formatText(powerText, LV_ALIGN_TOP_MID,0,0);
    lv_label_set_text(powerText, powerPrompt);

    lv_obj_t *accelText = lv_label_create(accel_tab);
    formatText(accelText, LV_ALIGN_TOP_MID,0,0);
    lv_label_set_text(accelText, accelPrompt);


    lv_obj_t *distText = lv_label_create(dist_tab);
    formatText(distText, LV_ALIGN_TOP_MID,0,0);
    lv_label_set_text(distText, distancePrompt);

    lv_obj_t *tempText = lv_label_create(temp_tab);
    formatText(tempText, LV_ALIGN_TOP_MID,0,0);
    lv_label_set_text(tempText, tempPrompt);

    lv_obj_t *humidityText = lv_label_create(humidity_tab);
    formatText(humidityText, LV_ALIGN_TOP_MID,0,0);
    lv_label_set_text(humidityText, humidityPrompt);

    lv_obj_t *home_button = create_icon_button(info_tab, LV_SYMBOL_HOME, btn_home_cb, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_width(home_button, 40);
    static lv_style_t style_transp;
    lv_style_init(&style_transp);
    lv_style_set_bg_opa(&style_transp, LV_OPA_TRANSP);
    lv_style_set_border_width(&style_transp, 0); // Optional: Removes borders

    lv_obj_add_style(home_button, &style_transp, LV_STATE_DEFAULT);
}

lv_obj_t *scr_settings_get(void)
{
    return s_screen;
};
