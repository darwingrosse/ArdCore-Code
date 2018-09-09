

//SIMPLE RANDOM GATES WITH ALTERNATING LEDS/TRIGGERS ON GATE OUTS and CLOCK
//MADE FOR ARDCORE 
//OUTS ON CLOCK, D0, AND D1
//Dan Snazelle 20


int ledPin=2;
int val=0;
int delayval=0;





void setup () { 
    //Serial.begin(9600);
    
    // set up the digital outputs

    pinMode(3, OUTPUT);
   // digitalWrite(3, LOW);
    pinMode(4, OUTPUT);
    //digitalWrite(4, LOW);
  

pinMode (ledPin, OUTPUT);
}

void loop () 
{
  delay(500);
  digitalWrite(3, HIGH);
  delay(500);
  digitalWrite(4, HIGH);
val=random (10,255) ;
analogWrite(ledPin,val);

   
delayval=(random(5,200))*19;
delay(delayval);
digitalWrite(3, LOW);
delay(500);
  digitalWrite(4, LOW);
 delay(500);
}


