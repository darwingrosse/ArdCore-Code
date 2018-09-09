//ARDCORE:NSCALE CV GEN
//based on Nscale.C in audio programming book, with revisions, modficiations, etc by Dan Snazelle.
//sketch created march 2012
//Knob A0=transpose
//knob A1=time between notes
// knob/jack A2-number of notes in scale
//knob/jack A3-root note for scale

//thanks to Douglas Ferguson for showing me how to convert number values into CV easily


     
     
     
 
//  ============================================================
//
//  License:
//
//  This software is licensed under the Creative Commons
//  "Attribution-NonCommercial license. This license allows you
//  to tweak and build upon the code for non-commercial purposes,
//  without the requirement to license derivative works on the
//  same terms. If you wish to use this (or derived) work for
//  commercial work, please contact 20 Objects LLC at our website
//  (www.20objects.com).
//
//  For more information on the Creative Commons CC BY-NC license,
//  visit http://creativecommons.org/licenses/
//
//  ================= start of global section ==================


// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

     
     #include <math.h>


//  constants related to the Arduino Nano pin use
const int clkIn = 2;           // the digital (clock) input
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)



//  variables used to control the current DIO output states
int digState[2] = {LOW, LOW};  // start with both set low
long digMilli[2] = {0, 0};      // used for trigger timing



//  ==================== start of setup() ======================
//  The stock setup() routine - see ArdCore_Template for info.





void setup () {
  
  analogReference(INTERNAL);

  //speed up http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1235060559
  TCCR0B = TCCR0B & 0b11111000 | 0x01;
  
  // set up the digital (clock) input
  pinMode(clkIn, INPUT);
  
  // set up the digital outputs
  for (int i=0; i<2; i++) {
    pinMode(digPin[i], OUTPUT);
    digitalWrite(digPin[i], LOW);
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

  
     

     
  void loop () {
  
 int newVal;
    int i;
    int notes=analogRead(2)/43; //0-23 values
      int octave= analogRead(0) / 86;  // gets us the range 0-11.
    int   midinote=analogRead(3)/8; //127 values
    int outputNumber=notes*5;   //number of notes in scale X5)

  
  
    int  time=analogRead(1);
       
       double frequency, ratio;
       double c0,c5;
       double intervals[24  ];
       
 





       
       ratio=pow(2.0,1.0/12.0);
       c5=220.0 * pow(ratio,3);
       c0=c5 * pow(0.5,5);
       frequency= c0 * pow(ratio, midinote);
       
       ratio= pow(2.0, 1.0/notes);
       for (i=0;i<notes; i++){
         intervals[i]=frequency;
         frequency *=ratio;
          delay(time*4);
       }
       
       for (i=0; i<notes; i++)
       {
         delay(time*14);
       
    newVal = keepInX(intervals[i],outputNumber);

//dacOutput(newVal+octave);

       dacOutput(newVal);
       
       
       }
       
       
  //hold the note 
 
  
     }

   
  
  






int keepInX(int v,int x)
{
 int ov = v;

 if (ov > x)
   ov = x - (ov - x);
 if (ov < 0)
   ov = 0 - ov;   

 return ov;
}





     
     
//  dacOutput(long) - deal with the DAC output
void dacOutput(long v)
{
	/* Most significant bits */
	PORTB ^= ~(~(PORTB & B00011111) ^ ((v & B11111000) >> 3));
	/* Least significant bits */
	PORTD ^= ~(~(PORTD & B11100000) ^ ((v & B00000111) << 5));
};

