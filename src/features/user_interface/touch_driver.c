#include "lvgl.h"
#include <stdbool.h>
#include <stdint.h>
#include "grlib/grlib.h"  // must be included before widget.h for trectangle stuff
#include "grlib/widget.h"

volatile int16_t i16Touch_X = 0;
volatile int16_t i16Touch_Y = 0;
volatile bool boolTouchPressed;

// Handles touchscreen events and stores the current touch state
// Called by Kentec/TivaWare interrupt
void TouchCallBack(uint32_t ui32Message, int32_t i32X, int32_t i32Y)
{
    switch (ui32Message)
    {
    case WIDGET_MSG_PTR_DOWN: // Do nothing I suppose
    case WIDGET_MSG_PTR_MOVE:
        i16Touch_X = i32X;
        i16Touch_Y = i32Y;
        boolTouchPressed = true;
        break;
    case WIDGET_MSG_PTR_UP:
        boolTouchPressed = false;
        break;
    default:
        break;
    }
}

// LVGL read callback - every scan period
static void prvTouchReadCb(lv_indev_t *indev, lv_indev_data_t *data){
    (void) indev;
    data->point.x = i16Touch_X;
    data->point.y = i16Touch_Y;
    data-> state = boolTouchPressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    // can return true if we are buffering multiple touch events
    // or false if no more data buffered beyond 


    // notes -- if kentec driver expects raw adc rather than pixel coords
    // expected via LVGL, we can transform x and y inside this function 
    // before assigning data->point by casting to a int32_t and scaling by 
    // x * 320/4096 and y * 240/4096
}

// Register input device with LVGL
void touch_driver_init(void){
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, prvTouchReadCb);

}
