
//  ============================================================
//
//  Program: ArdCore Master Clock
//
//  Description: Given incoming clock pulses, output two
//               triggers with varying delays, as well as
//               user-defined clock divisions/alternations.
//
//    Knob 1: Clock speed - turn to 0 for clocked-only functionality
//    Knob 2: Trigger time (5-261 ms)
//    Analog In 1: Additional division offset (1-16)
//    Analog In 2: HIGH to reset clock 1 & 2
//    Digital Out 1: Trigger output 1
//    Digital Out 2: Trigger output 2
//    Clock In: External trigger input
//    Analog Out: unused
//
//  Input Expander: unused
//  Output Expander: unused
//
//  Created:  07 Dec 2010
//  Modified: 17 Jan 2011  ddg Reduce default trigtime to 10 ms.
//                             Change milli store to uint.
//            25 Feb 2011  ddg Changed reset to handle both units
//                             Added voltage controlled offset.
//            11 May 2012  ddg Revert to saved version.
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

const int MAX_CLOCK = 31;

const int clockMap[8][32] = {
  {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
  {0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1},
  {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0},
  {0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0},
  {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0}
};

int currClock = 0;

//  constants related to the Arduino Nano pin use
const int clkIn = 2;           // the digital (clock) input
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)

//  variables for interrupt handling of the clock input
volatile int clkState = LOW;
int clockTick[2] = {1, 1};

//  variables used to control the current DIO output states
int digState[2] = {LOW, LOW};  // start with both set low
unsigned long prevMilli[2] = {0, 0};     // the last time of a loop

int currTick = 0; // ticks for the output expander
unsigned long currMilli = 0;
int currState = 0;

// variables for timing loop
unsigned long prevTiming = 0;    // the last time of a timed loop
int interval = 10;               // the last interval value
int doStep = 0;                  // do we perform a step move?

int onoffState = 0;              // the on/off state (from analog 3)
int oldState = 0;                // the old on/off state

int trigTime = 25;               // triggers are variable.

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
  
  attachInterrupt(0, isr, RISING);
}

//  ==================== start of loop() =======================
void loop()
{
  int i;
  unsigned long thisMillis = millis();
  
  doStep = 0;
  
  // service a clock trigger
  if (clkState == HIGH) {
     clkState = LOW;
     if (interval >= 1270) {
       doStep = 1;
     }
  }
  
  // check for a timer hit
  if ((interval < 1270) && ((thisMillis - prevTiming) > interval)) {
    prevTiming = thisMillis;
    doStep = 1;
  }

  // if we are off, don't do anything
  if (!onoffState) {
    doStep = 0;
  }

  // do our Step function
  if (doStep) {
    // fire off the step blinker
    digState[0] = HIGH;
    prevMilli[0] = thisMillis;
    digitalWrite(digPin[0], HIGH);

    currClock -= 1;
    if (currClock < 0) {
      currClock = MAX_CLOCK;
    }
    
    for (i=0; i<8; i++) {
      if (clockMap[i][MAX_CLOCK-currClock] > 0) {
        digitalWrite(pinOffset + i, HIGH);
      }
    }
  }
  
  // do a state change
  if (oldState != onoffState) {
    oldState = onoffState;
    
    if (onoffState == 1) {
      digitalWrite(digPin[1], HIGH);
      delay(5);
      digitalWrite(digPin[1], LOW);
    }
  }
  
  // deal with trigger turnoff
  if ((digState[0] == HIGH) && ((thisMillis - prevMilli[0]) > trigTime)) {
    digState[0] = LOW;
    digitalWrite(digPin[0], LOW);
    
    for (i=0; i<8; i++) {
      digitalWrite(pinOffset + i, 0);
    }
  }
  
  // get the current user settings
  interval = (((1023 - analogRead(0)) >> 4) * 20) + 30;
  trigTime = (analogRead(1) >> 2) + 5;
  
  onoffState = analogRead(3) > 512;
  if (onoffState != oldState) {
    prevTiming = 0;
    
    for (i=0; i<8; i++) {
      currClock = 0;
    }
  }
}

//  =================== convenience routines ===================

//  isr() - quickly handle interrupts from the clock input
//  ------------------------------------------------------
void isr()
{
  // Note: you don't want to spend a lot of time here, because
  // it interrupts the activity of the rest of your program.
  // In most cases, you just want to set a variable and get
  // out.
  clkState = HIGH;
}

//  ===================== end of program =======================
