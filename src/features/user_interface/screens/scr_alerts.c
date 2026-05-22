#include <stdint.h>
#include <stdbool.h>
#include "scr_alerts.h"
#include "lvgl.h"
#include "../gui_utils.h"
#include "../../data.h"
#include "../screen_manager.h"

static lv_obj_t *s_screen;

static void btn_home_cb(lv_event_t *e)
{
    screen_manager_goto(SCREEN_DASHBOARD);
}

static void event_cb(lv_event_t * e)
{
    updateGUIState(UI_MSG_FAULT_CLEARED, 0);
    screen_manager_goto(SCREEN_DASHBOARD);
}

void createMsgBox(lv_obj_t * parent)
{
    lv_obj_t * mbox1 = lv_msgbox_create(parent);

    lv_obj_t* title = lv_msgbox_add_title(mbox1, "WARNING");
    lv_obj_set_style_text_align(title,LV_ALIGN_CENTER,0);
    lv_obj_set_style_text_color(title, SCARY_RED, 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_22, 0);

    lv_msgbox_add_text(mbox1, "A fault has been latched. You MUST ackowledge the fault before continuing.");
    
    lv_obj_t * btn;
    btn = lv_msgbox_add_footer_button(mbox1, "Acknowledge");
    lv_obj_add_event_cb(btn, event_cb, LV_EVENT_CLICKED, NULL);



    return;
}


void scr_alerts_init(void)
{
    s_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(s_screen, LV_OBJ_FLAG_SCROLLABLE);
    setBackgroundColour(s_screen);

    createMsgBox(s_screen);


}
lv_obj_t *scr_alerts_get(void) { return s_screen; }