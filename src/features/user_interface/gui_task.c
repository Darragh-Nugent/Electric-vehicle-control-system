#include "driverlib/pin_map.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "drivers/rtos_hw_drivers.h"
#include "utils/uartstdio.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driver_lib/udma.h"

#include "driverlib/rom_map.h"
#include "motorlib.h"
#include "features/priorities.h"

#include "grlib/grlib.h"
#include "drivers/Kentec320x240x16_ssd2119_spi.h"
#include "drivers/touch.h"
#include "grlib/widget.h"

#include "driverlib/fpu.h"
#include "semphr.h"

#include "display.h"
#include "screens/scr_dashboard.h"
#include "screens/scr_motor.h"
#include "screens/scr_sensors.h"
#include "screens/scr_alerts.h"
#include "screen_manager.h"
#include "touch_driver.h"

#include "features/data.h"
#include "timers.h"
#include "gui_task.h"
#include "lvgl.h"
#include "grlib.h"

#define GUI_TICK 5

//*****************************************************************************
//
// Gloal variable used to store the frequency of the system clock.
//
//*****************************************************************************
extern volatile uint32_t g_ui32SysClock;

// Queue handle (producers include data.h and use this queue)
QueueHandle_t g_ui_queue;

tContext g_sContext;
static lv_display_t *my_display;
static lv_color_t draw_buf1[DISP_BUF_PIXELS]; // 20 line buffer
static lv_color_t draw_buf2[DISP_BUF_PIXELS]; // 20 line buffer

void vCreateGuiTask(void);
static void prvGuiTask(void *pvParameters);
void prvGuiHardwareInit(void);
//*****************************************************************************
//
// Tick Timer callback (This is required for LVGL)
// Runs from RTOS timer context (timer task) not the LVGL task
//
//*****************************************************************************

static void prvLvglTickCb(TimerHandle_t xTimer)
{
    (void)xTimer;
    // thread safe no mutex is required
    lv_tick_inc(GUI_TICK);
}

static void prvDispatchMsg(const UiMsg_t *msg)
{
    switch (msg->type)
    {

    // Motor data — update motor screen; dashboard shows summary
    case UI_MSG_MOTOR_RPM:
        scr_motor_set_rpm(msg->payload.f);
        scr_dashboard_set_rpm(msg->payload.f); // may need this for other motor
        break;

    case UI_MSG_MOTOR_CURRENT:
        scr_motor_set_current(msg->payload.f);
        break;

    case UI_MSG_MOTOR_IDLE:
        scr_motor_set_state((uint8_t)msg->payload.u);
        scr_dashboard_set_motor_state((uint8_t)msg->payload.u);
        break;
    case UI_MSG_MOTOR_RUNNING:
        scr_motor_set_state((uint8_t)msg->payload.u);
        scr_dashboard_set_motor_state((uint8_t)msg->payload.u);
        break;
    // Sensor data
    case UI_MSG_SENSOR_A:
        scr_sensors_set_value(0, msg->payload.f);
        scr_dashboard_set_sensor(0, msg->payload.f);
        break;
    case UI_MSG_SENSOR_B:
        scr_sensors_set_value(1, msg->payload.f);
        scr_dashboard_set_sensor(1, msg->payload.f);
        break;
    case UI_MSG_SENSOR_C:
        scr_sensors_set_value(2, msg->payload.f);
        scr_dashboard_set_sensor(2, msg->payload.f);
        break;

    // Faults — always visible regardless of active screen
    case UI_MSG_FAULT_RAISED:
        scr_alerts_raise(msg->payload.u);
        scr_dashboard_show_fault_banner(true);
        break;
    case UI_MSG_FAULT_CLEARED:
        scr_alerts_clear(msg->payload.u);
        scr_dashboard_show_fault_banner(false);
        break;

    default:
        break;
    }
}

// Drain the Queue and prevents sensors to flood the queue to prevent
// old values
static void prvDrainQueue(void)
{
    UiMsg_t msg;
    uint8_t safeguard = 16; // may change if it seems like its skipping alot
    while (safeguard-- && xQueueReceive(g_ui_queue, &msg, 0) == pdTRUE)
    {
        prvDispatchMsg(&msg);
    }
}

void display_init(void)
{
    my_display = lv_display_create(DISP_RES_WIDTH, DISP_RES_HEIGHT);

    lv_display_set_flush_cb(my_display, disp_flush);

    lv_display_set_buffers(my_display,
                           draw_buf1,
                           draw_buf2,
                           sizeof(draw_buf1),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
}

void vCreateGuiTask(void)
{
    g_ui_queue = xQueueCreate(UI_QUEUE_DEPTH, sizeof(UiMsg_t));

    prvGuiHardwareInit();
    BaseType_t ret = xTaskCreate(
        prvGuiTask,
        "GuiTask",
        4096,
        NULL,
        GUI_PRIORITY,
        NULL);

    configASSERT(ret == pdPASS);
    (void)ret;
}

void prvGuiHardwareInit(void)
{
    GrContextInit(&g_sContext, &g_sKentec320x240x16_SSD2119);
    GrContextForegroundSet(&g_sContext, ClrLightGreen);
    GrContextBackgroundSet(&g_sContext, ClrBlack);
}

void prvGuiTask(void *pvParameters)
{
    lv_init();
    display_init();
    touch_driver_init();
    screen_manager_init();
    // Software timer for LVGL
    TimerHandle_t xTickTimer = xTimerCreate(
        "LvTick",
        pdMS_TO_TICKS(GUI_TICK),
        pdTRUE,
        NULL,
        prvLvglTickCb);

    xTimerStart(xTickTimer, portMAX_DELAY);
    for (;;)
    {
        // Consume all pending data updates
        prvDrainQueue();

        // Lvgl rendering/timers
        uint32_t delay_ms = lv_timer_handler();

        // Clamp just to prevent UI sleeping for way too long
        if (delay_ms < 1)
            delay_ms = 1;
        if (delay_ms > 10)
            delay_ms = 10;

        // Wait -> allow other tasks to run
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}
