//  ============================================================
//
//  Program: ArdCore Slope Detector
//
//  Description: This sketch examines an input and fires triggers
//               whenever the slope of the incoming value changes
//               direction.
//
//  I/O Usage:
//    Knob 1: unused
//    Knob 2: unused
//    Analog In 1: Signal to be checked
//    Analog In 2: unused
//    Digital Out 1: Rising slope trigger
//    Digital Out 2: Dropping slope trigger
//    Clock In: unused
//    Analog Out: unused
//
//  Input Expander: unused
//  Output Expander: unused
//
//  Created:  02 Feb 2011
//  Modified: 13 Mar 2011 ddg - deJitter input read for stability
//                            - fixed trigger display output
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
const int trigTime = 10;       // the trigger timing

//  variables used to control the current DIO output states
int digState[2] = {LOW, LOW};  // start with both set low
unsigned long digMillis[2] = {0, 0};

// variables for comparator detection
int currentValue = 0;          // stored value for detection
int currentDir = 0;            // -1 for downward, 1 for upward

//  ==================== start of setup() ======================

void setup() {
  Serial.begin(9600);
  
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
  // get the current value
  int tempValue = deJitter(analogRead(2), currentValue);
  
  Serial.print(currentValue);
  Serial.print("-");
  Serial.println(tempValue);
  
  if ((currentDir <= 0) && (tempValue > currentValue)) {
    digState[0] = HIGH;
    digitalWrite(digPin[0], HIGH);
    digMillis[0] = millis();
    currentDir = 1;
  } else if ((currentDir >= 1) && (tempValue < currentValue)) {
    digState[1] = HIGH;
    digitalWrite(digPin[1], HIGH);
    digMillis[1] = millis();
    currentDir = -1;
  }
  
  currentValue = tempValue;
  
  // check for trigger turnoffs
  for (int i=0; i<2; i++) {
    if ((digState[i] == HIGH) && (digMillis[i] - millis() > trigTime)) {
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

//  deJitter(int, int) - smooth jitter input
//  ----------------------------------------
int deJitter(int v, int test)
{
  if (abs(v - test) > 8) {
    return v;
  }
  return test;
}

//  ===================== end of program =======================
