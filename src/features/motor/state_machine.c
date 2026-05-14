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
#include "features/sensors/api/sensors_api.h"            

#define CONTROL_PERIOD_MS 10
#define MOTOR_SERIALPLOT_ENABLE 1

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
    UARTprintf("Motor task started\n");
    uint16_t duty_value = 10;
    uint16_t period_value = 50;
    uint16_t lowSpeedCount = 0;
    uint16_t zeroSpeedCount = 0;
    
    const TickType_t controlPeriodTicks = pdMS_TO_TICKS(CONTROL_PERIOD_MS);
    const float controlPeriodSeconds = CONTROL_PERIOD_MS / 1000.0f;

    initMotorLib(period_value);
    setDuty(duty_value);
    
    initMotorControl();

    

    for(;;) {
        switch (motor_state)
        {
        case MOTOR_STATE_IDLE:
            vTaskDelay(pdMS_TO_TICKS(15000)); 
            UARTprintf("IDLE done, starting motor\n");
            motorSetSpeed(1500);
            // xSemaphoreTake(motorStartSemaphore, portMAX_DELAY); // give from UI,, comment out for testing while ui not done
            motorStart();
            break;
        case MOTOR_STATE_STARTING:
        {
            // if e-stop triggered: transition to braking
            static uint8_t validSpeedCount = 0;
            uint16_t actualSpeed = Sensor_GetSpeed();

            if (actualSpeed >= 800)
            {
                validSpeedCount++;
            }
            else
            {
                validSpeedCount = 0;
            }

            if (validSpeedCount >= 5)
            {
                validSpeedCount = 0;
                motorRunning();
            }
           
            else if (xSemaphoreTake(motorUpToSpeedSemaphore, pdMS_TO_TICKS(100)) != pdTRUE)
            {
                kickStartMotor();
            }
            
            break;
        }
        case MOTOR_STATE_RUNNING:
        {    
            uint16_t desiredSpeed = motorGetSpeed();
            uint16_t referenceSpeed = motorRampUpdate(desiredSpeed, false, controlPeriodSeconds);

            uint16_t actualSpeed = Sensor_GetSpeed(); 

            static uint16_t prevActualSpeed = 0;
            static uint16_t frozenSpeedCount = 0;

            if (actualSpeed == prevActualSpeed && referenceSpeed > 100)
            {
                frozenSpeedCount++;
            }
            else
            {
                frozenSpeedCount = 0;
                prevActualSpeed = actualSpeed;
            }

            if (frozenSpeedCount > 30)  // 300ms of identical readings
            {
                UARTprintf("RUNNING EXIT: sensor freeze\n");
                setDuty(0);
                motorPIReset();
                motorEStop();
                break;
            }

            if (referenceSpeed > 100 && actualSpeed == 0)
            {
                zeroSpeedCount++;
            }
            else
            {
                zeroSpeedCount = 0;
            }

            if (zeroSpeedCount > 5)
            {
                UARTprintf("RUNNING EXIT: sustained zero speed\n");
                setDuty(0);
                motorPIReset();
                motorEStop();
                break;
            }

            // Low-speed recovery only applies when speed is low but not zero.
            if (referenceSpeed > 100 && actualSpeed < 200)
            {
                lowSpeedCount++;
            }
            else
            {
                lowSpeedCount = 0;
            }

            // One-time recovery kick if the motor is slowing but still moving.
            if (lowSpeedCount == 5)
            {
                motorPIInit(MOTOR_DUTY_START);
                kickStartMotor();
            }

            // If recovery fails, enter e-stop braking.
            if (lowSpeedCount > 50)
            {
                UARTprintf("RUNNING EXIT: lowSpeed timeout\n");
                setDuty(0);
                motorPIReset();
                motorEStop();
                break;
            }

            uint16_t duty = motorPIUpdate(referenceSpeed, actualSpeed, controlPeriodSeconds);
            
            if (lowSpeedCount > 0 && duty < MOTOR_DUTY_START)
            {
                duty = MOTOR_DUTY_START;
            }

            setDuty(duty);

            #if MOTOR_SERIALPLOT_ENABLE
                motorSerialPlotOutput(desiredSpeed, referenceSpeed, actualSpeed, duty);
            #endif

            vTaskDelay(controlPeriodTicks);

            break;
        }
        case MOTOR_STATE_BRAKING:
        {

            UARTprintf("IN BRAKING\n");
            uint16_t referenceSpeed = motorRampUpdate(0, true, controlPeriodSeconds);
            uint16_t actualSpeed = Sensor_GetSpeed();

            uint16_t duty = motorPIUpdate(referenceSpeed, actualSpeed, controlPeriodSeconds);
            setDuty(duty);

            #if MOTOR_SERIALPLOT_ENABLE
                motorSerialPlotOutput(0, referenceSpeed, actualSpeed, duty);
            #endif

            if (actualSpeed <= 50)
            {
                setDuty(0);
                motorPIReset();
                motorControlResetReferenceSpeed();
                motorFaultLatched();
            }

            vTaskDelay(controlPeriodTicks);
            break;
        }
        case MOTOR_STATE_FAULT:
            UARTprintf("STATE: FAULT\n");
            
            hallSensorIntDisable(); // need to decide later where the best state is to call this.
            speed_semaphore_given = false;
            // xSemaphoreTake(faultAcknowledgedSemaphore, portMAX_DELAY); // give from UI
            xSemaphoreTake(faultAcknowledgedSemaphore, pdMS_TO_TICKS(5000)); // 5s timeout instead of portMAX_DELAY
            lowSpeedCount = 0;
            zeroSpeedCount = 0;
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
