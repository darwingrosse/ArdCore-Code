//Plug an AUDIO input into A2.....plug a clock into clock input (fast VCO)
//...the clock must be fast 
//plug in your AUDIO TO BE CRUSHED
//ideas...make the vco which is providing a clock, voltage controlled by an lfo
//go ahead and get your DIRTY CRUSHED AUDIO out of DAC out



//ARDCORE CRUSHER...thrown together from bits and pieces by Dan
/*
A2=audio input
Clock Input=FAST CLOCK input
DAC OUT=crushed audio out
*/


// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


//  constants related to the Arduino Nano pin use
const int clkIn = 2;           // the digital (clock) input
const int digPin[2] = {3, 4};  // the digital output pins



//  variables for interrupt handling of the clock input
volatile int clkState = LOW;



byte array[1500]; // create an array
const int pinOffset = 5;       // the first DAC pin (from 5-12)







void setup() {
   
  
 // Serial.begin (9600);
  // set up the digital (clock) input
  pinMode(clkIn, INPUT);
  
   analogReference(INTERNAL); // change the analog input reference voltage level
  
  // Note: Interrupt 0 is for pin 2 (clkIn)
  attachInterrupt(0, isr,RISING); 
   
  
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
   int i=0;
  // deal with possible interrupts
  if (clkState == HIGH & i < 1500) {
    clkState = LOW;
    i ++;
    dacOutput(array[i]); // play back sample  
  array[i] = analogRead(2) >> 1; // record sample

   
  }
   
  
  
  
 
   
 

 

 
 
  

}




void isr()
{
  clkState = HIGH;
  
  
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

//  ===================== end of pro


