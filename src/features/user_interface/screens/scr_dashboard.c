#include <stdint.h>
#include <stdbool.h>
#include "scr_dashboard.h"
#include "../screen_manager.h"
#include "lvgl.h"
#include "../gui_utils.h"
// #include "../sensors.h"

void scr_dashboard_set_rpm(float f) {};
void scr_dashboard_set_sensor(uint32_t idx, float f) {};
void scr_dashboard_set_motor_state(uint32_t state) {};

static lv_obj_t *s_screen;
static lv_obj_t *timeLabel;
static uint8_t seconds = 0;
static uint8_t minutes = 20;
static uint8_t hours = 9;
static uint8_t days = 0;

static lv_obj_t *lbl_temp;
static lv_obj_t *lbl_humidity;
static lv_obj_t *lbl_temp_info;

extern sensorThresholds_t g_thresholds;

static void btn_motor_cb(lv_event_t *e)
{
    (void)e;
    screen_manager_goto(SCREEN_MOTOR);
}

// May need to change for each sensors
static void btn_sensors_cb(lv_event_t *e)
{
    (void)e;
    screen_manager_goto(SCREEN_SENSORS);
}

static void btn_status_cb(lv_event_t *e)
{
    (void)e;
    screen_manager_goto(SCREEN_STATUS);
}

static void btn_alert_cb(lv_event_t *e)
{
    (void)e;
    screen_manager_goto(SCREEN_ALERT);
}

static void btn_settings_cb(lv_event_t *e)
{
    (void)e;
    screen_manager_goto(SCREEN_SETTINGS);
}

void handleCB(lv_timer_t *e)
{
    seconds += 1;
    if (seconds >= 60)
    {
        seconds = 0;
        minutes += 1;
        if (timeLabel)
            lv_label_set_text_fmt(timeLabel, "%02u:%02u", hours, minutes);
    }
    if (minutes >= 60)
    {
        minutes = 0;
        hours += 1;
        if (timeLabel)
            lv_label_set_text_fmt(timeLabel, "%02u:%02u", hours, minutes);
    }
    if (hours >= 24)
    {
        hours = 0;
        days += 1; // not really displayed but is here for future use
    }
    int16_t temp = 50;
    int16_t humidity = 100;
    // Sensor_GetTemp
    // Sensor_GetHumidity

    lv_label_set_text_fmt(lbl_temp, "%d", temp);
    lv_label_set_text_fmt(lbl_humidity, "%d", humidity);

    if (temp > g_thresholds.TH_TEMP)
        lv_label_set_text(lbl_temp_info, "Cooling on");
    else if (temp < g_thresholds.TH_TEMP)
        lv_label_set_text(lbl_temp_info, "Cooling off");
}

// Obtained from https://lvgl.io/docs/open/9.5/widgets/label.html
void lv_moving_title(lv_obj_t *s_screen)
{
    static lv_anim_t animation_template;
    static lv_style_t label_style;

    lv_anim_init(&animation_template);
    lv_anim_set_delay(&animation_template, 1000); /*Wait 1 second to start the first scroll*/
    lv_anim_set_repeat_delay(&animation_template,
                             500); /*Repeat the scroll 3 seconds after the label scrolls back to the initial position*/
    lv_anim_set_repeat_count(&animation_template, LV_ANIM_REPEAT_INFINITE);

    /*Initialize the label style with the animation template*/
    lv_style_init(&label_style);
    lv_style_set_anim(&label_style, &animation_template);

    lv_obj_t *label = lv_label_create(s_screen);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_label_set_long_mode(label, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR); /*Circular scroll*/
    lv_obj_set_width(label, 300);
    lv_label_set_text(label, "Zackariya Taylor, Isobel Jones, Darragh Nugent, Bon Nguyen,");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 80);
    lv_obj_add_style(label, &label_style, LV_STATE_DEFAULT); /*Add the style to the label*/
}

void scr_dashboard_init(void)
{
    // To Do: Modularise colour
    s_screen = lv_obj_create(NULL);
    setBackgroundColour(s_screen);

    // Label
    lv_obj_t *label = create_label(s_screen, "Group #30");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 30); // ignore label for now, return value is kept for possible future use
    lv_moving_title(s_screen);

    lv_obj_t *header = lv_obj_create(s_screen);
    lv_obj_set_size(header, LV_HOR_RES, 25);
    lv_obj_align(header, LV_ALIGN_OUT_TOP_MID, 0, 0);
    lv_obj_remove_flag(header, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(header, 0, 0);

    static lv_style_t no_border;
    lv_style_init(&no_border);
    lv_style_set_border_width(&no_border, 0);
    lv_obj_add_style(header, &no_border, 0);

    static lv_style_t style_date;
    lv_style_init(&style_date);
    lv_style_set_bg_color(&style_date, lv_color_hex(STATUS_CARD_BG));
    lv_style_set_bg_opa(&style_date, LV_OPA_COVER);
    lv_obj_add_style(header, &style_date, 0);

    lv_obj_t *dateLabel = lv_label_create(header);
    lv_label_set_text(dateLabel, "24/05/2026");
    lv_obj_center(dateLabel);

    timeLabel = lv_label_create(header);
    lv_label_set_text_fmt(timeLabel, "%u:%u", hours, minutes);
    lv_obj_align(timeLabel, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t *batteryLabel = lv_label_create(header);
    lv_label_set_text(batteryLabel, LV_SYMBOL_BATTERY_3);
    lv_obj_align(batteryLabel, LV_ALIGN_RIGHT_MID, 0, 0);

    lv_obj_t *wifiLabel = lv_label_create(header);
    lv_label_set_text(wifiLabel, LV_SYMBOL_WIFI);
    lv_obj_align_to(wifiLabel, batteryLabel, LV_ALIGN_OUT_LEFT_MID, -5, 0);

    lv_obj_t *bluetoothLabel = lv_label_create(header);
    lv_label_set_text(bluetoothLabel, LV_SYMBOL_BLUETOOTH);
    lv_obj_align_to(bluetoothLabel, wifiLabel, LV_ALIGN_OUT_LEFT_MID, -5, 0);

    lv_obj_set_style_text_color(dateLabel, lv_color_white(), 0);
    lv_obj_set_style_text_color(timeLabel, lv_color_white(), 0);
    lv_obj_set_style_text_color(batteryLabel, lv_color_white(), 0);
    lv_obj_set_style_text_color(wifiLabel, lv_color_white(), 0);
    lv_obj_set_style_text_color(bluetoothLabel, lv_color_white(), 0);

    // Create a container for the navigation bar at the bottom
    lv_obj_t *nav_bar = lv_obj_create(s_screen);
    lv_obj_set_size(nav_bar, LV_HOR_RES, 50);         // Set the navigation bar's height
    lv_obj_align(nav_bar, LV_ALIGN_BOTTOM_MID, 0, 0); // Align it to the bottom of the screen
    lv_obj_remove_flag(nav_bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(nav_bar, COLOR_STATUS_CARD_BG_LV, 0);
    lv_obj_set_style_border_width(nav_bar, 2, 0);
    lv_obj_set_style_border_color(nav_bar, COLOR_WHITE_LV, 0);

    // Create the buttons within the navigation bar, spaced evenly
    lv_obj_t *motor_btn = nav_button_init(nav_bar, "Motor", btn_motor_cb, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_t *sensors_btn = nav_button_init(nav_bar, "Sensors", btn_sensors_cb, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t *status_btn = nav_button_init(nav_bar, "Status", btn_status_cb, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_t *settings_btn = create_icon_button(s_screen, LV_SYMBOL_SETTINGS, btn_settings_cb, LV_ALIGN_TOP_LEFT, 8, 8);
    lv_obj_t *alert_btn = create_icon_button(s_screen, LV_SYMBOL_WARNING, btn_alert_cb, LV_ALIGN_TOP_RIGHT, -8, 8);
    // Align the buttons horizontally within the navigation bar
    lv_obj_set_width(motor_btn, 80);
    lv_obj_set_width(sensors_btn, 80);
    lv_obj_set_width(status_btn, 80);
    lv_obj_set_width(settings_btn, 40);
    lv_obj_set_width(alert_btn, 40);

    lv_obj_align(settings_btn, LV_ALIGN_TOP_LEFT, 8, 30);
    lv_obj_align(alert_btn, LV_ALIGN_TOP_RIGHT, -8, 30);

    // Add some spacing between buttons
    lv_obj_align(motor_btn, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_align(sensors_btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(status_btn, LV_ALIGN_RIGHT_MID, -10, 0);

    // Remove background of settings button
    static lv_style_t style_transp;
    lv_style_init(&style_transp);
    lv_style_set_bg_opa(&style_transp, LV_OPA_TRANSP);
    lv_style_set_border_width(&style_transp, 0); // Optional: Removes borders

    lv_obj_add_style(settings_btn, &style_transp, LV_STATE_DEFAULT);
    lv_obj_add_style(alert_btn, &style_transp, LV_STATE_DEFAULT);
    lv_obj_remove_flag(s_screen, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *temp = create_card(s_screen, "Temp", 170, 50,
                                 lv_palette_main(LV_PALETTE_YELLOW), &lbl_temp, LV_ALIGN_TOP_RIGHT);
    lv_obj_align_to(temp, nav_bar, LV_ALIGN_OUT_TOP_LEFT, 5, -5);

    lv_obj_t *humidity = create_card(s_screen, "Humidity", 170, 50,
                                     lv_palette_main(LV_PALETTE_BLUE), &lbl_humidity, LV_ALIGN_TOP_RIGHT);

    lv_obj_align_to(humidity, nav_bar, LV_ALIGN_OUT_TOP_RIGHT, -5, -5);

    lbl_temp_info = lv_label_create(temp);
    lv_obj_set_style_text_color(lbl_temp_info, lv_color_white(), 0);
    lv_obj_align(lbl_temp_info, LV_ALIGN_LEFT_MID, 10, 20);

    // Lv timer for time
    lv_timer_create(handleCB, 1000, NULL);
}
lv_obj_t *scr_dashboard_get(void) { return s_screen; }