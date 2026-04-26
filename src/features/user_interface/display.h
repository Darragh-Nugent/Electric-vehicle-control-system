#include "lvgl.h"
#include "scr_ssi.h"
#include "grlib/grlib.h"
#include "drivers/Kentec320x240x16_ssd2119_spi.h"
void disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);