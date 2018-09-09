//ARDCORE:NSCALE CV GEN
//based on Nscale.C in audio programming book, with revisions, modficiations, etc by Dan Snazelle.
//sketch created march 2012
//Knob A0=transpose
//knob A1=time between notes
// knob/jack A2-number of notes in scale
//knob/jack A3-root note for scale

//thanks to Douglas Ferguson for showing me how to convert number values into CV easily




// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)
//volatile int clkState = LOW;


void setup () {
  
  analogReference(INTERNAL);
  
  //speed up http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1235060559
  TCCR0B = TCCR0B & 0b11111000 | 0x01;
  
 
 
  
  
 // Serial.begin(9600);
  

  
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

  
     
     
     #include <math.h>
     
     void loop () {
       int notes, i, midinote;
        notes=map(analogRead(2),0,1023,2,24);
        int octave=map(analogRead(0),0,1023,1,notes); // 12 values see chords array above
    int  transpose=map(analogRead(1),0,1023,1,300);
       
       double frequency, ratio;
       double c0,c5;
       double intervals[24];
       
      
       midinote=map(analogRead(3),0,1023,1,127);
       
       ratio=pow(2.0,1.0/12.0);
       c5=220.0 * pow(ratio,3);
       c0=c5 * pow(0.5,5);
       frequency= c0 * pow(ratio, midinote);
       
       ratio= pow(2.0, 1.0/notes);
       for (i=0;i<notes; i++){
         intervals[i]=frequency;
         frequency *=ratio;
          delay(transpose*4);
       }
       
       for (i=0; i<notes; i++)
       {
         delay(transpose*14);
       
        dacOutput(intervals[i]+(notes*octave*4));
       
       
       }
       
       
  //hold the note 
 
  
     }
     
     
     
//  dacOutput(long) - deal with the DAC output
void dacOutput(long v)
{
	/* Most significant bits */
	PORTB ^= ~(~(PORTB & B00011111) ^ ((v & B11111000) >> 3));
	/* Least significant bits */
	PORTD ^= ~(~(PORTD & B11100000) ^ ((v & B00000111) << 5));
};

