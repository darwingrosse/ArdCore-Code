

//  ============================================================
//ARDCORE PROGRAM
//
//
//  Program: Note Harmonize (based on an idea in Curtis Rhodes "the computer 
// music tutorial")
//
//  Description: alters incoming notes
//
//  I/O Usage:
//    Knob A0:
//    Knob A1: 
//    Knob A2-CV jack: CV in (lfo, woggle,etc)
//    Knob A3-CV jack: transpose amount
//    Digital Out 1(DO1): 
//    Digital Out 2(DO2):
//    CLK:
//    DAC OUT: CV OUT

//

//
//  Created:  mar 2012 by Dan Snazelle
// //HARMONIZE
#define pinOffset  5    // the first DAC pin (from 5-12)

int inputNote,outputNote;
static boolean harmonyState;
int outValue = 0;              // the DAC output value
int oldTranspose = -1;         // the test transpose value
int transpose = 0;             // the transposition amount
int doQuant = 0;               // flag to do calcs
void setup() {

 // set up the 8-bit DAC output pins
  for (int i=0; i<8; i++) {
    pinMode(pinOffset+i, OUTPUT);
    digitalWrite(pinOffset+i, LOW);
  }
    

}

void loop () {
  
  
  // test for transpose change..checks to see if note changes
  transpose = analogRead(3) / 84;
  if (transpose != oldTranspose) {
    oldTranspose = transpose;
    doQuant = 1;
  }

  
  inputNote=analogRead(2)>>4;
  
  if(harmonyState) {//is high
  //harmonize by a 5th
  outputNote=inputNote+7;
  harmonyState=false;}
   else
    //perfect 4th
  {  outputNote=inputNote+5;
    harmonyState=true;}
    
    
    byte outValue = quantNote(outputNote);
    dacOutput(outputNote*transpose);
    
}
  
  
  void dacOutput(byte v) {
  PORTB = (PORTB & B11100000) | (v >> 3);
  PORTD = (PORTD & B00011111) | ((v & B00000111) << 5);
}
  
  
  //  quantNote(int) - drop an incoming value to a note value
//  -------------------------------------------------------
int quantNote(int v)
{
  int tempVal = v >> 4;  // decrease the value to 0-64 - ~ a 5 volt range
  tempVal += transpose;  // add the transposition
  return (tempVal << 2); // increase it to the full 8-bit range
}
