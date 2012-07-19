//  ============================================================
//
//  Program: ArdCore FM Oscillator
//
//  This program implements two sinewave oscillators in an
//  FM modulator/carrier configuration.
//
//  I/O Usage:
//    Knob A0: Main pitch
//    Knob A1: Modulator pitch offset
//    Analog In A2: 1 V/Oct pitch input
//    Analog In A3: Modulation index (depth)
//    Digital Out D0: Not used
//    Digital Out D1: Not used
//    Clock In: Carrier hard sync 
//    Analog Out: Audio out
//
//    This sketch was programmed by Alfonso Alba (fac)
//    E-mail: shadowfac@hotmail.com
//
//  Created:  March 2012
//  Modified: 
//
//  ============================================================
//
//  License:
//
//  This software is licensed under the Creative Commons
//  Attribution-NonCommercial license. This license allows you
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



#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

#include <avr/pgmspace.h>

#include "sine_table.h"
#include "pitch_table.h"
#include "offset_table.h"
#include "cv_table.h"

//  constants related to the Arduino Nano pin use
#define clkIn      2    // the digital (clock) input
#define digPin0    3    // the digital output pin D0
#define digPin1    4    // the digital output pin D1
#define pinOffset  5    // the first DAC pin (from 5-12)

  
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

  // Timer2 PWM Mode set to fast PWM 
  cbi (TCCR2A, COM2A0);
  sbi (TCCR2A, COM2A1);
  sbi (TCCR2A, WGM20);
  sbi (TCCR2A, WGM21);
  cbi (TCCR2B, WGM22);
  // Timer2 Clock Prescaler to : 1 
  sbi (TCCR2B, CS20);
  cbi (TCCR2B, CS21);
  cbi (TCCR2B, CS22);

  // Timer2 PWM Port Enable
  sbi(DDRB,3);                    // set digital pin 11 to output

  //cli();                         // disable interrupts to avoid distortion
  cbi (TIMSK0,TOIE0);              // disable Timer0 !!! delay is off now
  sbi (TIMSK2,TOIE2);              // enable Timer2 Interrupt
}


//  This is the main loop

boolean div1 = false, div2 = false, poll_inputs = false;
long n, p, out;
  
//  Some global variables
short curVal0 = 0;      // stores the value of analog input 0
short curVal1 = 0;      // stores the value of analog input 1
short curVal2 = 0;      // stores the value of analog input 2
short curVal3 = 0;      // stores the value of analog input 3

long m_phase = 0;      // modulator phase (12.20 fixed point)
long c_phase = 0;      // carrier phase (12.20 fixed point)
long m_dphase = 0;     // modulator frequency (12.20 fixed point)
long c_dphase = 0;     // carrier frequency (12.20 fixed point)
long m_depth = 0;      // modulation depth (22.10 fixed point)

long dphase;


void loop() {
  unsigned long ul1, ul2, ul3;
  float freq;
  
//  if (poll_inputs) {
    poll_inputs = false;
  
    curVal0 = deJitter(analogRead(0), curVal0);
    curVal1 = deJitter(analogRead(1), curVal1);
    curVal2 = deJitter(analogRead(2), curVal2);
    curVal3 = deJitter(analogRead(3), curVal3);
  
    ul1 = pgm_read_dword_near(pitch_table + curVal0);
    freq = *((float *)&ul1);
    ul2 = pgm_read_dword_near(cv_table + curVal2);
    freq *= *((float *)&ul2);
  
    freq *= ((long)1 << 20);
  
    // obtain main frequency 
    c_dphase = (long)(freq + 0.5);
  
    // obtain modulator frequency
    ul3 = pgm_read_dword_near(offset_table + curVal1);
    m_dphase = (long)(freq * *((float *)&ul3) + 0.5);
    
    // obtain modulation depth
    m_depth = curVal3;
//  }
}


ISR(TIMER2_OVF_vect) {
  div1 = !div1;
  if (div1) {
    div2 = !div2;
    if (div2) {
      m_phase += m_dphase;
      if (m_phase >= 1073741824) m_phase -= 1073741824;  
      out = sine_wave[m_phase >> 20];  
      
      // f_c = f_c * (1 + I * out_m), where I in (0,1) and out_m in (-1, 1)
      dphase = (c_dphase >> 10) * ((long)1024 + ((out * m_depth) >> 5));
      c_phase += dphase;
      if (c_phase >= 1073741824) c_phase -= 1073741824;  
      out = sine_wave[c_phase >> 20];
      dacOutputFast(out + 128);      
    }
  }
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
//  poll_inputs = true;
  c_phase = 0;
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
 bitWrite(PORTB, 4, (tmpVal & 128) > 0);
 bitWrite(PORTB, 3, (tmpVal & 64) > 0);
 bitWrite(PORTB, 2, (tmpVal & 32) > 0);
 bitWrite(PORTB, 1, (tmpVal & 16) > 0);
 bitWrite(PORTB, 0, (tmpVal & 8) > 0);
 bitWrite(PORTD, 7, (tmpVal & 4) > 0);
 bitWrite(PORTD, 6, (tmpVal & 2) > 0);
 bitWrite(PORTD, 5, tmpVal & 1);
}

void dacOutputFaster(byte v) {
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
  if (abs(v - test) > 4) {
    return v;
  }
  return test;
}



//  ===================== end of program =======================
