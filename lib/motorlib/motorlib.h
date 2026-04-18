/*
 * motorlib.h
 *
 *  Created on: May 2025
 *      Author: EGH456
 */

#ifndef MOTORLIB_H_
#define MOTORLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

// #include <xdc/runtime/Error.h>
#include <stdbool.h>
#include <stdint.h>

/*!
 *  @brief  This function sets the duty cycle of the high side PWM lines.
 *
 *  @pre    motor library has been initialized using initMotorLib()
 *
 *  @param  duty  16bit integer number for the setting the duty cycle
 *                in microseconds where the max integer = PWM Period
 *                set prior in the initMotorLib() function
 *                Valid values for duty are 0 - PWMStruct.MaxDuty
 *
 */
void setDuty(uint16_t duty);

/*!
 *  @brief  Main function which Commutates the motor phases A,B,C to the correct values based on the Hall sensor input.
 *
 *  @pre    motor library has been initialized using initMotorLib() and duty cycle as been set
 *
 *  @param  Hall_a         current value of the Hall A effect sensor as a bool (0 or 1)
 *
 *  @param  Hall_b         current value of the Hall B effect sensor as a bool (0 or 1)
 *
 *  @param  Hall_c         current value of the Hall C effect sensor as a bool (0 or 1)
 * *
 */
void updateMotor(bool Hall_a, bool Hall_b, bool Hall_c);


/*!
 *  @brief  Brakes motor by turning all phases high or low.
 *
 *  @pre    motor library has been initialized using initMotorLib() and duty cycle as been set
 *
 *  @param  brakeType      Determines hard or soft brake. If true then all phases set to high, else all phases set to low
 *
 * *
 */
void stopMotor(bool brakeType);

/*!
 *  @brief  Enables Motor Drive by setting enable pin to low
 *
 *
 * *
 */
void enableMotor();

/*!
 *  @brief  Disables Motor Drive by setting enable pin to High
 *
 *
 * *
 */
void disableMotor();

/*!
 *  @brief  Initialise GPIO and PWM module to ensure correct setup of High/Low side pins.
 *
 *  @param  pwm_duty_period   16 bit unsigned integer pwm period is in microseconds and should be set to be a value between 10 - 100 (100KHz - 10Khz)
 *
 *  @return true if the PWM modules initialised successfully. False if an error occurred likely due to already being opened elsewhere
 *
 * *
 */
bool initMotorLib(uint16_t pwm_period);

/*!
 *  @brief  getter function for internal PWM period.
 *
 *
 *  @param  pwm_period      recommended value is 24, relates to pwm frequency
 *                          and determines resolution of duty cycle value
 *                          see PWM device information to understand how pwm period limits range of duty cycle
 *                          max duty becomes pwm_period value
 *
 *
 *  @return bool            if initialisation was successful, error_block can be used to determine further issues
 *                          on what caused a failure to setup the motor.
 *
 * *
 */
uint16_t getMotorPWMPeriod();

#ifdef __cplusplus
}
#endif

#endif /* MOTORLIB_H_ */


