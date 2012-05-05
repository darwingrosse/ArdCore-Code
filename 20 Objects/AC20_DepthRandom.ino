//  ============================================================
//
//  Program: ArdCore Depth Random
//
//  Description: Do random output, but only to a user-selected
//               bit depth. Most useful with the expander module
//
//  I/O Usage:
//    Knob 1: Random Depth
//    Knob 2: 
//    Analog In 1: 
//    Analog In 2: 
//    Digital Out 1: Clock display #1
//    Digital Out 2: Clock display #2
//    Clock In: External clock input
//    Analog Out: 8-bit output
//
//  Input Expander: unused
//  Output Expander: 8 bits of output exposed
//
//  Created:  12 Feb 2011
//  Modified: 17 Apr 2012  ddg Updated for Arduino 1.0
//	      18 Apr 2012  ddg Changed dacOutput routine to Alba version
//            04 May 2012  ddg Fixed lack of interrupt issue
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

//  variables for interrupt handling of the clock input
volatile int clkState = LOW;

//  variables used to control the current DIO output states
int digState[2] = {HIGH, LOW};  // start with both set low
int randDepth;                  // the depth of randomization

//  ==================== start of setup() ======================

void setup() {

  Serial.begin(9600);
  randomSeed(analogRead(4));

  
  // set up the digital (clock) input
  pinMode(clkIn, INPUT);
  
  // set up the digital outputs
  for (int i=0; i<2; i++) {
    pinMode(digPin[i], OUTPUT);
    digitalWrite(digPin[i], digState[i]);
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
  int doClock = 0;
  if (clkState == HIGH) {
    clkState = LOW;  // reset for the next clock
    doClock = 1;
  }

  if (doClock) {
    int tempShift = analogRead(0) >> 7;
    int tempRand = (random(256) >> tempShift) << tempShift;
    dacOutput(tempRand);
    
    for (int i=0; i<2; i++) {
      digState[i] = digState[i] == 0;
      digitalWrite(digPin[i], digState[i]);
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
