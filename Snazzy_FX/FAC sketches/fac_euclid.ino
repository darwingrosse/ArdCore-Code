
//  ============================================================
//
//  Program: ArdCore Euclidean Pattern Generator
//
//  Description:
//
//    
//    Clock divisors and multipliers range from 1 to 16.
//
//    Outputs are organized in divider/multiplier pairs
//    First pair is D0 and D1, then OX outputs 00 to 07.
//
//  I/O Usage:
//    Knob A0: Pattern length (1 to 16)
//    Knob A1: Density - proportion of triggers (0 to 1)
//    Analog In A2: Shift - reset to step (0 to length-1)
//    Analog In A3: Reset trigger
//    Digital Out D0: Pattern output
//    Digital Out D1: Inverted output or Gate output (can be user-selected)
//    Clock In: Clock input
//    Analog Out: Extra CV out (e.g., for pitch, cutoff, etc)
//
//    This sketch was programmed by Alfonso Alba (fac)
//    E-mail: shadowfac@hotmail.com
//
//  Created:  Feb 2013
//  Modified: Feb 2013
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


// User definable options

#define VERBOSE        1    // Set to nonzero to print parameters in the console window
#define MAXLENGTH      16   // Set maximum pattern length (16 is recommended, but can also be higher)
#define TRIG_TIME      50   // Trigger length in milliseconds (default is 50, decrease for faster clocks)
#define CV_QUANTIZE    1    // Set to nonzero to quantize the CV OUT voltage in 1/12 volt increments
#define CV_PER_CLOCK   0    // 0 = change CV OUT at each clock, 1 = change CV OUT at each trigger in D0
#define D1_IS_GATE     0    // 0 = D1 is an inverted version of D0, 1 = D1 is a gated version of D0



//  constants related to the Arduino Nano pin use
#define clkIn      2    // the digital (clock) input
#define digPin0    3    // the digital output pin D0
#define digPin1    4    // the digital output pin D1
#define pinOffset  5    // the first DAC pin (from 5-12)

//  variables for interrupt handling of the clock input
volatile char clkState = LOW;
char resetState = LOW;


unsigned char pattern[MAXLENGTH];
unsigned char cv[MAXLENGTH];
unsigned char pos;
unsigned char length;
unsigned char density;
unsigned char reset_position;
unsigned char reset_state;

unsigned long oldMillis;
unsigned char trig_on;

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
  
  update();
  reset_state = reset_position;
  oldMillis = millis();
  trig_on = 0;
}



//  This is the main loop

void update() {
  int l, d, s;
  l = ((analogRead(0) * MAXLENGTH) >> 10) + 1;
  d = ((analogRead(1) * (l + 1)) >> 10);
  reset_position = ((analogRead(2) * l) >> 10);
  
  if ((l != length) || (d != density)) {
    length = l;
    density = d;
    euclid(pattern, cv, length, density);
    
    if (VERBOSE) {
      Serial.print("Length = ");
      Serial.print(length);
      Serial.print("  Density = ");
      Serial.print(density);
      Serial.print("  Shift = ");
      Serial.println(reset_position);
    }
  }
}


void loop() {
  unsigned long curMillis;
  
  // Turn
  if (trig_on && ((millis() - curMillis) > TRIG_TIME)) {
    trig_on = 0;
    digitalWrite(digPin0, 0);
    if (!D1_IS_GATE) digitalWrite(digPin1, 0);
  }
  
  if (clkState == HIGH) {
    clkState = LOW;
    
    // Process reset
    if (analogRead(3) > 512) {
      if (resetState == LOW) {
        resetState = HIGH;
        pos = reset_position;
      }
    }
    else resetState = LOW;
    
    digitalWrite(digPin0, pattern[pos]);
    digitalWrite(digPin1, D1_IS_GATE ? pattern[pos] : (1 - pattern[pos]));
    if (CV_PER_CLOCK || pattern[pos]) {
      dacOutputFaster(CV_QUANTIZE ? cv[pos] : quantize(cv[pos]));
    }
    
    trig_on = 1;

    pos++;
    if (pos >= length) pos = 0;
    
    update();
  }
      
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




// euclidean pattern generator

void add(unsigned char *v, int n, unsigned char delta) {
  while (n--) *v++ += delta;
}

void euclid(unsigned char *p, unsigned char *v, int length, int pulses) {
  char lp[MAXLENGTH];
  char sp[MAXLENGTH];
  unsigned char vlp[MAXLENGTH];
  unsigned char vsp[MAXLENGTH];
  
  int p_len, lp_len, sp_len, q, r, i, n, k;

  if (pulses == 0) {
    memset(p, 0, length);
    return;
  }

  lp[0] = 1; lp_len = 1; vlp[0] = (length << 4) | pulses;
  sp[0] = 0; sp_len = 1; vsp[1] = (pulses << 4) | length;
  n = pulses;
  k = length - pulses;

  while (k > 1) {
    if (k >= n) {
      q = k / n;
      r = k % n;
      for (i = 0; i < q; i++) { 
        memcpy(lp+lp_len, sp, sp_len);
        memcpy(vlp+lp_len, vsp, sp_len);
        add(vsp, sp_len, (q << 4) | r);
        lp_len += sp_len;
      }
      k = r;
    }
    else {
      memcpy(p, lp, lp_len);
      memcpy(lp + lp_len, sp, sp_len);
      memcpy(sp, p, lp_len);

      memcpy(v, vlp, lp_len);
      memcpy(vlp + lp_len, vsp, sp_len);
      memcpy(vsp, v, lp_len);

      i = lp_len;
      lp_len += sp_len;
      sp_len = i;
      
      r = n - k;
      n = k;
      k = r;
      
      add(vsp, sp_len, (r << 4));
    }
  }

  p_len = 0;
  for (i = 0; i < n; i++) { 
    memcpy(p+p_len, lp, lp_len); 
    memcpy(v+p_len, vlp, lp_len); 
    add(vlp, lp_len, (pulses << 4) | length);
    p_len += lp_len; 
  }
  for (i = 0; i < k; i++) { 
    memcpy(p+p_len, sp, sp_len); 
    memcpy(v+p_len, vsp, sp_len);
    add(vsp, sp_len, (length << 4) | pulses);
    p_len += sp_len; 
  }
}


// quantizer

unsigned char quantize(unsigned char in) {
  // Output goes from 0 to 5 volts, which covers a 5 octave range; that is, 60 semitones.
  // Since the output values go from 0 to 255, then each semitone should increases the output in 4.25
  // Therefore, we divide the output by 4.25, round the result, and multiply by 4.25 to obtain the
  // quantized value.
  const float mult = 242.3 / 60.0;
  return (unsigned char)(mult * (float)((int)((float)in / mult + 0.5)));
}



// faster dac output

void dacOutputFaster(byte v) {
  PORTB = (PORTB & B11100000) | (v >> 3);
  PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);
}
