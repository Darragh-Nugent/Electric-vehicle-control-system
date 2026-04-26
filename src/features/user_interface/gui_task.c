// src/features/user_interface/gui_task.c
//
// The sole FreeRTOS task responsible for:
//   1. GrLib context initialisation
//   2. Screen manager initialisation
//   3. Touch polling and screen switching
//   4. Periodic screen_update() calls
//
// No LVGL. No lv_timer_handler(). No flush callbacks.
#include <stdio.h>
#include <stdlib.h>
#include "gui_task.h"
#include "../data.h"
#include "grlib/grlib.h"
#include "drivers/Kentec320x240x16_ssd2119_spi.h"
#include "drivers/touch.h"                    // TivaWare touchscreen driver
#include "grlib/widget.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <string.h>

void vGuiTask(void *pvParams);
// ── Shared data (defined here, declared extern in ui_shared.h) ────────────
volatile UiData_t g_ui_data;
SemaphoreHandle_t g_ui_mutex;

// ── GrLib context (module-private) ────────────────────────────────────────
static tContext s_context;

// ── Touch state ───────────────────────────────────────────────────────────
// Written by TouchCallBack (interrupt context), read by GUI task.
volatile int16_t  g_touch_x       = 0;
volatile int16_t  g_touch_y       = 0;
volatile bool     g_touch_pressed = false;

// ── Touch callback (registered with TivaWare, called from interrupt) ──────
void TouchCallBack(uint32_t ui32Message, int32_t i32X, int32_t i32Y) {
    switch (ui32Message) {
        case WIDGET_MSG_PTR_DOWN:
        case WIDGET_MSG_PTR_MOVE:
            g_touch_x       = (int16_t)i32X;
            g_touch_y       = (int16_t)i32Y;
            g_touch_pressed = true;
            break;
        case WIDGET_MSG_PTR_UP:
            g_touch_pressed = false;
            break;
        default:
            break;
    }
}

// ── Touch → screen routing ────────────────────────────────────────────────
// Called each GUI tick. Checks touch state and switches screens if a
// navigation button was tapped.

#define NAV_Y    204   // matches scr_dashboard layout constant
#define SCR_H    240
#define SCR_W    320

// ── GUI task ──────────────────────────────────────────────────────────────

void vCreateGuiTask(void){
    xTaskCreate(vGuiTask,    "GUI",    1024, NULL, 2, NULL);
}

void vGuiTask(void *pvParams) {
    (void)pvParams;

    // Shared data mutex
    g_ui_mutex = xSemaphoreCreateMutex();
    configASSERT(g_ui_mutex);
    memset((void *)&g_ui_data, 0, sizeof(g_ui_data));

    GrContextInit(&s_context, &g_sKentec320x240x16_SSD2119);

    // Initialise all screens and show dashboard
    screen_manager_init(&s_context);

    for (;;) {

        prv_handle_touch();
        screen_update();

        // 3. Yield — motor/sensor tasks run here
        // 50ms = 20 Hz GUI refresh rate. Increase to 100ms if motor
        // task needs more CPU; decrease to 20ms for snappier updates.
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}