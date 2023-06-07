# Hovercraft Project

<img src="https://github.com/sammydrea/MiramarEngineering/assets/69533601/dda3c4ae-1353-4f9d-8d2e-f32c0c43bbfe" width="60%" height="60%">

## Code Implementation Overview: 
This project involves connecting a PS4 controller to an Arduino Nano 33 IoT's Bluetooth receiver for controlling the hovercraft. 
The final sketch enables control of the lifting fans using the controller's D-pad, control of the thrusting fan using the joysticks for precise navigation, 
and control of a stepper motor contraption to grab and release objects using the X and Triangle buttons.

## a. Bluetooth Connection:
The Bluetooth connection between the Arduino Nano 33 IoT and the PS4 controller is enabled by the [Bluepad32 library](https://gitlab.com/ricardoquesada/bluepad32-arduino) written by Ricardo Quesada. It initializes the Bluetooth module and defines the necessary variables and constants to handle the communication.

## b. Fan Control:
The 4 lifting fans are controlled by the up and down buttons on D-pad on the PS4 controller. The up button increments while the down button decrements the fan speed by a constant value. Because the fans are set up to counteract the torque produced from the motors, the 2 fans on one diagonal spin in the opposite direction as the other 2 fans of the opposing diagonal.

## c. Thrust Control:
The joysticks on the PS4 controller map to the power and direction of the thrusting fans, allowing the hovercraft to manuever with precision.
Due to the nature of joystick drifts, a "deadzone" was introduced in the code to prevent unintended spinning of the thrusting fan when the joysticks are in neutral position. An "offset" was also added to fix the shifting of the joystick values when the Arduino Nano was powered by an external power source as opposed to the steady power source from the computer.

## d. Stepper Motor Control:
The hardware team created a rack and pinion contraption that utilizes a stepper motor to grab and release objects. The final code maps the X and Triangle buttons to turn the stepper motor incremently. 
Therefore, the hovercraft driver can grab or release the objects by holding down the buttons to get the desired grip.
