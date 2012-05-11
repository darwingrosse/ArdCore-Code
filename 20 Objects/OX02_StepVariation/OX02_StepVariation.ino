//  ============================================================
//
//  Program: ArdCore Step Variation
//
//  Description: Given a two-dimensional pattern, step through
//               the pattern for the generation of individual
//               steps. Requires the Output Expander to be of
//               any real use...
//
//  I/O Usage:
//    Knob 1: Variation Selector
//    Knob 2: 
//    Analog In 1: 
//    Analog In 2: 
//    Digital Out 1: 
//    Digital Out 2: 
//    Clock In: External clock input
//    Analog Out: Step outputs
//
//  Input Expander: unused
//  Output Expander: Outputs 3-10 individually
//
//  Created:  11 Feb 2011
//  Modified: 17 Apr 2012  ddg Updated for Arduino 1.0
//	      18 Apr 2012  ddg Changed dacOutput routine to Alba version
//            08 May 2012  ddg Adapted from the AC18_VariationGenerator
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
const int digOffset = 5;       // the OX digital pin offset
const int trigTime = 5;        // 5 ms trigger time
const int SEQSIZE = 16;        // sequencer are 16 bytes long

//  variables for interrupt handling of the clock input
volatile int clkState = LOW;

//  variables used to control the current DIO output states
int trigState = 0;
int digState = -1;
int onState = 0;

// clocking variables
int gateTime = 25;
unsigned long lastMillis = 0;
int cStep = 0;

// sequence identifiers
int holdSeq = 0;
int hSeq = 0;
int cSeq = 0;

// pattern
int pattern[10][SEQSIZE] = {
  { 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7},
  { 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0},
  { 0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0},
  { 0, 2, 4, 6, 1, 3, 5, 7, 0, 2, 4, 6, 1, 3, 5, 7},
  { 0, 1, 2, 3, 0, 1, 2, 3, 4, 5, 6, 7, 4, 5, 6, 7},
  { 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 4, 1},
  { 0, 1, 2, 1, 2, 3, 2, 3, 4, 3, 4, 5, 4, 5, 6, 7},
  { 0, 2, 1, 3, 2, 4, 3, 5, 4, 6, 5, 7, 6, 5, 3, 1},
  { 0, 2, 4, 6, 1, 3, 5, 7,-1, 6, 5, 4, 3, 2, 1,-1},
  { 0, 7, 1, 6, 2, 5, 3, 4, 0, 7, 1, 6, 2, 5, 3, 4}
};

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
  
  // get the starting values
  readAnalog();
}

//  ==================== start of loop() =======================

void loop()
{
  // Check for clocking
  int doClock = 0;
  if (clkState) {
    doClock = 1;
    clkState = 0;
    Serial.println("Clock Hit");
  }
  
  // if clocked, do output
  if (doClock) {
    // kill the current step (if active)
    if (onState) {
      digitalWrite(pinOffset, LOW);
      digitalWrite(pinOffset + 1, LOW);
      digitalWrite(digOffset + digState, LOW);
      trigState = 0;
      onState = 0;
    }
    
    // increment the current step, adjust seq
    cStep++;
    if (!holdSeq)  cSeq = hSeq;
    
    if (cStep >= SEQSIZE) {
      cStep = 0;
      if (holdSeq) cSeq = hSeq;
    }

    // turn on the proper OX step
    digState = pattern[cSeq][cStep];
    Serial.println(digState);

    if (digState >= 0) {
      digitalWrite(digOffset + digState, HIGH);
      digitalWrite(pinOffset, HIGH);
      digitalWrite(pinOffset + 1, HIGH);
      
      onState = 1;
      trigState = 1;
      lastMillis = millis();
    }
  }
    
  // if trigTime complete, turn off
  if ((trigState) && (millis() - lastMillis > trigTime)) {
    digitalWrite(pinOffset, LOW);
    trigState = 0;
  }

  // if gateTime complete, turn off
  if ((onState) && (millis() - lastMillis > gateTime)) {
    digitalWrite(pinOffset + 1, LOW);
    digitalWrite(digOffset + digState, LOW);
    onState = 0;
  }

  readAnalog();
}

//  =================== convenience routines ===================

//  isr() - quickly handle interrupts from the clock input
//  ------------------------------------------------------
void isr()
{
  clkState = HIGH;
}

//  readAnalog() - read the analog inputs and set values
//  ----------------------------------------------------
void readAnalog()
{
  // Determine sequence and gate time
  hSeq = ((analogRead(0) / 103) + (analogRead(2) / 103)) % SEQSIZE; 
  gateTime = (analogRead(1) + 1) * 3;  
  holdSeq = (analogRead(3) > 511);
}

//  dacOutput(byte) - deal with the DAC output
//  -----------------------------------------
void dacOutput(byte v)
{
  PORTB = (PORTB & B11100000) | (v >> 3);
  PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);
}

//  ===================== end of program =======================
