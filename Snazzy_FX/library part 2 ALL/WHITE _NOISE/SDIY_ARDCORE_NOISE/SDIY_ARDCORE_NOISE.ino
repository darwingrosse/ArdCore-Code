//  ============================================================
//
//  Program: ArdCore WHITE NOISE
//
//  Description: 

//  I/O Usage:
//    Knob 1:
//    Knob 2: unused
//    Analog In 1: 
//    Analog In 2: unused
//    Digital Out 1: unused
//    Digital Out 2: unused
//    Clock In: unused
//    Analog Out: white noise
//




#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>


const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)


uint32_t state = 42;

void setup() {
pinMode(11, OUTPUT);
TCCR2A = _BV(COM2A1) | _BV(WGM20);
TCCR2B = _BV(CS20);
TIMSK2 = _BV(TOIE2);

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
  
}

void loop() { }

SIGNAL(TIMER2_OVF_vect)
{
 state = (state >> 1) ^ (-(state & 1) & 0xd0000001);
 OCR2A = state & 0xff;
 dacOutput(state & 0xff);
}

//  dacOutput(long) - deal with the DAC output
//  ------------------------------------------
void dacOutput(long v)
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

