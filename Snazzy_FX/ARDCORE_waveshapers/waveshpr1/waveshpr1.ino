
//QUICK N DIRTY WAVESHAPER
//A0=crush
//A1=wave offset
//A2=Wave shape depth/Cv
//A3=input gain/Audio In..this effects sound quite a bit.
//DAC OUT=audio out
//
//
//




// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

 	

unsigned long SHmil = 0;
int    S1out;
int  CENTERPOS;

const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)
//volatile int clkState = LOW;

int digState[2] = {HIGH, LOW};  // start with both set low

void setup () {
  
 
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
  sbi(ADCSRA,ADPS0);
  }
  
  
  
  
}

void loop () {
   CENTERPOS=analogRead(2)>>2;
  S1out=analogRead(3)>>2;
  
  distortion();

   dacOutput(S1out>>2);
    delayMicroseconds(map(analogRead(0), 0, 1023, 22000, 1));
}





// Swap high and low nibbles
void distortion(void) {
	unsigned int temp1, temp2,temp3;                
 	if (( S1out > 10) || ( S1out < -10)) {
		temp1 = (((int) S1out+CENTERPOS) >> 4) & 0xf;	// Original high
		temp2 = (((int) S1out+CENTERPOS) & 0xF) << 4; // Original low
                temp3=(analogRead(1));
		 S1out = ((((temp1 + temp2) & 0x7F&temp3) - CENTERPOS)) >> 5;  
	S1out=((S1out + CENTERPOS) & 0b111111)  -( CENTERPOS);
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
