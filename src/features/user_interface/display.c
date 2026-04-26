#include "lvgl.h"
#include "grlib/grlib.h"
#include "drivers/Kentec320x240x16_ssd2119_spi.h"

extern const tDisplay g_sKentec320x240x16_SSD2119;

void disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    // 1. Calculate how many pixels we are drawing
    uint32_t width = (area->x2 - area->x1 + 1);
    uint32_t height = (area->y2 - area->y1 + 1);

    // 2. Use the existing GrLib driver function to send the pixels
    // We use the pointer-based function inside the tDisplay struct
    for (int y = 0; y < height; y++)
    {
        g_sKentec320x240x16_SSD2119.pfnPixelDrawMultiple(
            g_sKentec320x240x16_SSD2119.pvDisplayData,
            area->x1, area->y1 + y, // X, Y start
            0,                  // No offsets
            width,              // Number of pixels to draw
            16,                 // Bits per pixel (RGB565)
            px_map,             // The pixel data from LVGL
            0                   // No palette
        );
    }
    // 3. IMPORTANT: Tell LVGL the hardware transfer is finished
    lv_display_flush_ready(disp);
}
