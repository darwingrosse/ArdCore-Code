//  ============================================================
//
//  Program: Dans Random
//
//  Description: 
//              puts out crazy audio (noisy) or works as CV or FM
//
//  I/O Usage:
//    Knob 1: Random Depth
//    Knob 2: Random Seed
//    Analog In 1: PLUG IN SOMETHING
//    Analog In 2: PLUG IN SOMETHING
//    Digital Out 1: weird sputterings
//    Digital Out 2: 
//    Clock In: 
//    Analog Out: 8-bit output
//
//  Input Expander: unused
//  
//
//  Created: feb 2012
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

const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)



//  variables used to control the current DIO output states
int digState[2] = {HIGH, LOW};  // start with both set low
int randDepth;                  // the depth of randomization

//  ==================== start of setup() ======================

void setup() {

  Serial.begin(9600);
  randomSeed(analogRead(1));

  
  
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
}

//  ==================== start of loop() =======================

void loop()
{
  

  if (analogRead(2)>=150) {
    int tempShift = analogRead(0) >> 7;
    int tempRand = (random(256) >> tempShift) << tempShift;
    int bomp=tempRand/analogRead(2);
  int  allright= (abs(bomp))*3;
    analogWrite(3,allright); 
    dacOutput(tempRand);
  }
    
    else{
       dacOutput(analogRead(3)*analogRead(2));
    }
    
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

//  ===================== end of program =======================
