#ifndef DATA_H
#define DATA_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "lvgl.h"

typedef int32_t (*graph_data_cb_t)(void);
typedef int32_t (*scale_data_cb_t)(void);

typedef struct{
    lv_obj_t *chart;
    lv_obj_t *scale;
    lv_timer_t *timer;
    int32_t scaleYMin;
    int32_t scaleYMax;
    int16_t overheadGap;
    graph_data_cb_t get_value_cb;
}graph_t;

typedef struct{
    lv_obj_t *scale;
    lv_obj_t *needle;
    lv_timer_t *timer;
    int16_t radius;
    int16_t needle_length;
    int32_t cur_value; // this is a temporary value, can be removed once sensors are actually connected
    lv_point_precise_t needle_points[2];
    scale_data_cb_t get_value_cb;
} roundScale_t;

//*****************************************************************************
//
// Mesage Types
//
//*****************************************************************************

typedef enum
{
    UI_MSG_MOTOR_RPM = 0,
    UI_MSG_MOTOR_CURRENT,
    UI_MSG_MOTOR_STARTING,
    UI_MSG_MOTOR_IDLE, // enabled/disabled/fault -> add more if needed
    UI_MSG_MOTOR_RUNNING,
    UI_MSG_MOTOR_BREAKING,
    UI_MSG_SENSOR_UPDATE_POWER, // change later, potentially add more
    UI_MSG_SENSOR_UPDATE_ACCELERATION,
    UI_MSG_SENSOR_UPDATE_DISTANCE,
    UI_MSG_SENSOR_UPDATE_HUMIDITY,
    UI_MSG_SENSOR_UPDATE_TEMP,
    UI_MSG_SENSOR_UPDATE_LUX,
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
inline bool ui_push_f(UiMsgType_t type, float value)
{
    // Create struct when the payload is a float
    UiMsg_t msg = {.type = type, .payload.f = value};
    return xQueueSend(g_ui_queue, &msg, 0); // Non blocking. Drop if queue is full as
                                            // UI is low prio.
}

// Producer function when payload is a uint
inline bool ui_push_u(UiMsgType_t type, uint16_t value)
{
    // Create struct when payload is fault code
    UiMsg_t msg = {.type = type, .payload.u = value};
    return xQueueSend(g_ui_queue, &msg, 0); // May have to change here
}

// Call this from ISR
inline bool ui_push_from_isr(UiMsgType_t type, float value)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    UiMsg_t msg = {.type = type, .payload.f = value};
    return xQueueSendFromISR(g_ui_queue, &msg, &xHigherPriorityTaskWoken);
}

// EXAMPLE MOTOR PRODUCER API USE IN ISR
// ui_push_from_isr(UI_MSG_MOTOR_RPM, current_rpm)

#endif