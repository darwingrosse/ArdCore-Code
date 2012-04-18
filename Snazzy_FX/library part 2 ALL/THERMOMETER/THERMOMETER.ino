//ARDCORE TEMPERATURE!!!

//open up SERIAL MONITOR TO TEST

//CALIBRATE BY CHANGING OFFSET


void setup () {
  Serial.begin(9600);
  Serial.println("Atmega 328 as thermometer");
  ADMUX = 1<<REFS1 | 1<<REFS0| 1<<MUX3; // 1.1v refernce, ADC channel 8
  ADCSRA= 1<<ADEN| 1<<ADSC| 0x07; //enable ADC, start conversin, 125 khz clock
}

#define OFFSET 334

void loop () {
  
  Serial.print (ADC-OFFSET);
  Serial.println("C");
  bitSet(ADCSRA, ADSC); //start next conversion
  delay(250);//wait
}


  
