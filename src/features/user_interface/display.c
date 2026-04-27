#include <stdlib.h>
#include <stdint.h>
#include "lvgl.h"
#include "grlib.h"
#include "drivers/Kentec320x240x16_ssd2119_spi.h"
extern tContext g_sContext;

void disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    uint16_t *buf16 = (uint16_t *)px_map;  // Pointer to the pixel buffer (RGB565 format)
    const tDisplay *pDisplay = g_sContext.psDisplay;  

    // Iterate over each row (y-axis)
    for (int32_t y = area->y1; y <= area->y2; y++)
    {
        // Calculate how many pixels are in this row
        int32_t pixel_count = area->x2 - area->x1 + 1;  

        // Allocate a temporary buffer to hold the byte-swapped pixel data for the row
        uint8_t row_pixels[pixel_count * 2];  // Each pixel is 2 bytes (RGB565)

        // Prepare the row data by byte-swapping the pixels
        for (int32_t x = 0; x < pixel_count; x++)
        {
            uint16_t px = buf16[x];  // Get the pixel
            uint16_t swapped_px = (px >> 8) | (px << 8);  // Swap bytes (LVGL is little-endian, SSD2119 is big-endian)

            // Store the byte-swapped pixels in the row_pixels buffer
            row_pixels[x * 2] = (swapped_px >> 8) & 0xFF;  // High byte
            row_pixels[x * 2 + 1] = swapped_px & 0xFF;     // Low byte
        }

        // Now, call pfnPixelDrawMultiple to draw the entire row
        pDisplay->pfnPixelDrawMultiple(
            pDisplay->pvDisplayData,  // Display context data
            area->x1,                 // Starting x position
            y,                        // Current y position
            area->x1,                 // Starting x in the row (same as area->x1)
            pixel_count,              // Number of pixels in the row
            16,                       // Bits per pixel (RGB565 is 16bpp)
            row_pixels,               // Pixel data (byte-swapped)
            NULL                      // No palette needed for RGB565
        );

        // Move to the next row in the pixel buffer
        buf16 += pixel_count;
    }

    // Notify LVGL that the display flush is done
    lv_display_flush_ready(disp);
}