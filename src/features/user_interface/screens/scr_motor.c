#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "scr_motor.h"
#include "../screen_manager.h"



#define COL_BG      0x00000000
#define COL_HEADER  0x00220033
#define COL_TEXT    0x00FFFFFF
#define COL_LABEL   0x00999999
#define COL_ACCENT  0x0000CCFF
#define COL_FAULT   0x00FF2200
#define COL_OK      0x0000CC66
#define SCR_W       320
#define SCR_H       240
#define HDR_H       28
#define NAV_H       36
#define NAV_Y       (SCR_H - NAV_H)

static tContext *s_ctx;

// Cached last-drawn values
static float   s_last_rpm     = -1.0f;
static float   s_last_current = -1.0f;
static uint8_t s_last_state   = 0xFF;

static void prv_draw_header(void) {
    tRectangle r = { 0, 0, SCR_W - 1, HDR_H - 1 };
    GrContextForegroundSet(s_ctx, COL_HEADER);
    GrRectFill(s_ctx, &r);
    GrContextForegroundSet(s_ctx, COL_TEXT);
    GrContextFontSet(s_ctx, g_psFontCmss14b);
    GrStringDraw(s_ctx, "Motor", -1, 8, 7, false);
}

static void prv_draw_back_button(void) {
    tRectangle btn = { 2, NAV_Y, 78, SCR_H - 1 };
    GrContextForegroundSet(s_ctx, 0x00223344);
    GrRectFill(s_ctx, &btn);
    GrContextForegroundSet(s_ctx, 0x00336699);
    GrRectDraw(s_ctx, &btn);
    GrContextForegroundSet(s_ctx, COL_TEXT);
    GrContextFontSet(s_ctx, g_psFontCmss12);
    GrStringDraw(s_ctx, "< Home", -1, 12, NAV_Y + 11, false);
}

// Draws a simple horizontal bar gauge
// x, y = top-left  w = total width  h = height
// val = current  max = max value  colour = fill colour
static void prv_draw_bar(int x, int y, int w, int h,
                          float val, float max_val, uint32_t colour) {
    int fill = (int)((val / max_val) * (float)w);
    if (fill < 0)   fill = 0;
    if (fill > w)   fill = w;

    // Background
    tRectangle bg = { x, y, x + w - 1, y + h - 1 };
    GrContextForegroundSet(s_ctx, 0x00111111);
    GrRectFill(s_ctx, &bg);

    // Fill
    if (fill > 0) {
        tRectangle fg = { x, y, x + fill - 1, y + h - 1 };
        GrContextForegroundSet(s_ctx, colour);
        GrRectFill(s_ctx, &fg);
    }

    // Border
    GrContextForegroundSet(s_ctx, 0x00444444);
    GrRectDraw(s_ctx, &bg);
}

void scr_motor_init(tContext *ctx) {
    s_ctx = ctx;
    s_last_rpm = s_last_current = -1.0f;
    s_last_state = 0xFF;
}

void scr_motor_draw(void) {
    tRectangle full = { 0, 0, SCR_W - 1, SCR_H - 1 };
    GrContextForegroundSet(s_ctx, COL_BG);
    GrRectFill(s_ctx, &full);
    prv_draw_header();
    prv_draw_back_button();

    // Draw static labels
    GrContextFontSet(s_ctx, g_psFontCmss12);
    GrContextForegroundSet(s_ctx, COL_LABEL);
    GrStringDraw(s_ctx, "Speed (RPM)", -1, 8, 42, false);
    GrStringDraw(s_ctx, "Current (A)", -1, 8, 110, false);
    GrStringDraw(s_ctx, "State",       -1, 8, 172, false);

    // Force update to paint values
    s_last_rpm = s_last_current = -1.0f;
    s_last_state = 0xFF;
    scr_motor_update();
}

void scr_motor_update(void) {
    char buf[24];
    UiData_t snap;

    UI_LOCK();
    snap.motor_rpm          = g_ui_data.motor_rpm;
    snap.motor_current_amps = g_ui_data.motor_current_amps;
    snap.motor_state        = g_ui_data.motor_state;
    UI_UNLOCK();

    // RPM value + bar (max 3000 RPM)
    if (snap.motor_rpm != s_last_rpm) {
        // Clear old text area
        tRectangle r = { 8, 58, 200, 82 };
        GrContextForegroundSet(s_ctx, COL_BG);
        GrRectFill(s_ctx, &r);

        snprintf(buf, sizeof(buf), "%.0f", snap.motor_rpm);
        GrContextFontSet(s_ctx, g_psFontCmss24b);
        GrContextForegroundSet(s_ctx, COL_TEXT);
        GrStringDraw(s_ctx, buf, -1, 8, 58, false);

        prv_draw_bar(8, 90, 300, 14, snap.motor_rpm, 3000.0f, COL_ACCENT);
        s_last_rpm = snap.motor_rpm;
    }

    // Current value + bar (max 20 A)
    if (snap.motor_current_amps != s_last_current) {
        tRectangle r = { 8, 126, 200, 150 };
        GrContextForegroundSet(s_ctx, COL_BG);
        GrRectFill(s_ctx, &r);

        snprintf(buf, sizeof(buf), "%.2f A", snap.motor_current_amps);
        GrContextFontSet(s_ctx, g_psFontCmss24b);
        GrContextForegroundSet(s_ctx, COL_TEXT);
        GrStringDraw(s_ctx, buf, -1, 8, 126, false);

        uint32_t bar_col = (snap.motor_current_amps > 15.0f) ? COL_FAULT : COL_OK;
        prv_draw_bar(8, 158, 300, 10, snap.motor_current_amps, 20.0f, bar_col);
        s_last_current = snap.motor_current_amps;
    }

    // State string
    if (snap.motor_state != s_last_state) {
        tRectangle r = { 8, 188, 250, 200 };
        GrContextForegroundSet(s_ctx, COL_BG);
        GrRectFill(s_ctx, &r);

        static const char *state_str[] = { "IDLE", "RUNNING", "FAULT" };
        static const uint32_t state_col[] = { COL_LABEL, COL_OK, COL_FAULT };
        uint8_t st = (snap.motor_state < 3) ? snap.motor_state : 2;

        GrContextFontSet(s_ctx, g_psFontCmss14b);
        GrContextForegroundSet(s_ctx, state_col[st]);
        GrStringDraw(s_ctx, state_str[st], -1, 8, 188, false);
        s_last_state = snap.motor_state;
    }
}