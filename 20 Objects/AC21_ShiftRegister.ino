//  ============================================================
//
//  Program: ArdCore Shift Register
//
//  Description: Record 8 values, provide shift register output
//               based on the first knob, and transpose based
//               on the second knob.
//
//  I/O Usage:
//    Knob 1: Shift register offset (1 to 8 steps)
//    Knob 2: Shift register transpose (-12 to 12 steps)
//    Analog In 1: Shift register input
//    Analog In 2: unused
//    Digital Out 1: Trigger on shift output
//    Digital Out 2: unused
//    Clock In: External clock input
//    Analog Out: 8-bit output
//
//  Input Expander: unused
//  Output Expander: 8 bits of output exposed
//
//  Created:  13 Feb 2011
//  Modified: 13 Mar 2011  ddg - Fixed variable name error.
//                             - Improved calculation of step selection
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
const int trigTime = 25;       // 25 ms default trigger time

//  variables for interrupt handling of the clock input
volatile int clkState = LOW;

//  variables used to control the current DIO output states
int digState[2] = {LOW, LOW};  // start with both set low
unsigned long dispMillis  = 0;

//  variables for shift register output
int value[9] = {-1,-1,-1,-1,-1,-1,-1,-1,-1};
int currValue = 0;

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
  
  // Note: Interrupt 0 is for pin 2 (clkIn)
  attachInterrupt(0, isr, RISING);
}

//  ==================== start of loop() =======================

void loop()
{
  int doShift = 0;

  if (clkState == HIGH) {
    clkState = LOW;  // reset for the next clock
    doShift = 1;
  }
  
  if (doShift) {
    // store the current value
    value[currValue] = analogRead(2) >> 2;
    currValue++;
    if (currValue > 8) {
      currValue = 0;
    }
    
    int tempOffset = 7 - (analogRead(0) >> 7);
    int outStep = (currValue + tempOffset) % 9;
    int outValue = value[outStep];
    int transAmt = ((analogRead(1) / 41) - 12) << 2;
    
    if (outValue > -1) {
      dacOutput(outValue + transAmt);
    
      digState[0] = HIGH;
      digitalWrite(digPin[0], HIGH);
      dispMillis = millis();
    }
    
    /*
    Serial.print(currValue);
    Serial.print('\t');
    Serial.print(outStep);
    Serial.print('\t');
    Serial.print(outValue);
    Serial.print('\t');
    Serial.print(transAmt);
    Serial.println();
    */
  }
  
  if ((digState[0]) && ((millis() - dispMillis) > trigTime)) {
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
