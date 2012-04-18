//ARDCORE LFSR SEQUENCER....CV CONTROLLED ON POTS A2/A3 (jacks as well)
// Uses a Galois LFSR
// to provide NOISE and OR CV out
//  with controls all the way to the left, you get noise. 
//with controls turned to the right, plug into a VCO for patterns
//interesting patterns evolve when you plug in lfos to A2 and A3 and set them both to full depth

//have fun!!


//DAC OUTPUT IS MAIN OUT




//2012 dan


// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#include <stdint.h>
uint32_t lfsr = 1;
unsigned period = 0;

unsigned bit;
const int pinOffset = 5;       // the first DAC pin (from 5-12)
                                                                                                                                                                                                                                                                                                                                                                                                                                                              



void setup () {
   // set up the 8-bit DAC output pins
  for (int i=0; i<8; i++) {
    pinMode(pinOffset+i, OUTPUT);
    digitalWrite(pinOffset+i, LOW);
    
    
    
    
  
  }
  
  
 // set the ADC to a higher prescale factor
   sbi(ADCSRA,ADPS2);
  cbi(ADCSRA,ADPS1);
  cbi(ADCSRA,ADPS0);
   
  
}


void loop () {


  
do {

  /* taps: 32 31 29 1; characteristic polynomial: x^32 + x^31 + x^29 + x + 1 */
  lfsr = (lfsr >> 1) ^ (-(lfsr & 1u) & 0xD0000001u);
 delay(analogRead(2)); 
  ++period;
   delay(analogRead(3)); 
  dacOutput(lfsr);
   delay(analogRead(2)); 
} while(lfsr != 1u);
 delay(analogRead(3)); 

}
















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
