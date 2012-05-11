//  ============================================================
//
//  Program: ArdCore Template
//
//  Description: A template application for the ArdCore A
//               module from 20 Objects LLC.This sketch acts
//               as a test fixture to verify proper operation
//               of the module.
//
//  NOTE: When you want to test or tune your module, you can
//        run this example program. Here are the steps:
//
//        1. Connect the OUT jack to an oscillator.
//        2. With the A0 knob all the way down, tune the oscillator.
//        3. Turn the A0 knob, adjust the ArdCore trimmer until the
//              octaves are as close to tunes as possible.
//        4. Put a clock source into the CLK jack.
//        5. Verify that the D0 LED is flashing, and the D0 jack
//              is outputting a trigger.
//        6. Change the A1 knob, verify that the D1 is doing a clock
//              division.
//        7. Verify that the D1 jack is outputting a gate at the
//              same rate as the D1 LED.
//        8. Open the Serial Monitor window of the Arduino software
//              environment.
//        9. Put a variable voltage into the A2 and A3 jacks. Verify
//              that they, along with the settings of the A0 and A1
//              knobs, are 0 at counter-clockwise, and 1023 at full
//              clockwise.
//
//        You are done! If it passes all the above tests, you are
//              good to go!
//
//  I/O Usage:
//    Knob 1: Octave selection, prints to serial window
//    Knob 2: Clock divider for digital out 2, prints to window
//    Analog In 1: Prints to serial window only (fixture test)
//    Analog In 2: Prints to serial window only (fixture test)
//    Digital Out 1: External clock display/trigger
//    Digital Out 2: Divided clock display/trigger
//    Clock In: External clock input
//    Analog Out: 8-bit output of octave (for tuning)
//
//  Input Expander: unused
//  Output Expander: 8 bits of output exposed
//
//  Created:  10 April 2011  (Complete rewrite as test fixture)
//  Modified: 17 April 2012  ddg  Updated for Arduino 1.0
//						18 April 2012	 ddg  Changed dacOutput routine to Alba version
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
const int trigTime = 25;       // 25 ms trigger timing

const int oct[6] = {0, 48, 96, 144, 192, 240}; // the six octave values

//  variables for interrupt handling of the clock input
volatile int clkState = LOW;
int clkDivide = 0;

//  variables used to control the current DIO output states
int digState[2] = {LOW, LOW};        // start with both set low
unsigned long digMilli[2] = {0, 0};  // a place to store millis()

//  ==================== start of setup() ======================

//  This setup routine should be used in any ArdCore sketch that
//  you choose to write; it sets up the pin usage, and sets up
//  initial state. Failure to properly set up the pin usage may
//  lead to damaging the Arduino hardware, or at the very least
//  cause your program to be unstable.

void setup() 
{

  // if you need to send data back to your computer, you need
  // to open the serial device. Otherwise, comment this line out.
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
  
  // set up an interrupt handler for the clock in. If you
  // aren't going to use clock input, you should probably
  // comment out this call.
  // Note: Interrupt 0 is for pin 2 (clkIn)
  attachInterrupt(0, isr, RISING);
}


void loop() 
{
  // check to see if the clock as been set
  if (clkState == HIGH) {
    clkState = LOW;
    
    digState[0] = HIGH;
    digMilli[0] = millis();
    digitalWrite(digPin[0], HIGH);
    
    clkDivide++;
    if (clkDivide > (analogRead(1) >> 6)) {
      clkDivide = 0;
      digState[1] = HIGH;
      digMilli[1] = millis();
      digitalWrite(digPin[1], HIGH);
    }
  }
    
  // output the current analog knob 0 setting as an octave voltage
  int tempOct = analogRead(0) / 171;  // (Gets you 0-6)
  dacOutput(oct[tempOct]);
  
  // do we have to turn off any of the digital outputs?
  for (int i=0; i<2; i++) {
    if ((digState[i] == HIGH) && (millis() - digMilli[i] > trigTime)) {
      digState[i] = LOW;
      digitalWrite(digPin[i], LOW);
    }
  }
  
  // print the analog input values
  Serial.print(analogRead(0));   // print the A2 input
  Serial.print('\t');            // print a tab character
  Serial.print(analogRead(1));   // print the A2 input
  Serial.print('\t');            // print a tab character
  Serial.print(analogRead(2));   // print the A2 input
  Serial.print('\t');            // print a tab character
  Serial.print(analogRead(3));   // print the A3 input
  Serial.println();              // line feed
}


//  =================== convenience routines ===================

//  These routines are some things you will need to use for
//  various functions of the hardware. Explanations are provided
//  to help you know when to use them.

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

//  dacOutput(long) - deal with the DAC output
//  ------------------------------------------
void dacOutput(byte v)
{
	/*
  // feed this routine a value between 0 and 255 and teh DAC
  // output will send it out.
  int tmpVal = v;
  for (int i=0; i<8; i++) {
    digitalWrite(pinOffset + i, tmpVal & 1);
    tmpVal = tmpVal >> 1;
  }
  */
  
  // replacement routine as suggested by Alphonso Alba
  // this code accomplishes the same thing as the original
  // code from above, but is approx 4x faster
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

//  quantNote(int) - drop an incoming value to a note value
//  -------------------------------------------------------
int quantNote(int v)
{
  // feed this routine the input from one of the analog inputs
  // and it will return the value in a 0-64 range, which is
  // roughly the equivalent of a 0-5V range.
  return v >> 4;
}

//  ===================== end of program =======================
