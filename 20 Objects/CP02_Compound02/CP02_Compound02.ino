#include <EEPROM.h>

//  ============================================================
//
//  Program: ArdCore QuadSketch #2
//
//  Description: A sketch that runs one of four sketches based
//               on the initial state of the A0 knob.
//
//  I/O Usage: dependent on sketch...
//  Sketch numbers:
//  0: VCADLoopEnvelope
//  1: ShapedLFO
//  2: DrunkenNote
//  3: SH101-Recorder
//
//  Created:  28 Oct 2012  ddg
//  Modified: 
//
//  ============================================================
//
//  License:
//
//  This software is licensed under the Creative Commons
//  "Attribution-NonCommercial license. This license allows you
//  to tweak and build upon the code for non-commercial purposes,
//  without the requirement to license derivative works on the
//  same terms. If you wish to use this (or derived) work for
//  commercial work, please contact 20 Objects LLC at our website
//  (www.20objects.com).
//
//  For more information on the Creative Commons CC BY-NC license,
//  visit http://creativecommons.org/licenses/
//
//  ================= start of global section ==================

//  --> General use variables

#define MAXPOS 63

//  constants related to the Arduino Nano pin use
const int clkIn = 2;           // the digital (clock) input
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)
const int trigTime = 25;       // ms for a trigger output

//  constant for actual 0-5V quantization (vs. >> 4)
const int qArray[61] = {
  0,   9,   26,  43,  60,  77,  94,  111, 128, 145, 162, 180, 
  197, 214, 231, 248, 265, 282, 299, 316, 333, 350, 367, 384, 
  401, 418, 435, 452, 469, 486, 503, 521, 538, 555, 572, 589, 
  606, 623, 640, 657, 674, 691, 708, 725, 742, 759, 776, 793, 
  810, 827, 844, 862, 879, 896, 913, 930, 947, 964, 981, 998, 
  1015};

volatile int clkState = LOW;

int digState[2] = {LOW, LOW};
unsigned long digMilli[2] = {0, 0};
unsigned long digTimes[2] = {10, 10};   // the timing settings

byte outValue = 0;             // the DAC output value

//  variable for timing loops
unsigned long prevMillis = 0;  // the last time of a timed loop
int interval = 10;             // the last interval value

//  common variables
float currValue = 0.0;

//  variable for sketch selection
int sketchVar = -1;

//  --> Variables for sketch 0: VCADLoopEnvelope
//  --------------------------------------------

int envState = 0;              // 0=off, 1=rising, -1 = falling
float riseValue = 0.0;
float fallValue = 0.0;

//  --> Variables for sketch 2: ShapedLFO
//  -------------------------------------

float upStep = 1.0;            // value update per millisecond (up)
float downStep = 1.0;          // value update per millisecond (down)
int currDir = 1;               // 1 for up, 0 for down

//  --> Variables for sketch 2: Drunken Note
//  ----------------------------------------

//  program variables of interest
int doStep = 0;                // Turn on to do a step
int randVal = 3;               // Gives us -1 to 1 to start

//  --> Variables for sketch 3: 101-SEQ
//  ------------------------------------

int clockTick[2] = {1, 1};

// variables for use of record/playback management
int currentPos = 0;
byte recordBuffer[MAXPOS + 1];

byte loopStart = 0;
byte loopEnd   = 0;

int recState = LOW;
int lastState = LOW;

byte loopMax = 0;

//  ==================== start of setup() ======================
void setup() {
  // get the sketch to use
  sketchVar = analogRead(0) >> 8;
  
  // set up the digital (clock) input
  pinMode(clkIn, INPUT);
  
  // set up the 8-bit DAC output pins
  for (int i=0; i<8; i++) {
    pinMode(pinOffset+i, OUTPUT);
    digitalWrite(pinOffset+i, LOW);
  }  

  // set up the digital outputs
  for (int i=0; i<2; i++) {
    pinMode(digPin[i], OUTPUT);
    digitalWrite(digPin[i], LOW);
  }
  
  switch (sketchVar) {
    case 0:
      setup_0();
      break;
    case 1:
      setup_1();
      break;
    case 2:
      // setup_2();
      break;
    case 3:
      // setup_3();
      break;
  }
  
  attachInterrupt(0, isr, RISING);
}

//  ==================== start of loop() =======================
void loop()
{
  switch (sketchVar) {
    case 0:
      loop_0();
      break;
    case 1:
      loop_1();
      break;
    case 2:
      loop_2();
      break;
    case 3:
      loop_3();
      break;
  }
}

//  ================= start of individual setups ================

void setup_0() {
  digitalWrite(digPin[1], HIGH);
}

void setup_1() {
  getUpdateValue();
}

void setup_3() {
  readEEPROM();
}

//  ================= start of individual loops =================


void loop_0()
{
  int doStart = 0;
  
  // if we get a clock, start the envelope
  if (clkState) {
    clkState = 0;
    doStart = 1;
  }
  
  // if we are at a start point, do rise handling
  if (doStart) {
    doStart = 0;
    envState = 1;    
  }
  
  // make the necessary change to the value
  if (envState == 1) {
    currValue += riseValue;
  } else if (envState == -1) {
    currValue -= fallValue;
  }
  
  // check for peak transition, do fall handling
  if (currValue > 255.0) {
    currValue = 255.0;
    envState = -1;
  }
  
  // check for end transition, do end handling
  if (currValue < 0.0) {
    currValue = 0.0;
    envState = 0;
  }
  
  // output the value
  dacOutput(int(currValue));
  
  if (envState == 0) {
    if (digState[0] != LOW) {
      digState[0] = LOW;
      digState[1] = HIGH;
      for (int i=0; i<2; i++) {
        digitalWrite(digPin[i], digState[i]);
      }
    } 
  } else {
    if (digState[0] != HIGH) {
      digState[0] = HIGH;
      digState[1] = LOW;
      for (int i=0; i<2; i++) {
        digitalWrite(digPin[i], digState[i]);
      }
    } 
  }

  // read the values and adjust
  int riseSetting = analogRead(0) + analogRead(2) + 5;
  int fallSetting = analogRead(1) + analogRead(3) + 5;
  
  riseValue = 255.0 / riseSetting;
  fallValue = 255.0 / fallSetting;  
}

void loop_1()
{
  // do an output
  float lastValue = currValue;
  unsigned long currMillis = millis();
  int timediff = currMillis - prevMillis;
  
  if (currValue > 255.0) {
    currDir = 0;
  } else {
    currDir = 1;
  }
  
  if (currDir) {
    currValue = currValue + (timediff * upStep);
  } else {
    currValue = currValue + (timediff * downStep);
  }
  
  while (currValue > 511.0) {
    currValue -= 511.0;
  }
  
  if (currValue <= 255.0) {
    dacOutput(int(currValue));
  } else {
    dacOutput(int(511.0 - currValue));
  }
  
  prevMillis = currMillis;
  
  // check for digital pin firing
  if ((lastValue <= 255.) && (currValue > 255.)) {
    digState[0] = HIGH;
    digitalWrite(digPin[0], HIGH);
    digMilli[0] = currMillis;
  }
  
  if (lastValue > currValue) {
    digState[1] = HIGH;
    digitalWrite(digPin[1], HIGH);
    digMilli[1] = currMillis;
  }
  
  // check for pin turn-off
  for (int i=0; i<2; i++) {
    if ((digState[i] == HIGH) && (currMillis - digMilli[i] > trigTime)) {
      digState[i] = LOW;
      digitalWrite(digPin[i], LOW);
    }
  }
  
  // check for new values
  getUpdateValue();
}

void getUpdateValue()
{
  float msPerCycle = ((1023 - analogRead(0)) + 20) * 3.0;
  float warpFactor = ((analogRead(1) >> 4) + 1) / 65.0;
  float oneOver = 1.0 - warpFactor;
  
  upStep = 255.0 / (msPerCycle * warpFactor);
  downStep = 255.0 / (msPerCycle * oneOver);
}

void loop_2()
{
  // service a clock trigger
  if (clkState == HIGH) {
    clkState = LOW;
    doStep = 1;
  }
  
  // test for a timer hit
  interval = ((1023 - analogRead(0)) >> 4) * 20;
  if ((interval < 1200) && ((millis() - prevMillis) > interval)) {
    prevMillis = millis();
    doStep = 1;
  }
  
  // do the random walk step
  if (doStep) {
    doStep = 0;
    int tmpOut = keepIn60(outValue + (analogRead(2) >> 4));
    dacOutput(tmpOut << 2);
    
    digitalWrite(digPin[0], HIGH);
    digState[0] = HIGH;
    digMilli[0] = millis();
    
    // get the value jump
    randVal = ((analogRead(1) >> 6) * 2) + 3;
    
    // calculate the next value
    int newVal = outValue;
    if (analogRead(3) < 511) {
      while (newVal == outValue) {
        newVal = keepIn60(outValue + random(randVal) - (randVal / 2));
      }
    } else {
      newVal = keepIn60(outValue + random(randVal) - (randVal / 2));
    }
    outValue = newVal;
  

  }
  
  // test for trigger turnoff
  if ((digState[0] == HIGH) && (millis() - digMilli[0] > trigTime)) {
    digitalWrite(digPin[0], LOW);
    digState[0] = LOW;
  }
}

void loop_3()
{
  int i, j, k;

  // deal with possible change of record/play mode
  i = analogRead(3) > 511;
  
  
  // This determines if a record mode setting needs to change. NOTE:
  // most of the complexity comes because of the auto-turnoff at the
  // end of recording a full buffer
  if ((i) && (recState == LOW) && (lastState == LOW)) {
    recState = HIGH;
    lastState = HIGH;
    currentPos = 0;
    loopMax = 0;
    digitalWrite(digPin[0], recState);
  } else if ((i == LOW) && (recState == HIGH) && (lastState == HIGH)) {
    recState = LOW;
    writeEEPROM();
    
    lastState = LOW;
    digitalWrite(digPin[0], recState);
    currentPos = loopStart;
  } else if ((i == HIGH) && (recState == LOW) && (lastState == HIGH)) {
    digitalWrite(digPin[0], recState);
  } else if ((i == LOW) && (recState == LOW) && (lastState == HIGH)) {
    lastState = LOW;
  }
  
  // deal with incoming clock ticks
  if (clkState == HIGH) {
    clkState = LOW;  // reset for the next clock
    
    if (recState == HIGH) {
      // store the data in the array
      recordBuffer[currentPos] = vQuant(analogRead(2));
      loopMax = currentPos;
      currentPos++;
      
      // echo the value to the analog out and D1 trigger
      dacOutput((byte) recordBuffer[loopMax] << 2);
      digitalWrite(digPin[1], HIGH);
      digState[1] = HIGH;
      digMilli[1] = millis();

      //switch to play mode at end of record buffer
      if (currentPos > MAXPOS){    
        recState = LOW;
        writeEEPROM();
        currentPos = loopStart;
      }
     } else {    
      // double-check position
      if (currentPos < loopStart) {
        currentPos = loopStart;
      }
      if (currentPos > loopEnd) {
        currentPos = loopStart;
      }

      // playback
      dacOutput((byte) recordBuffer[currentPos] << 2);
      digitalWrite(digPin[1], HIGH);
      digState[1] = HIGH;
      digMilli[1] = millis();
      
      // increment, and check for wrap-around
      currentPos++;
      if ((currentPos >= loopEnd) || (currentPos >= loopMax)) {
        currentPos = loopStart;
      }
     }
  }
  
  // test for trigger turnoff
  if ((digState[1] == HIGH) && (millis() - digMilli[1] > trigTime)) {
    digitalWrite(digPin[1], LOW);
    digState[1] = LOW;
  }

  // read the loop knobs values and update the settings
  i = analogRead(0) >> 4;  // array size dependent
  loopStart = i;
  if (loopStart > loopMax) {
    loopStart = loopMax;
  }

  i = analogRead(1) >> 4;  // array size dependent
  loopEnd = i;
  if (loopEnd > loopMax) {
    loopEnd = loopMax;
  }

  if (loopEnd < loopStart) {
    loopEnd = loopStart;
  }
}

//  =================== convenience routines ===================

//  isr() - quickly handle interrupts from the clock input
//  ------------------------------------------------------
void isr()
{
  clkState = HIGH;
}

//  deJitter(int, int) - smooth jitter input
//  ----------------------------------------
int deJitter(int v, int test)
{
  if (abs(v - test) > 4) {
    return v;
  }
  return test;
}

//  dacOutput(byte) - deal with the DAC output
//  -----------------------------------------
void dacOutput(byte v)
{
  PORTB = (PORTB & B11100000) | (v >> 3);
  PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);
}

//  vQuant(int) - properly convert an ADC reading to a value
//  ---------------------------------------------------------
int vQuant(int v)
{
  int tmp = 0;
  
  for (int i=0; i<61; i++) {
    if (v >= qArray[i]) {
      tmp = i;
    }
  }
  
  return tmp;
}

//  keepIn60(int) - keep things in the 0-60 (5V) range
//  --------------------------------------------------
int keepIn60(int v)
{
  int ov = v;
  
  if (ov > 60)
    ov = 60 - (ov - 60);
  if (ov < 0)
    ov = 0 - ov;   
    
  return ov;
}

//  writeEEPROM() - write the data to a tagged EEPROM set
//  -----------------------------------------------------
void writeEEPROM()
{
  const int eOffset = 5;
  
  // write out the tag
  EEPROM.write(0, 'A');
  EEPROM.write(1, 'C');
  EEPROM.write(2, '2');
  EEPROM.write(3, '7');
  
  EEPROM.write(4, loopMax);
  
  for (int i=0; i<=MAXPOS; i++) {
    EEPROM.write(eOffset+i, recordBuffer[i]);
  }
}

//  readEEPROM() - read the data from a tagged EEPROM set
//  -----------------------------------------------------
void readEEPROM()
{  
  const int eOffset = 5;
  
  if (EEPROM.read(0) != 'A')  goto clearit;
  if (EEPROM.read(1) != 'C')  goto clearit;
  if (EEPROM.read(2) != '2')  goto clearit;
  if (EEPROM.read(3) != '7')  goto clearit;
  
  loopMax = EEPROM.read(4);
  
  for (int i=0; i<=MAXPOS; i++) {
    recordBuffer[i] = EEPROM.read(eOffset + i);
  }
  
  return;
  
clearit:
  loopMax = 0;
  
  for (int i=0; i<=MAXPOS; i++) {
    recordBuffer[i] = 0;
  }
} 

//  ===================== end of program =======================

