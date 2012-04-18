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
  //pinMode(11, OUTPUT);
  DDRB |= 1 << 3;
  // I forget what this does!
  TCCR2A |= 1 << COM2A1;
  // use 0x01 for divisor of 1, i.e. 31.25kHz
  // or 0x02 for divisor of 8, or 0x03 for divisor of 64 (default)
  TCCR2B = TCCR2B & 0xf8 | 0x01;
} 

long t = 0;
int i = 0;

void loop() {
  if (++i != 64) return;
  i = 0;
  t++;
  OCR2A = ((t<<1)^((t<<1)+(t>>7)&t>>12))|t>>(4-(1^7&(t>>19)))|t>>7;
  return;
  
  switch ((t >> 22) & 3) {
    case 0:
      OCR2A = t*2*(char((t>>10)^(t>>10)-2)%11) | t >> 2 | (char(t>>15^t>>16^0x75)>>(7&t>>10)&1?0:-1);
      break;
    case 1:
      OCR2A = 255-((1L<<28)/(1+(t^0x5800)%0x8000) ^ t | t >> 4 | -t >> 10);
      break;
    case 2:
      OCR2A = ((t<<1)^((t<<1)+(t>>7)&t>>12))|t>>(4-(1^7&(t>>19)))|t>>7;
      break;
    case 3:
      OCR2A = ((((t>>6|t<<1)+(t>>5|t<<3|t>>3)|t>>2|t<<1) & t>>12) ^ t>>16);
      break;
  }    
}


