//  ============================================================
//
//  Program: ArdCore SimpleVCO
//
//  Description: A not-so-great VCO that uses direct manipulation
//               of the output pins (and a higher-speed analog
//               input routine) to make a loop() routing fast
//               enough to work as a VCO.
//
//  NOTE: Prescaler setup routines used from the Arduino Forum's
//        use "jmknapp", available at this link:
//        http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1208715493/11
//
//  I/O Usage:
//    Knob 1: Pitch offset
//    Knob 2: unused
//    Analog In 1: 1 V/Oct CV input
//    Analog In 2: unused
//    Digital Out 1: unused
//    Digital Out 2: unused
//    Clock In: unused
//    Analog Out: 8-bit output
//
//  Input Expander: unused
//  Output Expander: 8 bits of output exposed
//
//  Created:  19 Mar 2011  ddg
//  Modified: 17 Apr 2012  ddg Updated for Arduino 1.0
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

//  an array of microseconds for each "MIDI" note
const float usNote[128] = {
  122312.195312, 115447.359375, 108967.789062, 102851.898438, 97079.265625,
   91630.625000,  86487.789062,  81633.601562,  77051.859375, 72727.273438,
   68645.406250,  64792.632812,  61156.097656,  57723.679688, 54483.894531,
   51425.949219,  48539.632812,  45815.312500,  43243.894531, 40816.800781,
   38525.929688,  36363.636719,  34322.703125,  32396.316406, 30578.048828,
   28861.839844,  27241.947266,  25712.974609,  24269.816406, 22907.656250,
   21621.947266,  20408.400391,  19262.964844,  18181.818359, 17161.351562,
   16198.158203,  15289.024414,  14430.919922,  13620.973633, 12856.487305,
   12134.908203,  11453.828125,  10810.973633,  10204.200195,  9631.482422,
    9090.90918,    8580.675781,   8099.079102,   7644.512207,  7215.459961,
    6810.486816,   6428.243652,   6067.454102,   5726.914062,  5405.486816,
    5102.100098,   4815.741211,   4545.454590,   4290.337891,  4049.539551,
    3822.256104,   3607.729980,   3405.243408,   3214.121826,  3033.727051,
    2863.457031,   2702.743408,   2551.050049,   2407.870605,  2272.727295,
    2145.168945,   2024.769775,   1911.128052,   1803.864990,  1702.621704,
    1607.060913,   1516.863525,   1431.728516,   1351.371704,  1275.525024,
    1203.935303,   1136.363647,   1072.584473,   1012.384888,   955.564026,
     901.932495,    851.310852,    803.530457,    758.431763,   715.864258,
     675.685852,    637.762512,    601.967651,    568.181824,   536.292236,
     506.192444,    477.782013,    450.966248,    425.655426,   401.765228,
     379.215881,    357.932129,    337.842926,    318.881256,   300.983826,
     284.090912,    268.146118,    253.096222,    238.891006,   225.483124,
     212.827713,    200.882614,    189.607941,    178.966064,   168.921463,
     159.440628,    150.491913,    142.045456,    134.073059,   126.548111,
     119.445503,    112.741562,    106.413857,    100.441307,    94.803970,
      89.483032,     84.460732,     79.720314 };

unsigned long lastMicros = 0;
int digState = 0;
int currNote = 0;
int currOffset = 0;
      
void setup()
{
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
}


void loop()
{
  currNote = deJitter(analogRead(2), currNote) >> 3;
  currOffset = deJitter(analogRead(0), currOffset) >> 3;
  
  unsigned long currMicros = micros();
  
  if ((currMicros - lastMicros) > usNote[currNote + currOffset]) {
    digState = 255 - digState;
    dacOutput(digState);
    lastMicros = currMicros;
  }
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
