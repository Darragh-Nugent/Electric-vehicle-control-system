#include <stdlib.h>
#include <stdint.h>
#include "lvgl.h"
#include "grlib.h"
#include "drivers/Kentec320x240x16_ssd2119_spi.h"
#include "display.h"
extern tContext g_sContext;

static uint16_t line_buf[DISP_BUF_PIXELS]; 

// NOTE:
// line_buf assumes LVGL partial rendering with buffer size = DISP_BUF_WIDTH * DISP_BUF_LINES.
// LVGL guarantees flush areas will not exceed this size.
// If you change buffer size, render mode, or resolution, this may overflow.
// Update line_buf or implement chunking if that happens.
void disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    uint16_t *src = (uint16_t *)px_map;
    const tDisplay *pDisplay = g_sContext.psDisplay;

    int32_t width = area->x2 - area->x1 + 1;
    int32_t height = area->y2 - area->y1 + 1;
    int32_t total_pixels = width * height;

    for (int32_t i = 0; i < total_pixels; i++)
    {
        uint16_t px = src[i];
        line_buf[i] = px; 
    }

    // Send row by row WITHOUT reallocating
    uint16_t *buf_ptr = line_buf;

    for (int32_t y = area->y1; y <= area->y2; y++)
    {
        pDisplay->pfnPixelDrawMultiple(
            pDisplay->pvDisplayData,
            area->x1,
            y,
            area->x1,
            width,
            16,
            (uint8_t *)buf_ptr,
            NULL);

        buf_ptr += width;
    }

    lv_display_flush_ready(disp);
}