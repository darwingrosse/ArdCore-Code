//  ============================================================
//
//  Program: ArdCore Variation Generator
//
//  Description: Given a two-dimensional pattern, step through
//               the pattern for all digital outputs, varying
//               the vertical and horizontal offsets based on
//               knob settings and analog input.
//
//  I/O Usage:
//    Knob 1: Static vertical offset
//    Knob 2: Static horizontal offset
//    Analog In 1: Dynamic vertical offset
//    Analog In 2: Dynamic horizontal offset
//    Digital Out 1: Output 1
//    Digital Out 2: Output 2
//    Clock In: External clock input
//    Analog Out: Outputs 3-10
//
//  Input Expander: unused
//  Output Expander: Outputs 3-10 individually
//
//  Created:  11 Feb 2011
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
const int pinOffset = 3;       // the first digital pin
const int trigTime = 25;       // 25 ms trigger time

//  variables for interrupt handling of the clock input
volatile int clkState = LOW;

//  variables used to control the current DIO output states
int digState[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int onState = 0;

// clocking variables
unsigned long lastMillis = 0;
int cStep = 0;

// pattern
int pattern[10][16] = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0},
  {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0},
  {1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0},
  {0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0},
  {1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0},
  {0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0},
  {1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0},
  {0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1},
  {0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0}};

//  ==================== start of setup() ======================

void setup() {

  Serial.begin(9600);

  // set up the digital (clock) input
  pinMode(clkIn, INPUT);

  // set up the digital outputs
  for (int i=0; i<10; i++) {
    pinMode(pinOffset + i, OUTPUT);
    digitalWrite(pinOffset + i, LOW);
  }

  // Note: Interrupt 0 is for pin 2 (clkIn)
  attachInterrupt(0, isr, RISING);
}

//  ==================== start of loop() =======================

void loop()
{
  // Check for clocking
  int doClock = 0;
  if (clkState) {
    doClock = 1;
    clkState = 0;
  }

  // Determine offsets
  int vShift = (analogRead(0) >> 7) + (analogRead(2) >> 7);
  int hShift = (analogRead(1) >> 7) + (analogRead(3) >> 7);

  // if clocked, do output
  if (doClock) {
    for (int i=0; i<10; i++) {
      digState[i] = pattern[(i+vShift)%10][(cStep+hShift)%16];
      digitalWrite(pinOffset + i, digState[i]);
    }

    cStep++;
    if (cStep >= 16) {
      cStep = 0;
    }

    onState = 1;
    lastMillis = millis();
  }

  // if trigTime complete, turn off
  if ((onState) && (millis() - lastMillis > trigTime)) {
    for (int i=0; i<10; i++) {
      if (digState[i]) {
        digitalWrite(pinOffset + i, LOW);
      }
    }
    onState = 0;
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
