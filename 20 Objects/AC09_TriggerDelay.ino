//  ============================================================
//
//  Program: ArdCore Trigger Delay
//
//  Description: Keep track of the timing of triggers, and
//               delay the outputs appropriately. Note: this
//               routine only saves the last trigger point.
//               For more complex operation, you would have
//               to queue the triggers.
//
//  I/O Usage:
//    Knob 1: Delay time, trigger 1.
//    Knob 2: Delay time, trigger 2
//    Analog In 1: unused
//    Analog In 2: unused
//    Digital Out 1: outgoing trigger 1
//    Digital Out 2: outgoing trigger 2
//    Clock In: trigger input
//    Analog Out: unused
//
//  Input Expander: unused
//  Output Expander: unused
//
//  Created:  10 Dec 2010
//  Modified: 13 Mar 2011  ddg - Added license text
//                             - Cleaned timing vars
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
const int trigTime = 50;       // 50 ms triggers

//  variables for interrupt handling of the clock input
volatile int clkState = LOW;

//  variables used to control the current DIO output states
int digState[2] = {LOW, LOW};  // start with both set low

//  queues for delayed gates
unsigned long lastTrig[2] = {0, 0};
unsigned long lastMillis[2] = {0, 0};

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
  
  attachInterrupt(0, isr, RISING);
}

//  ==================== start of loop() =======================

void loop()
{
  if (clkState == HIGH) {
    clkState = LOW;  // reset for the next clock
    lastTrig[0] = lastTrig[1] = millis();
  }

  // check for trigger onset
  for (int i=0; i<2; i++) {
    if ((lastTrig[i] > 0) && (digState[i] == LOW) && 
     (millis() - lastTrig[i] > (analogRead(i)))) {
      digState[i] = HIGH;
      digitalWrite(digPin[i], HIGH);
      lastMillis[i] = millis();
      lastTrig[i] = 0;
    }
  }
  
  // check for trigger turn-off
  for (int i=0; i<2; i++) {
    if ((digState[i] == HIGH) && (millis() - lastMillis[i] > trigTime)) {
      digState[i] = LOW;
      digitalWrite(digPin[i], LOW);
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
