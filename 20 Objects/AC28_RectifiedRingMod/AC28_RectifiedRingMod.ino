//Ardcore Ring Modulator
//8/10/12

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

const int digPin[2] = {3, 4};
const int pinOffset = 5;
int digState[2] = {LOW, LOW};

int inputA;
int inputB;
byte output;

void setup()
{
  // clear the digital pins
  for (int i=0; i<2; i++) {
    pinMode(digPin[i], OUTPUT);
    digitalWrite(digPin[i], digState[i]);
  }

  // clear the analog output
  for (int i=0; i<8; i++) {
    pinMode(pinOffset+i, OUTPUT);
    digitalWrite(pinOffset+i, LOW);
  }
  
  // set the ADC to a higher prescale factor
  sbi(ADCSRA,ADPS2);
  cbi(ADCSRA,ADPS1);
  cbi(ADCSRA,ADPS0);  

}

void loop () 
{
  // Note: Since the analog inputs only measure values above 0V, if you feed the
  //       ArdCore a standard bipolar waveform, you will get a ring modulation of
  //       the half-rectified waveforms, which will have a very clangorous sound.
  
  inputA = analogRead(2) >> 2;      // get the A2 input, then reduce it to 8-bit
  inputB = analogRead(3) >> 2;      // get the A3 input, then reduce it to 8-bit
  output = (inputA * inputB) >> 1;  // multiply, then shift right by 1 (the same as /2)
  dacOutput(output);
}

void dacOutput(byte v)
{
  PORTB = (PORTB & B11100000) | (v >> 3);
  PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);
}

