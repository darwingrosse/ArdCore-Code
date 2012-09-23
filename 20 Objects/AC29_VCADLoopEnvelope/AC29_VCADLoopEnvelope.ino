//  ============================================================
//
//  Program: ArdCore VCEnvGenerator
//
//  Description: A sketch that produces an attack and decay
//               envelope upon the receipt of a trigger at the
//               clock input.
//
//  I/O Usage:
//    Knob 1: Attack time standard
//    Knob 2: Decay time standard
//    Analog In 1: Attack time voltage adder
//    Analog In 2: Decay time voltage adder
//    Digital Out 1: Gate on envelope active
//    Digital Out 2: Gate on envelope inactive
//    Clock In: Trigger in starts the envelope section
//    Analog Out: 8-bit output
//
//  Input Expander: unused
//  Output Expander: 8 bits of output exposed
//
//  Created:  19 Mar 2011  ddg
//  Modified: 17 Apr 2012  ddg Updated for Arduino 1.0
//	      18 Apr 2012  ddg Changed dacOutput routine to Alba version
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

//  constants related to the Arduino Nano pin use
const int clkIn = 2;           // the digital (clock) input
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)
const int trigTime = 25;       // ms for a trigger output

volatile int clkState = LOW;

int digState[2] = {LOW, LOW};
unsigned long digMilli[2] = {0, 0};

int envState = 0;              // 0=off, 1=rising, -1 = falling
float riseValue = 0.0;
float fallValue = 0.0;
float currValue = 0.0;

//  ==================== start of setup() ======================
void setup() {
  // set up the digital (clock) input
  pinMode(clkIn, INPUT);
  attachInterrupt(0, isr, RISING);
  
  // set up the 8-bit DAC output pins
  for (int i=0; i<8; i++) {
    pinMode(pinOffset+i, OUTPUT);
    digitalWrite(pinOffset+i, LOW);
  }  

  // set up the digital outputs
  for (int i=0; i<2; i++) {
    pinMode(digPin[i], OUTPUT);
    digitalWrite(digPin[i], i);
  }  
}

//  ==================== start of loop() =======================

void loop()
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
  if (abs(v - test) > 8) {
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

//  ===================== end of program =======================
