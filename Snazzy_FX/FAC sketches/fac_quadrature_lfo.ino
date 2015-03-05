//  ============================================================
//
//  Program: ArdCore Quadrature VC-LFO
//
//  Description:
//
//    This LFO sketch outputs two copies of the same LFO, 
//    but one of them is displaced by 90 degrees. 

//    The "original" LFO comes from the analog output of the
//    Ardcore. The second LFO comes from the D0 output as a 
//    PWM wave, so it needs to be integrated to become an actual 
//    LFO, for example, by running it through a slew limiter.
//    The original LFO at the analog output can be quite steppy,
//    so it may be necessary to also run it through a slew to
//    smooth it out.
//
//    Frequency is controlled by the A0 knob and A2 input
//    and the range is between 0.01 Hz and 200 Hz.
//
//    The LFO shape can be varied from a downwards ramp to
//    a triangle to a upwards ramp by knob A1 and input A3.
//    Additionally, a pulse LFO is also output at D1;
//    the pulsewidth is also controlled by A1 and A3.
//
//    The phase of the LFO can also be reset to zero with
//    a pulse on the clock input.
//
//    The combination of clock reset and external modulations
//    of frequency and shape may lead to some interesting patterns
//    and very different signals from D0 and the analog output.
//
//  I/O Usage:
//    Knob A0: LFO Frequency
//    Knob A1: LFO Shape (from ramp-down to triangle to ramp-up)
//    Analog In A2: LFO Frequency modulation
//    Analog In A3: LFO Shape modulation
//    Digital Out D0: Quadrature output (as a PWM wave)
//    Digital Out D1: Pulse output (same phase as Analog Out)
//    Clock In: LFO Reset (sets Analog Out phase to zero)
//    Analog Out: Audio out
//
//    This sketch was programmed by Alfonso Alba (fac)
//    E-mail: shadowfac@hotmail.com
//
//  Created:  25 May 2011
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
#define clkIn      2    // the digital (clock) input
#define digPin0    3    // the digital output pin D0
#define digPin1    4    // the digital output pin D1
#define pinOffset  5    // the first DAC pin (from 5-12)


//  Some global variables
short curVal0 = 0;      // stores the value of analog input 0
short curVal1 = 0;      // stores the value of analog input 1
short curVal2 = 0;      // stores the value of analog input 2
short curVal3 = 0;      // stores the value of analog input 3

unsigned long oldMicros = 0;
float phase = 0;

//  variables for interrupt handling of the clock input
volatile char clkState = LOW;



//  ==================== start of setup() ======================

//  This setup routine should be used in any ArdCore sketch that
//  you choose to write; it sets up the pin usage, and sets up
//  initial state. Failure to properly set up the pin usage may
//  lead to damaging the Arduino hardware, or at the very least
//  cause your program to be unstable.

void setup() 
{
  // if you need to send data back to your computer, you need
  // to open the serial device. Otherwise, comment this line out.
  Serial.begin(9600);
  
  // set up the digital (clock) input
  pinMode(clkIn, INPUT);
  
  // set up the digital outputs
  pinMode(digPin0, OUTPUT);
  digitalWrite(digPin0, LOW);
  pinMode(digPin1, OUTPUT);
  digitalWrite(digPin1, LOW);
  
  // set up the 8-bit DAC output pins
  for (int i=0; i<8; i++) {
    pinMode(pinOffset+i, OUTPUT);
    digitalWrite(pinOffset+i, LOW);
  }
  
  // set up an interrupt handler for the clock in. If you
  // aren't going to use clock input, you should probably
  // comment out this call.
  // Note: Interrupt 0 is for pin 2 (clkIn)
  attachInterrupt(0, isr, RISING);
  
  oldMicros = micros();
}


//  This is the main loop

void loop() 
{
  static unsigned long curMicros, deltaMicros;
  float frequency, shape, phase2, p, y;
  
  // check to see if the clock as been set
  if (clkState == HIGH) {
    clkState = LOW;      // clock pulse acknowledged
    phase = 0;
  }
  
  curVal0 = deJitter(analogRead(0), curVal0);
  curVal1 = deJitter(analogRead(1), curVal1);
  curVal2 = deJitter(analogRead(2), curVal2);
  curVal3 = deJitter(analogRead(3), curVal3);

  frequency = (float)(curVal0 + curVal2) * 0.1;
  if (frequency < 0.01) frequency = 0.01;
  
  shape = (float)(curVal1 + curVal3) / 1024.0f;
  if (shape < 0.01) shape = 0.01;
  if (shape > 0.99) shape = 0.99;
  
  curMicros = micros();
  deltaMicros = curMicros - oldMicros;
  oldMicros = curMicros;
  phase += (float)deltaMicros * frequency * 0.000001;
  phase2 = phase - 0.25; // 90 degrees
  
  p = phase - floor(phase);
  y = (p <= shape) ? (p / shape) : ((1 - p) / (1 - shape));
  dacOutput((long)(y * 255.0f));
  digitalWrite(digPin1, (p <= shape) ? HIGH : LOW);

  p = phase2 - floor(phase2);
  y = (p <= shape) ? (p / shape) : ((1 - p) / (1 - shape));
  analogWrite(digPin0, (long)(y * 255.0f)); 

  // Debug section
  //Serial.print("Frequency = ");
  //Serial.print(frequency);
  //Serial.println("Hz\n");
  //Serial.print("Shape = ");
  //Serial.println(shape);
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
void dacOutput(long v)
{
  // feed this routine a value between 0 and 255 and teh DAC
  // output will send it out.
  int tmpVal = v;
  for (int i=0; i<8; i++) {
    digitalWrite(pinOffset + i, tmpVal & 1);
    tmpVal = tmpVal >> 1;
  }
}


// This routine was taken from the Ardcore Oscillator example
// and it's supposed to be faster than dacOutput
void dacOutputFast(long v)
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
