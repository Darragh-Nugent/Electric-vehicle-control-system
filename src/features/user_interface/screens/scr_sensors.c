
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "scr_sensors.h"


#define COL_BG     0x00000000
#define COL_TEXT   0x00FFFFFF
#define COL_LABEL  0x00999999
#define COL_HDR    0x00003322
#define SCR_W      320
#define SCR_H      240
#define HDR_H      28

static tContext *s_ctx;
static float s_last[3] = { -999.0f, -999.0f, -999.0f };

static void prv_draw_sensor_row(int idx, float val) {
    char label[16], buf[24];
    snprintf(label, sizeof(label), "Sensor %c", 'A' + idx);
    snprintf(buf,   sizeof(buf),   "%.2f", val);

    int y = HDR_H + 10 + idx * 64;

    tRectangle r = { 0, y, SCR_W - 1, y + 58 };
    GrContextForegroundSet(s_ctx, COL_BG);
    GrRectFill(s_ctx, &r);

    GrContextFontSet(s_ctx, g_psFontCmss12);
    GrContextForegroundSet(s_ctx, COL_LABEL);
    GrStringDraw(s_ctx, label, -1, 8, y + 4, false);

    GrContextFontSet(s_ctx, g_psFontCmss24b);
    GrContextForegroundSet(s_ctx, COL_TEXT);
    GrStringDraw(s_ctx, buf, -1, 8, y + 24, false);

    GrContextForegroundSet(s_ctx, 0x00222222);
    GrLineDrawH(s_ctx, 0, SCR_W - 1, y + 58);
}

void scr_sensors_init(tContext *ctx) {
    s_ctx = ctx;
    s_last[0] = s_last[1] = s_last[2] = -999.0f;
}

void scr_sensors_draw(void) {
    tRectangle full = { 0, 0, SCR_W - 1, SCR_H - 1 };
    GrContextForegroundSet(s_ctx, COL_BG);
    GrRectFill(s_ctx, &full);

    tRectangle hdr = { 0, 0, SCR_W - 1, HDR_H - 1 };
    GrContextForegroundSet(s_ctx, COL_HDR);
    GrRectFill(s_ctx, &hdr);
    GrContextForegroundSet(s_ctx, COL_TEXT);
    GrContextFontSet(s_ctx, g_psFontCmss14b);
    GrStringDraw(s_ctx, "Sensors", -1, 8, 7, false);

    s_last[0] = s_last[1] = s_last[2] = -999.0f;
    scr_sensors_update();
}

void scr_sensors_update(void) {
    float vals[3];
    UI_LOCK();
    vals[0] = g_ui_data.sensor_a;
    vals[1] = g_ui_data.sensor_b;
    vals[2] = g_ui_data.sensor_c;
    UI_UNLOCK();

    for (int i = 0; i < 3; i++) {
        if (vals[i] != s_last[i]) {
            prv_draw_sensor_row(i, vals[i]);
            s_last[i] = vals[i];
        }
    }
}