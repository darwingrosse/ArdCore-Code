//  ============================================================
//
//  Program: ArdCore AutoSwitch
//
//  Description: This sketch takes analog input into 3/4
//               and will switch them into the output every
//               time a clock value is received, or auto-
//               switched.
//
//               NOTE: Due to the nature of 8-bit output
//                     values, incoming values will be
//                     automatically quantized to 1/4 steps
//
//  I/O Usage:
//    Knob 1: Autoswitching speed (0-4 = clock only)
//    Knob 2: unused
//    Analog In 1: Input 1
//    Analog In 2: Input 2
//    Digital Out 1: HIGH when input 1 is switched in
//    Digital Out 2: HIGH when input 2 is switched in
//    Clock In: External switching input
//    Analog Out: 8-bit output
//
//  Input Expander: unused
//  Output Expander: 8 bits of output exposed
//
//  Created:  05 Feb 2011
//  Modified: 13 Mar 2011 - ddg  Updated timer handling
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

//  variables for interrupt handling of the clock input
volatile int clkState = LOW;

//  variables used to control the current DIO output states
int digState[2] = {HIGH, LOW};  // start with both set low
int doSwitch = 0;               // 1 if a switch needs turning
unsigned long autoMillis = 0;   // time since last autoswitch

//  ==================== start of setup() ======================

void setup() {
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
  
  // set up an interrupt handler for the clock in.
  attachInterrupt(0, isr, RISING);
}

//  ==================== start of loop() =======================

void loop()
{
  // check for clock input
  if (clkState) {
    doSwitch = 1;
    clkState = 0;
  }
  
  // check for autoswitch
  int tempTime = (1023 - analogRead(0) + 5) * 4;
  if (tempTime > 4000) {
    tempTime = 0;
  }

  if ((tempTime > 0) && (millis() - autoMillis > tempTime)) {
    doSwitch = 1;
  }

  // do a switch if necessary
  if (doSwitch) {
    digState[0] = !digState[0];
    digState[1] = !digState[0];
    
    for (int i=0; i<2; i++) {
      digitalWrite(digPin[i], digState[i]);
    }
    
    doSwitch = 0;
    autoMillis = millis();
  }
  
  // pass through the proper value
  if (digState[0]) {
    dacOutput(analogRead(2) >> 2);
  } else {
    dacOutput(analogRead(3) >> 2);
  }
}

//  =================== convenience routines ===================

//  isr() - quickly handle interrupts from the clock input
//  ------------------------------------------------------
void isr()
{
  // Note: you don't want to spend a lot of time here, because
  // it interrupts the activity of the rest of your program.
  // In most cases, you just want to set a variable and get
  // out.
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
