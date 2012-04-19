//  ============================================================
//
//  Program: ArdCore Trigger to Gate
//
//  Description: Accept a trigger in the clock input, then
//               generate two user-adjustable gates.
//
//  I/O Usage:
//    Knob 1: Output 1 duration
//    Knob 2: Output 2 duration
//    Analog In 1: Gate 1 duration adder
//    Analog In 2: Gate 2 duration adder
//    Digital Out 1: Gate output 1
//    Digital Out 2: Gate output 2
//    Clock In: External clock input
//    Analog Out: unused
//
//  Input Expander: unused
//  Output Expander: unused
//
//  Created:  08 Dec 2010
//  Modified: 25 Feb 2011  ddg Tightened up timing
//                             Added analog input variation
//            17 Apr 2012  ddg Updated for Arduino 1.0
//						18 Apr 2012	 ddg Changed dacOutput routine to Alba version
//
//  ============================================================

//  ================= start of global section ==================

//  constants related to the Arduino Nano pin use
const int clkIn = 2;           // the digital (clock) input
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)

//  variables for interrupt handling of the clock input
volatile int clkState = LOW;

//  variables used to control the current DIO output states
int digState[2] = {LOW, LOW};  // start with both set low
long prevMill[2] = {0, 0};           // the last time of a timed loop
long interval[2] = {50, 50};          // the interval for the 1st loop

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
  
  // Note: Interrupt 0 is for pin 2 (clkIn)
  attachInterrupt(0, isr, RISING);
}

//  ==================== start of loop() =======================

void loop()
{
  if (clkState == HIGH) {
    clkState = LOW;  // reset for the next clock

    for (int i=0; i<2; i++) {
      digState[i] = HIGH;
      digitalWrite(digPin[i], HIGH);
      prevMill[i] = millis();
    }
  }
  
  for (int i=0; i<2; i++) {
    interval[i] = (analogRead(i) * 2) + (analogRead(i+2) * 2) + 20;
    if ((digState[i] == HIGH) && (millis() - prevMill[i] > interval[i])) {
      digState[i] = LOW;
      digitalWrite(digPin[i], LOW);
    }
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
