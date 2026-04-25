#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"

//*****************************************************************************
//
// Mesage Types
//
//*****************************************************************************

typedef enum
{
    UI_MSG_MOTOR_RPM = 0,
    UI_MSG_MOTOR_CURRENT,
    UI_MSG_MOTOR_STATE, // enabled/disabled/fault -> add more if needed
    UI_MSG_SENSOR_A,    // change later, potentially add more
    UI_MSG_SENSOR_B,
    UI_MSG_SENSOR_C,
    UI_MSG_FAULT_RAISED, // payload: fault code
    UI_MSG_FAULT_CLEARED,
} UiMsgType_t;

typedef struct
{
    UiMsgType_t type;

    // Data could either be a float (RPM, current, sensor value)
    // or fault code, state enum, etc
    union
    {
        float f;
        uint32_t u; // could be subject to change
    } payload;
} UiMsg_t;

// Queue handle - created by ui_task and used here for producers
extern QueueHandle_t g_ui_queue;

//*****************************************************************************
//
// Producer Function - Call in any task to push to the UI queue (NOT from ISR)
//
//*****************************************************************************

// Producer function when payload is a float
static inline void ui_push_f(UiMsgType_t type, float value)
{
    // Create struct when the payload is a float
    UiMsg_t msg = {.type = type, .payload.f = value};
    xQueueSend(g_ui_queue, &msg, 0); // Non blocking. Drop if queue is full as 
                                    // UI is low prio.
}

// Producer function when payload is a uint
static inline void ui_push_u(UiMsgType_t type, float value)
{
    // Create struct when payload is fault code
    UiMsg_t msg = {.type = type, .payload.u = value};
    xQueueSend(g_ui_queue, &msg, 0); // May have to change here
}

// Call this from ISR
static inline void ui_push_from_isr(UiMsgType_t type, float value)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    UiMsg_t msg = {.type = type, .payload.f = value};
    xQueueSendFromISR(g_ui_queue, &msg, &xHigherPriorityTaskWoken);
}

// EXAMPLE MOTOR PRODUCER API USE IN ISR
// ui_push_from_isr(UI_MSG_MOTOR_RPM, current_rpm)