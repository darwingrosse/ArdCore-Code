//AUTOMATIC VOLTAGE METER
//Use this sketch to take readings from your modules..use the data how you wish. (to control Processing sketches, or use this code in another sketch which converts the values into CV or gates)
//A2 is the jack used for reading external values (will take between 0-5v...averages over 2500 samples....for quickly changing waveforms, reduce "SAMPLES" value
//read the values in SERIAL MONITOR

#define SAMPLES 2500

void setup () {
  Serial.begin(9600);
  Serial.println("Arduino AUTO voltmeter");
  ADMUX=1<<REFS0|1<<MUX1;//SELECT ADC (A2), AREF=AVCC (5.0v)
  ADCSRA= 1<<ADEN|1<<ADSC|1<<ADATE|1<<ADIE|1<<ADPS2|1<<ADPS1|1<<ADPS0;
  //B11101111
  ADCSRB=0;//free running mode
  bitSet(DIDR0,ADC0D);//disable digital input on adco via DIDR0 register
}

void loop () {
  //notihing
}

ISR (ADC_vect) {
  static unsigned int i=0; //sample counter
  static unsigned long voltage=0;// voltage reading accumulator
  
  voltage += ADC; //accumulate voltage readings
  i++;//also count samples taken
  ADMUX=1<<REFS0|1<<MUX1;//reselect ADC2 and AREF=avcc(5v)
  
  if(i >=SAMPLES) {
    Serial.print((((voltage*5.0) /1024 ) /SAMPLES),4);//report
    Serial.println("V"); //label units of measure
    voltage=0;//resertt readings
    i=0;//reset sampel cont
  }
  
}
    
