// // src/screens/scr_dashboard.c
// //
// // Dashboard panel — single APP_DRAWN canvas covering the content area.
// //
// //   - All drawing happens inside OnDashboardPaint(), called by the
// //     widget system on the GUI task's WidgetMessageQueueProcess() call.
// //   - Reads g_ui_data under mutex, snapshots, releases, then draws.

// #include "scr_dashboard.h"
// #include "../../data.h"
// #include "grlib/grlib.h"
// #include "grlib/widget.h"
// #include "grlib/canvas.h"
// #include "utils/ustdlib.h"
// #include "drivers/Kentec320x240x16_ssd2119_spi.h"

// // ── Colours ───────────────────────────────────────────────────────────────
// // Using GrLib named constants from grlib/grlib.h
// #define COL_BG          ClrBlack
// #define COL_HEADER_BG   ClrDarkBlue
// #define COL_HEADER_TEXT ClrWhite
// #define COL_LABEL       ClrSilver
// #define COL_VALUE       ClrWhite
// #define COL_ACCENT      ClrSkyBlue
// #define COL_OK          ClrGreen
// #define COL_FAULT       ClrRed
// #define COL_WARN        ClrYellow

// // ── Layout ────────────────────────────────────────────────────────────────
// // The canvas covers y=24 to y=189 (166px tall), matching the reference.
// // x=0 to x=319 full width.
// #define PANEL_X     0
// #define PANEL_Y     24
// #define PANEL_W     320
// #define PANEL_H     166

// // Row layout inside the panel (y coords relative to screen, not panel)
// #define ROW1_Y      36      // Motor RPM
// #define ROW2_Y      90      // Motor current
// #define ROW3_Y      144     // Status

// // Bar gauge geometry
// #define BAR_X       10
// #define BAR_W       280
// #define BAR_H       12

// // ── Widget declaration ────────────────────────────────────────────────────
// // APP_DRAWN: widget system calls OnDashboardPaint() to render content.
// // Parent = NULL (added to tree by WidgetAdd in main.c).
// Canvas(g_sDashboardPanel,   // widget variable name
//        NULL,                // parent (set when added to tree)
//        NULL,                // next sibling
//        NULL,                // first child
//        &g_sKentec320x240x16_SSD2119,
//        PANEL_X, PANEL_Y,
//        PANEL_W, PANEL_H,
//        CANVAS_STYLE_APP_DRAWN,
//        COL_BG,              // fill colour (used if CANVAS_STYLE_FILL set)
//        0, 0, 0, 0, 0,
//        OnDashboardPaint);   // paint callback

// // ── Private helpers ───────────────────────────────────────────────────────

// // Draw a horizontal bar gauge.
// // filled_w = number of pixels to fill (pre-computed by caller).
// static void prv_draw_bar(tContext *ctx,
//                           int16_t x, int16_t y,
//                           int16_t total_w, int16_t h,
//                           int16_t filled_w,
//                           uint32_t fill_col) {
//     tRectangle r;

//     // Background
//     r.i16XMin = x;
//     r.i16YMin = y;
//     r.i16XMax = x + total_w - 1;
//     r.i16YMax = y + h - 1;
//     GrContextForegroundSet(ctx, ClrDarkGray);
//     GrRectFill(ctx, &r);

//     // Fill
//     if (filled_w > 0) {
//         r.i16XMax = x + filled_w - 1;
//         GrContextForegroundSet(ctx, fill_col);
//         GrRectFill(ctx, &r);
//     }

//     // Border
//     r.i16XMin = x;
//     r.i16XMax = x + total_w - 1;
//     GrContextForegroundSet(ctx, ClrGray);
//     GrRectDraw(ctx, &r);
// }

// // ── Paint callback ────────────────────────────────────────────────────────
// // Called by WidgetMessageQueueProcess() — runs in the GUI task context.
// // The widget system clips drawing to the canvas bounds automatically.

// void OnDashboardPaint(tWidget *psWidget, tContext *psContext) {
//     char buf[24];
//     UiData_t snap;
//     tRectangle r;

//     // ── 1. Snapshot shared data ───────────────────────────────────────
//     xSemaphoreTake(g_ui_mutex, portMAX_DELAY);
//     snap.motor_rpm     = g_ui_data.motor_rpm;
//     snap.motor_current = g_ui_data.motor_current;
//     snap.motor_state   = g_ui_data.motor_state;
//     snap.fault_flags   = g_ui_data.fault_flags;
//     xSemaphoreGive(g_ui_mutex);

//     // ── 2. Clear canvas ───────────────────────────────────────────────
//     r.i16XMin = PANEL_X;
//     r.i16YMin = PANEL_Y;
//     r.i16XMax = PANEL_X + PANEL_W - 1;
//     r.i16YMax = PANEL_Y + PANEL_H - 1;
//     GrContextForegroundSet(psContext, COL_BG);
//     GrRectFill(psContext, &r);

//     // ── 3. Motor RPM ──────────────────────────────────────────────────
//     GrContextFontSet(psContext, &g_sFontCm12);
//     GrContextForegroundSet(psContext, COL_LABEL);
//     GrStringDraw(psContext, "Motor Speed", -1, BAR_X, ROW1_Y, false);

//     // Format: cast to int
//     int32_t irpm = (int32_t)snap.motor_rpm;
//     usprintf(buf, "%d RPM", irpm);

//     GrContextFontSet(psContext, &g_sFontCm20);
//     GrContextForegroundSet(psContext, COL_VALUE);
//     GrStringDraw(psContext, buf, -1, BAR_X, ROW1_Y + 16, false);

//     // RPM bar (max 3000?)
//     int16_t rpm_fill = (int16_t)((snap.motor_rpm / 3000.0f) * BAR_W);
//     if (rpm_fill > BAR_W) rpm_fill = BAR_W;
//     if (rpm_fill < 0)     rpm_fill = 0;
//     prv_draw_bar(psContext, BAR_X, ROW1_Y + 42, BAR_W, BAR_H,
//                  rpm_fill, COL_ACCENT);

//     // ── 4. Motor current ──────────────────────────────────────────────
//     GrContextFontSet(psContext, &g_sFontCm12);
//     GrContextForegroundSet(psContext, COL_LABEL);
//     GrStringDraw(psContext, "Current", -1, BAR_X, ROW2_Y, false);

//     // 7.35 A → display as "7.3 A"
//     int32_t amps_whole = (int32_t)snap.motor_current;
//     int32_t amps_frac  = (int32_t)((snap.motor_current - (float)amps_whole) * 10.0f);
//     usprintf(buf, "%d.%d A", amps_whole, amps_frac);

//     GrContextFontSet(psContext, &g_sFontCm20);
//     uint32_t cur_col = (snap.motor_current > 8.0f) ? COL_WARN : COL_VALUE;
//     if (snap.motor_current > 12.0f) cur_col = COL_FAULT;
//     GrContextForegroundSet(psContext, cur_col);
//     GrStringDraw(psContext, buf, -1, BAR_X, ROW2_Y + 16, false);

//     // Current bar (max 15 A)
//     int16_t cur_fill = (int16_t)((snap.motor_current / 15.0f) * BAR_W);
//     if (cur_fill > BAR_W) cur_fill = BAR_W;
//     if (cur_fill < 0)     cur_fill = 0;
//     prv_draw_bar(psContext, BAR_X, ROW2_Y + 42, BAR_W, BAR_H,
//                  cur_fill, cur_col);

//     // ── 5. Status row ─────────────────────────────────────────────────
//     GrContextFontSet(psContext, &g_sFontCm12);
//     GrContextForegroundSet(psContext, COL_LABEL);
//     GrStringDraw(psContext, "Status", -1, BAR_X, ROW3_Y, false);

//     GrContextFontSet(psContext, &g_sFontCm18);
//     if (snap.fault_flags != 0) {
//         GrContextForegroundSet(psContext, COL_FAULT);
//         GrStringDraw(psContext, "FAULT", -1, BAR_X, ROW3_Y + 16, false);
//     } else {
//         static const char *state_str[] = { "IDLE", "RUNNING", "FAULT" };
//         static const uint32_t state_col[] = { COL_LABEL, COL_OK, COL_FAULT };
//         uint8_t st = (snap.motor_state < 3) ? snap.motor_state : 0;
//         GrContextForegroundSet(psContext, state_col[st]);
//         GrStringDraw(psContext, state_str[st], -1, BAR_X, ROW3_Y + 16, false);
//     }
// }

// // ── Refresh request ───────────────────────────────────────────────────────
// // Called from the GUI task each tick to queue a repaint.
// // WidgetPaint() posts to the widget message queue — it does not draw
// // immediately. Drawing happens on the next WidgetMessageQueueProcess() call.

// void scr_dashboard_refresh(void) {
//     WidgetPaint((tWidget *)&g_sDashboardPanel);
// }