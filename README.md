# AUTOVAC
The aim of this project is to design a working model of an autonomous vacuum cleaner from scratch.

INTRODUCTION:
The objective of this project is to design a working model of an autonomous vacuum cleaner from scratch, which can detect and avoid all the obstacles and vacuums the house simultaneously. The vacuum cleaner is made from high power DC motor and reusable plastic. It has a simple setup and easy maintenance.

DESIGN:
Autovac consists of TIVA launchpad, 2 HC-SR04 Ultrasonic sensor, 2 DC motors and wheels, 1 DRV8833 dual motor driver. Two timers of the ARM Cortex M4 are used for calculating the distance of the obstacles using ultrasonic sensors. DC motor is driven by providing PWM voltage from the microcontroller.

