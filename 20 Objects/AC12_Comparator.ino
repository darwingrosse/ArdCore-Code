//  ============================================================
//
//  Program: ArdCore Comparator
//
//  Description: Using a manual setting, this sketch will 
//               provide two triggers - one when the value
//               moves from below-to-above, and the other
//               when it moves from above-to-below.
//
//  I/O Usage:
//    Knob 1: Level setting
//    Knob 2: unused
//    Analog In 1: Signal to be checked against setting
//    Analog In 2: unused
//    Digital Out 1: Rising trigger
//    Digital Out 2: Dropping trigger
//    Clock In: unused
//    Analog Out: unused
//
//  Input Expander: unused
//  Output Expander: 8 bits of output exposed
//
//  Created:  31 Jan 2011
//  Modified: 13 Mar ddg - Cleaned up trigger and check logic.
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
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)
const int trigTime = 25;       // the trigger timing

//  variables used to control the current DIO output states
int digState[2] = {LOW, LOW};  // start with both set low
unsigned long digMillis[2] = {0, 0};

// variables for comparator detection
int currentState = 0;          // -1 for low, 1 for high

//  ==================== start of setup() ======================

void setup() {
  // Serial.begin(9600);
  
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
  // get the current breakpoint
  int tempBreak = analogRead(0);
  int tempValue = analogRead(2);
  
  /*
  Serial.print(tempBreak);
  Serial.print('\t');
  Serial.print(tempValue);
  Serial.print('\t');
  */
  
  // check for a change in location
  if ((currentState <= 0) && (tempValue > tempBreak)) {
    // Serial.print("up");
    digState[0] = HIGH;
    digitalWrite(digPin[0], HIGH);
    digMillis[0] = millis();
    currentState = 1;
  } else if ((currentState >= 0) && (tempValue < tempBreak)) {
    // Serial.print("down");
    digState[1] = HIGH;
    digitalWrite(digPin[1], HIGH);
    digMillis[1] = millis();
    currentState = -1;
  }
  
  // Serial.println();
  
  // check for trigger turnoffs
  for (int i=0; i<2; i++) {
    if ((digState[i] == HIGH) && (millis() - digMillis[i] > trigTime)) {
      digState[i] = LOW;
      digitalWrite(digPin[i], LOW);
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
