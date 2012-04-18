
#include "wiring_private.h"
#undef round
#undef abs
long myValue;
const int pinOffset = 5;       // the first DAC pin (from 5-12)

long t = 0;
int newValue;

long i = 0;

void setup()  { 
 
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




void loop() {
 
  newValue=analogRead(2);
  if (++i != 64) //return;
  i = 0;
  t++;
myValue =((~t>>2)*((127&t*(7&t>>10))<(245&t*(2+(5&t>>14)))));



//  return;
  
 // COMMENT THIS FOR TOTALLY DIFFERENT SOUNDS
 
 switch( t*(((t>>12)|(t>>newValue))&(63&(t>>4))))
 {
    case 0:
    myValue = (((1-(((t+10)>>((t>>9)&((t>>14))))&(t>>4&-2)))*2)*(((t>>10)^((t+((t>>6)&127))>>10))&1)*32+128);


      break;
    case 1:
    myValue =(((t>>5&t)-(t>>5)+(t>>5&t))+(t*((t>>14)&14)));
;
;
      break;
    case 2:
    myValue =(((t>>4)*(13&(0x8898a989>>(t>>11&30)))&255)+((((t>>9|(t>>2)|t>>newValue)*10+4*((t>>2)&t>>15|t>>newValue))&255)>>1));
      break;
    case 3:
    myValue = ((t*5&t>>7)|(t*3&t>>10));
;
      break;  
  }    
  
  dacOutput(myValue);
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

