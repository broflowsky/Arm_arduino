//Valentin Puyfourcat
//2 DOF Swap Dock

/*
  Stepper
    tutorial
    https://www.makerguides.com/tb6600-stepper-motor-driver-arduino-tutorial/

    Library
    https://www.airspayce.com/mikem/arduino/AccelStepper/classAccelStepper.html#a608b2395b64ac15451d16d0371fe13ce

    Protothreading
    https://create.arduino.cc/projecthub/reanimationxp/how-to-multithread-an-arduino-protothreading-tutorial-dd2c37

*/

#include <Servo.h>
#include <AccelStepper.h>
#include "ArmConfig.h"

//Motors
Servo clamp_servo;
Servo forward_servo;
AccelStepper vertical_stepper = AccelStepper(VERTICAL_STEPPER_MOTOR_INTERFACE, VERTICAL_STEPPER_STEP_PIN, VERTICAL_STEPPER_DIR_PIN);
AccelStepper horizontal_stepper = AccelStepper(HORIZONTAL_STEPPER_MOTOR_INTERFACE, HORIZONTAL_STEPPER_STEP_PIN, HORIZONTAL_STEPPER_DIR_PIN);

//Comunnication
const byte numChars = 32;
char receivedChars[numChars];
boolean newData = false;
boolean swap = false;

//LED indication Task Scheduler

byte ledPins[] = {LED_BIT_0, LED_BIT_1, LED_BIT_2, LED_BIT_3};



//Functions, check ArmConfig.h for function definitions
void OpenClamp();
void CloseClamp();
void MotorsSetup();
void LedSetup();
void MoveVerticalStepper(long);
void MoveClamp(int);
void RecvWithStartEndMarkers();//read serial
void ProcessNewData();//decide which action to take
void Actuate();
void TaskSchedulerLED(byte); //display LED



void setup() {
  Serial.begin(9600);
  LedSetup();
  MotorSetup();

  //   horizontal_stepper.runToNewPosition(-3000);
  //MoveClamp(FORWARD_MAX_POS);


  //
  //
  MoveHorizontalStepper(-HORIZONTAL_STEPPER_MAX_POS);
    MoveVerticalStepper(-VERTICAL_STEPPER_DRONE_POS);
  //    delay(2000);
  

}
void loop() {
//    RecvWithStartEndMarkers();
//    ProcessNewData();
//   // swap = true;
//    Actuate();
//  //  delay(3000);

//  MoveVerticalStepper(VERTICAL_STEPPER_DRONE_POS);
//  delay(2000);
//
//  MoveVerticalStepper(VERTICAL_STEPPER_BATTERY_UP_POS);
//  delay(2000);
//  
// MoveVerticalStepper(VERTICAL_STEPPER_HOME_POS);
// delay(2000);

}
void Actuate() {
  if (swap) {

    Serial.println("Starting swapping procedure");

    TaskSchedulerLED(LED_HOME_POSITION);

    //Reach drone
    TaskSchedulerLED(LED_DRONE_APPROACH);
    MoveVerticalStepper(VERTICAL_STEPPER_DRONE_POS);
    delay(1000);

    MoveHorizontalStepper(HORIZONTAL_STEPPER_MAX_POS);
    delay(1000);

    //Grab Battery
    TaskSchedulerLED(LED_BATTERY_REACH);
    MoveClamp(FORWARD_MAX_POS);
    delay(1000);

    TaskSchedulerLED(LED_CLAW_TRIGGER);
    CloseClamp();
    delay(1000);

    MoveVerticalStepper(VERTICAL_STEPPER_BATTERY_UP_POS);
    delay(1000);


    TaskSchedulerLED(LED_BATTERY_RETRACT);
    MoveClamp(FORWARD_MIN_POS);
    delay(1000);

    MoveHorizontalStepper(HORIZONTAL_STEPPER_HOME_POS);
    delay(1000);


    //Reach Empty charger
    TaskSchedulerLED( LED_CHARGER_APPROACH);
    MoveVerticalStepper(VERTICAL_STEPPER_CHARGER_POS);
    delay(1000);

    MoveHorizontalStepper(HORIZONTAL_STEPPER_MAX_POS);
    delay(1000);

    //Place battery
    TaskSchedulerLED(LED_BATTERY_INSERT);
    MoveClamp(FORWARD_MAX_POS);
    delay(1000);

    //inswert battery
    MoveVerticalStepper(VERTICAL_STEPPER_CHARGER_POS2);
    delay(1000);

    TaskSchedulerLED(LED_CLAW_RELEASE);
    OpenClamp();
    delay(1000);

    TaskSchedulerLED(LED_CHARGER_APPROACH2);
    MoveClamp(FORWARD_MIN_POS);
    delay(1000);


    MoveHorizontalStepper(HORIZONTAL_STEPPER_HOME_POS);
    delay(1000);

    MoveVerticalStepper(VERTICAL_STEPPER_HOME_POS);
    delay(1000);


    TaskSchedulerLED(LED_HOME_POSITION);
    swap = !swap;
    Serial.println("<SWAPDONE>");
  }
}
void MotorSetup() {
  // Clamp servo
  clamp_servo.attach(CLAMP_PIN);
  clamp_servo.write(CLAMP_OPENED_POS);

  //forward servo
  forward_servo.attach(FORWARD_PIN);
  forward_servo.write(FORWARD_MIN_POS);

  //Stepper
  vertical_stepper.setMaxSpeed(VERTICAL_STEPPER_MAX_SPEED);
  vertical_stepper.setAcceleration(VERTICAL_STEPPER_MAX_ACCEL);
  vertical_stepper.moveTo(VERTICAL_STEPPER_HOME_POS);
  vertical_stepper.runToPosition();

  horizontal_stepper.setMaxSpeed(HORIZONTAL_STEPPER_MAX_SPEED);
  horizontal_stepper.setAcceleration(HORIZONTAL_STEPPER_MAX_ACCEL);
  horizontal_stepper.moveTo(HORIZONTAL_STEPPER_HOME_POS);
  horizontal_stepper.runToPosition();

  //Set LED indicators
  TaskSchedulerLED(LED_HOME_POSITION);
}
void TaskSchedulerLED(byte task) {

  if (!task) {
    byte count = 10;
    while (count-- > 0) {
      for (byte i = 0; i < 4 ; ++i) {
        digitalWrite(ledPins[i], bitRead(1, i));
        delay(10);
      }
      for (byte i = 0; i < 4 ; ++i) {
        digitalWrite(ledPins[i], bitRead(2, i));
        delay(10);
      }
      for (byte i = 0; i < 4 ; ++i) {
        digitalWrite(ledPins[i], bitRead(4, i));
        delay(10);
      }
      for (byte i = 0; i < 4 ; ++i) {
        digitalWrite(ledPins[i], bitRead(8, i));
        delay(10);
      }

    }
  }
  for (byte i = 0; i < NUM_PIN ; ++i)
    digitalWrite(ledPins[i], bitRead(task, i));
}


void CloseClamp() {

  for (byte pos = CLAMP_OPENED_POS; pos > CLAMP_CLOSED_POS; --pos) {
    clamp_servo.write(pos);
    delay(20);
  }

}
void OpenClamp() {

  for (byte pos = CLAMP_CLOSED_POS; pos < CLAMP_OPENED_POS; ++pos) {
    clamp_servo.write(pos);
    //delay(1);
  }
}

void MoveVerticalStepper(long newPosition) {
  vertical_stepper.runToNewPosition(newPosition);
}
void MoveHorizontalStepper(long newPosition) {
  horizontal_stepper.runToNewPosition(newPosition);
}
void MoveClamp(int newPosition) {
  if (newPosition > forward_servo.read()) {
    for (int i = forward_servo.read(); i < newPosition; ++i) {
      forward_servo.write(i);
      delay(20);
    }
  }
  else {
    for (int i = forward_servo.read(); i > newPosition; --i) {
      forward_servo.write(i);
      delay(20);
    }
  }

}
void RecvWithStartEndMarkers() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (recvInProgress) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }
    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}
void ProcessNewData() {
  if (newData == true) {
    newData = false;
    if (strcmp(receivedChars, "SWAP_START") == 0) {
      swap = true;
    }

    if (strcmp(receivedChars, "IDENTIFY_PORT") == 0) {
      Serial.println("<ARM>");
    }
  }
}
