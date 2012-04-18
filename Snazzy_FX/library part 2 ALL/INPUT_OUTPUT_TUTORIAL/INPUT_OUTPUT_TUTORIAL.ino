
const int pinOffset = 5;


long input=analogRead(2);//here is your in A2 input



void setup () {
 
  
  // set up the 8-bit DAC output pins
  for (int i=0; i<8; i++) {
    pinMode(pinOffset+i, OUTPUT);
    digitalWrite(pinOffset+i, LOW);
  }

}

void loop () {
  
  dacOutput(input); //send your input through your output!!!!
  
}



//  dacOutput(long) - deal with the DAC output
//  ------------------------------------------
void dacOutput(int v)
{
  int tmpVal = v;
  for (int i=0; i<8; i++) {
    digitalWrite(pinOffset + i, tmpVal & 1);
    tmpVal = tmpVal >> 1;
  }
}

//  ==========
