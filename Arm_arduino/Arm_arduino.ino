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
#define HOME_POS 500 //TODO
#define DRONE_POS 1000 //TODO
#define CHARGER_POS 0 //TODO
#define MAX_SPEED 100
#define MAX_ACCEL 50 
AccelStepper stepper = new AccelStepper(MOTOR_INTERFACE, DIR_PIN, STEP_PIN);

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


void setup() {
  Serial.begin(9600);
  MotorSetup();
}

void loop() {
  recvWithStartEndMarkers();
  processNewData();
  acutate();

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
  stepper.moveTo(HOME_POS);
  stepper.runToPosition();
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

    if (recvInProgress == true) {
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
void Acutate() {
  if (swap) {
    Serial.println("Swapping");
    
    MoveStepper(DRONE_POS);
    MoveClamp(FORWARD_MAX_POS);
    delay(2000);
    CloseClamp();
    Serial.println("Clamp is closed.")

    MoveClamp(FORWARD_MIN_POS);
    //may add delay here
    MoveStepper(CHARGER_POS);
    //may add delay here
    MoveClamp(FORWARD_MAX_POS);//may need to differentiate drone and charger positions as they may be different
    OpenClamp();
    MoveClamp(FORWARD_MIN_POS);
    MoveStepper(HOME_POS);
    
    

    
  }
  else {

    myservo.writeMicroseconds(1500);
  }
}
