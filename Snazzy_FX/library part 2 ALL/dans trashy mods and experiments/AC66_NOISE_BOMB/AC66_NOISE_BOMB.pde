//  ============================================================
//
//  Program: ArdCore NoiseBomb
//
//  Description: When the second analog input goes high, the
//               system begins recording input until its
//               memory buffer is full. It will then repeat
//               this at the output every time it receives
//               a trigger in the clock input.
//
//  NOTE: Prescaler setup routines used from the Arduino Forum's
//        use "jmknapp", available at this link:
//        http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1208715493/11
//
//  I/O Usage:
//    Knob 1: Noise control 1
//    Knob 2: Loop Length/Noise control 2
//    Analog In 1: 
//    Analog In 2: 
//    Digital Out 1: 
//    Digital Out 2: 
//    Clock In: Frequency of waveform
//    Analog Out: 8-bit output
//
//  Input Expander: unused
//  Output Expander: 8 bits of output exposed
//
//  Created:  13 Feb 2011
//  Modified: 13 Mar 2011  ddg - Completely redone record engine
//                             - use of highspeed write option for
//                               DAC output
//            20 Mar 2011  ddg - Use prescaler setup to speed up
//                               analogRead calls.
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
//  ================= start of global section ==================

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
const int pinOffset = 5;       // the first DAC pin (from 5-12)
const int trigTime = 25;       // the default trigger time of 25 ms

int ctr;

//  variables for interrupt handling of the clock input
volatile int clkState = LOW;

//  variables used to control the current DIO output states
int digState[2] = {LOW, LOW};  // start with both set low
unsigned long digTimer[2] = {0, 0};

//  variables for system recording
int isRecording = 0;
int everRec = 0;

int value[256]={ 
  127,130,133,136,139,143,146,149,152,155,158,161,164,167,170,173,
  176,179,182,184,187,190,193,195,198,200,203,205,208,210,213,215,
  217,219,221,224,226,228,229,231,233,235,236,238,239,241,242,244,
  245,246,247,248,249,250,251,251,252,253,253,254,254,254,254,254,
  255,254,254,254,254,254,253,253,252,251,251,250,249,248,247,246,
  245,244,242,241,239,238,236,235,233,231,229,228,226,224,221,219,
  217,215,213,210,208,205,203,200,198,195,193,190,187,184,182,179,
  176,173,170,167,164,161,158,155,152,149,146,143,139,136,133,130,
  127,124,121,118,115,111,108,105,102,99,96,93,90,87,84,81,
  78,75,72,70,67,64,61,59,56,54,51,49,46,44,41,39,
  37,35,33,30,28,26,25,23,21,19,18,16,15,13,12,10,
  9,8,7,6,5,4,3,3,2,1,1,0,0,0,0,0,
  0,0,0,0,0,0,1,1,2,3,3,4,5,6,7,8,
  9,10,12,13,15,16,18,19,21,23,25,26,28,30,33,35,
  37,39,41,44,46,49,51,54,56,59,61,64,67,70,72,75,
  78,81,84,87,90,93,96,99,102,105,108,111,115,118,121,124,

};  //SINE SHAPE TABLE
int looplen = 256;
int blue [256];  //random storage

int recpos = 0;
int playpos = 0;

//  ==================== start of setup() ======================

void setup() {
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
  
  // Note: Interrupt 0 is for pin 2 (clkIn)
  attachInterrupt(0, isr, RISING);
  
  // clean up value store
 // for (int i=0; i<512; i++) {
   // value[i] = 0;
  //}
  
  
  for (ctr=0; ctr<256; ctr++) {
    blue[ctr]=(random(10,255)%99)+1;
  }
}

//  ==================== start of loop() =======================
void loop()
{
int doClock = 0;

 // deal with possible interrupts
 if (clkState == HIGH) {
   clkState = LOW;
   doClock = 1;
 }

 // get the new loop length
 int looplen = (deJitter(analogRead(1) >> 1 + 1, blue[playpos]));

   if (doClock) {
     // first deal with playing
     dacOutput(value[playpos] >> 2);
     playpos += (analogRead(0) % blue[playpos]) + 1;
     playpos = playpos % looplen;
  }

}
//  =================== convenience routines ===================

//  isr() - quickly handle interrupts from the clock input
//  ------------------------------------------------------
void isr()
{
  clkState = HIGH;
}

//  deJitter(int, int) - smooth jitter input
//  ----------------------------------------
int deJitter(int v, int test)
{
  if (abs(v - test) > 8) {
    return v;
  }
  return test;
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

//  ===================== end of program =======================
