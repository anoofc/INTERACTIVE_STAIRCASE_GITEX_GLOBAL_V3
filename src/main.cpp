/**
 * Written by     : 	Anoof Chappangathil
 * Email          :   anoofdoc@gmail.com
 * Phone          :   +971 50 762 1347
 * Company        :   Interactive Technical Service LLC
 * GitHub         :   https://github.com/anoofc/
 * LinkedIn       :   https://www.linkedin.com/in/anoofc/
 * Date           : 	09/10/2024
 * Description    :   This code implements a Staircase Lighting System using an Arduino Mega 2560.
 *                    The system uses two sensors to detect movement at the top and bottom of the staircase.
 *                    When a sensor is triggered, the lights on the staircase will sequentially turn on or off,
 *                    creating a visual effect of lights following the person as they move up or down the stairs.
 *                    The system uses the SparkFunDMX library to control the lighting via DMX protocol.
 *                    The code includes debounce logic to prevent false triggers and delays to control the timing
 *                    of the light sequences.
 *                    The system also includes a debug mode to print status messages to the serial monitor. 
 */


#define DEBUG             1

#define SENSOR1           25
#define SENSOR2           26


#define DEBOUNCE_DELAY    500
#define STRIP_CLEAR_DELAY 10000
#define STEP_UPDATE_DELAY 500
#define STEP_CLEAR_DELAY  150

#define NUM_OF_STEPS      16

#include <Arduino.h>
#include <SparkFunDMX.h>

SparkFunDMX dmx;

uint32_t sensorUpdateMillis = 0;
uint32_t stepUpdateMillis = 0;
uint32_t stripClearMillis = 0;
uint32_t stepClearUpdateMillis = 0;
uint8_t step_count = 1;
uint8_t step_down_count = NUM_OF_STEPS;
uint8_t sequence_active = 0;

void io_Setup() {
  Serial.println("Setting up IO");
  pinMode(SENSOR1, INPUT_PULLUP);
  pinMode(SENSOR2, INPUT_PULLUP);

  dmx.initWrite(20);
  
}

void showStep(int step){
  dmx.write(step, 255);
  dmx.update(); dmx.update();
  if (DEBUG) {Serial.print("Showing Step: "); Serial.println(step);}
}

void clearStep(int step){
  dmx.write(step, 0);
  dmx.update(); dmx.update();
  if (DEBUG) {Serial.print("Clearing Step: "); Serial.println(step);}
}

void stepUpSequence(){
  if (step_count <= NUM_OF_STEPS  && sequence_active == 1){
    if (millis() - stepUpdateMillis < STEP_UPDATE_DELAY){ return; }
    // if (DEBUG) {Serial.print("Step Count: "); Serial.println(step_count);}
    showStep(step_count);
    stepUpdateMillis = millis();
    step_count++;
    if (step_count > NUM_OF_STEPS){
      if (DEBUG) {Serial.println("UP Sequence Completed");}
      stripClearMillis = millis();
    }
  }
}

void stepDownSequence(){
  if (step_down_count > 0 && sequence_active == 2){
    if (millis() - stepUpdateMillis < STEP_UPDATE_DELAY){ return; }
    // if (DEBUG) {Serial.print("Step Count: "); Serial.println(step_down_count);}
    showStep(step_down_count);
    stepUpdateMillis = millis();
    step_down_count--;
    if (step_down_count == 0){
      stripClearMillis = millis();
      if (DEBUG) {Serial.println("DOWN Sequence Completed");}
    }
  }
}

void clearStepsSequence(){
  if (sequence_active == 3){
    if (millis() - stepUpdateMillis < STEP_CLEAR_DELAY){ return; }
    clearStep(step_count);
    step_count--;
    stepUpdateMillis = millis();
    if (step_count == 0){
      if (DEBUG) {Serial.println("Steps Cleared!!!");}
      sequence_active = 0; step_count = 1; 
    }
  }
  if (sequence_active == 4){
    if (millis() - stepUpdateMillis < STEP_CLEAR_DELAY){ return; }
    clearStep(step_down_count);
    step_down_count++;
    stepUpdateMillis = millis();
    if (step_down_count > NUM_OF_STEPS){
      if (DEBUG) {Serial.println("Steps Cleared!!!");}
      sequence_active = 0; step_down_count = NUM_OF_STEPS;
    }
  }
}
void clearSequenceCheck(){
  if (sequence_active == 1 && step_count > NUM_OF_STEPS){
    if (millis() - stripClearMillis < STRIP_CLEAR_DELAY){ return; }
    sequence_active = 3;
    // if (DEBUG) {Serial.println("Steps Cleared!!!");}
  } else if (sequence_active == 2 && step_down_count == 0){
    if (millis() - stripClearMillis < STRIP_CLEAR_DELAY){ return; }
    sequence_active = 4;
    // if (DEBUG) {Serial.println("Steps Cleared!!!");}
  }
}

void readSensors(){
  if (digitalRead(SENSOR1) == HIGH){
    if (millis() - sensorUpdateMillis < DEBOUNCE_DELAY){ return; }
      sensorUpdateMillis = millis();
      stripClearMillis = millis();
      if (DEBUG) {Serial.println("Sensor 1 Triggered");}
      if (sequence_active != 2){ sequence_active = 1; }
  }
  if (digitalRead(SENSOR2) == HIGH){
    if (millis() - sensorUpdateMillis < DEBOUNCE_DELAY){ return; }
      sensorUpdateMillis = millis();
      stripClearMillis = millis();
      if (DEBUG) {Serial.println("Sensor 2 Triggered");}
      if (sequence_active != 1) {sequence_active = 2;}
  }
}

void readSerial(){
  if (Serial.available() > 0) {
    char incoming = Serial.read();
    if (incoming == 'A'){
      sequence_active = 0; step_count = 1; step_down_count = NUM_OF_STEPS;
      // TODO: Call Function to start the sequence.
    }
    if (incoming == 'B'){
      sequence_active = 0;
      // TODO: Call Function to Clear the sequence.
    }
  }
}

void debugPins(){
  Serial.println("S1: " + String(digitalRead(SENSOR1)) + " \t S2: " + String(digitalRead(SENSOR2)));
}

void setup() {
  Serial.begin(9600);
  io_Setup();
}

void loop() {
  // readSerial();
  readSensors();
  stepUpSequence();
  stepDownSequence();
  clearSequenceCheck();
  clearStepsSequence();
  // debugPins();
}
