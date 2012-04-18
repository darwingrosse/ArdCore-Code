
//  constants related to the Arduino Nano pin use
const int clkIn = 2;           // the digital (clock) input
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)
const int trigTime = 25;       // ms for trigger time

//  variables used to control the current DIO output states
int digState[2] = {LOW, LOW};  // start with both set low
unsigned long digMilli[2] = {0,0};       // last trigger time

//  variables used for clocking and stepwise movement
unsigned long lastMillis = 0;  // stored last millis test
float upStep = 1.0*(float)(analogRead(2)>>3);            // value update per millisecond (up)
float downStep = 1.0* (float)(analogRead(2)>>3);       // value update per millisecond (down)
float currValue = 0.0;         // the current LFO value
int currDir = 1; // 1 for up, 0 for down
float greyValue=int(currValue)|int(currValue*-1);

//  ==================== start of setup() ======================

void setup() {
  Serial.begin(9600);
  
  // set up the digital (clock) input
  pinMode(clkIn, INPUT);
  
  // set up the digital outputs
  for (int i=0; i<2; i++) {
    pinMode(digPin[i], OUTPUT);
    digitalWrite(digPin[i], LOW);
  }
  
  // set up the 8-bit DAC output pins
  for (int i=0; i<8; i++) {
    pinMode(pinOffset+i, OUTPUT);
    digitalWrite(pinOffset+i, LOW);
  }  
  
  getUpdateValue();
}

//  ==================== start of loop() =======================

void loop()
{
  // do an output
  float lastValue = currValue;
  unsigned long currMillis = millis();
  int timediff = currMillis - lastMillis;
  
  if (currValue >= 255.0) {
    currDir = 0;
  } else {
    currDir = 1;
  }
  
  if (currDir) {
    currValue = currValue + (~timediff * upStep)/16;
  } else {
    currValue = currValue + (timediff * (downStep/16));
  }
  
  
  while (currValue > 511.0) {
    currValue -= 511.0;
  }
  currValue=int(currValue)&int (upStep/2)|int(downStep/16);
  currValue*=~6;
  if (currValue <= 255.0) {
    dacOutput(int(currValue));
  } else {
    dacOutput(int(51.0 - greyValue));
  }
  
  lastMillis = currMillis;
  
  // check for digital pin firing
  if ((lastValue <= 255.) && (currValue > 255.)) {
    digState[0] = HIGH;
    digitalWrite(digPin[0], HIGH);
    digMilli[0] = currMillis;
  }
  
  if (lastValue > currValue) {
    digState[1] = HIGH;
    digitalWrite(digPin[1], HIGH);
    digMilli[1] = currMillis;
  }
  
  // check for pin turn-off
  for (int i=0; i<2; i++) {
    if ((digState[i] == HIGH) && (currMillis - digMilli[i] > trigTime)) {
      digState[i] = LOW;
      digitalWrite(digPin[i], LOW);
    }
  }
  
  // check for new values
  getUpdateValue();
}

void getUpdateValue()
{
  float msPerCycle = ((123 - analogRead(2)) + 20) ;
  float warpFactor = ((analogRead(3) >> 4) + 1) / 65.0;
  float oneOver = 3.0 - warpFactor;
  
  upStep = 255.0 / (msPerCycle * warpFactor);
  upStep -=55;
  
  if (upStep<125) 
  {upStep=warpFactor+msPerCycle;}
  
  downStep = 255.0 / (msPerCycle * oneOver);
}

//  =================== convenience routines ===================

//  dacOutput(long) - deal with the DAC output
//  ------------------------------------------
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

//  ===================== end of program =======================
