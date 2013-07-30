/*  ============================================================================

Simplified ArdCore Communication Sketch (SACS)

This sketch is provided to provide a simplified serial communication protocol
between the ArdCore 001 micro-based synthesizer module and a serial-connected
computer. This is provided as an alternative to the Firmata software, and
offers the following improvements:

- No hardware setup is required by your software, everything is preset
  to work the way that the ArdCore works.
- The clock input is interrupt driven, so it works as quickly as possible.
- The analog inputs have de-jittering applied, and only report values when 
  the values change.
- Latency is limited on input by round-robining the analog input reads
  (which tend to be time-consuming).
  
================================================================================
  
The messages that are used to set the pin outpus are as follows:

Digital Pin 0 (Arduino pin 3):
  0xF0 1/0 (on/off)
  
Digital Pin 1 (Arduino pin 4):
  0xF1 1/0 (on/off)
  
Analog Output Pins 0-7 (Arduino pins 5-12, individually):
  0xF2 <pin number> 1/0 (on/off)
  
Analog Output (Arduino pins 5-12):
  0xF3 <low nibble> <high nibble>
  
The messages received from the device are as follows:

Clock Input Received:
  0xFF
  
Analog Input Data Received:
  0xF0+pin <low 7 bits> <high 7 bits>

System messages:

Resend all of the stored values:
  0xFF
  
================================================================================

Created:  22 Feb 2013
Modified: 
  
================================================================================

License:

  This software is licensed under the Creative Commons
  "Attribution-NonCommercial license. This license allows you
  to tweak and build upon the code for non-commercial purposes,
  without the requirement to license derivative works on the
  same terms. If you wish to use this (or derived) work for
  commercial work, please contact 20 Objects LLC at our website
  (www.20objects.com).

  For more information on the Creative Commons CC BY-NC license,
  visit http://creativecommons.org/licenses/

============================================================================= */

// constants related to the Arduino Nano pin use
// ---------------------------------------------
const int clkIn = 2;           // the digital (clock) input
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)

// input queueing variables
// ------------------------
const int OPT_UNDEFINED = 0;
const int OPT_DIG_0 = 1;
const int OPT_DIG_1 = 2;
const int OPT_ANA_INDIV = 3;
const int OPT_ANA_GROUP = 4;
const int OPT_RESEND = 99;

byte inoption = -1;                 // defined above
byte inqueue[5] = {0, 0, 0, 0, 0};  // the incoming byte queue.
int inrecv = 0;                     // the number of bytes received.

// output queueing variables
// -------------------------
const int MAX_INPIN = 4;      // read up to 4 pins (add more if you have an input expander)

volatile int clkState = LOW;  // clock input notifier

int aValue = 0;               // the most recently read value
int aPin = -1;                // the most recently read analog pin
int aQueue[8] = {0, 0, 0, 0, 0, 0, 0, 0};

//  ==================== start of setup() ======================
void setup() 
{
  Serial.begin(57600);      // 57600 baud rate for computer comm
  
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
  
  // Note: Interrupt 0 is for pin 2 (clkIn)
  attachInterrupt(0, isr, RISING);
}

//  ==================== start of loop() =======================
void loop()
{
  int doClock = LOW;
  byte inByte = 0;
  
  if (clkState) {
    clkState = LOW;
    doClock = HIGH;
  }
  
  if (doClock) {
    // if a clock was received, it gets the highest priority!
    Serial.write(0xFF);
  } else if (Serial.available() > 0) {
    // the next priority is if there is serial data available.
    inByte = Serial.read();

    if (inByte >= 0) {
      // if this is a notification byte - start the process
      // Serial.println(inByte);

      if (inByte >= 0xF0) {
        inrecv = 1;
        inqueue[0] = inByte;
        switch (inByte) {
          case 0xF0:  // digital pin 0
            inoption = OPT_DIG_0;
            break;
          case 0xF1:  // digital pin 1
            inoption = OPT_DIG_1;
            break;
          case 0xF2:  // analog pin individual
            inoption = OPT_ANA_INDIV;
            break;
          case 0xF3:  // analog pin group
            inoption = OPT_ANA_GROUP;
            break;
          case 0xFF:  // resend all - and do it right now!
            sendAllAnalog();
            inoption = OPT_UNDEFINED;
            break;
        }
      } else {
        inrecv++;
        switch (inoption) {
          case OPT_UNDEFINED:  // throw it away
            break;
          case OPT_DIG_0:
            digitalWrite(digPin[0], inByte > 0);
            inoption = OPT_UNDEFINED;
            break;
          case OPT_DIG_1:
            digitalWrite(digPin[1], inByte > 0);
            inoption = OPT_UNDEFINED;
            break;
          case OPT_ANA_INDIV:
            inqueue[inrecv-1] = inByte;
            if (inrecv == 3) {
              if (inqueue[1] < 8) {
                digitalWrite(pinOffset + inqueue[1], inqueue[2] > 0);
              }
              inoption = OPT_UNDEFINED;
            }
            break;
          case OPT_ANA_GROUP:
            inqueue[inrecv-1] = inByte;
            if (inrecv == 3) {
              byte tmp = (inqueue[1] & B00001111) + ((inqueue[2] & B00001111) << 4);
              dacOutput(tmp);
              inoption = OPT_UNDEFINED;
            }
            break;
          default:
            inoption = OPT_UNDEFINED;
            break;
        }
      }
    }
  } else {
    // lowest priority, if no incoming data, do an analog read and send
    aPin++;
    if (aPin >= MAX_INPIN) {
      aPin = 0;
    }
    
    aValue = analogRead(aPin);
    if (aValue != aQueue[aPin]) {
      aQueue[aPin] = aValue;
      sendAnalog(aPin);
    }
  }
}

void sendAllAnalog()
{
  for (int i=0; i<MAX_INPIN; i++) {
    sendAnalog(i);
  }
}

void sendAnalog(int pin)
{
  int tmpval = aQueue[pin];
  
  Serial.write(byte(0xF0 + pin));
  Serial.write(byte(tmpval & 127));
  Serial.write(byte(tmpval >> 7));
}

//  =================== convenience routines ===================

//  These routines are some things you will need to use for
//  various functions of the hardware. Explanations are provided
//  to help you know when to use them.

//  isr() - quickly handle interrupts from the clock input
//  ------------------------------------------------------
void isr()
{
  // Note: you don't want to spend a lot of time here, because
  // it interrupts the activity of the rest of your program.
  // In most cases, you just want to set a variable and get
  // out.
  clkState = HIGH;
}

//  dacOutput(long) - deal with the DAC output
//  ------------------------------------------
void dacOutput(byte v)
{
  // replacement routine as suggested by Alphonso Alba
  // this code accomplishes the same thing as the original
  // code but is approx 4x faster
  // PORTB = (PORTB & B11100000) | (v >> 3);
  // PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);
  
  int tmp = 1;
  for (int i=0; i<8; i++) {
    tmp = 1 << i;
    digitalWrite(i+pinOffset, (v & tmp) > 0);
  }
}

//  deJitter(int, int) - smooth jitter input
//  ----------------------------------------
int deJitter(int v, int test)
{
  // this routine just make sure we have a significant value
  // change before we bother implementing it. This is useful
  // for cleaning up jittery analog inputs.
  if (abs(v - test) > 8) {
    return v;
  }
  return test;
}

//  ===================== end of program =======================
