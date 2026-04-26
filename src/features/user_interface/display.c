#include "lvgl.h"
#include "scr_ssi.h"
#include "grlib/grlib.h"
#include "drivers/Kentec320x240x16_ssd2119_spi.h"

void disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    uint32_t width  = area->x2 - area->x1 + 1;
    uint32_t height = area->y2 - area->y1 + 1;

    // 1. Set drawing window on SSD2119
    SSD2119_WriteCommand(SSD2119_H_RAM_START_REG);
    SSD2119_WriteData(area->x1);

    SSD2119_WriteCommand(SSD2119_H_RAM_END_REG);
    SSD2119_WriteData(area->x2);

    SSD2119_WriteCommand(SSD2119_V_RAM_POS_REG);
    SSD2119_WriteData((area->y2 << 8) | area->y1);

    // 2. Set GRAM pointer
    SSD2119_WriteCommand(SSD2119_X_RAM_ADDR_REG);
    SSD2119_WriteData(area->x1);

    SSD2119_WriteCommand(SSD2119_Y_RAM_ADDR_REG);
    SSD2119_WriteData(area->y1);

    SSD2119_WriteCommand(SSD2119_RAM_DATA_REG);

    // 3. Stream pixel data directly
    uint32_t size = width * height;
    uint16_t *pixels = (uint16_t *)px_map;

    for(uint32_t i = 0; i < size; i++)
    {
        SSD2119_WriteCommand(pixels[i]);
    }

    // 4. Tell LVGL done
    lv_display_flush_ready(disp);
}