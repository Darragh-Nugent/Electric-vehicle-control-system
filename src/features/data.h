#ifndef DATA_H
#define DATA_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "lvgl.h"

typedef int32_t (*graph_data_cb_t)(void);
typedef int32_t (*scale_data_cb_t)(void);

typedef struct
{
    lv_obj_t *chart;
    lv_obj_t *scale;
    lv_timer_t *timer;
    int32_t scaleYMin;
    int32_t scaleYMax;
    int16_t overheadGap;
    graph_data_cb_t get_value_cb;
} graph_t;

typedef struct
{
    lv_obj_t *scale;
    lv_obj_t *needle;
    lv_timer_t *timer;
    int16_t radius;
    int16_t needle_length;
    int32_t cur_value; // this is a temporary value, can be removed once sensors are actually connected
    int32_t max_value;
    lv_point_precise_t needle_points[2];
    scale_data_cb_t get_value_cb;
} roundScale_t;


typedef struct
{
    uint16_t TH_POWER;
    uint16_t TH_ACCEL;
    uint16_t TH_DIST;
    int16_t TH_TEMP;
} sensorThresholds_t;

extern const char *motor_state_names[];
//*****************************************************************************
//
// Mesage Types
//
//*****************************************************************************

typedef enum
{
    UI_MSG_MOTOR_RPM = 0,
    UI_MSG_STATE_NONE,
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


#endif