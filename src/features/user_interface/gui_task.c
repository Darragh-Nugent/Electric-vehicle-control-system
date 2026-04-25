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

#include "motorlib.h"
#include "features/priorities.h"

#include "drivers/Kentec320x240x16_ssd2119_spi.h"
#include "drivers/touch.h"
#include "grlib/widget.h"

#include "semphr.h"

#include "screens/scr_dashboard.h"
#include "screens/scr_motor.h"
#include "screens/scr_sensors.h"
#include "screens/scr_alerts.h"

#include "features/data.h"
#include "timers.h"
#include "gui_task.h"
#include "lvgl.h"

//*****************************************************************************
//
// Gloal variable used to store the frequency of the system clock.
//
//*****************************************************************************
uint32_t g_ui32SysClock;
// -> possibly need to change based on other branches/files

// Queue handle (producers include data.h and use this queue)
QueueHandle_t g_ui_queue;

// Internal state
typedef enum
{
    SCREEN_DASHBOARD = 0,
    SCREEN_MOTOR,
    SCREEN_SENSORS,
    SCREEN_ALERTS,
} ActiveScreen_t;

static ActiveScreen_t active_screen = SCREEN_DASHBOARD;

static volatile int16_t i16Touch_X, i16Touch_Y;
static volatile bool boolTouchPressed;

void vCreateGuiTask(void);
static void prvGuiTask(void *pvParameters);
void TouchCallBack(uint32_t, uint32_t, uint32_t);

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
    lv_tick_inc(5);
}

statric void prvDispatchMsg(const UiMsg_t *msg)
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

    case UI_MSG_MOTOR_STATE:
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
static void prvDrainQueue(void){
    UiMsg_t msg;
    uint8_t safeguard = 16; // may change if it seems like its skipping alot
    while (safeguard-- && xQueueReceive(g_ui_queue, &msg, 0) == pdTRUE){
        prvDispatchMsg(&msg);
    }
}

void vCreateGuiTask(void)
{
    xTaskCreate(
        prvGuiTask,
        "GuiTask",
        256,
        NULL,
        GUI_PRIORITY,
        NULL);
}

void prvGuiInit(void)
{
    //
    // The FPU should be enabled because some compilers will use floating-
    // point registers, even for non-floating-point code.  If the FPU is not
    // enabled this will cause a fault.  This also ensures that floating-
    // point operations could be added to this application and would work
    // correctly and use the hardware floating-point unit.  Finally, lazy
    // stacking is enabled for interrupt handlers.  This allows floating-
    // point instructions to be used within interrupt handlers, but at the
    // expense of extra stack usage.
    //
    FPUEnable();
    FPULazyStackingEnable();

    //
    // Run from the PLL at 120 MHz.
    // Note: SYSCTL_CFG_VCO_240 is a new setting provided in TivaWare 2.2.x and
    // later to better reflect the actual VCO speed due to SYSCTL#22.
    //
    g_ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                             SYSCTL_OSC_MAIN |
                                             SYSCTL_USE_PLL |
                                             SYSCTL_CFG_VCO_240),
                                            120000000);

    //
    // Initialize the display driver.
    //
    Kentec320x240x16_SSD2119Init(g_ui32SysClock);

    //
    // Configure and enable uDMA -> add in if performance is required
    //
    // SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    // SysCtlDelay(10);
    // uDMAControlBaseSet(&psDMAControlTable[0]);
    // uDMAEnable();

    //
    // Initialize the touch screen driver and have it route its messages to the
    // widget tree.
    //
    TouchScreenInit(g_ui32SysClock);
    TouchScreenCallbackSet(TouchCallBack);
}

void prvGuiTask(void *pvParameters)
{
    g_ui_queue = xQueueCreate(UI_QUEUE_DEPTH, sizeof(UiMsg_t))

    lv_init();
    prvGuiInit();
    scr_dashboard_init();
    scr_motor_init();
    scr_sensors_init();
    scr_alerts_init();

    lv_screen_load(dashboard_get());
    active_screen = SCREEN_DASHBOARD;

    // Software timer for LVGL
    TimerHandle_t xTickTimer = xTimerCreate(
        "LvTick",
        pdMS_TO_TICKS(5),
        pdTRUE,
        NULL,
        prvLvglTickCb,
    )

    xTimerStart(xTickTimer, portMAX_DELAY);

    for (;;){
        // Consume all pending data updates
        prvDrainQueue();

        // Lvgl rendering/timers
        uint32_t delay_ms = lv_timer_handler();

        // Clamp just to prevent UI sleeping for way too long
        if (delay < 1) delay_ms = 1;
        if (delay > 20) delay_ms = 20;

        // Wait
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}

/// @brief Handles touchscreen events and stores the current touch state
/// @param ui32Message
/// @param i32X
/// @param i32Y
void TouchCallBack(uint32_t ui32Message, int32_t i32X, int32_t i32Y)
{
    // Finger touches the screen
    if (ui32Message == WIDGET_MSG_PTR_DOWN)
    {
        boolTouchPressed = true;
    }
    // Fringer drags across the screen
    else if (ui32Message == WIDGET_MSG_PTR_MOVE && boolTouchPressed)
    {
        i16Touch_X = i32X;
        i16Touch_Y = i32Y;
    }
    // Finger is lifted
    else if (ui32Message == WIDGET_MSG_PTR_UP)
    {
        boolTouchPressed = false;
    }
}