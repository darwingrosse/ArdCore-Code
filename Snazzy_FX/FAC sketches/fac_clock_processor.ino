//  ============================================================
//
//  Program: ArdCore Clock Processor
//
//  Description:
//
//    This sketch requires the Ardcore OX expansion.
//
//    It provides five clock dividers, and five multipliers,
//    all based on the same clock input.
//    
//    Clock divisors and multipliers range from 1 to 16.
//
//    Outputs are organized in divider/multiplier pairs
//    First pair is D0 and D1, then OX outputs 00 to 07.
//
//  I/O Usage:
//    Knob A0: Output selection (0 to 9)
//    Knob A1: Divisor/Multiplier value (from 1 to 16)
//    Analog In A2: Trigger to Reset
//    Analog In A3: Trigger to Rotate
//    Digital Out D0: Divider 1 output
//    Digital Out D1: Multiplier 1 output
//    Clock In: Clock input
//    Analog Out: Extra modulator
//
//    This sketch was programmed by Alfonso Alba (fac)
//    E-mail: shadowfac@hotmail.com
//
//  Created:  Dec 2012
//  Modified: Dec 2012
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

#define numBeats   2    // number of beats for clock period average


//  Some global variables
unsigned int curVal0 = 0;      // stores the value of analog input 0
unsigned int curVal1 = 0;      // stores the value of analog input 1
unsigned int curVal2 = 0;      // stores the value of analog input 2
unsigned int curVal3 = 0;      // stores the value of analog input 3

unsigned long oldMicros = 0;

//  variables for interrupt handling of the clock input
volatile char clkState = LOW;
char resetState = LOW;
char rotateState = LOW;

unsigned long value[10];    // Divisor / Multiplier values
unsigned long counter[10];  // Counters
unsigned long interval[10]; // Time interval (period) for each output
unsigned char output[10];   // Outputs

unsigned long period[numBeats];  // Period between the last numBeats beats
unsigned long avgPeriod;
int periodIdx = 0;
int averageReady = 0;

int editing = 0;
unsigned long editing_time = 0;


int reset = 0;
int rotate = 0;
int oldVal = 1;
int oldIdx = 0;



//  ==================== start of setup() ======================

//  This setup routine should be used in any ArdCore sketch that
//  you choose to write; it sets up the pin usage, and sets up
//  initial state. Failure to properly set up the pin usage may
//  lead to damaging the Arduino hardware, or at the very least
//  cause your program to be unstable.

void setup() 
{
  int i;
  char label[10][3] = { "D0", "D1", "00", "01", "02", "03", "04", "05", "06", "07" };
  
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
  
  // Initialize values according to initial inputs
  randomSeed(analogRead(0)+analogRead(1));
  
  switch ((analogRead(0) > 512) + 2 * (analogRead(1) > 512)) {
    case 0: for (i = 0; i < 10; i++) value[i] = 1; break;
    case 1: for (i = 0; i < 10; i++) value[i] = (i >> 1) + 2; break;
    case 2: for (i = 0; i < 10; i++) value[i] = 1 << (i >> 1); break;
    case 3: for (i = 0; i < 10; i++) value[i] = random(16) + 1; break;
  }
  
  for (i = 0; i < 10; i++) counter[i] = 0;
  
  Serial.println("Output frequencies");
  for (i = 0; i < 10; i++) {
    Serial.print(label[i]);
    Serial.print(": Clock ");
    Serial.print((i & 1) ? "* " : "/ ");
    Serial.println(value[i]);
  }
  
  oldIdx = (analogRead(0) * 10) >> 10;
  oldVal = (analogRead(1) >> 6) + 1;
  oldMicros = micros();
}


//  This is the main loop

void loop() 
{
  static unsigned long curMicros, curPeriod, elapsed, temp;
  int idx, val, i, k;
  
  curMicros = micros();

  // Process inputs
  idx = (analogRead(0) * 10) >> 10;
  val = (analogRead(1) >> 6) + 1;
  
  if ((idx != oldIdx) || (val != oldVal)) {
    oldIdx = idx;
    oldVal = val;
    value[idx] = val;
    editing = 1;
    editing_time = curMicros;
    show_output(idx);
  }

  if (editing) {
    elapsed = curMicros - editing_time;
    if (elapsed > 0x7FFFFFFFUL) elapsed += 0xFFFFFFFFUL;      
    if (elapsed > 1000000) {
      editing = 0;
    }
  }
  
  // Process reset
  if (analogRead(2) > 512) {
    if (resetState == LOW) {
      resetState = HIGH;
      reset = 1;
    }
  }
  else resetState = LOW;
  
  // Process rotate
  if (analogRead(3) > 512) {
    if (rotateState == LOW) {
      rotateState = HIGH;
      rotate = 1;
    }
  }
  else rotateState = LOW;
  
  // check to see if the clock as been set
  if (clkState == HIGH) {
    clkState = LOW;      // clock pulse acknowledged
    
    // Process reset
    if (reset) {
      memset(counter, 0, 10 * sizeof(long));
      reset = 0;
    }
  
    // Process rotate
    if (rotate) {
      temp = value[9];
      for (i = 8; i >= 0; i--) {
        value[i+1] = value[i];
      }
      value[0] = temp;
      rotate = 0;
    }


    // Update period of each output (in order to follow clock)
    if (averageReady) {
      for (i = 0; i < 10; ) {
        interval[i] = avgPeriod * value[i];
        i++;
       
        interval[i] = avgPeriod / value[i];
        i++;
      }
    }
    

    // Update average period
    curPeriod = curMicros - oldMicros;
    if (curPeriod > 0x7FFFFFFFUL) curPeriod += 0xFFFFFFFFUL;
    period[periodIdx++] = curPeriod;
    if (periodIdx >= numBeats) { periodIdx = 0; averageReady = 1; }
    
    //Serial.println(curPeriod);
    
    if (averageReady) {
      avgPeriod = 0;
      for (i = 0; i < numBeats; i++) avgPeriod += period[i];
      avgPeriod /= numBeats;    
    }
    
    // Process clock dividers
    for (i = 0; i < 5; i++) {
      k = i << 1;
      if (++counter[k] >= value[k]) {
        output[k] = 1;
        counter[k] = 0;
      }
      else output[k] = 0;
    }
    
    // Reset clock multipliers
    for (i = 0; i < 5; i++) {
      k = (i << 1) + 1;
      counter[k] = 0;
    }
    
    // Update old time
    oldMicros = curMicros;
  }
  
  // Process clock multipliers
  if (averageReady) {
    for (i = 0; i < 5; i++) {
      k = (i << 1) + 1;
      elapsed = curMicros - counter[k];
      if (elapsed > 0x7FFFFFFFUL) elapsed += 0xFFFFFFFFUL;
      if (elapsed > interval[k]) {
        output[k] = 1;
        counter[k] = curMicros;
      }
      else if (elapsed > 20000) output[k] = 0;
    }
  }
  
  // Turn clock dividers off
  elapsed = curMicros - oldMicros;
  if (elapsed > 0x7FFFFFFFUL) elapsed += 0xFFFFFFFFUL;
  if (elapsed > 20000) {
    for (i = 0; i < 5; i++) output[i << 1] = 0;
  }
    
  // Output values (unless in editing mode)
  if (!editing) {
    digitalWrite(digPin0, output[0]);
    digitalWrite(digPin1, output[1]);
    bitWrite(PORTB, 4, output[9]);
    bitWrite(PORTB, 3, output[8]);
    bitWrite(PORTB, 2, output[7]);
    bitWrite(PORTB, 1, output[6]);
    bitWrite(PORTB, 0, output[5]);
    bitWrite(PORTD, 7, output[4]);
    bitWrite(PORTD, 6, output[3]);
    bitWrite(PORTD, 5, output[2]);
  } 
}


void show_output(int o) {
  digitalWrite(digPin0, o == 0);
  digitalWrite(digPin1, o == 1);
  bitWrite(PORTB, 4, o == 9);
  bitWrite(PORTB, 3, o == 8);
  bitWrite(PORTB, 2, o == 7);
  bitWrite(PORTB, 1, o == 6);
  bitWrite(PORTB, 0, o == 5);
  bitWrite(PORTD, 7, o == 4);
  bitWrite(PORTD, 6, o == 3);
  bitWrite(PORTD, 5, o == 2);
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

