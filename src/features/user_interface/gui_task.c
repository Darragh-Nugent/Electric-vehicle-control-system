// #include "gui_task.h"
// #include "../data.h"
// #include "screens/scr_dashboard.h"
// #include "grlib/widget.h"
// #include "FreeRTOS.h"
// #include "task.h"
// #include "driver_lib/sysctl.h"
// #include "driver_lib/udma.h"
// #include "grlib.h"
// #include "drivers/touch.h"
// #include "drivers/Kentec320x240x16_ssd2119_spi.h"
// #include "widget.h"
// #include "semphr.h"
// extern uint32_t g_ui32SysClock;
// tContext sContext;
// tDMAControlTable psDMAControlTable[64] __attribute__((aligned(1024)));
// SemaphoreHandle_t g_ui_mutex = NULL;
// volatile UiData_t g_ui_data;
// void vGuiTask(void *pvParams);
// void vCreateGuiTask(void)
// {
//     Kentec320x240x16_SSD2119Init(g_ui32SysClock);

//     GrContextInit(&sContext, &g_sKentec320x240x16_SSD2119);

//     g_ui_mutex = xSemaphoreCreateMutex();
//     // configASSERT here will hard-fault before scheduler if mutex fails
//     configASSERT(g_ui_mutex != NULL);

//     // DMA init
//     SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
//     SysCtlDelay(10);
//     uDMAControlBaseSet(&psDMAControlTable[0]);
//     uDMAEnable();

//     // Touch init
//     TouchScreenInit(g_ui32SysClock);
//     TouchScreenCallbackSet(WidgetPointerMessage);

//     // Widget tree setup (ONE TIME ONLY)
//     WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sDashboardPanel);
//     WidgetPaint(WIDGET_ROOT);
//     xTaskCreate(vGuiTask, "GUI", 1024, NULL, 2, NULL);
// }

// void vGuiTask(void *pvParams)
// {
//     (void)pvParams;
//     GrContextForegroundSet(&sContext, ClrRed);
//     GrStringDraw(&sContext, "GUI TASK ALIVE", -1, 10, 10, 0);
//     for (;;)
//     {
//         // // 1. Push latest state into UI layer
//         // scr_dashboard_refresh();

//         // // 2. Process touch + widget events
//         // WidgetMessageQueueProcess();

//         // // 3. Let other tasks run
//         // vTaskDelay(pdMS_TO_TICKS(40));

//         GrContextForegroundSet(&sContext, ClrWhite);
//         GrRectFill(&sContext, &(tRectangle){0, 0, 319, 239});

//         GrContextForegroundSet(&sContext, ClrRed);
//         GrStringDraw(&sContext, "TEST", -1, 10, 10, 0);

//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
// }
