//  ============================================================
//
//  Program: ArdCore Standards
//
//  Description: Produce 1 volt and stepped outputs based on
//               knob settings
//
//  I/O Usage:
//    Knob 1: Output octave (1V/oct)
//    Knob 2: Output step (0-11 steps)
//    Analog In 1: unused
//    Analog In 2: unused
//    Digital Out 1: unused
//    Digital Out 2: unused
//    Clock In: unused
//    Analog Out: 8-bit output
//
//  Input Expander: unused
//  Output Expander: 8 bits of output exposed
//
//  Created:  13 Feb 2011
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
const int clkIn = 2;           // the digital (clock) input
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)

int last_1 = -200;            // start from scratch
int last_2 = -200;            // start from scratch

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
}

//  ==================== start of loop() =======================

void loop()
{
  last_1 = deJitter(analogRead(0), last_1);
  last_2 = deJitter(analogRead(1), last_2);

  int outValue = (last_1 / 170) * 48;
  outValue += (last_2 / 86) * 4;
  if (outValue > 255) {
    outValue = 255;
  }
  
  dacOutput(outValue);
}

//  =================== convenience routines ===================

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
  // this routine just make sure we have a significant value
  // change before we bother implementing it. This is useful
  // for cleaning up jittery analog inputs.
  if (abs(v - test) > 8) {
    return v;
  }
  return test;
}

//  ===================== end of program =======================
