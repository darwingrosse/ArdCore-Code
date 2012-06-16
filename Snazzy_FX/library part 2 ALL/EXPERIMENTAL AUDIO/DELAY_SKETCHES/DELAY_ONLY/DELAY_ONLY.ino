/* Delay...DIRTY BUT SOFT

Delay Input= jack A3...KNOB A3=INPUT LEVEL...turn this down quite a bit
Delay time=knob/jack A2
knob A0=bitcrush


*/



// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif
//sampBuffer storages
#define storage 1500  //168 is 500, 328 is 1000.
byte sampBuffer[storage];

//lowpass filter
int lowV = 131;
unsigned long lowMill = 0;

//SH
float SHrv = 0;
unsigned long SHmil = 0;

byte keys[5] = {0,0,0,0,0};
byte keysp[5] = {0,0,0,0,0};

//analog
int analogInput = 2;
byte anaCount = 0;
int anaVs[4];

//modulator
unsigned long Mmill = 0;
boolean mhl = true;

//digital delay
int dlyCounter = 0;

//audio out variable
long signalOut;

//synth volume 
int time = 0;

const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)
//volatile int clkState = LOW;

//int digState[2] = {HIGH, LOW};  // start with both set low

void setup () {
  
  analogReference(INTERNAL);
  
  //speed up http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1235060559
  TCCR0B = TCCR0B & 0b11111000 | 0x01;
  
  //audio output
  //pinMode(9, OUTPUT);
  
  //buttons pins

 
  
//  Serial.begin(9600);

  
  
 // Serial.begin(9600);
  
   // set up the digital (clock) input
 // pinMode(clkIn, INPUT);
 
 
    // set up the digital outputs
 
  
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


void loop () {
  
  
  //  Serial.begin(9600);




  
  //analogReads
  signalOut = analogRead(3) >> 2; //volume change
  inputRead();

  //Effacts
  digitalDelay();
// looper();
//  modulator();
  //distortion();
 // SHgate();
  //lowpass();
  
  //write to output
 dacOutput(signalOut);
  delayMicroseconds(map(anaVs[1], 0, 1023, 20000, 1));
  
}







void digitalDelay(){
  
    time = map(analogRead(2), 0, 1023,0, (storage - 1));//storageORY ARRAY MINUS 1...MAPPED FOR DELAY
  
    dlyCounter++;
    if(dlyCounter > (storage-1)){
      dlyCounter = 0;
    }
    


      //int pitch=(analogRead(1));
    int counterModulo = (dlyCounter+(time)) % (storage - 1);  //try (time*2) or time*3 for more interesting DIRTY effects and longer time
   // counterModulo=counterModulo/(pitch>>5);
  // delayMicroseconds(map(anaVs[1], 0, 1023, 20000, 1));
    sampBuffer[dlyCounter] = (signalOut+sampBuffer[counterModulo])>>1 ;
   // int signal2=sampBuffer[dlyCounter];
    signalOut = sampBuffer[dlyCounter];//%analogRead(1)>>3
   //    aWrite(signal2>>3);
  }


void looper(){
  
  
    
    int l = map(anaVs[3], 0, 1023, (storage-1), 4);
    
    dlyCounter++;
    if(dlyCounter > l){
      dlyCounter = 0;
    }
    signalOut = sampBuffer[dlyCounter];
  }




//output to audio
void aWrite(byte i){
  i = constrain(i, 0, 255);
  analogWrite(3,i); 
}

//analog Reads
void inputRead(){
 
  anaVs[2] = analogRead(2);
   anaVs[1] = analogRead(0);
    anaVs[3] = analogRead(1);
}
















//  =================== convenience routines ===================

//  isr() - quickly handle interrupts from the clock input
//  ------------------------------------------------------
//void isr()
//{
  //clkState = HIGH;
//}

//  dacOutput(long) - deal with the DAC output
//  ------------------------------------------
void dacOutput(long v)
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
