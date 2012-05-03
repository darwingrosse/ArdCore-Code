//  ============================================================
//
//  Program: ArdCore Triple LFO
//
//  Description:
//
//    This sketch produces three LFOs with independent
//    frequency control: LFO1 and LFO2 have triangle
//    waveforms, while LFO3 has a square waveform.
//    The three LFOs can be reset simultaneously.
//
//    Note that LFO2 must be run through a slew limiter
//    to obtain a smooth waveform.
//
//    The action of the A3 input is customizable and
//    can be used to modulate the frequency or waveshape
//    of any (or all) of the LFOs. 
//    See Customization Section below for details.
//
//  I/O Usage:
//    Knob A0: LFO1 Frequency
//    Knob A1: LFO2 Frequency
//    Analog In A2: LFO3 Frequency
//    Analog In A3: (customizable action - see below)
//    Digital Out D0: LFO2 output (as a PWM wave)
//    Digital Out D1: LFO3 square output
//    Clock In: LFO Reset (sets all phases to zero)
//    Analog Out: LFO1 output
//
//    This sketch was programmed by Alfonso Alba (fac)
//    E-mail: shadowfac@hotmail.com
//
//  Created:  21 Jan 2012
//  Modified: 6 March 2012
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


// ************* START OF CUSTOMIZATION SECTION ****************

// The following macros determine which LFOs can be reset
// by sending a pulse through the Clock input.
//
// If you do not want a LFO to obey a reset pulse, just
// comment the corresponding line.

#define RESET_LFO1
#define RESET_LFO2
#define RESET_LFO3


// The following macros define the action of the A3 input on each LFO.
// These actions are inactive by default because they require an
// input voltage on A3 between 0 and 5V (where 2.5V equals no modulation)

// Waveshape in LFOs 1 and 2 goes from down-saw to triangle to up-saw,
// Waveshape in LFO3 controls pulsewidth.

// Frequency modulation has a multiplicative effect on LFO frequencies.

// Note that you can comment both macros for each LFO so that A3 has 
// no action at all on that LFO, or you can uncomment both macros 
// and have A3 modulate both waveshape and frequency simultaneously.

//#define A3AFFECTS_LFO1_WAVESHAPE
//#define A3AFFECTS_LFO2_WAVESHAPE
//#define A3AFFECTS_LFO3_WAVESHAPE

//#define A3AFFECTS_LFO1_FREQUENCY
//#define A3AFFECTS_LFO2_FREQUENCY
//#define A3AFFECTS_LFO3_FREQUENCY


// ************** END OF CUSTOMIZATION SECTION ****************



//  constants related to the Arduino Nano pin use
#define clkIn      2    // the digital (clock) input
#define digPin0    3    // the digital output pin D0
#define digPin1    4    // the digital output pin D1
#define pinOffset  5    // the first DAC pin (from 5-12)


//  Some global variables
unsigned int curVal0 = 0;      // stores the value of analog input 0
unsigned int curVal1 = 0;      // stores the value of analog input 1
unsigned int curVal2 = 0;      // stores the value of analog input 2
unsigned int curVal3 = 0;      // stores the value of analog input 3

unsigned long oldMicros = 0;
float phase1 = 0, phase2 = 0, phase3 = 0, delta_t;

//  variables for interrupt handling of the clock input
volatile char clkState = LOW;



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
  pinMode(digPin0, OUTPUT);
  digitalWrite(digPin0, LOW);
  pinMode(digPin1, OUTPUT);
  digitalWrite(digPin1, LOW);
  
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
  
  oldMicros = micros();
}


//  This is the main loop

void loop() 
{
  static unsigned long curMicros;
  float freq1, freq2, freq3, shape1, shape2, shape3, p, y;
  
  // check to see if the clock as been set
  if (clkState == HIGH) {
    clkState = LOW;      // clock pulse acknowledged
    #ifdef RESET_LFO1
    phase1 = 0;
    #endif
    #ifdef RESET_LFO2
    phase2 = 0;
    #endif
    #ifdef RESET_LFO3
    phase3 = 0;
    #endif
  }
  
  curVal0 = deJitter(analogRead(0), curVal0);
  curVal1 = deJitter(analogRead(1), curVal1);
  curVal2 = deJitter(analogRead(2), curVal2);
  curVal3 = deJitter(analogRead(3), curVal3);

  freq1 = (float)(curVal0 + 1.0) * 0.01;
  freq2 = (float)(curVal1 + 1.0) * 0.01;
  freq3 = (float)(curVal2 + 1.0) * 0.01;  
  shape1 = 0.5;
  shape2 = 0.5;
  shape3 = 0.5;
  
  p = (float)(curVal3 - 512.0) / 1030.0;
  #ifdef A3AFFECTS_LFO1_WAVESHAPE
  shape1 += p;
  #endif
  #ifdef A3AFFECTS_LFO2_WAVESHAPE
  shape2 += p;
  #endif
  #ifdef A3AFFECTS_LFO3_WAVESHAPE
  shape3 += p;
  #endif
  
  p = pow(8, (float)(curVal3 - 512.0) / 512.0);
  #ifdef A3AFFECTS_LFO1_FREQUENCY
  freq1 *= p;
  #endif
  #ifdef A3AFFECTS_LFO2_FREQUENCY
  freq2 *= p;
  #endif
  #ifdef A3AFFECTS_LFO3_FREQUENCY
  freq3 *= p;
  #endif
  
  
  curMicros = micros();
  delta_t = (curMicros - oldMicros) * 0.000001;
  oldMicros = curMicros;
  
  phase1 += delta_t * freq1;
  phase2 += delta_t * freq2;
  phase3 += delta_t * freq3;
  
  p = phase1 - floor(phase1);
  y = (p <= shape1) ? (p / shape1) : ((1 - p) / (1 - shape1));
  dacOutputFast((long)(y * 255.0f)); 

  p = phase2 - floor(phase2);
  y = (p <= shape2) ? (p / shape2) : ((1 - p) / (1 - shape2));
  analogWrite(digPin0, (long)(y * 255.0f));
  
  p = phase3 - floor(phase3);
  y = (p <= shape3) ? HIGH : LOW;
  digitalWrite(digPin1, y);
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
void dacOutput(long v)
{
  // feed this routine a value between 0 and 255 and teh DAC
  // output will send it out.
  int tmpVal = v;
  for (int i=0; i<8; i++) {
    digitalWrite(pinOffset + i, tmpVal & 1);
    tmpVal = tmpVal >> 1;
  }
}


// This routine was taken from the Ardcore Oscillator example
// and it's supposed to be faster than dacOutput
void dacOutputFast(long v)
{
 int tmpVal = v;
 bitWrite(PORTD, 5, tmpVal & 1);
 bitWrite(PORTD, 6, (tmpVal & 2) > 0);
 bitWrite(PORTD, 7, (tmpVal & 4) > 0);
 bitWrite(PORTB, 0, (tmpVal & 8) > 0);
 bitWrite(PORTB, 1, (tmpVal & 16) > 0);
 bitWrite(PORTB, 2, (tmpVal & 32) > 0);
 bitWrite(PORTB, 3, (tmpVal & 64) > 0);
 bitWrite(PORTB, 4, (tmpVal & 128) > 0);
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
