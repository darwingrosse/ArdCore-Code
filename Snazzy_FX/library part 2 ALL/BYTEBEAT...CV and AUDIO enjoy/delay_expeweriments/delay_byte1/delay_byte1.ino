
#include "dsp.h" // include hardware timers and definitions
const int pinOffset = 5;       // the first DAC pin (from 5-12)
int i = 0;
long myValue,myValue2;
void setup() {
  
     // set up the 8-bit DAC output pins
  for (int i=0; i<8; i++) {
    pinMode(pinOffset+i, OUTPUT);
    digitalWrite(pinOffset+i, LOW);
    
  }

//Serial.begin(115200); //start fast serial comm

setupIO(); //run hardware timers setup

}

const short arraySize = 900; //max delay size for ATmega328 ~950
short delayLength = 1;
short timer = 0;

short timeout = 2048; //keep timeout low to keep from lowering srate
float feedback = 0.5;

unsigned short delArr[arraySize];
unsigned short input, out, pointer=0, delPointer;


//Main processing loop - one cycle per sample
void loop() {
	//read controll values from pots
	if(timer>timeout){ //check the clock
		//read the scaled pins
		delayLength = ( ( (float)arraySize-1 ) * ((float)analogRead(3)/1024)) + 1;
		feedback = (float)analogRead(1) / 1000.0; //limit feedback
int t=delayLength;
int j=feedback;
                                                                                                                                                                                                                                         
myValue=(((1-(((t+10)>>((t>>9)&((t>>14))))&(t>>4&-2)))*2)*(((t>>10)^((t+((t>>6)&127))>>10))&1)*32+128);
myValue2=((j*(j>>8|t>>9)&46&j>>8)) ^ (j&j>>13|j>>6);



		timer = 0; //reset the clock
	}



	//count the clock
	timer++; 
 
	//read the input value (signal => sample)
	input = analogRead(2);
 
	//update delPointer (where are we in the array)
	delPointer = pointer + myValue;
	delPointer = delPointer % arraySize;
 
	//insert sample into the array with feedback.
	delArr[pointer] = input + (delArr[delPointer] * myValue2);


//feedback=(int) (feedback);

	//set the output sample
	out = delArr[delPointer] + input;
	out = out >> 2; //reduce vol to prevent clipping. [adjust this if you change code]
 
	//move the array pointer
	pointer++;
	pointer = pointer % arraySize;

int output=out;
	//write to the pins [play the sample]
	dacOutput(output);//(left, out);
	//output(right, out);

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
