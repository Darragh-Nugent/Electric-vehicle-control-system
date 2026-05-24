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
#include "features/motor/motor_api.h"

#include "features/data.h"
#include "timers.h"
#include "gui_task.h"
#include "lvgl.h"
#include "semphr.h"

#define GUI_TICK 5

//*****************************************************************************
//
// Gloal variable used to store the frequency of the system clock.
//
//*****************************************************************************
extern volatile uint32_t g_ui32SysClock;

extern SemaphoreHandle_t motorStartSemaphore;

UiMsg_t g_ui_state;

tContext g_sContext;
static lv_display_t *my_display;
static lv_color_t draw_buf1[DISP_BUF_PIXELS]; // 20 line buffer
static lv_color_t draw_buf2[DISP_BUF_PIXELS]; // 20 line buffer
sensorThresholds_t g_thresholds;

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

// GUI uses an internal queue, which is populated via the screens through user input and consumed here in the task.
// - side note - relevant getters and setters for motor/sensors are called via api functions depending on which screen is currently displayed (not implemented yet)
static void prvDispatchMsg(const UiMsg_t *msg)
{
    switch (msg->type)
    {

    // Motor data — update motor screen; dashboard shows summary
    case UI_MSG_MOTOR_RPM:
        motorSetSpeed(msg->payload.u);
        UARTprintf("MOTOR: SETTING RPM: %d\n", msg->payload.u);
        break;

    case UI_MSG_MOTOR_CURRENT:
        scr_motor_set_current(msg->payload.u); // is this needed?
        break;
    case UI_MSG_MOTOR_IDLE:
        motorSetState(msg->payload.u);
        UARTprintf("MOTOR: SETTING STATE TO IDLE\n");
        break;
    case UI_MSG_MOTOR_STARTING:
        // motorSetState(msg->payload.u);
        UARTprintf("MOTOR: SETTING STATE TO STARTING\n");
        break;
    case UI_MSG_MOTOR_RUNNING:
        motorSetState(msg->payload.u);
        xSemaphoreGive(motorStartSemaphore);
        UARTprintf("MOTOR: SETTING STATE TO RUNNING\n");
        break;
    case UI_MSG_MOTOR_BREAKING:
        motorSetState(msg->payload.u);
        UARTprintf("MOTOR: SETTING STATE TO BREAKING\n");
        break;
    // Sensor data
    case UI_MSG_SENSOR_UPDATE_POWER:
        // Sensor_UpdatePower(msg->payload.u);
        g_thresholds.TH_POWER = msg->payload.u;
        UARTprintf("SENSOR: UPDATING POWER: %d\n", msg->payload.u);
        break;
    case UI_MSG_SENSOR_UPDATE_ACCELERATION:
        // Sensor_UpdateAccel(msg->payload.u);
        g_thresholds.TH_ACCEL = msg->payload.u;
        UARTprintf("SENSOR: UPDATING ACCELERATION: %d\n", msg->payload.u);
        break;
    case UI_MSG_SENSOR_UPDATE_DISTANCE:
        // Sensor_UpdateDistance(msg->payload.u);
        g_thresholds.TH_DIST = msg->payload.u;
        UARTprintf("SENSOR: UPDATING DISTANCE: %d\n", msg->payload.u);
        break;
    case UI_MSG_SENSOR_UPDATE_HUMIDITY:
        // Sensor_UpdateHumidity(msg->payload.u);
        UARTprintf("SENSOR: UPDATING HUMIDITY: %d\n", msg->payload.u);
        break;
    case UI_MSG_SENSOR_UPDATE_TEMP:
        g_thresholds.TH_TEMP = msg->payload.u;
        // Sensor_UpdateTemp(msg->payload.u);
        UARTprintf("SENSOR: UPDATING TEMP: %d\n", msg->payload.u);
        break;
    case UI_MSG_SENSOR_UPDATE_LUX:
        // Sensor_UpdateLux(msg->payload.u);
        UARTprintf("SENSOR: UPDATING LUX: %d\n", msg->payload.u);
        break;
    // Faults — always visible regardless of active screen
    case UI_MSG_FAULT_RAISED:
        screen_manager_goto(SCREEN_ALERT);
        break;
    case UI_MSG_FAULT_CLEARED:
        g_ui_state.type = UI_MSG_STATE_NONE;
        motorAcknowledgeFault();
        break;

    default:
        break;
    }
    // Reset state, avoid processing stale data
    if (g_ui_state.type != UI_MSG_FAULT_RAISED)
        g_ui_state.type = UI_MSG_STATE_NONE;
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
    UARTprintf("Inside create gui\n");
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
        motor_state_t state = motorGetState();//MOTOR_STATE_FAULT; // portmax delay, be careful this doesnt delay the UI
        if (state == MOTOR_STATE_FAULT)
            g_ui_state.type = UI_MSG_FAULT_RAISED;
        prvDispatchMsg(&g_ui_state);

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
