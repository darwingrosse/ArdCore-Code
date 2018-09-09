//  Program: ArdCore TRIANGLE VCO
//STILL IN EXPERIMENTAL STAGE...range is limited...still fun though

//

//
//  I/O Usage:
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

//  ================= start of global section ================== 
const int pinOffset = 5;       // the first DAC pin (from 5-12)


 void setup() { 
  // set up the 8-bit DAC output pins
  for (int i=0; i<8; i++) {
    pinMode(pinOffset+i, OUTPUT);
    digitalWrite(pinOffset+i, LOW);
  }
  //uncomment to make SLLLLLOOOOW
  noInterrupts();
   CLKPR=1<<CLKPCE;
   CLKPR=0;
   interrupts();
  
  


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


   for (int n = 0; n < 100; ++n) {
  // OCR2B = n * 2;
    dacOutput (n );
    
    

    delayMicroseconds(val); 
  }

  
}




void loop(){

genTriangle();
} 

