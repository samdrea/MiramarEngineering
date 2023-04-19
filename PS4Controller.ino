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
//Includes the Arduino Stepper Library
#include <Stepper.h>

ControllerPtr myControllers[BP32_MAX_CONTROLLERS];

// Defines the number of steps per rotation
const int stepsPerRevolution = 2038;

// Creates an instance of stepper class
// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
Stepper myStepper = Stepper(stepsPerRevolution, 9, 11, 10, 12);

// Arduino setup function. Runs in CPU 1
void setup() {
  // Initialize serial
  Serial.begin(9600);
  while (!Serial) {
    // wait for serial port to connect.
    // You don't have to do this in your game. This is only for debugging
    // purposes, so that you can see the output in the serial console.
    ;
  }

  String fv = BP32.firmwareVersion();
  Serial.print("Firmware version installed: ");
  Serial.println(fv);

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

  // BP32.pinMode(27, OUTPUT);
  // BP32.digitalWrite(27, 0);

  // This call is mandatory. It setups Bluepad32 and creates the callbacks.
  BP32.setup(&onConnectedController, &onDisconnectedController);

  // "forgetBluetoothKeys()" should be called when the user performs
  // a "device factory reset", or similar.
  // Calling "forgetBluetoothKeys" in setup() just as an example.
  // Forgetting Bluetooth keys prevents "paired" gamepads to reconnect.
  // But might also fix some connection / re-connection issues.
  BP32.forgetBluetoothKeys();
}

// This callback gets called any time a new gamepad is connected.
// Up to 4 gamepads can be connected at the same time.
void onConnectedController(ControllerPtr ctl) {
  bool foundEmptySlot = false;
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
      Serial.print("CALLBACK: Controller is connected, index=");
      Serial.println(i);
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
      Serial.println(buf);
      break;
    }
  }
  if (!foundEmptySlot) {
    Serial.println(
        "CALLBACK: Controller connected, but could not found empty slot");
  }
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

  if (!foundGamepad) {
    Serial.println(
        "CALLBACK: Controller disconnected, but not found in myControllers");
  }
}

void processGamepad(ControllerPtr gamepad) {
  /*
   * gamepad->buttons()
   * buttons numbers
   * x is 1, square is 4, circle is 2, triangle is 8
   * 
   * gamepad->dpad()
   * dpad numbers (The arrows on the left of PS4)
   * up is 1, right is 4, down is 2, left is 8
   * 
   * 
   * gamepad->throttle()
   *  (right pot) goes 0 -1020
   * gamepad->brake() 
   *  (left pot) goes 0 - 1020
   */

 // button x is 
 if (gamepad->buttons() == 1) {
    // Rotate CW slowly at 5 RPM
    myStepper.setSpeed(15);
    myStepper.step(1024);
 }
/*
  char buf[160];
  snprintf(
      buf, sizeof(buf) - 1,
      "idx=%d, dpad: 0x%02x, buttons: 0x%04x, axis L: %4li, %4li, axis "
      "R: %4li, %4li, brake: %4ld, throttle: %4li, misc: 0x%02x, battery: %d",
      gamepad->index(),        // Gamepad Index
      gamepad->dpad(),         // DPAD
      gamepad->buttons(),      // bitmask of pressed buttons
      gamepad->axisX(),        // (-511 - 512) left X Axis
      gamepad->axisY(),        // (-511 - 512) left Y axis
      gamepad->axisRX(),       // (-511 - 512) right X axis
      gamepad->axisRY(),       // (-511 - 512) right Y axis
      gamepad->brake(),        // (0 - 1023): brake button
      gamepad->throttle(),     // (0 - 1023): throttle (AKA gas) button
      gamepad->miscButtons(),  // bitmak of pressed "misc" buttons
      gamepad->battery()       // 0=Unknown, 1=empty, 255=full
  );
  Serial.println(buf);
*/
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
