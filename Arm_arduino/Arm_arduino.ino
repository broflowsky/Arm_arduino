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
#define ZERO_POS 0
#define HOME_POS 2000 //TODO
#define DRONE_POS 1000 //TODO
#define CHARGER_POS 0 //TODO
#define MAX_SPEED 3000 // up to 4000 steps per second on 16 Mhz Atmega328
#define MAX_ACCEL 500
AccelStepper stepper = AccelStepper(MOTOR_INTERFACE, STEP_PIN, DIR_PIN);

//Comunnication
const byte numChars = 32;
char receivedChars[numChars];
boolean newData = false;
boolean swap = false;


//Forward declaration
void OpenClamp();
void CloseClamp();
void MotorsSetup();
void MoveStepper(long);
void MoveClamp(int);
void RecvWithStartEndMarkers();//read serial
void ProcessNewData();//decide which action to take
void Actuate();

bool isDone = false;
void setup() {
  Serial.begin(9600);
  MotorSetup();
}

void loop() {
  //  RecvWithStartEndMarkers();
  //  ProcessNewData();
  //  Actuate();

  stepper.runToNewPosition(-10000);

  

}
void MotorSetup() {

  // Clamp servo
  clamp_servo.attach(CLAMP_PIN);
  clamp_servo.write(CLAMP_OPENED_POS);


  //forward servo
  forward_servo.attach(FORWARD_PIN);
  forward_servo.write(FORWARD_MIN_POS);

  //Stepper
  stepper.setMaxSpeed(MAX_SPEED);
  stepper.setAcceleration(MAX_ACCEL);
  stepper.moveTo(ZERO_POS);
  stepper.runToPosition();

  Serial.println("Arm in Home configuration.");
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
    delay(1);
  }

}
void MoveStepper(long newPosition) {
  stepper.runToNewPosition(newPosition);
}
void MoveClamp(int newPosition) {
  forward_servo.write(newPosition);
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

  }
}
void Actuate() {
  if (swap) {
    Serial.println("Swapping");

    MoveStepper(DRONE_POS);
    MoveClamp(FORWARD_MAX_POS);
    //delay until robot has reached, may not be necessary
    delay(2000);
    CloseClamp();
    Serial.println("Clamp is closed.");

    //Take away battery from drone and move down towards charger
    MoveClamp(FORWARD_MIN_POS);
    //may add delay here
    MoveStepper(CHARGER_POS);
    //may add delay here
    //insert battery into charger
    MoveClamp(FORWARD_MAX_POS);//may need to differentiate drone and charger positions as they may be different
    //Let go of the battery
    OpenClamp();
    Serial.println("Battery inserted");
    //Return arm to home position
    MoveClamp(FORWARD_MIN_POS);
    MoveStepper(HOME_POS);
    Serial.println("Arm in Home configuration.");
  }

}
