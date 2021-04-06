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
// positive positions are above home position, hence positives pos > current pos moves up
#define STEPPER_DIR_PIN 4
#define STEPPER_STEP_PIN 5
#define STEPPER_MOTOR_INTERFACE 1

#define STEPPER_HOME_POS 0
#define STEPPER_DRONE_POS 2000
#define STEPPER_CHARGER_POS -2000
#define STEPPER_MAX_SPEED 3000 // up to 4000 steps per second on 16 Mhz Atmega328
#define STEPPER_MAX_ACCEL 350
AccelStepper stepper = AccelStepper(STEPPER_MOTOR_INTERFACE, STEPPER_STEP_PIN, STEPPER_DIR_PIN);

//Comunnication
const byte numChars = 32;
char receivedChars[numChars];
boolean newData = false;
boolean swap = false;

//LED indication Task Scheduler
#define NUM_PIN 4
byte ledPins[] = {10,11,12,9};

#define LED_HOME_POSITION 0
#define LED_DRONE_APPROACH 1
#define LED_BATTERY_REACH 2
#define LED_CLAW_TRIGGER 3
#define LED_BATTERY_RETRACT 4
#define LED_CHARGER_APPROACH 5
#define LED_BATTERY_INSERT 6
#define LED_CLAW_RELEASE 7
#define LED_HOME_APPROACH 8


//Forward declaration
void OpenClamp();
void CloseClamp();
void MotorsSetup();
void MoveStepper(long);
void MoveClamp(int);
void RecvWithStartEndMarkers();//read serial
void ProcessNewData();//decide which action to take
void Actuate();
void TaskSchedulerLED(byte); //display LED



void setup() {
  Serial.begin(9600);
  pinMode(9,OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  
  MotorSetup();
}

void loop() {
  //  RecvWithStartEndMarkers();
  //  ProcessNewData();
  swap = true;
  Actuate();
  delay(1000);
  
  //stepper.runToNewPosition(1000);

}
void TaskSchedulerLED(byte task){
  if(task <= 8)
    for(byte i = 0; i<NUM_PIN ;++i)
      digitalWrite(ledPins[i],bitRead(task,i));
}

void MotorSetup() {

  // Clamp servo
  clamp_servo.attach(CLAMP_PIN);
  clamp_servo.write(CLAMP_OPENED_POS);


  //forward servo
  forward_servo.attach(FORWARD_PIN);
  forward_servo.write(FORWARD_MIN_POS);

  //Stepper
  stepper.setMaxSpeed(STEPPER_MAX_SPEED);
  stepper.setAcceleration(STEPPER_MAX_ACCEL);
  stepper.moveTo(STEPPER_HOME_POS);
  stepper.runToPosition();

//Set LED indicators
  TaskSchedulerLED(LED_HOME_POSITION);

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

    TaskSchedulerLED(LED_DRONE_APPROACH);
    MoveStepper(STEPPER_DRONE_POS);
    delay(1000);

    TaskSchedulerLED(LED_BATTERY_REACH);
    MoveClamp(FORWARD_MAX_POS);
    delay(1000);
  
    TaskSchedulerLED(LED_CLAW_TRIGGER);
    CloseClamp();
    delay(1000);

    TaskSchedulerLED(LED_BATTERY_RETRACT);
    MoveClamp(FORWARD_MIN_POS);
    delay(1000);

    TaskSchedulerLED( LED_CHARGER_APPROACH);
    MoveStepper(STEPPER_CHARGER_POS);
    delay(1000);
    
    TaskSchedulerLED(LED_BATTERY_INSERT);
    MoveClamp(FORWARD_MAX_POS);
    delay(1000);
    
    //Let go of the battery
    TaskSchedulerLED(LED_CLAW_RELEASE);
    OpenClamp();
  
    //Return arm to home position
    TaskSchedulerLED(LED_HOME_APPROACH);
    MoveClamp(FORWARD_MIN_POS);
    MoveStepper(STEPPER_HOME_POS);
    delay(1000);
    TaskSchedulerLED(LED_HOME_POSITION);
    swap = !swap;
  }

}
