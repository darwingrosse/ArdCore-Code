//  ============================================================
//
//  Program: ArdCore BouncingBall
//
//  Description: A sketch that produces an attack and decay
//               envelope upon the receipt of a trigger at the
//               clock input.
//
//  I/O Usage:
//    Knob 1: First bounce time (0 - 2000 ms)
//    Knob 2: Bounce limit (0-100%)
//    Analog In 1: Decay factor (0-100%)
//    Analog In 2: Friction adder (0-50 ms)
//    Digital Out 1: Trigger out on bounce
//    Digital Out 2: HIGH at the end of the bounce
//    Clock In: Trigger in starts the bounce
//    Analog Out: 8-bit output of current location
//
//  Input Expander: unused
//  Output Expander: 8 bits of output exposed
//
//  Created:  26 Jul 2013  ddg
//  Modified: 
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
const int clkIn = 2;           // the digital (clock) input
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)
const int trigTime = 10;       // ms for a trigger output

volatile int clkState = LOW;

int digState = LOW;
unsigned long digMilli = 0;

// accumulator stuff
float accStart = 500.0;
float accValue = 0.0;
float accFact = 0.5;
float accFric = 0.0;
float accLimit = 0.1;

// timer stuff
unsigned long currTime = 0;
unsigned long nextTime = 0;
long isRunning = 0;

void setup() {
  // Serial.begin(9600);
  
  // set up the digital (clock) input
  pinMode(clkIn, INPUT);
  
  // set up the 8-bit DAC output pins
  for (int i=0; i<8; i++) {
    pinMode(pinOffset+i, OUTPUT);
    digitalWrite(pinOffset+i, LOW);
  }  

  // set up the digital outputs
  for (int i=0; i<2; i++) {
    pinMode(digPin[i], OUTPUT);
    digitalWrite(digPin[i], LOW);
  }
  
  // set up the interrupt
  attachInterrupt(0, isr, RISING);

  // now flip the D1 pin
  digitalWrite(digPin[1], HIGH);
}

void loop() {
  int doOut = 0;
  currTime = millis();
  
  // deal with new clock start
  if (clkState) {
    clkState = 0;
    accValue = accStart;
    isRunning = 1;
    doOut = 1;
        
    // for debugging
    /*
    Serial.print(accValue);
    Serial.print("  ");
    Serial.print(accFact);
    Serial.print("  ");
    Serial.print(accFric);
    Serial.print("  ");
    Serial.println(accLimit);
    */
  }
  
  // deal with timing overage
  if ((!doOut) && (isRunning) && (currTime > nextTime)) {
    doOut = 1;
  }

  // deal with output
  if (doOut) {
    if (digState == HIGH) {
      digitalWrite(digPin[0], LOW);
      delay(50);
    }
    
    // output the bounce
    digitalWrite(digPin[0], HIGH);
    digState = HIGH;
    digMilli = currTime;
    
    // output the running value
    float tmp = (accValue / accStart) * 255.0;
    if (tmp > 255.0) tmp = 255.0;
    if (tmp < 0.0) tmp = 0.0;
    byte outval = floor(tmp);
    dacOutput(outval);
    
    // for debugging...
    // Serial.println(outval);
    
    // decrement the accumulator
    accValue *= accFact;
    accValue -= accFric;
    
    // set up the timer
    nextTime = currTime + floor(accValue);
    
    // check for underage
    if (accValue < accLimit) {
      isRunning = 0;
    }
    
    // if off, report it
    if (!isRunning) {
      digitalWrite(digPin[1], HIGH);
    } else {
      digitalWrite(digPin[1], LOW);
    }
  }
  
  // check for trigger shutdown
  if ((digState == HIGH) && (currTime - digMilli > trigTime)) {
    digitalWrite(digPin[0], LOW);
    digState = LOW;
  }

  // read the encoders
  accStart = (analogRead(0) / 1023.0) * 2000.0;         // 2 seconds max
  accLimit = (analogRead(1) / 2050.0) * accStart + 5.0; // around 50% of full
  accFact = analogRead(2) / 2050.0 + 0.495;             // 49-99%
  accFric = (analogRead(3) / 1023.0) * 100.0;           // 0-.1 second friction
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
  } else {
    return test;
  }
}

//  dacOutput(byte) - deal with the DAC output
//  -----------------------------------------
void dacOutput(byte v)
{
  PORTB = (PORTB & B11100000) | (v >> 3);
  PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);
}

//  ===================== end of program =======================
