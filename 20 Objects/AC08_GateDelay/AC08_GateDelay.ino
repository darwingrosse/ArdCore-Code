//  ============================================================
//
//  Program: ArdCore Gate Delay
//
//  Description: Keep track of the timing of gates, and delay
//               incoming gates by a user-specified amount.
//               For simplicity sake, this routine only stores
//               a single gate - for more complex delays, you
//               would need to queue the requests.
//
//  I/O Usage:
//    Knob 1: Delay time, gate 1.
//    Knob 2: Delay time, gate 2
//    Analog In 1: incoming gate 1
//    Analog In 2: incoming gate 2
//    Digital Out 1: outgoing gate 1
//    Digital Out 2: outgoing gate 2
//    Clock In: unused
//    Analog Out: unused
//
//  Input Expander: unused
//  Output Expander: unused
//
//  Created:  10 Dec 2010
//  Modified: 12 Mar 2011  ddg - Added license text
//                             - Improved on/off testing
//                             - Cleaned up gate handling
//            17 Apr 2012  ddg Updated for Arduino 1.0
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

//  variables used to control the current DIO output states
int digState[2] = {LOW, LOW};  // start with both set low

//  queues for delayed gates
int gateActive[2] = {0, 0};
int lastActive[2] = {0, 0};

unsigned long startMillis[2] = {0, 0};
unsigned long endMillis[2] = {0, 0};

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
  
}

//  ==================== start of loop() =======================

void loop()
{
  // check analog inputs for gate change.
  for (int i=0; i<2; i++) {
    int tempInVal = (analogRead(i+2) > 250);

    // only do something on a change
    if (tempInVal != lastActive[i]) {
      if (tempInVal) {
        gateActive[i] = 1;
        startMillis[i] = millis();
      } else {
        endMillis[i] = millis();
      }
      lastActive[i] = tempInVal;
    }    
  }

  // deal with turning things on and off  
  for (int i=0; i<2; i++) {
    int tempInVal = (analogRead(i) * 2);

    unsigned long tempSM = startMillis[i] + tempInVal;
    unsigned long tempEM = endMillis[i] + tempInVal;
    
    // check for delay ons
    if ((digState[i] == LOW) && (startMillis[i] > 0) &&
      (millis() > tempSM)) {
        digState[i] = HIGH;
        digitalWrite(digPin[i], HIGH);
        startMillis[i] = 0;
    }
  
    // check for delay offs
    if ((digState[i] == HIGH) && (endMillis[i] > 0) && 
      (millis() > tempEM)) {
      digState[i] = LOW;
      digitalWrite(digPin[i], LOW);
      endMillis[i] = 0;
    }
  }
    
}

//  dacOutput(byte) - deal with the DAC output
//  -----------------------------------------
void dacOutput(byte v)
{
  PORTB = (PORTB & B11100000) | (v >> 3);
  PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);
}

//  ===================== end of program =======================
