
//  ============================================================
//
//  Program: Experimental Noise/CV amusement park
//
//  Description: This sketch throws a few things together for massive confusion
//
//  I/O Usage:
//    Knob 1: Frequency of sine modulator(may need to be jiggled)
//    Knob 2: offset
//    Knob 3/CV IN A2-CV OR AUDIO IN 1/   
//    Knob 4/CV in A3: CV or audio in 2
//    Digital Out 1: 
//    Digital Out 2: 
//    Clock In: 
//    Analog Out: Crazy CV or audio out...I like it as a CV source for a vco
//
// 
//  Created:   Feb 2012////Dan
// 
//
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






// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif



#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI (3.141592654)
#endif
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)
//volatile int clkState = LOW;
int value[512];
int looplen = 512;

double w,y;
int digState[2] = {HIGH, LOW};  // start with both set low


double z1,z2;
double a, d, g;

//int inDac = analogRead(2);


void setup () {
  
 
 // Serial.begin(9600);
  
   // set up the digital (clock) input
 // pinMode(clkIn, INPUT);
  
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
  
  
  
  
}


void loop () {
  int i;
   byte value;
  
  
   
    
    
     if (millis()>1) {
    
int w=(analogRead(2)>>2);
 int y=(analogRead(3)>>1);   
   a=a*analogRead(3);
   d=d*analogRead(1);
z2 = z1;
z1 = w;
w = w + d*(1.0 + a)*z1 - a*z2;
y = w*a - d*(1.0 + a)*z1 + z2;


int radio= (0.5*(y + w + g*(w - y))); 

radio=(radio+w+y);


int i,nsamps;
	double samp;
	double twopi = 2.0 * M_PI;
	double angleincr;
	/* set number of points to create */
int potValue=map(analogRead(0),0, 1023, 31111,14);
	nsamps =potValue ;
	/* make one complete cycle */
    angleincr = twopi / nsamps;
	for(i=0;i < nsamps; i++){
		samp = sin(angleincr *i);
samp=( (unsigned char)((samp*127.5)+128));
dacOutput(radio*(samp/3));

}




}
}



//  =================== convenience routines ===================

//  isr() - quickly handle interrupts from the clock input
//  ------------------------------------------------------
//void isr()
//{
  //clkState = HIGH;
//}

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
