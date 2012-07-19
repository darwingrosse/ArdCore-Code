// ARDCORE MODIFIED Auduino, the Lo-Fi granular synthesiser

// A0: Grain 1 pitch
// A1: Grain 2 decay
// A2: Grain 1 decay
// A3: Grain 2 pitch
//CLOCK IN: WORKING
//
// D0: PWM OUT
//DAC: Audio out OR USE AS A CV SOURCE






//
// by Peter Knight, Tinker.it http://tinker.it
//
// Help:      http://code.google.com/p/tinkerit/wiki/Auduino
// More help: http://groups.google.com/group/auduino
//

//
// Changelog:
// 19 Nov 2008: Added support for ATmega8 boards
// 21 Mar 2009: Added support for ATmega328 boards
// 7 Apr 2009: Fixed interrupt vector for ATmega328 boards
// 8 Apr 2009: Added support for ATmega1280 boards (Arduino Mega)

/*
10/07/10 Prodical contributed:
- additional mapping modes - diatonic major and minor and pentatonic major and minor
- switchable between modes by a single button which cycles through each and differentiates by an LED blinking as per the mode number
- a light dependent resistor (LDR) - calibrated in the first five seconds after switching on - replacing the main (grain repetition) frequency pot on a switch (using an external interrupt) with an LED indicator when it’s active but which also dims according on the LDR value
more details at http://blog.lewissykes.info
*/

/*
  Calibration
 
 Demonstrates one techinque for calibrating sensor input.  The
 sensor readings during the first five seconds of the sketch
 execution define the minimum and maximum of expected values
 attached to the sensor pin.
 
 The sensor minumum and maximum initial values may seem backwards.
 Initially, you set the minimum high and listen for anything 
 lower, saving it as the new minumum. Likewise, you set the
 maximum low and listen for anything higher as the new maximum.
 
 The circuit:
 * Analog sensor (potentiometer will do) attached to analog input 0
 * LED attached from digital pin 9 to ground
 
 created 29 Oct 2008
 By David A Mellis
 Modified 17 Jun 2009
 By Tom Igoe
 
 http://arduino.cc/en/Tutorial/Calibration
 
 This example code is in the public domain.
 
 */

// AUDUINO code STARTS
#include <avr/io.h>
#include <avr/interrupt.h>

uint16_t syncPhaseAcc;
uint16_t syncPhaseInc;
uint16_t grainPhaseAcc;
uint16_t grainPhaseInc;
uint16_t grainAmp;
uint8_t grainDecay;
uint16_t grain2PhaseAcc;
uint16_t grain2PhaseInc;
uint16_t grain2Amp;
uint8_t grain2Decay;

// Map Analogue channels
#define SYNC_CONTROL         (4)
#define GRAIN_FREQ_CONTROL   (3)
#define GRAIN_DECAY_CONTROL  (2)
#define GRAIN2_FREQ_CONTROL  (1)
#define GRAIN2_DECAY_CONTROL (0)

// Changing these will also requires rewriting audioOn()
#if defined(__AVR_ATmega8__)
//
// On old ATmega8 boards.
//    Output is on pin 11
//
#define LED_PIN       13
#define LED_PORT      PORTB
#define LED_BIT       5
#define PWM_PIN       11
#define PWM_VALUE     OCR2
#define PWM_INTERRUPT TIMER2_OVF_vect
#elif defined(__AVR_ATmega1280__)
//
// On the Arduino Mega
//    Output is on pin 3
//
#define LED_PIN       13
#define LED_PORT      PORTB
#define LED_BIT       7
#define PWM_PIN       3 //3
#define PWM_VALUE     OCR3C
#define PWM_INTERRUPT TIMER3_OVF_vect
#else
//
// For modern ATmega168 and ATmega328 boards
//    Output is on pin 3
//
#define PWM_PIN       3 //3
#define PWM_VALUE     OCR2B
#define LED_PIN       13
#define LED_PORT      PORTB
#define LED_BIT       5
#define PWM_INTERRUPT TIMER2_OVF_vect
#endif
// AUDUINO code ENDS


// BUTTON, SWITCH, LDR & LEDs - START
// Button
#define BUTTON_PIN (6)  // the number of the pushbutton pin 
int buttonValue;        // variable for reading the button status
int buttonState;        // variable to hold the button state
int mapMode = 0;        // What scale/mapping mode is in use?

// LDR
//#define LDRSWITCH (4)
// switch replaced by external interrupt
volatile int LDRswitchState = LOW;
#define LDR_PIN  (5)
#define LDRLED_PIN  (9)
//Callibration variables
int LDRValue = 0;   // the sensor value
int LDRMin = 1023;  // minimum sensor value
int LDRMax = 0;     // maximum sensor value

// PWM_VALUE LED
#define FRQLED_PIN (11) // not as consistent as pin 13      

// mapmode LED
#define mapModeLED_PIN (10)    // the number of the LED pin  
int mapModeLEDState = LOW;     // ledState used to set the LED
long previousMillis = 0;       // will store last time LED was updated
int BlinkRate = 5;            // no of blinks per second i.e. fps - empirically tested as just slow enough to count
int BlinkCount = 0;           // variable to store no of blinks
int BlinkLoopLength = 14;    // err... blink loop length

// BUTTON, SWITCH, LDR & LEDs - ENDS


// MAPPINGS - START
// Smooth logarithmic mapping
//
uint16_t antilogTable[] = {
  64830,64132,63441,62757,62081,61413,60751,60097,59449,58809,58176,57549,56929,56316,55709,55109,
  54515,53928,53347,52773,52204,51642,51085,50535,49991,49452,48920,48393,47871,47356,46846,46341,
  45842,45348,44859,44376,43898,43425,42958,42495,42037,41584,41136,40693,40255,39821,39392,38968,
  38548,38133,37722,37316,36914,36516,36123,35734,35349,34968,34591,34219,33850,33486,33125,32768
};
uint16_t mapPhaseInc(uint16_t input) {
  return (antilogTable[input & 0x3f]) >> (input >> 6);
}

// Stepped chromatic mapping
//
uint16_t midiTable[] = {
  0,17,18,19,20,22,23,24,26,27,29,31,32,34,36,38,41,43,46,48,51,54,58,61,65,69,73,
  77,82,86,92,97,103,109,115,122,129,137,145,154,163,173,183,194,206,218,231,
  244,259,274,291,308,326,346,366,388,411,435,461,489,518,549,581,616,652,691,
  732,776,822,871,923,978,1036,1097,1163,1232,1305,1383,1465,1552,1644,1742,
  1845,1955,2071,2195,2325,2463,2610,2765,2930,3104,3288,3484,3691,3910,4143,
  4389,4650,4927,5220,5530,5859,6207,6577,6968,7382,7821,8286,8779,9301,9854,
  10440,11060,11718,12415,13153,13935,14764,15642,16572,17557,18601,19708,20879,
  22121,23436,24830,26306,27871
};
uint16_t mapMidi(uint16_t input) {
  return (midiTable[(1023-input) >> 3]);
}

//// Stepped Pentatonic mapping
//
uint16_t pentatonicTable[54] = {
  0,19,22,26,29,32,38,43,51,58,65,77,86,103,115,129,154,173,206,231,259,308,346,
  411,461,518,616,691,822,923,1036,1232,1383,1644,1845,2071,2463,2765,3288,
  3691,4143,4927,5530,6577,7382,8286,9854,11060,13153,14764,16572,19708,22121,26306
};

uint16_t mapPentatonic(uint16_t input) {
  uint8_t value = (1023-input) / (1024/53);
  return (pentatonicTable[value]);
}

// Lewis added - I've got an Excel spreadsheet with these workings out on my blog...
// Stepped major Diatonic mapping
//
uint16_t majordiatonicTable[76] = {
  0,17,19,22,23,26,29,32,34,38,43,46,51,58,65,69,77,86,92,103,115,129,137,154,173,183,206,231,259,274,308,346,366,
  411,461,518,549,616,691,732,822,923,1036,1097,1232,1383,1465,1644,1845,2071,2195,2463,2765,2930,3288,
  3691,4143,4389,4927,5530,5859,6577,7382,8286,8779,9854,11060,11718,13153,14764,16572,17557,19708,22121,23436,26306
};

uint16_t mapmajorDiatonic(uint16_t input) {
  uint8_t value = (1023-input) / (1024/53);
  return (majordiatonicTable[value]);
}

// Stepped minor Diatonic mapping
//
uint16_t minordiatonicTable[76] = {
  0,17,19,20,23,26,27,31,34,38,41,46,51,54,61,69,77,82,92,103,109,122,137,154,163,183,206,218,244,274,308,326,366,
  411,435,489,549,616,652,732,822,871,978,1097,1232,1305,1465,1644,1742,1955,2195,2463,2610,2930,3288,
  3484,3910,4389,4927,5220,5859,6577,6968,7821,8779,9854,10440,11718,13153,13935,15642,17557,19708,20879,23436,26306
};

uint16_t mapminorDiatonic(uint16_t input) {
  uint8_t value = (1023-input) / (1024/53);
  return (minordiatonicTable[value]);
}

// Stepped major Pentatonic mapping
//
uint16_t majorpentatonicTable[55] = {
  0,17,19,22,26,29,34,38,43,51,58,69,77,86,103,115,137,154,173,206,231,274,308,346,
  411,461,549,616,691,822,923,1097,1232,1383,1644,1845,2195,2463,2765,3288,
  3691,4389,4927,5530,6577,7382,8779,9854,11060,13153,14764,17557,19708,22121,26306
};

uint16_t mapmajorPentatonic(uint16_t input) {
  uint8_t value = (1023-input) / (1024/53);
  return (majorpentatonicTable[value]);
}

// Stepped minor Pentatonic mapping
//
uint16_t minorpentatonicTable[55] = {
  0,17,20,23,26,31,34,41,46,51,61,69,82,92,103,122,137,163,183,206,244,274,326,366,
  411,489,549,652,732,822,978,1097,1305,1465,1644,1955,2195,2610,2930,3288,
  3910,4389,5220,5859,6577,7821,8779,10440,11718,13153,15642,17557,20879,23436,26306
};

uint16_t mapminorPentatonic(uint16_t input) {
  uint8_t value = (1023-input) / (1024/53);
  return (pentatonicTable[value]);
}
// MAPPINGS - END


void audioOn() {
#if defined(__AVR_ATmega8__)
  // ATmega8 has different registers
  TCCR2 = _BV(WGM20) | _BV(COM21) | _BV(CS20);
  TIMSK = _BV(TOIE2);
#elif defined(__AVR_ATmega1280__)
  TCCR3A = _BV(COM3C1) | _BV(WGM30);
  TCCR3B = _BV(CS30);
  TIMSK3 = _BV(TOIE3);
#else
  // Set up PWM to 31.25kHz, phase accurate
  TCCR2A = _BV(COM2B1) | _BV(WGM20);
  TCCR2B = _BV(CS20);
  TIMSK2 = _BV(TOIE2);
#endif
}


void setup() {

  pinMode(PWM_PIN,OUTPUT);
  audioOn();
  pinMode(LDRLED_PIN,OUTPUT);
  pinMode(FRQLED_PIN,OUTPUT);   
  pinMode(mapModeLED_PIN,OUTPUT);
  pinMode(BUTTON_PIN,INPUT);
  attachInterrupt(0, LDRswitched, CHANGE);

  // Calibration
  //Serial.begin(9600);
  // turn on LED to signal the start of the calibration period:
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);
  // calibrate during the first five seconds 
  while (millis() < 5000) {
    LDRValue = analogRead(LDR_PIN);
    // record the maximum sensor value
    if (LDRValue > LDRMax) {
      LDRMax = LDRValue;
    }
    // record the minimum sensor value
    if (LDRValue < LDRMin) {
      LDRMin = LDRValue;
    }
  }
  // signal the end of the calibration period
  digitalWrite(9, LOW);
  // END calibration code

}


void LDRswitched()
{
  LDRswitchState = !LDRswitchState;
}


void loop() {
  // The loop is pretty simple - it just updates the parameters for the oscillators.
  //
  // Avoid using any functions that make extensive use of interrupts, or turn interrupts off.
  // They will cause clicks and poops in the audio.

  // Smooth frequency mapping
  //syncPhaseInc = mapPhaseInc(analogRead(SYNC_CONTROL)) / 4;

  // Stepped mapping to MIDI notes: C, Db, D, Eb, E, F...
  //syncPhaseInc = mapMidi(analogRead(SYNC_CONTROL));

  //  // Stepped pentatonic mapping: D, E, G, A, B
  //  syncPhaseInc = mapPentatonic(analogRead(SYNC_CONTROL));

  // could add switch here to choose between midiIn() and the twisty-pot pitch control

  //BEGIN Calibration
  // read the sensor:
  LDRValue = analogRead(LDR_PIN);
  // apply the calibration to the sensor reading
  LDRValue = map(LDRValue, LDRMin, LDRMax, 0, 1023);
  // in case the sensor value is outside the range seen during calibration
  LDRValue = constrain(LDRValue, 0, 1023);
  //    // fade the LED using the calibrated value: (this moved below)
  //    analogWrite(LDRLED_PIN, LDRValue);
  //    Serial.println(LDRValue);
  //END Calibration

  // button presses cycle through scales/mapping modes
  buttonValue = digitalRead(BUTTON_PIN);      // read input value and store it in val
  if (buttonValue != buttonState) {         // the button state has changed!
    if (buttonValue == 0) {                // check if the button is pressed
      if (mapMode == 0) {          // if set to smooth logarithmic mapping
        mapMode = 1;               // switch to stepped chromatic mapping
      } 
      else {
        if (mapMode == 1) {        // if stepped chromatic mapping
          mapMode = 2;             // switch to stepped major Diatonic mapping
        } 
        else {
          if (mapMode == 2) {      // if stepped major Diatonic mapping
            mapMode = 3;           // switch to stepped minor Diatonic mapping
          }
          else {
            if (mapMode == 3) {      // if stepped minor Diatonic mapping
              mapMode = 4;           // switch to stepped major Pentatonic mapping 
            }
            else {
              if (mapMode == 4) {      // if stepped major Pentatonic mapping 
                mapMode = 5;           // switch to stepped minor Pentatonic mapping 
              }
              else {
                if (mapMode == 5) {      // if stepped major Pentatonic mapping 
                  mapMode = 0;           // switch back to smooth logarithmic mapping
                }
              }
            }
          }
        }
      }
    }
    buttonState = buttonValue;                 // save the new state in our variable     
  }

  // mapMode LED indicator - blinks number of scale/mapping mode on a fixed 'frame' cycle
  if (millis() - previousMillis > 1000/BlinkRate)
  {
    BlinkCount =  BlinkCount + 1;
    // save the last time you blinked the LED 
    previousMillis = millis();
    digitalWrite(mapModeLED_PIN, LOW);
    if (BlinkCount <= (mapMode+1)*2) 
    {
      //if the LED is off turn it on and vice-versa:
      if (mapModeLEDState == LOW) 
      {
        mapModeLEDState = HIGH;
      }
      else{
        mapModeLEDState = LOW;
      }
      // set the LED with the ledState of the variable:
      digitalWrite(mapModeLED_PIN, mapModeLEDState);
    }
    // resets Blink loop after 14 blinks
    else if (BlinkCount >= BlinkLoopLength) 
    {
      BlinkCount = 0;   
    }
    //debug
    //Serial.println(BlinkCount);
  }

  //Map modes to select with button.  LED blinks as per mode currently in use
  //selects which array to pull data from to get SyncPhaseInc
  //1. Smooth logarithmic mapping
  if (mapMode == 0) {
    if (LDRswitchState == HIGH) {
      syncPhaseInc = mapPhaseInc(1023-LDRValue) / 4;
      analogWrite(LDRLED_PIN, LDRValue);
    }
    else
    {
      syncPhaseInc = mapPhaseInc(analogRead(SYNC_CONTROL)) / 4;
      digitalWrite(LDRLED_PIN, LOW);
    }
  }
  //2. Stepped chromatic mapping to MIDI notes: C,C#,D,Eb,F,F#,G,Ab,A,Bb,B
  if (mapMode == 1) {
    if (LDRswitchState == HIGH) {
      syncPhaseInc = mapMidi(1023-LDRValue);
      analogWrite(LDRLED_PIN, LDRValue);
    }
    else
    {
      syncPhaseInc = mapMidi(analogRead(SYNC_CONTROL));
      digitalWrite(LDRLED_PIN, LOW);
    }
  }
  //3. Stepped major Diatonic mapping: C,D,E,F,G,A,B
  if (mapMode == 2) {
    if (LDRswitchState == HIGH) {
      syncPhaseInc = mapmajorDiatonic(1023-LDRValue);
      analogWrite(LDRLED_PIN, LDRValue);
    }
    else
    {
      syncPhaseInc = mapmajorDiatonic(analogRead(SYNC_CONTROL));
      digitalWrite(LDRLED_PIN, LOW);
    }
  }
  //4. Stepped minor Diatonic mapping: C,D,Eb,F,G,Ab,Bb
  if (mapMode == 3) {
    if (LDRswitchState == HIGH) {
      syncPhaseInc = mapminorDiatonic(1023-LDRValue);
      analogWrite(LDRLED_PIN, LDRValue);
    }
    else
    {
      syncPhaseInc = mapminorDiatonic(analogRead(SYNC_CONTROL));
      digitalWrite(LDRLED_PIN, LOW);
    }
  }
  //5. Stepped major Pentatonic mapping
  if (mapMode == 4) {
    if (LDRswitchState == HIGH) {
      syncPhaseInc = mapmajorPentatonic(1023-LDRValue);
      analogWrite(LDRLED_PIN, LDRValue);
    }
    else
    {
      syncPhaseInc = mapmajorPentatonic(analogRead(SYNC_CONTROL));
      digitalWrite(LDRLED_PIN, LOW);
    }
  }
  //6. Stepped major Diatonic mapping
  if (mapMode == 5) {
    if (LDRswitchState == HIGH) {
      syncPhaseInc = mapminorPentatonic(1023-LDRValue);
      analogWrite(LDRLED_PIN, LDRValue);
    }
    else
    {
      syncPhaseInc = mapminorPentatonic(analogRead(SYNC_CONTROL));
      digitalWrite(LDRLED_PIN, LOW);
    }
  }

  //input from pots
  grainPhaseInc  = mapPhaseInc(analogRead(GRAIN_FREQ_CONTROL)) / 2;
  grainDecay     = analogRead(GRAIN_DECAY_CONTROL) / 8;
  grain2PhaseInc = mapPhaseInc(analogRead(GRAIN2_FREQ_CONTROL)) / 2;
  grain2Decay    = analogRead(GRAIN2_DECAY_CONTROL) / 4;

  digitalWrite(FRQLED_PIN, PWM_VALUE);

}


SIGNAL(PWM_INTERRUPT)
{
  uint8_t value;
  uint16_t output;

  syncPhaseAcc += syncPhaseInc;
  if (syncPhaseAcc < syncPhaseInc) {
    // Time to start the next grain
    grainPhaseAcc = 0;
    grainAmp = 0x7fff;
    grain2PhaseAcc = 0;
    grain2Amp = 0x7fff;
    LED_PORT ^= 1 << LED_BIT; // Faster than using digitalWrite
  }

  // Increment the phase of the grain oscillators
  grainPhaseAcc += grainPhaseInc;
  grain2PhaseAcc += grain2PhaseInc;

  // Convert phase into a triangle wave
  value = (grainPhaseAcc >> 7) & 0xff;
  if (grainPhaseAcc & 0x8000) value = ~value;
  // Multiply by current grain amplitude to get sample
  output = value * (grainAmp >> 8);

  // Repeat for second grain
  value = (grain2PhaseAcc >> 7) & 0xff;
  if (grain2PhaseAcc & 0x8000) value = ~value;
  output += value * (grain2Amp >> 8);

  // Make the grain amplitudes decay by a factor every sample (exponential decay)
  grainAmp -= (grainAmp >> 8) * grainDecay;
  grain2Amp -= (grain2Amp >> 8) * grain2Decay;

  // Scale output to the available range, clipping if necessary
  output >>= 9;
  if (output > 255) output = 255;

  // Output to PWM (this is faster than using analogWrite)  
  PWM_VALUE = output;
}


