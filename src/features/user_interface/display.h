#include "lvgl.h"
#include "grlib/grlib.h"
#include "drivers/Kentec320x240x16_ssd2119_spi.h"

#define DISP_RES_WIDTH 320
#define DISP_RES_HEIGHT 240

#define DISP_BUF_WIDTH   320
#define DISP_BUF_LINES   20
#define DISP_BUF_PIXELS  (DISP_BUF_WIDTH * DISP_BUF_LINES)

void disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);