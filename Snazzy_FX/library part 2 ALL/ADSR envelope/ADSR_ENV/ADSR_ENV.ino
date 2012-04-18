//  Program: ArdCore EXPERIMENTAL ADSR ENVELOPE
//CURRENTLY SUSTAIN  VALUE IS FIXED IN PROGRAM WHILE Attack, Decay, and Release are controllable


//

//
//  I/O Usage:
//    Knob 1: Attack
//    Knob 2: Decay
//    Knob 3/Jack A2: GATE INPUT
//  Knob 4/Jack A3: RELEASE KNOB AND/OR RELEASE CV
//    Digital Out 1:
//    Digital Out 2: 
//    Clock In:
//    Analog Out:ADSR OUT

//  Input Expander: unused
//  Output Expander:unused
//
//  Created:  FEB 2012 by Dan Snazelle
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
//
//  constants related to the Arduino Nano pin use
const int clkIn = 2;           // the digital (clock) input
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)
const int trigTime = 25;       // ms for a trigger output

volatile int clkState = LOW;

int digState[2] = {LOW, LOW};
unsigned long digMilli[2] = {0, 0};




int  envstart;
int attackpeak;




int envval=340;
float attackValue = 0.0;
float decayValue = 0.0;
float sustainValue= 0.0;
float  releaseValue=0.0;
float currValue = 0.0;

//  ==================== start of setup() ======================
void setup() {
  
//  Serial.begin(9600);
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
  
 
}

//  ==================== start of loop() =======================

void loop()
{
  int gateRead=analogRead(2);
float decay=255.0/(analogRead(1)+5);
float release=255.0/(analogRead(3)+5);
float sustain=255.0/(50+5);//***************CHANGE THIS VALUE TO SET UP YOUR SUSTAIN LEVEL!!!!!***************
int off;
float attack=255.0/(analogRead(0)+5);
 
  
   if((analogRead(2))>120)
   { 
       if (attackpeak < 1) //envstart is set to 1 by the gate
        {envval = (envval + attack); //step up to full level


        if (envval >= 1023) 
         {attackpeak = 1;envval = 1023;}}       //switch to decay
             if (attackpeak > 0)
               {
                if (envval > sustain) //stop at sustain level
                  { envval = (envval - decay);}
                    if (envval <= 1)
                       {attackpeak = 0;envstart = 0;envval = 0;}
                                           }        
                                     }
      

                
                
                  if  ((analogRead(2))<100){
                              if (envval > 1)
                            {envval = envval - release; //gate is done, do the release
                                    if (envval < 0) {envval = 0;}        
                                                            }
                                                else {envval = 0;}
                                                          }
      
                         dacOutput((envval>>2));
                         //Serial.print("Gate Value=");
                         //Serial.println(gateRead);
}
  
  
  
  
  
  
  



//  =================== convenience routines ===================

//  isr() - quickly handle interrupts from the clock input
//  ------------------------------------------------------


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
