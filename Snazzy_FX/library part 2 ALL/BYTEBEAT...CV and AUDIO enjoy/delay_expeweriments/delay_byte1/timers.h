#define pwm3 OCR2B
#define pwm5 OCR0B
#define pwm6 OCR0A
#define pwm9 OCR1A
#define pwm10 OCR1B
#define pwm11 OCR2A

int getTimer(int pin);
int getChannel(int pin);

// - - -- timer settings

#define phaseCorrect 0
#define fastPWM 1

void waveformGenerationMode(int pin, int type);
void waveformGenerationMode(int pin, int type, int bits); // pins 9 and 10
void timerPrescale(int pin, int prescale);
int getPrescale01(int prescale);
int getPrescale2(int prescale);

// - - -- analog prescaling

#define analogPrescale2 B001
#define analogPrescale4 B010
#define analogPrescale8 B011
#define analogPrescale16 B100
#define analogPrescale32 B101
#define analogPrescale64 B110
#define analogPrescale128 B111

void analogPrescale(int divisionFactor);
