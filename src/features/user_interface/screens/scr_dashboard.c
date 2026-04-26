// Dashboard — default home screen.
// Shows summary values for all systems and navigation buttons.
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "scr_dashboard.h"
#include "../screen_manager.h"

// ── GrLib colour constants (RGB565 packed as 0x00RRGGBB for GrLib) ────────
#define COL_BG          0x00000000   // black background
#define COL_HEADER      0x00003366   // dark blue header bar
#define COL_TEXT        0x00FFFFFF   // white text
#define COL_LABEL       0x00AAAAAA   // grey label text
#define COL_ACCENT      0x001199FF   // bright blue accent
#define COL_WARN        0x00FF8800   // amber warning
#define COL_FAULT       0x00FF2200   // red fault
#define COL_BTN_BG      0x00223344   // dark button fill
#define COL_BTN_BORDER  0x00336699   // button border

// ── Layout constants ──────────────────────────────────────────────────────
#define SCR_W       320
#define SCR_H       240
#define HDR_H       28
#define NAV_H       36
#define NAV_Y       (SCR_H - NAV_H)  // 204

// Value rows (3 rows between header and nav bar)
#define ROW_H       ((NAV_Y - HDR_H) / 3)   // ~58 px each
#define ROW1_Y      HDR_H
#define ROW2_Y      (HDR_H + ROW_H)
#define ROW3_Y      (HDR_H + ROW_H * 2)

// Nav button widths (4 buttons across 320px with 2px gaps)
#define NAV_BTNW    78
#define NAV_BTN0_X  2
#define NAV_BTN1_X  82
#define NAV_BTN2_X  162
#define NAV_BTN3_X  242

// ── Module state ──────────────────────────────────────────────────────────
static tContext *s_ctx = NULL;

// Cached values — update() only redraws when these change
static float    s_last_rpm     = -1.0f;
static float    s_last_sensor_a = -1.0f;
static uint32_t s_last_faults  = 0xFFFFFFFF;
static uint8_t  s_last_state   = 0xFF;

// ── Private helpers ───────────────────────────────────────────────────────

static void prv_draw_header(void) {
    tRectangle r = { 0, 0, SCR_W - 1, HDR_H - 1 };
    GrContextForegroundSet(s_ctx, COL_HEADER);
    GrRectFill(s_ctx, &r);

    GrContextForegroundSet(s_ctx, COL_TEXT);
    GrContextFontSet(s_ctx, g_psFontCmss14b);
    GrStringDraw(s_ctx, "System Dashboard", -1, 8, 7, false);
}

static void prv_draw_nav_buttons(void) {
    // Static navigation bar — drawn once as part of full draw()
    static const char *labels[] = { "Home", "Motor", "Sensors", "Alerts" };
    int xs[] = { NAV_BTN0_X, NAV_BTN1_X, NAV_BTN2_X, NAV_BTN3_X };

    GrContextFontSet(s_ctx, g_psFontCmss12);

    for (int i = 0; i < 4; i++) {
        tRectangle btn = {
            xs[i], NAV_Y,
            xs[i] + NAV_BTNW - 1, SCR_H - 1
        };

        // Highlight active screen button
        GrContextForegroundSet(s_ctx, (i == 0) ? COL_ACCENT : COL_BTN_BG);
        GrRectFill(s_ctx, &btn);

        GrContextForegroundSet(s_ctx, COL_BTN_BORDER);
        GrRectDraw(s_ctx, &btn);

        GrContextForegroundSet(s_ctx, COL_TEXT);
        // Centre the label inside the button horizontally
        int lw = GrStringWidthGet(s_ctx, labels[i], -1);
        int lx = xs[i] + (NAV_BTNW - lw) / 2;
        GrStringDraw(s_ctx, labels[i], -1, lx, NAV_Y + 11, false);
    }
}

// Draw a labelled value row.
// label : left-aligned small grey text
// value : right-aligned large white text
static void prv_draw_value_row(int row_y, const char *label, const char *value) {
    // Clear the row
    tRectangle r = { 0, row_y, SCR_W - 1, row_y + ROW_H - 2 };
    GrContextForegroundSet(s_ctx, COL_BG);
    GrRectFill(s_ctx, &r);

    // Separator line
    GrContextForegroundSet(s_ctx, COL_BTN_BORDER);
    GrLineDrawH(s_ctx, 0, SCR_W - 1, row_y + ROW_H - 1);

    // Label (small, grey, top-left of row)
    GrContextFontSet(s_ctx, g_psFontCmss12);
    GrContextForegroundSet(s_ctx, COL_LABEL);
    GrStringDraw(s_ctx, label, -1, 8, row_y + 6, false);

    // Value (larger, white, vertically centred in row)
    GrContextFontSet(s_ctx, g_psFontCmss18b);
    GrContextForegroundSet(s_ctx, COL_TEXT);
    int vw = GrStringWidthGet(s_ctx, value, -1);
    GrStringDraw(s_ctx, value, -1, SCR_W - vw - 10, row_y + (ROW_H / 2) - 7, false);
}

// ── Public API ────────────────────────────────────────────────────────────

void scr_dashboard_init(tContext *ctx) {
    s_ctx = ctx;
    // Reset cache so first update() always draws
    s_last_rpm      = -1.0f;
    s_last_sensor_a  = -1.0f;
    s_last_faults   = 0xFFFFFFFF;
    s_last_state    = 0xFF;
}

void scr_dashboard_draw(void) {
    // Full screen clear
    tRectangle full = { 0, 0, SCR_W - 1, SCR_H - 1 };
    GrContextForegroundSet(s_ctx, COL_BG);
    GrRectFill(s_ctx, &full);

    prv_draw_header();
    prv_draw_nav_buttons();

    // Draw all value rows with current data (force full repaint)
    s_last_rpm     = -1.0f;
    s_last_sensor_a = -1.0f;
    s_last_faults  = 0xFFFFFFFF;
    s_last_state   = 0xFF;
    scr_dashboard_update();
}

void scr_dashboard_update(void) {
    char buf[32];
    UiData_t snap;

    // Snapshot shared data under mutex — copy then release immediately
    UI_LOCK();
    snap = (UiData_t){ g_ui_data.motor_rpm,
                       g_ui_data.motor_current_amps,
                       g_ui_data.motor_state,
                       g_ui_data.sensor_a,
                       g_ui_data.sensor_b,
                       g_ui_data.sensor_c,
                       g_ui_data.fault_flags };
    UI_UNLOCK();

    // Row 1: Motor RPM — only redraw if value changed
    if (snap.motor_rpm != s_last_rpm || snap.motor_state != s_last_state) {
        if (snap.motor_state == 2) {
            GrContextForegroundSet(s_ctx, COL_FAULT);
        }
        snprintf(buf, sizeof(buf), "%.0f RPM", snap.motor_rpm);
        prv_draw_value_row(ROW1_Y, "Motor speed", buf);
        s_last_rpm   = snap.motor_rpm;
        s_last_state = snap.motor_state;
    }

    // Row 2: Sensor A
    if (snap.sensor_a != s_last_sensor_a) {
        snprintf(buf, sizeof(buf), "%.1f", snap.sensor_a);
        prv_draw_value_row(ROW2_Y, "Sensor A", buf);
        s_last_sensor_a = snap.sensor_a;
    }

    // Row 3: Fault status
    if (snap.fault_flags != s_last_faults) {
        if (snap.fault_flags == 0) {
            prv_draw_value_row(ROW3_Y, "Status", "OK");
        } else {
            snprintf(buf, sizeof(buf), "FAULT 0x%02X", (unsigned)snap.fault_flags);
            // Override colour for fault row
            GrContextForegroundSet(s_ctx, COL_FAULT);
            GrContextFontSet(s_ctx, g_psFontCmss18b);
            tRectangle r = { 0, ROW3_Y, SCR_W - 1, ROW3_Y + ROW_H - 2 };
            GrContextForegroundSet(s_ctx, COL_BG);
            GrRectFill(s_ctx, &r);
            GrContextForegroundSet(s_ctx, COL_FAULT);
            GrStringDraw(s_ctx, buf, -1, 8, ROW3_Y + (ROW_H / 2) - 7, false);
        }
        s_last_faults = snap.fault_flags;
    }
}