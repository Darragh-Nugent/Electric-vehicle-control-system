#  Electric Vehicle Control Systems 

## Overview
This project implements a real-time embedded system for controlling and monitoring an electric vehicle (EV). The system is runs on a TM4C1294 microcontroller with FreeRTOS. It integrates three core subsystems: Motor Control, Sensing, and User Interface.
---
## Features

### Motor Control

- Finite State Machine: Idle, Starting, Running, E-Stop Braking, Fault Latched  
- Closed-loop speed control using hall sensor feedback (PI controller)  
- Acceleration and deceleration ramp limiting  
- Emergency stop handling with controlled braking  
- Modular API for motor operations  

### Sensing

- Motor power estimation via ADC current measurements  
- Speed calculation using Hall effect sensors  
- Ambient light sensing for environment awareness  
- RTOS-based pipeline: ISR → Queue → Filtering → Application  
- Digital filtering applied outside ISRs for stable measurements  

### User Interface

- Start/Stop motor controls  
- Real-time system status display  
- Speed input via slider (RPM)  
- Visual motor status indicators (LED/colour-coded)  
- Threshold configuration for safety triggers  
- E-stop acknowledgement mechanism  
- Live sensor data plotting  

## System Architecture
The system uses FreeRTOS to manage concurrency:
- High-priority motor control task (1–10 ms periodic)
- Sensor acquisition and filtering tasks
- GUI update task
- ISR-driven event handling with queues and semaphores  

## Debugging & Validation
- UART output for real-time plotting (SerialPlot)
- Monitoring of speed, PWM, and sensor signals
- Verification of control performance and safety responses  

## Future Improvements/Additional Criterions
- Advanced control algorithms (PID tuning, adaptive control)  
- Integration of additional sensors (IMU, distance, temperature)  
- Enhanced GUI with improved visualisation and usability  


## Authors
Group Project – #30
