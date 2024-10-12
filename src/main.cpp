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

#define DEBOUNCE_DELAY    2000
#define STEP_UPDATE_DELAY 550
#define STEP_CLEAR_DELAY  2300

#define NUM_OF_STEPS      16

#include <Arduino.h>
#include <SparkFunDMX.h>

SparkFunDMX dmx;

#define   MAX_ACTIVE_SEQUENCES      3

uint8_t   sequence_active_up        [MAX_ACTIVE_SEQUENCES]                = {0};
uint8_t   currentStep               [MAX_ACTIVE_SEQUENCES]                = {0};
uint32_t  stepUpdateMillis          [MAX_ACTIVE_SEQUENCES]                = {0};
uint32_t  stepClearUpdateMillisUp   [MAX_ACTIVE_SEQUENCES][NUM_OF_STEPS]  = {{0}};
uint8_t   upSeq_active              [MAX_ACTIVE_SEQUENCES][NUM_OF_STEPS]  = {{0}};

uint8_t   sequence_active_down      [MAX_ACTIVE_SEQUENCES]                = {0};
uint8_t   currentStepDown           [MAX_ACTIVE_SEQUENCES]                = {NUM_OF_STEPS};
uint32_t  stepUpdateMillisDown      [MAX_ACTIVE_SEQUENCES]                = {0};
uint32_t  stepClearUpdateMillisDown [MAX_ACTIVE_SEQUENCES][NUM_OF_STEPS]  = {0};
uint8_t   downSeq_active            [MAX_ACTIVE_SEQUENCES][NUM_OF_STEPS]  = {0};

uint32_t  sensorUpdateMillis      = 0;
uint32_t  stripClearMillis        = 0;

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

void upSequence() {
  for (uint8_t i = 0; i < MAX_ACTIVE_SEQUENCES; i++) {
    if (sequence_active_up[i]) {
      if (millis() - stepUpdateMillis[i] > STEP_UPDATE_DELAY) {
        if (currentStep[i] < NUM_OF_STEPS) {
          // Turn on the current step
          showStep(currentStep[i] + 1);
          stepUpdateMillis[i] = millis();
          stepClearUpdateMillisUp[i][currentStep[i]] = millis();
          upSeq_active[i][currentStep[i]] = 1;
          // if (DEBUG) { Serial.print("Step Count: "); Serial.println(currentStep[i] + 1); }
          currentStep[i]++;
          if (currentStep[i] == NUM_OF_STEPS) {
            sequence_active_up[i] = 0;
            currentStep[i] = 0;
          }
        }
      }
    }
  }
}

void triggerUpSequence() {
  for (uint8_t i = 0; i < MAX_ACTIVE_SEQUENCES; i++) {
    if (!sequence_active_up[i]) {
      sequence_active_up[i] = 1;
      currentStep[i] = 0;
      stepUpdateMillis[i] = millis();
      break;
    }
  }
}

void downSequence() {
  for (uint8_t i = 0; i < MAX_ACTIVE_SEQUENCES; i++) {
    if (sequence_active_down[i]) {
      if (millis() - stepUpdateMillisDown[i] > STEP_UPDATE_DELAY) {
        if (currentStepDown[i] > 0) {
          // Turn on the current step
          showStep(currentStepDown[i]);
          stepUpdateMillisDown[i] = millis();
          stepClearUpdateMillisDown[i][currentStepDown[i] - 1] = millis();
          downSeq_active[i][currentStepDown[i] - 1] = 1;
          // if (DEBUG) { Serial.print("Step Count: "); Serial.println(currentStepDown[i]); }
          currentStepDown[i]--;
          if (currentStepDown[i] == 0) {
            sequence_active_down[i] = 0;
            currentStepDown[i] = NUM_OF_STEPS;
          }
        }
      }
    }
  }
}

void triggerDownSequence() {
  for (uint8_t i = 0; i < MAX_ACTIVE_SEQUENCES; i++) {
    if (!sequence_active_down[i]) {
      sequence_active_down[i] = 1;
      currentStepDown[i] = NUM_OF_STEPS;
      stepUpdateMillisDown[i] = millis();
      break;
    }
  }
}

void clearSequence() {
  for (uint8_t i = 0; i < MAX_ACTIVE_SEQUENCES; i++) {
    for (uint8_t j = 0; j < NUM_OF_STEPS; j++) {
      if (millis() - stepClearUpdateMillisUp[i][j] > STEP_CLEAR_DELAY) {
        if (upSeq_active[i][j] == 1) {
          clearStep(j + 1);
          upSeq_active[i][j] = 0;
        }
      }
      if (millis() - stepClearUpdateMillisDown[i][j] > STEP_CLEAR_DELAY) {
        if (downSeq_active[i][j] == 1) {
          clearStep(j + 1);
          downSeq_active[i][j] = 0;
        }
      }
    }
  }
}
void readSensors() {
  bool anySequenceActiveUp = false;
  bool anySequenceActiveDown = false;

  // Check if any up sequence is active
  for (uint8_t i = 0; i < MAX_ACTIVE_SEQUENCES; i++) {
    if (sequence_active_up[i] == 1) {
      anySequenceActiveUp = true;
      break;
    }
  }

  // Check if any down sequence is active
  for (uint8_t i = 0; i < MAX_ACTIVE_SEQUENCES; i++) {
    if (sequence_active_down[i] == 1) {
      anySequenceActiveDown = true;
      break;
    }
  }

  // Check if all steps are cleared
  bool allStepsClearedUp = true;
  bool allStepsClearedDown = true;

  for (uint8_t i = 0; i < MAX_ACTIVE_SEQUENCES; i++) {
    for (uint8_t j = 0; j < NUM_OF_STEPS; j++) {
      if (upSeq_active[i][j] == 1) {
        allStepsClearedUp = false;
      }
      if (downSeq_active[i][j] == 1) {
        allStepsClearedDown = false;
      }
    }
  }

  // If no up sequence is active and all up steps are cleared, check SENSOR1
  if (!anySequenceActiveDown && allStepsClearedDown) {
    if (digitalRead(SENSOR1) == LOW) {
      if (millis() - sensorUpdateMillis >= DEBOUNCE_DELAY) {
        sensorUpdateMillis = millis();
        stripClearMillis = millis();
        if (DEBUG) { Serial.println("Sensor 1 Triggered"); }
        triggerUpSequence();
      }
    }
  }

  // If no down sequence is active and all down steps are cleared, check SENSOR2
  if (!anySequenceActiveUp && allStepsClearedUp) {
    if (digitalRead(SENSOR2) == LOW) {
      if (millis() - sensorUpdateMillis >= DEBOUNCE_DELAY) {
        sensorUpdateMillis = millis();
        stripClearMillis = millis();
        if (DEBUG) { Serial.println("Sensor 2 Triggered"); }
        triggerDownSequence();
      }
    }
  }
}

void readSerial(){
  if (Serial.available() > 0) {
    char incoming = Serial.read();
    if (incoming == 'A'){
      // TODO: Call Function to start the sequence.
    }
    if (incoming == 'B'){
      // TODO: Call Function to Clear the sequence.
    }
  }
}

void debugPins(){
  Serial.println("S1: " + String(digitalRead(SENSOR1)) + " \t S2: " + String(digitalRead(SENSOR2)));
}

void sequenceHandler(){
  bool anySequenceActiveUp = false;
  for (uint8_t i = 0; i < MAX_ACTIVE_SEQUENCES; i++) {
    if (sequence_active_up[i] == 1) {
      anySequenceActiveUp = true;
      break;
    }
  }
  if (anySequenceActiveUp) {
    upSequence();
  }
  bool anySequenceActiveDown = false;
  for (uint8_t i = 0; i < MAX_ACTIVE_SEQUENCES; i++) {
    if (sequence_active_down[i] == 1) {
      anySequenceActiveDown = true;
      break;
    }
  }
  if (anySequenceActiveDown) {
    downSequence();
  }
  clearSequence();
}

void setup() {
  Serial.begin(9600);
  io_Setup();
}

void loop() {
  // readSerial();
  readSensors();
  sequenceHandler();
}
