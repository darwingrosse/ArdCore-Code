#include "timers.h"
#include "arduino.h"

int getTimer(int pin) {
  switch(pin) {
    case 5: case 6: return 0;
    case 9: case 10: return 1;
  }
  return 2; // 3, 11
}

int getChannel(int pin) {
  switch(pin) {
    case 6: case 10: case 11: return 0;
  }
  return 1; // 3, 9, 5
}

// - - -- timer settings

void waveformGenerationMode(int pin, int type) {
  int timer = getTimer(pin);
  int wgm = type == phaseCorrect ? B001 : B011;
  if(timer == 0) {
    TCCR0B &= ~(B1 << 3); // clear WGM02
    TCCR0A &= ~B11; // clear WGM01 and WGM00
    TCCR0A |= wgm; // set WGM01 and WGM00
  } else if(timer == 2) {
    TCCR2B &= ~(B1 << 3); // clear WGM23
    TCCR2A &= ~B11; // clear WGM21 and WGM20
    TCCR2A |= wgm; // set WGM21 and WGM20
  } 
}

void waveformGenerationMode(int pin, int type, int bits) {
  int timer = getTimer(pin);
  if(timer == 1) {
    TCCR1B &= ~(B11 << 3); // clear WGM13 and WGM12
    TCCR1A &= ~B11; // clear WGM11 and WGM10
    TCCR1B |= (type << 3); // set WGM12
    TCCR1A |= (bits - 7);
  }
}

void timerPrescale(int pin, int prescale) {
  int timer = getTimer(pin);
  if(timer == 0) {
    TCCR0B &= ~B111; // clear CS02 CS01 CS00
    TCCR0B |= getPrescale01(prescale);
  } else if(timer == 1) {
    TCCR1B &= ~B111; // clear CS12 CS11 CS10
    TCCR1B |= getPrescale01(prescale);
  } else if(timer == 2) {
    TCCR2B &= ~B111; // clear CS22 CS21 CS20
    TCCR2B |= getPrescale2(prescale);
  }
}

int getPrescale01(int prescale) {
  switch(prescale) {
    case 1: return B001;
    case 8: return B010;
    case 64: return B011;
    case 256: return B100;
    case 1024: return B101;
    default: return B000;
  }
}

int getPrescale2(int prescale) {
  switch(prescale) {
    case 1: return B001;
    case 8: return B010;
    case 32: return B011;
    case 64: return B100;
    case 128: return B101;
    case 256: return B110;
    case 1024: return B111;
    default: return B000;
  }
}

// - - -- analog prescaling

void analogPrescale(int divisionFactor) {
  ADCSRA &= ~B111; // clear analog prescale
  ADCSRA |= divisionFactor;
}
