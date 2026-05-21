
#include "lvgl.h"
#include "../screen_manager.h"
#include "scr_status.h"
#include "../gui_utils.h"
// #include "../sensors.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint16_t value;
    uint32_t seq;
} sensor_sample_t;

void addRotation(void);
void Sensor_Init(void);

sensor_sample_t Sensor_GetLux(void){return (sensor_sample_t){.value = 400, .seq = 1};};
sensor_sample_t Sensor_GetTemp(void){};
sensor_sample_t Sensor_GetHumidity(void){};


static lv_obj_t *s_screen;

/* Value labels */
static lv_obj_t *lbl_temp;
static lv_obj_t *lbl_humidity;
static lv_obj_t *lbl_lux;


static void status_cb(lv_timer_t *t)
{
    (void)t;

    sensor_sample_t temp = Sensor_GetTemp();
    sensor_sample_t hum  = Sensor_GetHumidity();
    sensor_sample_t lux  = Sensor_GetLux();


    lv_label_set_text_fmt(lbl_temp,     "%u C",  temp.value);
    lv_label_set_text_fmt(lbl_humidity, "%u %%", hum.value);
    lv_label_set_text_fmt(lbl_lux,      "%u lx", lux.value);

}

/* --------------------------------------------------------- */
/* Button Handler                                            */
/* --------------------------------------------------------- */

static void btn_home_cb(lv_event_t *e)
{
    (void)e;
    screen_manager_goto(SCREEN_DASHBOARD);
}

/* --------------------------------------------------------- */
/* Screen Init                                               */
/* --------------------------------------------------------- */

void scr_status_init(void)
{
    s_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(s_screen, LV_OBJ_FLAG_SCROLLABLE);

    /* Background */
    setBackgroundColour(s_screen);

    /* Title */
    lv_obj_t *title = lv_label_create(s_screen);
    lv_label_set_text(title, "Sensor Status");

    lv_obj_set_style_text_font(title, &lv_font_montserrat_22, 0);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 12);

    /* Cards */
    create_card(s_screen, "Temperature", 10, 40,
                lv_palette_main(LV_PALETTE_RED), &lbl_temp);

    create_card(s_screen, "Humidity", 170, 40,
                lv_palette_main(LV_PALETTE_BLUE), &lbl_humidity);

    create_card(s_screen, "Light", 90, 145,
                lv_palette_main(LV_PALETTE_YELLOW), &lbl_lux);

    /* Home Button */
    lv_obj_t *home_button =
        create_icon_button(s_screen,
                           LV_SYMBOL_HOME,
                           btn_home_cb,
                           LV_ALIGN_TOP_LEFT,
                           8, 8);
    lv_obj_set_width(home_button,40);
    /* Update Timer */
    lv_timer_create(status_cb, 250, NULL);
    status_cb(NULL);
}

lv_obj_t *scr_status_get(void)
{
    return s_screen;
}