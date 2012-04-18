
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

  myValue= (((t*(t>>12)&(201*t/100)&(199*t/100))&(t*(t>>14)&(t*301/100)&(t*399/100)))+((t*(t>>16)&(t*202/100)&(t*198/100))-(t*(t>>18)&(t*302/100)&(t*298/100))));

analogWrite(3, myValue);


  return;



return;
}
