//THIS IS THE TEMPLATE FOR ALL CV PATTERN EXPERIMENTS
//can be used as audio or CV source


#include "wiring_private.h"

long myValue;
const int pinOffset = 5;       // the first DAC pin (from 5-12)
const int digPin[2] = {3, 4};  // the digital output pins

int newValue;

long i = 0;
int digState[2] = {LOW, LOW};  // start with both set low
void setup()  { 
 
  
   // set up the digital outputs
  for (int i=0; i<2; i++) {
    pinMode(digPin[i], OUTPUT);
    digitalWrite(digPin[i], digState[i]);
  }
    
    
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
    delayMicroseconds(1+analogRead(2));

 myval=(t*5&t>>7|t*38&t>>8);
 delayMicroseconds(1+analogRead(2));
 long myval2=myval/4;
 long myval3=myval*2;
 delayMicroseconds(1+analogRead(2));
 long myval4=myval/8;
 dacOutput(myval3-myval);
 
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

