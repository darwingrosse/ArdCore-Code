//  ============================================================
//
//  Program: ArdCore Note Delay
//
//  Description: Watch for an incoming note, then delay it for
//               the selected amount of time. Allow optional
//               note transposition +/- 12 steps.
//
//  NOTE: Requires Alexander Brevig's SimpleFIFO library, 
//        available at:
//        http://code.google.com/p/alexanderbrevig/downloads/list
//
//  I/O Usage:
//    Knob 1: Delay time.
//    Knob 2: unused
//    Analog In 1: incoming note value
//    Analog In 2: unused
//    Digital Out 1: trigger on playback
//    Digital Out 2: unused
//    Clock In: triggers queueing operation
//    Analog Out: outgoing note value
//
//  Input Expander: unused
//  Output Expander: unused
//
//  Created:  28 Jan 2011
//  Modified: 13 Mar 2011 ddg - Added license text
//                            - Created new queueing system
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

#include <SimpleFIFO.h>

struct StoreNote {
  int value;
  unsigned long saveTime;
};

//  constants related to the Arduino Nano pin use
const int clkIn = 2;           // the digital (clock) input
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)
const int trigTime = 25;       // default trigger time to 25 milliseconds

//  variables used to control the current DIO output states
int digState[2] = {LOW, LOW};  // start with both set low
unsigned long digTime = 0;     // the delay time for a trigger

//  variables for clocking
int clkState = LOW;

//  variable for internal use
SimpleFIFO<StoreNote, 100> sFIFO;  // store up to 100 notes
StoreNote currNote;

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
  
  // set up clock interrupt  
  attachInterrupt(0, isr, RISING);
}

//  ==================== start of loop() =======================

void loop()
{
  int doStore = 0;
  
  // deal with a clock
  if (clkState == HIGH) {
    doStore = 1;
    clkState = LOW;
  }
  
  // deal with a store request
  if (doStore) {
    StoreNote newNote;
    newNote.value = analogRead(2) >> 4;
    newNote.saveTime = millis();
    sFIFO.enqueue(newNote);
    doStore = 0;
  }
  
  // get the current delay time
  int tempDelay = (analogRead(0) * 2);
    
  // check for note firing
  if (sFIFO.count() > 0) {
    currNote = sFIFO.peek();
    while ((sFIFO.count() > 0) && (millis() - currNote.saveTime > tempDelay)) {
      currNote = sFIFO.dequeue();
      dacOutput(currNote.value << 2);
      
      digitalWrite(digPin[0], HIGH);
      digState[0] = HIGH;
      digTime = millis();
      
      if (sFIFO.count() > 0) {
        currNote = sFIFO.peek();
      }
    }
  }
  
  // check for digital turn-off
  if ((digState[0] == HIGH) && (millis() - digTime > trigTime)) {
    digState[0] = LOW;
    digitalWrite(digPin[0], LOW);
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
