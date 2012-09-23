#include <EEPROM.h>

//  ============================================================
//
//  Program: ArdCore SH-101 voltage recorder emulation
//
//  Description: Given incoming clock pulses, record pitch values in a 64 step buffer in record mode,
//  and play them back in play mode
//
//    Knob 1: Loop start point
//    Knob 2: Loop end point
//    Analog In 1: Incoming pitch to be recorded
//    Analog In 2: Set HIGH for record mode on
//    Digital Out 1: record mode status
//    Digital Out 2: gate on output
//    Clock In: External trigger input
//    Analog Out: Pitch output as individual bits.
//
//  Input Expander: unused
//  Output Expander: unused
//
//  Created:  13 July 2012
//  Modified: 14 July 2012  ddg  Some minor updates
//
//
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

#define MAXPOS 63

//  constants related to the Arduino Nano pin use
const int clkIn = 2;           // the digital (clock) input
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)
const int trigTime = 10;       // triggers are 10 ms.

//  constant for actual 0-5V quantization (vs. >> 4)
const int qArray[61] = {
  0,   9,   26,  43,  60,  77,  94,  111, 128, 145, 162, 180, 
  197, 214, 231, 248, 265, 282, 299, 316, 333, 350, 367, 384, 
  401, 418, 435, 452, 469, 486, 503, 521, 538, 555, 572, 589, 
  606, 623, 640, 657, 674, 691, 708, 725, 742, 759, 776, 793, 
  810, 827, 844, 862, 879, 896, 913, 930, 947, 964, 981, 998, 
  1015};

//  variables for interrupt handling of the clock input
volatile int clkState = LOW;
int clockTick[2] = {1, 1};

//  variables used to control the current DIO output states
int digState[2] = {LOW, LOW};  // start with both set low
long digMilli[2] = {0, 0};      // used for trigger timing

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
  Serial.begin(9600);
  
  // set up the digital (clock) input
  pinMode(clkIn, INPUT);
  
  // set up the digital outputs
  for (int i=0; i<2; i++) {
    pinMode(digPin[i], OUTPUT);
    digitalWrite(digPin[i], LOW);
  }
  
  // set up the 8-bit DAC output pins
  for (int i=0; i<8; i++) {
    pinMode(pinOffset+i, OUTPUT);
    digitalWrite(pinOffset+i, LOW);
  }
  
  readEEPROM();
  attachInterrupt(0, isr, RISING);
}

//  ==================== start of loop() =======================
void loop()
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
