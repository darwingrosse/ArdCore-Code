
//  ============================================================
//
//  Program: ArdCore ClockDivide
//
//  Description: Given incoming clock pulses, output two
//               triggers with varying delays
//
//    Knob 1: Division of clock out 1 (1-32)
//    Knob 2: Division of clock out 2 (1-32)
//    Analog In 1: Additional division offset (1-16)
//    Analog In 2: HIGH to reset clock 1 & 2
//    Digital Out 1: Trigger output 1
//    Digital Out 2: Trigger output 2
//    Clock In: External trigger input
//    Analog Out: unused
//
//  Input Expander: unused
//  Output Expander: unused
//
//  Created:  07 Dec 2010
//  Modified: 17 Jan 2011  ddg Reduce default trigtime to 10 ms.
//                             Change milli store to uint.
//            25 Feb 2011  ddg Changed reset to handle both units
//                             Added voltage controlled offset.
//            17 Apr 2012  ddg Updated for Arduino 1.0
//						18 Apr 2012	 ddg  Changed dacOutput routine to Alba version
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
const int trigTime = 10;       // triggers are 10 ms.

//  variables for interrupt handling of the clock input
volatile int clkState = LOW;
int clockTick[2] = {1, 1};

//  variables used to control the current DIO output states
int digState[2] = {LOW, LOW};  // start with both set low
unsigned long prevMilli[2] = {0, 0};     // the last time of a loop

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
  
  attachInterrupt(0, isr, RISING);
}

//  ==================== start of loop() =======================
void loop()
{
  int i;

  Serial.print(analogRead(0));
  Serial.print('\t');
  Serial.print(analogRead(1));
  Serial.print('\t');
  Serial.print(analogRead(2));
  Serial.print('\t');
  Serial.println(analogRead(3));
  
  // deal with possible reset
  for (i=0; i<2; i++) {
    if (analogRead(3) > 511) {
      clockTick[i] = (analogRead(i) >> 6) + 1;
    }
  }
  
  // deal with incoming clock ticks
  if (clkState == HIGH) {
    clkState = LOW;  // reset for the next clock
    
    for (int i=0; i<2; i++) {
      clockTick[i] --;
      if (clockTick[i] < 1) {
        digState[i] = HIGH;
        prevMilli[i] = millis();
        digitalWrite(digPin[i], HIGH);
        
        clockTick[i] = (analogRead(i) >> 6) + 1 + (analogRead(2) >> 6);
      }
    }
  }
  
  // deal with trigger turnoff
  for (int i=0; i<2; i++) {
    if ((digState[i] == HIGH) && ((millis() - prevMilli[i]) > trigTime)) {
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
