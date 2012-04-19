//  ============================================================
//
//  Program: ArdCore VoltageRecorder
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
//    Knob 1: Playback sample hop size
//    Knob 2: Loop Length
//    Analog In 1: Control voltages to be recorded
//    Analog In 2: HIGH to record the buffer with voltages
//    Digital Out 1: HIGH when recording
//    Digital Out 2: Trigger when recording stops
//    Clock In: Playback trigger
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
//            17 Apr 2012  ddg Updated for Arduino 1.0
//						18 Apr 2012	 ddg Changed dacOutput routine to Alba version
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

//  variables for interrupt handling of the clock input
volatile int clkState = LOW;

//  variables used to control the current DIO output states
int digState[2] = {LOW, LOW};  // start with both set low
unsigned long digTimer[2] = {0, 0};

//  variables for system recording
int isRecording = 0;
int everRec = 0;

int value[512];
int looplen = 512;

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
  for (int i=0; i<512; i++) {
    value[i] = 0;
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
  int temploop = (deJitter(analogRead(1) >> 1 + 1, looplen));

  /*
  // uncomment this section if you want the loop to
  // clear and restart when made longer
  if (temploop > looplen) {
    for (int i=looplen; i<temploop; i++) {
      value[i] = 0;
    }
    recpos = 0;
    playpos = 0;
  }
  */
  
  looplen = temploop;

  if (doClock) {
    // first deal with playing
    dacOutput(value[playpos] >> 2);
    
    if (everRec) {
      playpos += (analogRead(0) >> 7) + 1;
      playpos = playpos % looplen;
    }
    
    // next, deal with recording
    if (isRecording) {
      // if (recpos < looplen) {  // use this if you want to limit 
                                  // loop length recording to the
                                  // loop size defined by analog(1)
      if (recpos < 512) {         // otherwise, use this...
        value[recpos] = analogRead(2);
        everRec = 1;
        recpos++;
      } else {
        isRecording = 0;
        recpos = 0;
        
        digState[0] = LOW;
        digitalWrite(digPin[0], LOW);
        
        digState[1] = HIGH;
        digitalWrite(digPin[1], HIGH);
        digTimer[1] = millis();
      }
    }
  }

  // do we need to start recording?
  if ((!isRecording) && (analogRead(3) > 256)) {
    isRecording = 1;
    recpos = 0;
    digState[0] = HIGH;
    digitalWrite(digPin[0], HIGH);
  }

  // do we need to turn off the rec off trigger?  
 if ((digState[1]) && (millis() - digTimer[1] > trigTime)) {
   digitalWrite(digPin[1], LOW);
   digState[1] = LOW;
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

//  dacOutput(byte) - deal with the DAC output
//  -----------------------------------------
void dacOutput(byte v)
{
  PORTB = (PORTB & B11100000) | (v >> 3);
	PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);
}

//  ===================== end of program =======================
