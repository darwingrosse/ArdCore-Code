//  ============================================================
//
//  Program: ArdCore GateSequence
//
//  Description: This sketch provides a gate sequence based
//               on an array kept in memory. The output is
//               sent out the 8-bit output, but is primarily
//               used with the output expander.
//
//  I/O Usage:
//    Knob 1: Tempo (0 = clocked only)
//    Knob 2: unused
//    Analog In 1: unused
//    Analog In 2: unused
//    Digital Out 1: Duplicate of gate 0
//    Digital Out 2: Duplicate of gate 1
//    Clock In: External clock input
//    Analog Out: 8-bit output (8 gates)
//
//  Input Expander: unused
//  Output Expander: 8 bits of output exposed
//
//  Created:  03 Feb 2011
//  Modified: 13 Mar 2011 - ddg Code cleanup
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
const int trigTime = 10;       // 10 ms trigger time

//  variables for interrupt handling of the clock input
volatile int clkState = LOW;
int clkStep = 0;

//  the generalized output sequence
int outSeq[8][16] = {
  {1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0},
  {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0},
  {1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1},
  {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
  {0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0},
  {1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1}};

// timer variables
unsigned long clkMilli = 0;
int trigState = 0;

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

  // set up the interrupt  
  attachInterrupt(0, isr, RISING);
}

//  ==================== start of loop() =======================

void loop()
{
  int tmpClock = 0;
  
  int tmpTimer = ((511 - (analogRead(0) >> 1)) * 2) + 15;
  if (tmpTimer > 1030) {
    tmpTimer = 0;
  }
  
  if (clkState == HIGH) {
    tmpClock = 1;
    clkState = LOW;
  }
  
  if ((tmpTimer > 0) && ((millis() - clkMilli) > tmpTimer)) {
    tmpClock = 1;
  }
  
  if (tmpClock) {
    // update the time
    clkMilli = millis();
    
    // output the gates
    for (int i=0; i<8; i++) {
      digitalWrite(pinOffset+i, outSeq[i][clkStep]);
    }
    
    // output the digitals
    for (int i=0; i<2; i++) {
      digitalWrite(digPin[i], outSeq[i][clkStep]);
    }

    // update the pointer
    clkStep++;
    if (clkStep >= 16) {
      clkStep = 0;
    }
    
    trigState = 1;
  }
  
  if (((millis() - clkMilli) > trigTime) && (trigState)) {
    trigState = 0;
    for (int i=0; i<8; i++) {
      digitalWrite(pinOffset + i, LOW);
    }
    for (int i=0; i<2; i++) {
      digitalWrite(digPin[i], LOW);
    }
  }
}

//  =================== convenience routines ===================

//  dacOutput(byte) - deal with the DAC output
//  -----------------------------------------
void dacOutput(byte v)
{
  PORTB = (PORTB & B11100000) | (v >> 3);
	PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);
}

//  isr() - quickly handle interrupts from the clock input
//  ------------------------------------------------------
void isr()
{
  clkState = HIGH;
}

//  ===================== end of program =======================
