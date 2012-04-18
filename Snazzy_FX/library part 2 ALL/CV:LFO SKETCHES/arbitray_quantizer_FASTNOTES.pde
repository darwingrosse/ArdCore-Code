//  ============================================================
//
//  Program: MODDED arbitrary FAST quantizer
//
//  Description: take in something and put out weird CV
//
//  I/O Usage:
//    Knob 1: Transposition of note
//    Knob 2: Duration of output gate
//    Analog In 1: Input voltage to be quantized
//    Analog In 2: If HIGH, suppress quantization
//    Digital Out 1: Trigger on output
//    Digital Out 2: Gate on output
//    Clock In: External trigger of quantization
//    Analog Out: Quantizer output
//
//  Input Expander: unused
//  Output Expander: 8 bits of output exposed
//
//  Created:  28 Nov 2010
//  Modified: 04 Dec 2010  ddg Various bug fixes.
//            06 Dec 2010  ddg Rework suppression after HW fix.
//            17 Jan 2011  ddg Change milli store to uint
//                             reduce trigger time to 10 ms
//            19 Feb 2011  ddg Force quantization on clock
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
int digState[2] = {LOW, LOW};  // start with both set low
unsigned long digMilli[2] = {0, 0};     // the timing variables
unsigned long digTimes[2] = {10, 10};   // the timing settings

// variables of interest
int inValue = 0;               // the input value
int testValue = -1;            // the test value
int quantValue = -1;           // the quantized value

int outValue = 0;              // the DAC output value
int oldTranspose = -1;         // the test transpose value
int transpose = 0;             // the transposition amount

int doQuant = 0;               // flag to do calcs
int gateDuration = 0;  // the duration of the output gate
float notesArray[35] = { 16, 48.1, 99, 11, 5.22, 12.5, 99, 154, 9.5, 10.4, 25.7, 6.66, 56, 3, 88.1, 154.3, 2.9, 186.1, 99.3, 20.5 ,44.4 , 21.1, 45.2, 77.7, 54.2, 23.22, 23.3, 68.4, 89.5,30.7,  23.4, 6.2, 9.55, 43.5, 9} ;

//  ==================== start of setup() ======================
//  Standard setup routine - see ArdCore_Template for info.

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

  // set up the clock input
  attachInterrupt(0, isr, RISING);
}

//  ==================== start of loop() =======================

void loop()
{
  // test for transpose change
  transpose = analogRead(0) / 86;
  if (transpose != oldTranspose) {
    oldTranspose = transpose;
    doQuant = 1;
  }

  // test for input change
  inValue = analogRead(2);
  
      doQuant = 1;
    
  
  
  

  // test for clock tick (even if suppressed, we quantize on clock)
  if (clkState == HIGH) {
    clkState = LOW;
    doQuant = 1;
  }
  
  // do the quantization
  if (doQuant) {
    // send the note
    outValue = quantNote(inValue);
    dacOutput(outValue);
    
    // do the triggers and gates
    for (int i=0; i<2; i++) {
      digitalWrite(digPin[i], HIGH);
      digState[i] = HIGH;
      digMilli[i] = millis();
    }
    
    doQuant = 0;
  }
  
  // get the current gate time  
  digTimes[1] = analogRead(1);
  
  // test for trigger and gate turnoff
  for (int i=0; i<2; i++) {
    if ((digState[i] == HIGH) && (millis() - digMilli[i] > digTimes[i])) {
      digitalWrite(digPin[i], LOW);
      digState[i] = LOW;
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

//  dacOutput(int) - deal with the DAC output
//  -----------------------------------------
void dacOutput(int v)
{
  int tmpVal = v;
  for (int i=0; i<8; i++) {
    digitalWrite(pinOffset + i, tmpVal & 1);
    tmpVal = tmpVal >> 1;
  }
}

//  deJitter(int, int) - smooth jitter input
//  ----------------------------------------
int deJitter(int v, int test)
{
  // A musically useful range of deJittering for a quantizer
  // is an input wobble of +/- 8. If you are working with
  // continuous values, you might want to change this to a
  // smaller value, or simply uncomment the next line:
  //
  // return v;
  
  if (abs(v - test) > 8) {
    return v;
  }
  return test;
}

//  quantNote(int) - drop an incoming value to a note value
//  -------------------------------------------------------
int quantNote(int v)
{  
  int outVal;

  int tempVal = v ;
  
  
  
  tempVal= v/29 ;//(around 35 range);  // decrease the value to 0-64 - ~ a 5 volt range

 
  outVal = notesArray[tempVal];
  
  outVal += transpose;  // add the transposition
  return (outVal<<2 ) ; // increase it to the full 8-bit range
}
//  ===================== end of program =======================
