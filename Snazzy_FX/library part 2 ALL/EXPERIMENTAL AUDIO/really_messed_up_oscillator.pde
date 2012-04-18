//REA:Lly messed up oscillator
//works with ardcore
//A0=pitch knob
//A2=CV in




int CV_mod;
int potval;
void setup() {
  //Serial.begin(9600);
  pinMode(3, OUTPUT);      
}

void loop()
{
  while(1){
    CV_mod=analogRead(2)*3;
    if (CV_mod==0){
    CV_mod=1;
    }
    potval=analogRead(0);
    
    digitalWrite(3, HIGH);
    delayMicroseconds(12*potval*CV_mod);
    digitalWrite(3, LOW);
    Serial.println(potval);
  }
}


