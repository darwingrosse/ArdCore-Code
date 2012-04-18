
//OUTPUT=D0

//PLUG IN A JACK TO DO AND WAIT....THIS PUTS OUT AUDIO...give it time!!!


#include "wiring_private.h"
#undef round
#undef abs



void setup()  { 
 
  pinMode(3, OUTPUT);
 
} 

long t = 0;
int i = 0;

void loop() {
  if (++i != 64) return;
  i = 0;
  t++;

  long myValue;

  myValue= 
((t*(t>>8|t>>9)&46&t>>8)) ^ (t&t>>13|t>>6);



analogWrite(3, myValue);


  return;



return;
}
