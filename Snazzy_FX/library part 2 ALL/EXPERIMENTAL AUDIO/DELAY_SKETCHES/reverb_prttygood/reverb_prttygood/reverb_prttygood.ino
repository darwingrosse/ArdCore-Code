/*

REVERB EXPERIMENT

A0= 
A1=
A2 KNOB=INPUT LEVEL...keep this low
A3 KNOB=INPUT LEVEL
A2 JACK=
A3 JACK=AUDIO INPUT
DAC OUT=AUDIO OUT

*/





// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif
int flip;
#define CENTERPOS 	    128
int pos; 			// pointer into next position to store

unsigned long SHmil = 0;
char signal[1800];	//stores the signal in an array
int    S1out;
float SHrv = 0;
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)
//volatile int clkState = LOW;

int digState[2] = {HIGH, LOW};  // start with both set low

void setup () {
  
  


 // Serial.begin(9600);
  
   // set up the digital (clock) input
 // pinMode(clkIn, INPUT);
 
 
    // set up the digital outputs
  for (int i=0; i<2; i++) {
    pinMode(digPin[i], OUTPUT);
    digitalWrite(digPin[i], digState[i]);
  }
  
    // set up the 8-bit DAC output pins
  for (int i=0; i<8; i++) {
    pinMode(pinOffset+i, OUTPUT);
    digitalWrite(pinOffset+i, LOW);
    
    
 
  }
  
  //voltage ref regs
   cbi (ADMUX,REFS1);
sbi  (ADMUX, REFS0);
  
       // set the ADC to a higher prescale factor...this sets it to /64.... a much lower sample rate...which means longer reverb
  sbi(ADCSRA,ADPS2);
  sbi(ADCSRA,ADPS1);
   cbi(ADCSRA,ADPS0);
  
   cbi (TCCR0B,CS00);
  sbi (TCCR0B,CS01);
   cbi (TCCR0B,CS02
  
  
  
  
);

 
  
  
  
}

void loop () {
  
  flip = 0;
    S1out=analogRead(3);
  char getinput=analogRead(3)>>2-CENTERPOS;
  if(pos>=1800)
      	    pos = 0;	
        // input will probably be the direct input from the ADC

        // Read when ready 
        // output to port A
        signal[pos] =getinput;
        pos++;

        // Next output
        S1out = signal[pos-1];
	

 //dacOutput(((S1out + CENTERPOS)>>2)<<2);
  
// SHgate();
 // distortion();
reverb();
   dacOutput(S1out);
//   delayMicroseconds(map(analogRead(0), 0, 1023, 22000, 1));
}



/*void SHgate(){
  int anaVs[2]={analogRead(2)};

  int l = map(anaVs[2  ], 0, 1023, 700, 0);
    
   if(millis() - SHmil > l){
     SHmil = millis();
     SHrv = random(1,100)*0.00999;
   }
   
     {
     S1out -= 131;
     S1out *= SHrv;
     S1out += 131;
      S1out = constrain(S1out, 90, 165);
      //delayMicroseconds(map(analogRead(1), 0, 1023, 2000, 1));
     
   }
}


*/
// reverb function
void reverb(){

       //counter of output from echo array
       int echo1=0;
       int echo2=0;
        
       // assuming there is a global variable "output" 
       // added to current input with a time delay.  
       			echo1=pos-900; 
       		echo2=pos-1799;
       		if(echo1<0)
       			echo1=1800+pos-900;
       		if(echo2<0)
       			echo2=1800+pos-1799;               		
       	S1out = S1out + (signal[echo1]>>2) + (signal[echo2]>>3);
       		      				 
       		      				 
}





    

  
  

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
