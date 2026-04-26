#include "lvgl.h"
#include <stdbool.h>
#include <stdint.h>
#include "grlib/grlib.h"  // must be included before widget.h for trectangle stuff
#include "grlib/widget.h"


void TouchCallBack(uint32_t ui32Message, int32_t i32X, int32_t i32Y);
void touch_driver_init(void);