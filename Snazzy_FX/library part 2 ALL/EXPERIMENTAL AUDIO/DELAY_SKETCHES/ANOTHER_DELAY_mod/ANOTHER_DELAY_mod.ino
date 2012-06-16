/* Delay Test MOD

Delay Input= jack A2...KNOB A2=INPUT LEVEL...turn this down quite a bit
Delay time=knob/jack A3
knob A0=mod1
knob a1=FEEDBACK!


*/




// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif
short input  ;
short output;
const short bufferlength = 800; 
short buffer[bufferlength];
short i = 0; 
short j;
short amplitude = 5;



//  constants related to the Arduino Nano pin use
const int clkIn = 2;           // the digital (clock) input
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)
const int trigTime = 25;       // the default trigger time of 25 ms

//  variables for interrupt handling of the clock input
volatile int clkState = LOW;

short sin_table[8] = {0,707,1000,707,0,-707,-1000,-707}; //sine values


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
  
  
void loop() {
  
  
 float delayLength = ( ( (float)bufferlength-1 ) * ((float)analogRead(0)/1024.0)) + 1;
		float feedback = (float)analogRead(1) / 1600.0; //limit feedback
  {
  input = analogRead(2)>>1; 
output=input + 0.1*amplitude*buffer[i];
j=i%analogRead(3);
output=output+buffer[j];
buffer[i] = input + (buffer[i] * feedback);
dacOutput(output);
buffer[i] = input; 
i++; 
if (i >= bufferlength) i = 0;
}
if (j >= bufferlength-1) j = 0;
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

