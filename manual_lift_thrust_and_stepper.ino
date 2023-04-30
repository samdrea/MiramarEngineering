/*
 * This example shows how to use the Controller API.
 *
 * Supported on boards with NINA W10x. In particular these boards:
 *  - Arduino MKR WiFi 1010,
 *  - UNO WiFi Rev.2,
 *  - Nano RP2040 Connect,
 *  - Nano 33 IoT,
 *  - Arduino Arduino MKR Vidor 4000
 * 
 * References : Ricardo Quesada 
 */
 
#include <Bluepad32.h>
#include <Stepper.h>
#include <Servo.h>

// Lifting pins
#define MOTOR_LF_PIN 12 // left front (yellow)
#define MOTOR_RF_PIN 3 // right front (brown)
#define MOTOR_LB_PIN 5 // left back (purple)
#define MOTOR_RB_PIN 6 // right back ( grey)

// Thrusting pins
#define leftFanPin 16 //(orange)
#define rightFanPin 17 //(green)

// Creates an instance of stepper class
// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
// Defines the number of steps per rotation for stepper motor
const int stepsPerRevolution = 50;
Stepper myStepper = Stepper(stepsPerRevolution, 9, 11, 10, 4);

// Motor speed variables for lift
double motor_speed_LF = 0;
double motor_speed_RF = 0;
double motor_speed_LB = 0;
double motor_speed_RB = 0;

// Motor speed variables for thrust
int potValueYRight = 0;
int potValueYLeft = 0;

// Servo objects
Servo motorLF;
Servo motorRF;
Servo motorLB;
Servo motorRB;
Servo leftFan;
Servo rightFan;

// Constants for motors
int motor_stop_speed = 1500; // value for stopping motor
int motor_max_speed = 2000; // fastest spin speed for one way
int motor_min_speed = 1000; // fastest spin speed for the other way

// Counter for speed increments
int speed_change = 50;
int total_speed_change = 0;

// Motor speed when starting... Apparently 1500 and 1600 is not stop for lifting motor
int starting_motor_speed = 1500; // CHANGE THIS if motor still moves at bluetooth connection

// Joystick variables
int deadzone = 50; // absolute value of deadzone on potentiometer (30 is a good value)
int offset = 0; // (-100 for lifting fan) negative if stopping point is above joystick
const int joystick_max = 512; // max value for joystick down
const int joystick_min = -508; // min value for joystick up


// PS4 Controller pointer
ControllerPtr myControllers[BP32_MAX_CONTROLLERS];

// defines the speed of rotation in rpm
const int stepperSpeed = 60;

// Arduino setup function. Runs in CPU 1
void setup() {


  // Initialize serial
  Serial.begin(9600);

  // Copyright 2021 - 2023, Ricardo Quesada
// SPDX-License-Identifier: Apache 2.0 or LGPL-2.1-or-later
  // To get the BD Address (MAC address) call:
  const uint8_t* addr = BP32.localBdAddress();
  Serial.print("BD Address: ");
  for (int i=0;i<6;i++) {
     Serial.print(addr[i], HEX);
     if (i<5)
      Serial.print(":");
    else
     Serial.println();
   }


  // This call is mandatory. It setups Bluepad32 and creates the callbacks.
  BP32.setup(&onConnectedController, &onDisconnectedController);

  // "forgetBluetoothKeys()" should be called when the user performs
  // a "device factory reset", or similar.
  // Calling "forgetBluetoothKeys" in setup() just as an example.
  // Forgetting Bluetooth keys prevents "paired" gamepads to reconnect.
  // But might also fix some connection / re-connection issues.
  BP32.forgetBluetoothKeys();

  // set speed of stepper motor
  myStepper.setSpeed(stepperSpeed);
  

  // Initialize the motors
  motorLF.attach(MOTOR_LF_PIN);
  motorRF.attach(MOTOR_RF_PIN);
  motorLB.attach(MOTOR_LB_PIN);
  motorRB.attach(MOTOR_RB_PIN);
  leftFan.attach(leftFanPin);
  rightFan.attach(rightFanPin);

  // Set the initial motor speeds to zero
  motorLF.writeMicroseconds(starting_motor_speed);
  motorRF.writeMicroseconds(starting_motor_speed);
  motorLB.writeMicroseconds(starting_motor_speed);
  motorRB.writeMicroseconds(starting_motor_speed);
  leftFan.writeMicroseconds(starting_motor_speed); 
  rightFan.writeMicroseconds(starting_motor_speed);


  // Wait for the motors to start
  delay(5000);
  
}

// This callback gets called any time a new gamepad is connected.
// Up to 4 gamepads can be connected at the same time.
void onConnectedController(ControllerPtr ctl) {
  bool foundEmptySlot = false;
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
   //   Serial.print("CALLBACK: Controller is connected, index=");
   //   Serial.println(i);
      myControllers[i] = ctl;
      foundEmptySlot = true;

      // Optional, once the gamepad is connected, request further info about the
      // gamepad.
      ControllerProperties properties = ctl->getProperties();
      char buf[80];
      sprintf(buf,
              "BTAddr: %02x:%02x:%02x:%02x:%02x:%02x, VID/PID: %04x:%04x, "
              "flags: 0x%02x",
              properties.btaddr[0], properties.btaddr[1], properties.btaddr[2],
              properties.btaddr[3], properties.btaddr[4], properties.btaddr[5],
              properties.vendor_id, properties.product_id, properties.flags);
   //   Serial.println(buf);
      break;
    }
  }
 // if (!foundEmptySlot) {
  //  Serial.println(
    //    "CALLBACK: Controller connected, but could not found empty slot");
 // }
}

void onDisconnectedController(ControllerPtr ctl) {
  bool foundGamepad = false;

  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      Serial.print("CALLBACK: Controller is disconnected from index=");
      Serial.println(i);
      myControllers[i] = nullptr;
      foundGamepad = true;
      break;
    }
  }
  
  // Debugging purposes
   if (!foundGamepad) {
      Serial.println(
         "CALLBACK: Controller disconnected, but not found in myControllers");
   }
}

void processGamepad(ControllerPtr gamepad) {

   // Left joystick y values for left thrusting fan
  Serial.print("Left: ");
  if (gamepad->axisY() + offset < deadzone && gamepad->axisY() + offset> -1 * deadzone) { // any values in deadzone will not turn motor
    potValueYLeft = motor_stop_speed;
  } else if (gamepad->axisY() >= deadzone){ // values above deadzone will spin motor one way gradually
    potValueYLeft = map(gamepad->axisY() + offset - deadzone, 0, joystick_max - deadzone + offset, motor_stop_speed, motor_max_speed);
  } else { // values below deadzone will spin motor the other way
    potValueYLeft = map(gamepad->axisY() + offset + deadzone, joystick_min + deadzone + offset, 0, motor_min_speed, motor_stop_speed);
  }
  
  // Right joystick y values for right thrusting fan
  Serial.print("Right: ");
  if (gamepad->axisRY() + offset < deadzone && gamepad->axisRY() + offset > -1 * deadzone) { // any values in deadzone will not turn motor
    potValueYRight = motor_stop_speed;
  } else if (gamepad->axisRY() >= deadzone){ // values above deadzone will spin motor one way gradually
    potValueYRight = map(gamepad->axisRY() + offset - deadzone, 0, joystick_max - deadzone + offset, motor_stop_speed, motor_max_speed);
  } else { // values below deadzone will spin motor the other way
    potValueYRight = map(gamepad->axisRY() + offset + deadzone, joystick_min + deadzone + offset, 0, motor_min_speed, motor_stop_speed);
  }

  // DPAD for lift
// if up button is pressed, increase lifting fans all by increment
  if (gamepad->dpad() == 1) { 
    total_speed_change += speed_change;
    
  } else if (gamepad->dpad() == 2 ){ 
    total_speed_change -= speed_change;
  }

  // the left lifting fans will move one way
  motor_speed_LF = motor_stop_speed + total_speed_change;
  motor_speed_LB = motor_stop_speed + total_speed_change;

  // the right lifting fans will move the other way
  motor_speed_RF = motor_stop_speed - total_speed_change;
  motor_speed_RB = motor_stop_speed - total_speed_change;

  // buttons for grabbing the payload
  if (gamepad->buttons() == 8) {  // if triangle(top) pressed, close the contraption
    myStepper.step(stepsPerRevolution);
  } else if (gamepad->buttons() == 1 ){  // if x(bottom) pressed, release the contraption
    myStepper.step(-stepsPerRevolution);
  }
  
  // Print the values for debugging
  Serial.print("Right joystick value: ");
  Serial.print(gamepad->axisRY());
  Serial.print(", Left Front Motor: ");
  Serial.print(motor_speed_LF);
  Serial.print(", Right Front Motor: ");
  Serial.print(motor_speed_RF);
  Serial.print(", Left Back Motor: ");
  Serial.print(motor_speed_LB);
  Serial.print(", Right Back Motor: ");
  Serial.print(motor_speed_RB);
  Serial.print(", Right thrust: ");
  Serial.print(potValueYRight);
  Serial.print(", Left thrust: ");
  Serial.println(potValueYLeft);


  // write the values to the esc
  motorLF.writeMicroseconds(motor_speed_LF);
  motorRF.writeMicroseconds(motor_speed_RF);
  motorLB.writeMicroseconds(motor_speed_LB);
  motorRB.writeMicroseconds(motor_speed_RB);
  leftFan.writeMicroseconds(potValueYLeft);    
  rightFan.writeMicroseconds(potValueYRight);    


}


// Arduino loop function. Runs in CPU 1
void loop() {
  // This call fetches all the controller info from the NINA (ESP32) module.
  // Just call this function in your main loop.
  // The controllers pointer (the ones received in the callbacks) gets updated
  // automatically.
  BP32.update();

  // It is safe to always do this before using the controller API.
  // This guarantees that the controller is valid and connected.
  for (int i = 0; i < BP32_MAX_CONTROLLERS; i++) {
    ControllerPtr myController = myControllers[i];

    if (myController && myController->isConnected()) {
      if (myController->isGamepad())
        processGamepad(myController);
    }

  }
  
  delay(150);
}
