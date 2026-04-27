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

motor_state_t motor_state = MOTOR_STATE_IDLE;
static void motorTask( void *pvParameters );
void kickStartMotor(void);

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

    initMotorLib(period_value);
    setDuty(duty_value);

    for(;;) {
        switch (motor_state)
        {
        case MOTOR_STATE_IDLE:
            motorStart();
            // motor is stopped and outputs are disabled
            // -> starting when user selects start.
            break;
        case MOTOR_STATE_STARTING:
            kickStartMotor();
            motorRunning();
            // -> running when a sufficient speed is reached.
            break;
        case MOTOR_STATE_RUNNING:
            // closed-loop control must be implemented here.
            break;
        case MOTOR_STATE_BRAKING:
            // entered immediately when a fault occurs
            // deccelerate here
            break;
        case MOTOR_STATE_FAULT:
            // the motor has come to a stop
            // fault must be acknowledged by user.
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
