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

//*****************************************************************************
//
// Gloal variable used to store the frequency of the system clock.
//
//*****************************************************************************
uint32_t g_ui32SysClock;
// -> possibly need to change based on other branches/files

//
static volatile int16_t i16Touch_X, i16Touch_Y;
static volatile bool boolTouchPressed;

void vCreateGuiTask(void);
static void prvGuiTask(void *pvParameters);
void TouchCallBack(uint32_t,uint32_t,uint32_t);

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
}

/// @brief Handles touchscreen events and stores the current touch state
/// @param ui32Message
/// @param i32X
/// @param i32Y
void TouchCallback(uint32_t ui32Message, int32_t i32X, int32_t i32Y)
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