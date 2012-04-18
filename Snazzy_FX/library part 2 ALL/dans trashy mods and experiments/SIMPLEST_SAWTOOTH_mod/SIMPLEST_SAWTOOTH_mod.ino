const int pinOffset=5;

void setup () {
  
  for(int i=0; i<8; i++){
  pinMode(pinOffset+i, OUTPUT);
  digitalWrite(pinOffset + i,LOW);
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


void genSaw() {
  char t;
  while(1) {
    for(t=0; t<=255; t++)
    {dacOutput(((t+255)^(t*2/4))*(analogRead(2)>>2));
    delayMicroseconds(2);
    }
  }
}
  
  

void loop(){

genSaw();
} 


