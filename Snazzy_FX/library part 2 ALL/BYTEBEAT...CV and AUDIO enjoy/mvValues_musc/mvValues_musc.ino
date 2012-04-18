
#include "wiring_private.h"

long myValue;
const int pinOffset = 5;       // the first DAC pin (from 5-12)


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
   long t;
   long myval;
  for(t=0;;t++){

 myval=(t*(((t>>12)|(t/(analogRead(2)/12)))&((analogRead(3)/12)&(t>>4))));
 long myval2=myval/4;
 long myval3=myval*2;
 long myval4=myval/8;
 dacOutput(myval4);
 
  }

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

