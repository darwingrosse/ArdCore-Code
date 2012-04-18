// Based on http://arduino.cc/en/Tutorial/Fading by David A. Mellis 
// and Tom Igoe, http://www.arduino.cc/playground/Code/PwmFrequency,
// and viznut's, skurk's, and raer's discoveries documented in 
// http://countercomplex.blogspot.com/2011/10/algorithmic-symphonies-from-one-line-of.html
#include "wiring_private.h"
#undef round
#undef abs

// The speaker is on Arduino pin 11, which is port B pin 3, pin 17 on the PDIP.
// The signal on this pin is OC2A.

void setup()  { 
  // set bit 3 in data direction register B to 1 to configure Port B pin 3 for output:
 // pinMode(11, OUTPUT);
  pinMode(3, OUTPUT);
  //DDRB |= 1 << 3;
 // // I forget what this does!
  //TCCR2A |= 1 << COM2A1;
  // use 0x01 for divisor of 1, i.e. 31.25kHz
  // or 0x02 for divisor of 8, or 0x03 for divisor of 64 (default)
 // TCCR2B = TCCR2B & 0xf8 | 0x01;
} 

long t = 0;
int i = 0;

void loop() {
  if (++i != 64) return;
  i = 0;
  t++;
 // OCR2A 
  long myValue;

  myValue= (((t*(t>>12)&(201*t/100)&(199*t/100))&(t*(t>>14)&(t*301/100)&(t*399/100)))+((t*(t>>16)&(t*202/100)&(t*198/100))-(t*(t>>18)&(t*302/100)&(t*298/100))));

analogWrite(3, myValue);


  return;



return;
}
