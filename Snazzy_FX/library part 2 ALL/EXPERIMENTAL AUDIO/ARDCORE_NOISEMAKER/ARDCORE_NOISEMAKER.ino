

/*ARDCORE NOISEMAKER


AS SIMPLE AS IT GETS


JACKS SEND A VALUE TO THE dacOutput FUNCTION

A0 X A2= CV or AUDIO

TRY PUTTING LFO INTO JACK A2 and turn knobs AO and A2 for AUDIO OUT

or use as CV

Dan Snazelle

*/


const int pinOffset = 5;       // the first DAC pin (from 5-12)


void setup() {
   
  

  
   
   
  
  // set up the 8-bit DAC output pins
  for (int i=0; i<8; i++) {
   pinMode(pinOffset+i, OUTPUT);
    digitalWrite(pinOffset+i, LOW);
}
}

void loop() {
 dacOutput(analogRead(2)*analogRead(0));
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

