//  ============================================================
//
//  Program: ArdCore Analog Tracker
//
//  Description: Watch the analog input and follow the values with
//               the eight OX outputs
//
//  I/O Usage:
//    Knob 1: Sensitivity (input level expansion)
//    Knob 2: 
//    Analog In 1: Analyzed input
//    Analog In 2: 
//    Digital Out 1: Trigger on value change
//    Digital Out 2: Trigger on max input
//    Clock In: 
//    Analog Out: Step outputs
//
//  Input Expander: unused
//  Output Expander: Scaled output gate (1 at a time)
//
//  Created:  17 May 2012  ddg
//  Modified: 
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
const int digPin[2] = {3, 4};  // the digital pin array
const int dacOffset = 5;       // the OX/DAC pin offset
const int trigTime = 25;       // 25 ms trigger time
const int SETTLEVAL = 3;       // the number of consecutive values
                               //   to consider the read to be
                               //   settled.

//  variables used to control the D0/D1 outputs
int digState[2] = {LOW, LOW};
unsigned long digMilli[2] = {0, 0};
unsigned long currMillis = 0;

//  variables used to control the current output states
int cSense = -1;
int cCalc = -1;
int cStep = -1;

// variables used to settle the analog input before update
int sCalc = -1;
int sCount = 0;

//  ==================== start of setup() ======================

void setup() {

  // Serial.begin(9600);
  
  // set up the digital (clock) input
  pinMode(clkIn, INPUT);
  
  // set up the digital outputs
  for (int i=0; i<2; i++) {
    pinMode(digPin[i], OUTPUT);
    digitalWrite(digPin[i], LOW);
  }
  
  // set up the digital outputs
  for (int i=0; i<8; i++) {
    pinMode(dacOffset + i, OUTPUT);
    digitalWrite(dacOffset + i, LOW);
  }
  
  // get the starting values
  readAnalog();
}

//  ==================== start of loop() =======================

void loop()
{
  unsigned long thisMilli = millis();
  readAnalog();

  /*
  Serial.print(cSense);
  Serial.print('\t');
  Serial.print(cCalc);
  Serial.println();
  */
  
  // Settle down the input values (since the analog reads done
  // at a high rate can be unstable) by requiring 3 reads of the
  // same value before you can proceed.
  if (cCalc == sCalc) {
    sCount++;
  } else {
    sCalc = cCalc;
    sCount = 0;
  }
  
  // If the values are different and the read is settles, update
  // the output and start it over.
  if ((cCalc != cStep) && (sCount >= SETTLEVAL)) {
    cStep = cCalc;
    dacOutput(1 << cStep);
    
    digitalWrite(digPin[0], HIGH);
    digState[0] = HIGH;
    digMilli[0] = thisMilli;
    
    if (cStep == 7) {
      digitalWrite(digPin[1], HIGH);
      digState[1] = HIGH;
      digMilli[1] = thisMilli;
    }
    
    sCount = 0;
  }
  
  // turn off D0/D1 if necessary
  for (int i=0; i<2; i++) {
    if ((digState[i]) && (thisMilli - digMilli[i] > trigTime)) {
      digitalWrite(digPin[i], LOW);
      digState[i] = LOW;
    }
  }

}

//  =================== convenience routines ===================

//  readAnalog() - read the analog inputs and set values
//  ----------------------------------------------------
void readAnalog()
{
  cSense = (128 - (analogRead(0) >> 3));
  cCalc = analogRead(2) / cSense;
  if (cCalc > 7)  cCalc = 7;
  if (cCalc < 0)  cCalc = 0;
}

//  dacOutput(byte) - deal with the DAC output
//  -----------------------------------------
void dacOutput(byte v)
{
  PORTB = (PORTB & B11100000) | (v >> 3);
  PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);
}

//  ===================== end of program =======================
