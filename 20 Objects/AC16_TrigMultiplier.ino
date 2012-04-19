//  ============================================================
//
//  Program: ArdCore Trigger Multiplier
//
//  Description: Taking the reading of the first knob (analog 1
//               and 2) we calculate the amount of time to provide
//               multiplied trigger outputs from the digital
//               outputs. The analog inputs are used to surpress
//               the multiplied output, allowing for "ratcheting"
//               of the output values.
//
//  I/O Usage:
//    Knob 1: Output 1 multiplier
//    Knob 2: Output 2 multiplier
//    Analog In 1: HIGH to supress output 1
//    Analog In 2: HIGH to supress output 2
//    Digital Out 1: Output 1
//    Digital Out 2: Output 2
//    Clock In: External clock input
//    Analog Out: unused
//
//  Input Expander: unused
//  Output Expander: unused
//
//  Created:  07 Feb 2011
//  Modified: 14 Mar 2011 ddg - Cleaned up timing calcs.
//            17 Apr 2012  ddg Updated for Arduino 1.0
//						18 Apr 2012	 ddg Changed dacOutput routine to Alba version
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
const int queueSize = 6;       // the size of the timing queue
const int trigTime = 25;       // default of 25 ms trigger time

//  variables for interrupt handling of the clock input
volatile int clkState = LOW;

//  variables used to control the current DIO output states
int digState[2] = {LOW, LOW};        // start with both set low
unsigned long digMilli[2] = {0,0};   // the number of milliseconds since last firing
long digTimes[2] = {10000,10000};

unsigned long lastMilli = 0;         // the last time millis() was called
unsigned long digQueue[queueSize];   // the timing for the last six clock hits
int currentQueue = 0;
int queueReady = 0;

//  ==================== start of setup() ======================

void setup() {
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
  
  // Note: Interrupt 0 is for pin 2 (clkIn)
  attachInterrupt(0, isr, RISING);
}

//  ==================== start of loop() =======================

void loop()
{
  // check for clock input
  if (clkState) {
    if (lastMilli != 0) {
      digQueue[currentQueue] = millis() - lastMilli;
      currentQueue++;
      
      if (currentQueue >= queueSize) {
        currentQueue = 0;
        queueReady = 1;
      }
      
      if (queueReady) {
        long tempInterval = 0;
        for (int i=0; i<queueSize; i++) {
          tempInterval += digQueue[i];
        }
        tempInterval = tempInterval / queueSize;
        
        digTimes[0] = (tempInterval / ((analogRead(0) >> 6) + 1));
        digTimes[1] = (tempInterval / ((analogRead(1) >> 6) + 1));
        
        for (int i=0; i<2; i++) {
          digState[i] = HIGH;
          digMilli[i] = millis();
          digitalWrite(digPin[i], digState[i]);
        }
      }
    }
    lastMilli = millis();
    clkState = 0;
  }
  
  if (queueReady) {
    // check for elapsed time handling
    for (int i=0; i<2; i++) {
      if (millis() - digMilli[i] > digTimes[i]) {
        digMilli[i] = millis();
        if ((analogRead(i+2) < 512) && (digState[i] != HIGH)) {
          digState[i] = HIGH;
          digitalWrite(digPin[i], digState[i]);
        }
      }
    }

    // check for trigger turn-offs
    for (int i=0; i<2; i++) {
      if ((digState[i] == HIGH) && (millis() - digMilli[i] > trigTime)) {
        digState[i] = LOW;
        digitalWrite(digPin[i], digState[i]);
      }
    }
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

//  ===================== end of program =======================
