/*

Useful time based modulator for audio

A0= screwy
A1=noisy
A2 time
A3 INPUT...keep this pretty low!


DAC OUT=AUDIO OUT

*/




// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif
unsigned long SHmil = 0;
int    S1out;
float SHrv = 0;
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)
//volatile int clkState = LOW;

int digState[2] = {HIGH, LOW};  // start with both set low

void setup () {
  
  
 // Serial.begin(9600);
  
   // set up the digital (clock) input
 // pinMode(clkIn, INPUT);
 
 
    // set up the digital outputs
  for (int i=0; i<2; i++) {
    pinMode(digPin[i], OUTPUT);
    digitalWrite(digPin[i], digState[i]);
  }
  
    // set up the 8-bit DAC output pins
  for (int i=0; i<8; i++) {
    pinMode(pinOffset+i, OUTPUT);
    digitalWrite(pinOffset+i, LOW);
    
    
      // set the ADC to a higher prescale factor
  sbi(ADCSRA,ADPS2);
  cbi(ADCSRA,ADPS1);
  cbi(ADCSRA,ADPS0);
  }
  
  
  
  
}

void loop () {
  S1out=analogRead(3)>>2;
 SHgate();
  
   dacOutput(S1out);
    delayMicroseconds(map(analogRead(0), 0, 1023, 2000, 1));
}



void SHgate(){
  int anaVs[2]={analogRead(2)};

  int l = map(anaVs[2], 0, 1023, 700, 0);
    
   if(millis() - SHmil > l){
     SHmil = millis();
     SHrv = random(1,100)*0.00999;
   }
   
     {
     S1out -= 131;
     S1out *= SHrv;
     S1out += 131;
     delayMicroseconds(map(analogRead(1), 0, 1023, 2000, 1));
     S1out = constrain(S1out, 90, 165);
   }
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
