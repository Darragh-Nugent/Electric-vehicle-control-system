#include "lvgl.h"
#include "../screen_manager.h"
#include "scr_status.h"
#include <stdint.h>
#include <stdbool.h>
#include "../gui_utils.h"
static lv_obj_t *s_screen;


static void btn_home_cb(lv_event_t *e)
{
    (void)e;
    screen_manager_goto(SCREEN_SENSORS);
}


static void temp_status_cb(lv_timer_t *t)
{

}






void scr_status_init(void)
{

}

lv_obj_t *scr_status_get(void)
{
    return s_screen;
};
