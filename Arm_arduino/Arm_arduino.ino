//Valentin Puyfourcat
//2 DOF Swap Dock

/*
  Stepper tutorial and library
  https://www.makerguides.com/tb6600-stepper-motor-driver-arduino-tutorial/
  https://www.airspayce.com/mikem/arduino/AccelStepper/classAccelStepper.html#a608b2395b64ac15451d16d0371fe13ce

*/

#include <Servo.h>
#include <AccelStepper.h>


//  Clamp servo - 180° roation
#define CLAMP_PIN 2
#define CLAMP_OPENED_POS 100
#define CLAMP_CLOSED_POS 87
Servo clamp_servo;

// Forward servo - 180° rotation

#define FORWARD_PIN 3
#define FORWARD_MAX_POS 180// TODO
#define FORWARD_MIN_POS 0 //TODO
Servo forward_servo;


//  UP Down Stepper motor
#define DIR_PIN 4
#define STEP_PIN 5
#define MOTOR_INTERFACE 1
AccelStepper stepper = new AccelStepper(MOTOR_INTERFACE, DIR_PIN, STEP_PIN);



bool OpenClamp();
bool CloseClamp();
void MotorsSetup();

void setup() {

  Serial.begin(9600);
  MotorSetup();

}

void loop() {





}
void MotorSetup() {

  // Clamp servo
  clamp_servo.attach(CLAMP_PIN);
  clamp_servo.write(CLAMP_OPENED_POS);


  //forward servo
  forward_servo.attach(FORWARD_PIN);
  forward_servo.write(FORWARD_MIN_POS);

//Stepper
  stepper.setMaxSpeed(100);
  stepper.setAcceleration(500);
  stepper.moveTo(0);
  stepper.runToPosition();
}
bool CloseClamp() {

  for (byte pos = CLAMP_OPENED_POS; pos > CLAMP_CLOSED_POS; --pos) {
    clamp_servo.write(pos);
    delay(20);
  }
  return true;
}
bool OpenClamp() {

  for (byte pos = CLAMP_CLOSED_POS; pos < CLAMP_OPENED_POS; ++pos) {
    clamp_servo.write(pos);
    delay(1);
  }
  return true;
}
