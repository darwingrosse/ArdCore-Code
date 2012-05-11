//  ============================================================
//
//  Program: ArdCore Glissando
//
//  Description: A stepped (1/4 step increments) gliss generator
//               that produces a trigger when the destination is 
//               reached.
//
//  I/O Usage:
//    Knob 1: Base gliss rate
//    Knob 2: unused
//    Analog In 1: Destination voltage
//    Analog In 2: unused
//    Digital Out 1: Trigger when destination is reached
//    Digital Out 2: unused
//    Clock In: unused
//    Analog Out: Gliss output
//
//  Input Expander: unused
//  Output Expander: 8 bits of output exposed
//
//  Created:  29 Jan 2011
//  Modified: 13 Mar 2011  ddg - Code cleanup
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

//  constants related to the Arduino Nano pin use
const int digPin = 3;          // the digital output pin
const int pinOffset = 5;       // the first DAC pin (from 5-12)
const int trigTime = 25;       // default to 25 ms trigger time

long glissMillis = 0;          // the number of milliseconds for the gliss
long glissSegment = 0;         // the millis for each step
unsigned long lastMilli = 0;   // the last millisecond value measured

int digState = LOW;            // the state of the digital output (trigger)
unsigned long trigMilli = 0;   // the last millisecond value from a trigger

int currentValue = -1;         // the currently held value
int destinationValue = -1;     // the value we are heading for

//  ==================== start of setup() ======================

void setup() {
  Serial.begin(9600);
  
  // clear out the right output.
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  
  // set up the digital outputs
  pinMode(digPin, OUTPUT);
  digitalWrite(digPin, LOW);
  
  // set up the 8-bit DAC output pins
  for (int i=0; i<8; i++) {
    pinMode(pinOffset+i, OUTPUT);
    digitalWrite(pinOffset+i, LOW);
  }
}

//  ==================== start of loop() =======================

void loop()
{
  int tempInput;
  
  // deal with first call
  if (currentValue == -1) {
    currentValue = destinationValue = analogRead(2);
  }

  // get the current timing
  glissMillis = (analogRead(0) >> 2) * 10;
  
  // do a step move if necessary
  if ((destinationValue >= 0) && 
      (currentValue != destinationValue) && 
      (millis()-lastMilli > glissSegment)) {

    /*        
    Serial.print(currentValue);
    Serial.print("-");
    Serial.print(destinationValue);
    Serial.print("-");
    Serial.print(lastMilli);
    Serial.print("-");
    Serial.print(millis());
    Serial.print("-");
    Serial.println(glissSegment);
    */
    
    if ((glissSegment < 1) || (glissSegment > 1500)) {
      currentValue = destinationValue;
    } else {   
      // update the current value
      if (currentValue > destinationValue) {
        currentValue--;
      } else {
        currentValue++;
      }
    }
    
    // send the current value
    dacOutput(currentValue);
    
    // if destination reached, hit the gate
    if (currentValue == destinationValue) {
      digState = HIGH;
      digitalWrite(digPin, HIGH);
      trigMilli = millis();
    }
    
    lastMilli = millis();
  }
    
  // check for a trigger shutdown
  if ((digState == HIGH) && (millis() - trigMilli > trigTime)) {
    digState = LOW;
    digitalWrite(digPin, LOW);
  }
  
  // check for a new input value
  tempInput = deJitter((analogRead(2) >> 2), destinationValue);
  if (tempInput != destinationValue) {
    destinationValue = tempInput;
    glissSegment = glissMillis / abs(destinationValue - currentValue);
    /*
    Serial.print(tempInput);
    Serial.print('\t');
    Serial.println(glissSegment);
    */
  }
}

//  =================== convenience routines ===================

//  dacOutput(byte) - deal with the DAC output
//  -----------------------------------------
void dacOutput(byte v)
{
  PORTB = (PORTB & B11100000) | (v >> 3);
	PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);
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

//  ===================== end of program =======================
