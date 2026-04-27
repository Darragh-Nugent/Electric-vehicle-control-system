#include <stdlib.h>
#include <stdint.h>
#include "lvgl.h"
#include "grlib.h"
#include "drivers/Kentec320x240x16_ssd2119_spi.h"
extern tContext g_sContext;

void disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    uint16_t *buf16 = (uint16_t *)px_map;
    const tDisplay *pDisplay = g_sContext.psDisplay;  // hardware function table

    for (int32_t y = area->y1; y <= area->y2; y++)
    {
        for (int32_t x = area->x1; x <= area->x2; x++)
        {
            uint16_t px = *buf16++;

            // Byte-swap for SSD2119 (big-endian SPI, LVGL is little-endian)
            px = (px >> 8) | (px << 8);

            // Call the driver's pixel function directly through the vtable
            pDisplay->pfnPixelDraw(pDisplay->pvDisplayData, x, y, (uint32_t)px);
        }
    }

    lv_display_flush_ready(disp);
}