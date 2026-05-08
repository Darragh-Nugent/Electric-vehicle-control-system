#include "lvgl.h"
#include "../screen_manager.h"
#include "scr_motor.h"
#include <stdint.h>
#include <stdbool.h>
#include "../gui_utils.h"
static lv_obj_t *s_screen;
static int32_t temp = 20;
void scr_sensor2_get_x(float x) {};

void scr_sensor2_set_x(float x) {};

static void btn_home_cb(lv_event_t *e)
{
    (void)e;
    screen_manager_goto(SCREEN_SENSORS);
}

// obtained from https://lvgl.io/docs/open/9.2/examples
lv_obj_t * lv_scale_temp(lv_obj_t *s_screen)
{
    lv_obj_t * scale = lv_scale_create(s_screen);
    lv_obj_set_size(scale, 90, 170);
    lv_scale_set_label_show(scale, true);
    lv_scale_set_mode(scale, LV_SCALE_MODE_VERTICAL_RIGHT);
    lv_obj_align(scale, LV_ALIGN_BOTTOM_MID, 0, -25);

    lv_scale_set_total_tick_count(scale, 21);
    lv_scale_set_major_tick_every(scale, 5);

    lv_obj_set_style_length(scale, 10, LV_PART_INDICATOR);
    lv_obj_set_style_length(scale, 5, LV_PART_ITEMS);
    lv_scale_set_range(scale, 0, 100);

    static const char * custom_labels[] = {"-20 °C","0 °C", "25 °C", "50 °C", "75 °C", "100 °C", NULL};
    lv_scale_set_text_src(scale, custom_labels);

    static lv_style_t indicator_style;
    lv_style_init(&indicator_style);

    /* Label style properties */
    lv_style_set_text_font(&indicator_style, LV_FONT_DEFAULT);
    lv_style_set_text_color(&indicator_style, lv_palette_darken(LV_PALETTE_BLUE, 3));

    /* Major tick properties */
    lv_style_set_line_color(&indicator_style, lv_palette_darken(LV_PALETTE_BLUE, 3));
    lv_style_set_width(&indicator_style, 10U);      /*Tick length*/
    lv_style_set_line_width(&indicator_style, 2U);  /*Tick width*/
    lv_obj_add_style(scale, &indicator_style, LV_PART_INDICATOR);

    static lv_style_t minor_ticks_style;
    lv_style_init(&minor_ticks_style);
    lv_style_set_line_color(&minor_ticks_style, lv_palette_lighten(LV_PALETTE_BLUE, 2));
    lv_style_set_width(&minor_ticks_style, 5U);         /*Tick length*/
    lv_style_set_line_width(&minor_ticks_style, 2U);    /*Tick width*/
    lv_obj_add_style(scale, &minor_ticks_style, LV_PART_ITEMS);

    static lv_style_t main_line_style;
    lv_style_init(&main_line_style);
    /* Main line properties */
    lv_style_set_line_color(&main_line_style, lv_palette_darken(LV_PALETTE_BLUE, 3));
    lv_style_set_line_width(&main_line_style, 2U); // Tick width
    lv_obj_add_style(scale, &main_line_style, LV_PART_MAIN);

    /* Add a section */
    static lv_style_t section_minor_tick_style;
    static lv_style_t section_label_style;
    static lv_style_t section_main_line_style;

    lv_style_init(&section_label_style);
    lv_style_init(&section_minor_tick_style);
    lv_style_init(&section_main_line_style);

    /* Label style properties */
    lv_style_set_text_font(&section_label_style, LV_FONT_DEFAULT);
    lv_style_set_text_color(&section_label_style, lv_palette_darken(LV_PALETTE_RED, 3));

    lv_style_set_line_color(&section_label_style, lv_palette_darken(LV_PALETTE_RED, 3));
    lv_style_set_line_width(&section_label_style, 5U); /*Tick width*/

    lv_style_set_line_color(&section_minor_tick_style, lv_palette_lighten(LV_PALETTE_RED, 2));
    lv_style_set_line_width(&section_minor_tick_style, 4U); /*Tick width*/

    /* Main line properties */
    lv_style_set_line_color(&section_main_line_style, lv_palette_darken(LV_PALETTE_RED, 3));
    lv_style_set_line_width(&section_main_line_style, 4U); /*Tick width*/

    /* Configure section styles */
    lv_scale_section_t * section = lv_scale_add_section(scale);
    lv_scale_section_set_range(section, 75, 100);
    lv_scale_section_set_style(section, LV_PART_INDICATOR, &section_label_style);
    lv_scale_section_set_style(section, LV_PART_ITEMS, &section_minor_tick_style);
    lv_scale_section_set_style(section, LV_PART_MAIN, &section_main_line_style);

    lv_obj_set_style_bg_color(scale, lv_palette_main(LV_PALETTE_BLUE_GREY), 0);
    lv_obj_set_style_bg_opa(scale, LV_OPA_50, 0);
    lv_obj_set_style_pad_left(scale, 30, 0);
    lv_obj_set_style_radius(scale, 8, 0);
    lv_obj_set_style_pad_ver(scale, 20, 0);

    return scale;
}

static void temp_timer_cb(lv_timer_t *t)
{
    lv_obj_t *bar = lv_timer_get_user_data(t);
    int32_t value = lv_rand(-1,1);
    temp += value;

    // if(xQueueReceive(tempQueue, &temp, 0) == pdPASS)
    // {
    //     lv_bar_set_value(bar, temp, LV_ANIM_ON);
    // }
    // OR a api call that handles queues internally
    lv_bar_set_value(bar, temp, LV_ANIM_ON);
}

lv_obj_t * lv_temp_bar(lv_obj_t * s_screen)
{
    static lv_style_t style_indic;

    lv_style_init(&style_indic);
    lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);
    lv_style_set_bg_color(&style_indic, lv_palette_main(LV_PALETTE_RED));
    lv_style_set_bg_grad_color(&style_indic, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_bg_grad_dir(&style_indic, LV_GRAD_DIR_VER);

    lv_obj_t * bar = lv_bar_create(s_screen);
    lv_obj_add_style(bar, &style_indic, LV_PART_INDICATOR);
    lv_obj_set_size(bar, 20, 150);
    lv_obj_center(bar);
    lv_bar_set_range(bar, -20, 75);
    lv_bar_set_start_value(bar, -20, LV_ANIM_OFF);
    lv_timer_create(temp_timer_cb, 100, bar);

    // lv_anim_t a;
    // lv_anim_init(&a);
    // lv_anim_set_exec_cb(&a, set_temp);
    // lv_anim_set_duration(&a, 3000);
    // lv_anim_set_playback_duration(&a, 3000);
    // lv_anim_set_var(&a, bar);
    // lv_anim_set_values(&a, -20, 40);
    // lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    // lv_anim_start(&a);

    return bar;
}




void scr_sensor2_init(void)
{
    s_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(s_screen, COLOR_BACKGROUND_GREEN, LV_PART_MAIN);

    // To DO:
    // Add relevant buttons and diagnostics for motor

    lv_obj_t *label = create_label(s_screen, "Sensor 2");
    (void)label; // ignore label for now, return value is kept for possible future use
    lv_obj_t* scale = lv_scale_temp(s_screen);
    lv_obj_t* bar = lv_temp_bar(s_screen);
    lv_obj_align_to(bar, scale, LV_ALIGN_LEFT_MID, -20, -10); //-20
    lv_obj_t *prev_button = create_icon_button(s_screen, LV_SYMBOL_PREV, btn_home_cb, LV_ALIGN_TOP_LEFT, 8, 8);
    (void)prev_button;
}

lv_obj_t *scr_sensor2_get(void)
{
    return s_screen;
};
