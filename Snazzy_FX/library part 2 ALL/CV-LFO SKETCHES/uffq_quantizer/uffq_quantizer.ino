//7 bit r2r on port d

here's an idiot version (totally untested off the top of my head)....

int CVinPin = 3; // input CV
int val = 0; // variable to store the value read

void setup()
{
//SET PINS 0-6 AS OUTPUTS
DDRD = B01111111;
}

void loop()
{
val = analogRead(CVinPin ); // read the input pin 0..1023
val /= 8; //0..127
PORTD = val;
}

