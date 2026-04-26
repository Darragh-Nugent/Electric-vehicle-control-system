// src/features/user_interface/ui_shared.h
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "semphr.h"

typedef struct {
    // Motor
    float    motor_rpm;
    float    motor_current_amps;
    uint8_t  motor_state;        // 0=idle 1=running 2=fault

    // Sensors (treat as opaque floats — your black boxes)
    float    sensor_a;
    float    sensor_b;
    float    sensor_c;

    // Alerts
    uint32_t fault_flags;        // bitmask, 0 = no faults
} UiData_t;

// Written by motor/sensor tasks, read by GUI task.
// Always acquire g_ui_mutex before reading or writing.
extern volatile UiData_t g_ui_data;
extern SemaphoreHandle_t g_ui_mutex;

// Convenience macros for producers (motor/sensor tasks)
#define UI_LOCK()   xSemaphoreTake(g_ui_mutex, portMAX_DELAY)
#define UI_UNLOCK() xSemaphoreGive(g_ui_mutex)