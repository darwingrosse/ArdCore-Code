//  ============================================================
//
//  Program: ArdCore Logic Module
//
//  Description: This sketch reads the two analog inputs and
//               produces AND, OR and XOR outputs from the
//               digital outputs and the analog output.
//
//  I/O Usage:
//    Knob 1: Unused
//    Knob 2: Unused
//    Analog In 1: Input 1
//    Analog In 2: Input 2
//    Digital Out 1: AND output (HIGH if both on)
//    Digital Out 2: OR output (HIGH if one is on)
//    Clock In: Unused
//    Analog Out: XOR output (all HIGH if only one is on)
//
//  Input Expander: unused
//  Output Expander: 8 bits of output exposed
//
//  Created:  09 Feb 2011
//  Modified: 17 Apr 2012  ddg Updated for Arduino 1.0
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
const int analogIn[2] = {2, 3};  // the analog inputs to measure
const int digPin[2] = {3, 4};    // the digital output pins
const int pinOffset = 5;         // the first DAC pin (from 5-12)

//  variables used to control the current DIO output states
int anaState[2] = {0, 0};      // the analog input reads
int digState[2] = {LOW, LOW};  // start with both set low

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
  anaState[0] = analogRead(analogIn[0]) > 200;
  anaState[1] = analogRead(analogIn[1]) > 200;

  digitalWrite(digPin[0], (anaState[0] && anaState[1]));
  digitalWrite(digPin[1], (anaState[0] || anaState[1]));
  
  if (anaState[0] != anaState[1]) {
    dacOutput(255);
  } else {
    dacOutput(0);
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

//  ===================== end of program =======================
