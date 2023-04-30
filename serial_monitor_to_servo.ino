#include <Servo.h>

byte servoPin = 2;
Servo servo;

void setup() {
  
 Serial.begin(9600);
 servo.attach(servoPin);

 servo.writeMicroseconds(1500); // send "stop" signal to ESC.

 delay(7000); // delay to allow the ESC to recognize the stopped signal
}

void loop() {
  
  Serial.println("Enter PWM signal value 1100 to 1900, 1500 to stop");
  
  while (Serial.available() == 0);
  
  int val = Serial.parseInt(); 

  //default max was 1900

// both thrusting and lifting motor
  // max limit is 2000
  // min limit is 1000
  if(val < 900 || val > 2500)
  {
    Serial.println("not valid");
  }
  else
  {
    servo.writeMicroseconds(val); // Send signal to ESC.
  }
}
