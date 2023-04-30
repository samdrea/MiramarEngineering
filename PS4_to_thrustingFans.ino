// Copyright 2021 - 2023, Ricardo Quesada
// SPDX-License-Identifier: Apache 2.0 or LGPL-2.1-or-later

/*
 * This example shows how to use the Controller API.
 *
 * Supported on boards with NINA W10x. In particular these boards:
 *  - Arduino MKR WiFi 1010,
 *  - UNO WiFi Rev.2,
 *  - Nano RP2040 Connect,
 *  - Nano 33 IoT,
 *  - Arduino Arduino MKR Vidor 4000
 */
 
#include <Bluepad32.h>
#include <Servo.h>

// Pin variables
byte leftFanPin = 2;
byte rightFanPin = 3; // try the right first

// Create the servo objects for the thrusting fans
Servo leftFan;
Servo rightFan;

// Constants for motors
int motor_stop_speed = 1500; // value for stopping motor
int motor_max_speed = 2000; // fastest spin speed for one way
int motor_min_speed = 1000; // fastest spin speed for the other way

// Motor speed when starting... Apparently 1500 and 1600 is not stop for lifting motor
int starting_motor_speed = 1500; // CHANGE THIS if motor still moves at bluetooth connection

// Joystick variables
int deadzone = 50; // absolute value of deadzone on potentiometer (30 is a good value)
int offset = -100; // (-100 for lifting fan) negative if stopping point is above joystick
int potValueYRight = 0;
int potValueYLeft = 0;
const int joystick_max = 512; // max value for joystick down
const int joystick_min = -508; // min value for joystick up


// PS4 Controller pointer
ControllerPtr myControllers[BP32_MAX_CONTROLLERS];


// Arduino setup function. Runs in CPU 1
void setup() {
  
  // Initialize serial
  Serial.begin(9600);
  
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

    // Attach pins to the fans
  leftFan.attach(leftFanPin);
  rightFan.attach(rightFanPin);

  // Initialize fans 
  leftFan.writeMicroseconds(starting_motor_speed); // send "stop" signal to ESC.
  rightFan.writeMicroseconds(starting_motor_speed);
  delay(5000); // delay to allow the ESC to recognize the stopped signal
  
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
  
  // Left joystick y values 
  Serial.print("Left: ");
  if (gamepad->axisY() + offset < deadzone && gamepad->axisY() + offset> -1 * deadzone) { // any values in deadzone will not turn motor
    potValueYLeft = motor_stop_speed;
  } else if (gamepad->axisY() >= deadzone){ // values above deadzone will spin motor one way gradually
    potValueYLeft = map(gamepad->axisY() + offset - deadzone, 0, joystick_max - deadzone + offset, motor_stop_speed, motor_max_speed);
  } else { // values below deadzone will spin motor the other way
    potValueYLeft = map(gamepad->axisY() + offset + deadzone, joystick_min + deadzone + offset, 0, motor_min_speed, motor_stop_speed);
  }
  
  // Right joystick y values 
  Serial.print("Right: ");
  if (gamepad->axisRY() + offset < deadzone && gamepad->axisRY() + offset > -1 * deadzone) { // any values in deadzone will not turn motor
    potValueYRight = motor_stop_speed;
  } else if (gamepad->axisRY() >= deadzone){ // values above deadzone will spin motor one way gradually
    potValueYRight = map(gamepad->axisRY() + offset - deadzone, 0, joystick_max - deadzone + offset, motor_stop_speed, motor_max_speed);
  } else { // values below deadzone will spin motor the other way
    potValueYRight = map(gamepad->axisRY() + offset + deadzone, joystick_min + deadzone + offset, 0, motor_min_speed, motor_stop_speed);
  }


  // write the values to the esc
  leftFan.writeMicroseconds(potValueYLeft);    // Send the signal to the ESC
  rightFan.writeMicroseconds(potValueYRight);    // Send the signal to the ESC

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
