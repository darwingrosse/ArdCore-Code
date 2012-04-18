//  Program: ArdCore TRIANGLE VCO
//STILL IN EXPERIMENTAL STAGE...range is limited...still fun though

//

//
//  I/O Usage:FILTER IT!!!
//    Knob 1: 
//    Knob 2: 
//    Analog In 1: Control voltages INPUT
//    Analog In 2:
//    Digital Out 1:
//    Digital Out 2: 
//    Clock In:
//    Analog Out: Triangle PUT
//
//  Input Expander: unused
//  Output Expander:unused
//
//  Created:  FEB 2012 by Dan Snazelle
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
const int pinOffset = 5;       // the first DAC pin (from 5-12)

  void setup() {
  //8 bit timer, Fast PWM, TOP=0xFF, clk=16MHz
  DDRD |= 0x08;
  TCCR2A = _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(CS20);
  
  
  // set up the 8-bit DAC output pins
  for (int i=0; i<8; i++) {
    pinMode(pinOffset+i, OUTPUT);
    digitalWrite(pinOffset+i, LOW);
  }
  
  
  


}


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

   
void genTriangle(void){
  int val=analogRead(2);
  
     val=map (analogRead(2), 0, 1023, 145, 5);


   for (int n = 0; n < 128; ++n) {
  //  OCR2B = n * 2;
    dacOutput (n * 2);
    
    

    delayMicroseconds(val); 
  }
  int value = 255;
  for (int n = 128; n < 256; ++n) {
  // OCR2B = value;
   dacOutput(value);
    value -= 2;
    delayMicroseconds(val); 
     
  }

  
}




void loop(){

genTriangle();
} 


