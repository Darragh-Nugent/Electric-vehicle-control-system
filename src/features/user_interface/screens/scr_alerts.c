#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "scr_alerts.h"
#include "grlib.h"


#define COL_BG     0x00000000
#define COL_TEXT   0x00FFFFFF
#define COL_HDR    0x00330000
#define COL_FAULT  0x00FF2200
#define COL_OK     0x0000CC66
#define SCR_W      320
#define SCR_H      240
#define HDR_H      28

static tContext  *s_ctx;
static uint32_t   s_last_faults = 0xFFFFFFFF;

// Fault bit definitions 
static const struct { uint32_t bit; const char *label; } k_faults[] = {
    { 0x01, "xx" },
    { 0x02, "xxx"    },
    { 0x04, "xxxx"},
    { 0x08, "xxxxxx"},
    { 0x10, "xxxxx"   },
};
#define FAULT_COUNT  (sizeof(k_faults) / sizeof(k_faults[0]))

void scr_alerts_init(tContext *ctx) {
    s_ctx = ctx;
    s_last_faults = 0xFFFFFFFF;
}

void scr_alerts_draw(void) {
    tRectangle full = { 0, 0, SCR_W - 1, SCR_H - 1 };
    GrContextForegroundSet(s_ctx, COL_BG);
    GrRectFill(s_ctx, &full);

    tRectangle hdr = { 0, 0, SCR_W - 1, HDR_H - 1 };
    GrContextForegroundSet(s_ctx, COL_HDR);
    GrRectFill(s_ctx, &hdr);
    GrContextForegroundSet(s_ctx, COL_TEXT);
    GrContextFontSet(s_ctx, g_psFontCmss14b);
    GrStringDraw(s_ctx, "Alerts", -1, 8, 7, false);

    s_last_faults = 0xFFFFFFFF;
    scr_alerts_update();
}

void scr_alerts_update(void) {
    uint32_t faults;
    UI_LOCK();
    faults = g_ui_data.fault_flags;
    UI_UNLOCK();

    if (faults == s_last_faults) return;
    s_last_faults = faults;

    // Clear content area
    tRectangle r = { 0, HDR_H, SCR_W - 1, SCR_H - 1 };
    GrContextForegroundSet(s_ctx, COL_BG);
    GrRectFill(s_ctx, &r);

    if (faults == 0) {
        GrContextFontSet(s_ctx, g_psFontCmss18b);
        GrContextForegroundSet(s_ctx, COL_OK);
        GrStringDraw(s_ctx, "No active faults", -1, 20, 100, false);
        return;
    }

    // List active faults
    GrContextFontSet(s_ctx, g_psFontCmss14);
    int y = HDR_H + 10;
    for (size_t i = 0; i < FAULT_COUNT && y < SCR_H - 20; i++) {
        if (faults & k_faults[i].bit) {
            GrContextForegroundSet(s_ctx, COL_FAULT);
            GrStringDraw(s_ctx, "! ", -1, 8, y, false);
            GrContextForegroundSet(s_ctx, COL_TEXT);
            GrStringDraw(s_ctx, k_faults[i].label, -1, 28, y, false);
            y += 26;
        }
    }
}