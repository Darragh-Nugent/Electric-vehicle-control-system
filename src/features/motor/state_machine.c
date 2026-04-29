#include "driverlib/pin_map.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOSConfig.h"
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
#include "driverlib/interrupt.h"

#include "motorlib.h"
#include "features/priorities.h"
#include "states.h"
#include "motor_api.h"
#include "motor_control.h"
// #include "sensors_api.h"                 // UNCOMMENT ONCE READY !!

#define CONTROL_PERIOD_MS 10

motor_state_t motor_state = MOTOR_STATE_IDLE;
static void motorTask( void *pvParameters );
void kickStartMotor(void);

extern SemaphoreHandle_t motorStartSemaphore;
extern SemaphoreHandle_t motorUpToSpeedSemaphore;
extern SemaphoreHandle_t faultAcknowledgedSemaphore;
extern volatile bool speed_semaphore_given;
extern void hallSensorIntDisable(void);

void vCreateMotorTask(void)
{
    xTaskCreate(
        motorTask,
        "motorTask",
        configMINIMAL_STACK_SIZE,
        NULL,
        MOTOR_CONTROL_PRIORITY,
        NULL
    );
}

static void motorTask( void *pvParameters )
{
    uint16_t duty_value = 10;
    uint16_t period_value = 50;
    
    const TickType_t controlPeriodTicks = pdMS_TO_TICKS(CONTROL_PERIOD_MS);
    const float controlPeriodSeconds = CONTROL_PERIOD_MS / 1000.0f;

    initMotorLib(period_value);
    setDuty(duty_value);
    
    initMotorControl();
    // UARTprintf("Motor task started\n");
    
    for(;;) {
        switch (motor_state)
        {
        case MOTOR_STATE_IDLE:
            xSemaphoreTake(motorStartSemaphore, portMAX_DELAY); // give from UI
            motorStart();
            break;
        case MOTOR_STATE_STARTING:
            // if e-stop triggered: transition to braking
            if (xSemaphoreTake(motorUpToSpeedSemaphore, pdMS_TO_TICKS(100)) == pdTRUE)
            {
                motorRunning();
            }
            else {
                kickStartMotor();
            }
            break;
        case MOTOR_STATE_RUNNING:
        {    
            
            uint16_t desiredSpeed = motorGetSpeed();
            uint16_t referenceSpeed = motorRampUpdate(desiredSpeed, false, controlPeriodSeconds);

            // uint16_t actualSpeed = Sensor_GetSpeed(); // UNCOMMENT ONCE READY !!!
            uint16_t actualSpeed = 0; // placeholder

            uint16_t duty = motorPIUpdate(referenceSpeed, actualSpeed, controlPeriodSeconds);
            setDuty(duty);

            // UARTprintf("Desired: %u, Reference: %u, Actual: %u, Duty: %u\n", desiredSpeed, referenceSpeed, actualSpeed, duty);

            vTaskDelay(controlPeriodTicks);

            break;
        }
        case MOTOR_STATE_BRAKING:
        {
            uint16_t referenceSpeed = motorRampUpdate(0, true, controlPeriodSeconds);

            // uint16_t actualSpeed = Sensor_GetSpeed();   /// UNCOMMENT ONCE READY !!!
            uint16_t actualSpeed = 0; // placeholder

            uint16_t duty = motorPIUpdate(referenceSpeed, actualSpeed, controlPeriodSeconds);

            setDuty(duty);

            // UARTprintf("E-STOP Reference: %u, Actual: %u, Duty: %u\n", referenceSpeed, actualSpeed, duty);
            
            if (referenceSpeed == 0) // a placeholder,, should be actualSpeed == 0, or maybe <=5 in case theres noise
            {
                setDuty(0);
                motorPIReset();
                motorFaultLatched();
            }

            vTaskDelay(controlPeriodTicks);

            break;
        }
        case MOTOR_STATE_FAULT:
            hallSensorIntDisable(); // need to decide later where the best state is to call this.
            speed_semaphore_given = false;
            xSemaphoreTake(faultAcknowledgedSemaphore, portMAX_DELAY); // give from UI
            motorInit();
            break;
        default:
            break;
        }
    }
}

// Single-time read/update to get motor running from idle.
void kickStartMotor(void)
{
    bool hall_a = GPIOPinRead(GPIO_PORTM_BASE, GPIO_PIN_3);
    bool hall_b = GPIOPinRead(GPIO_PORTH_BASE, GPIO_PIN_2);
    bool hall_c = GPIOPinRead(GPIO_PORTN_BASE, GPIO_PIN_2);
    updateMotor(hall_a, hall_b, hall_c);
}
