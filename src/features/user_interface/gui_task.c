#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "grlib/grlib.h"
#include "grlib/widget.h"
#include "utils/uartstdio.h"
#include "gui_task.h"
#include "../priorities.h"


static void vGuiTask(void *pvParam) {
    (void)pvParam;

    UARTprintf("GUI task started\n");
    UARTprintf("Free heap before paint: %u bytes\n",
               (unsigned)xPortGetFreeHeapSize());

    // Allow the scheduler and other tasks to fully settle.
    // SPI bus and touch interrupt need a few ms to stabilise
    // after the scheduler takes over from main().
    vTaskDelay(pdMS_TO_TICKS(100));

    // First and only call to WidgetPaint(WIDGET_ROOT).
    // This triggers the full initial render of all widgets.
    // Runs here — on the task stack — which is sized to handle it.
    // All hardware is already initialised so the SPI writes succeed.
    WidgetPaint(WIDGET_ROOT);

    // Confirm stack headroom immediately after the heaviest operation.
    // Target: watermark > 200 words. If below 100, increase task stack.
    UBaseType_t watermark = uxTaskGetStackHighWaterMark(NULL);
    UARTprintf("GUI watermark after paint: %u words (%u bytes)\n",
               (unsigned)watermark, (unsigned)(watermark * sizeof(StackType_t)));

    while (1) {
        // Drain the widget message queue completely each tick.
        // Processes touch events, queued WidgetPaint() calls from
        // callbacks (OnNext, OnButtonPress, etc.), and auto-repeat.
        WidgetMessageQueueProcess();

        // 10ms delay — 100Hz pump rate.
        // Do NOT use 50ms: a button press generates 3-4 messages
        // (PTR_DOWN, PTR_MOVE, PTR_UP, PAINT). At 50ms you drop messages.
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void vCreateGUITask(void) {
    UARTprintf("Creating GUI task\n");
    xTaskCreate(vGuiTask, "GUI", 4096, NULL, GUI_PRIORITY, NULL);
}