/*
 *
 * DDS Sine Generator mit ATMEGS 168
 * Timer2 generates the  31250 KHz Clock Interrupt
 *
 * KHM 2009 /  Martin Nawrath
 * Kunsthochschule fuer Medien Koeln
 * Academy of Media Arts Cologne

 */

#include "avr/pgmspace.h"

// table of 256 sine values / one sine period / stored in flash memory
PROGMEM  prog_uchar sine256[]  = {
  127,130,133,136,139,143,146,149,152,155,158,161,164,167,170,173,176,178,181,184,187,190,192,195,198,200,203,205,208,210,212,215,217,219,221,223,225,227,229,231,233,234,236,238,239,240,
  242,243,244,245,247,248,249,249,250,251,252,252,253,253,253,254,254,254,254,254,254,254,253,253,253,252,252,251,250,249,249,248,247,245,244,243,242,240,239,238,236,234,233,231,229,227,225,223,
  221,219,217,215,212,210,208,205,203,200,198,195,192,190,187,184,181,178,176,173,170,167,164,161,158,155,152,149,146,143,139,136,133,130,127,124,121,118,115,111,108,105,102,99,96,93,90,87,84,81,78,
  76,73,70,67,64,62,59,56,54,51,49,46,44,42,39,37,35,33,31,29,27,25,23,21,20,18,16,15,14,12,11,10,9,7,6,5,5,4,3,2,2,1,1,1,0,0,0,0,0,0,0,1,1,1,2,2,3,4,5,5,6,7,9,10,11,12,14,15,16,18,20,21,23,25,27,29,31,
  33,35,37,39,42,44,46,49,51,54,56,59,62,64,67,70,73,76,78,81,84,87,90,93,96,99,102,105,108,111,115,118,121,124

};
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

int ledPin = 13;                 // LED pin 7
int testPin = 7;
int t2Pin = 6;
byte bb;

double dfreq;
// const double refclk=31372.549;  // =16MHz / 510
const double refclk=31376.6;      // measured

// variables used inside interrupt service declared as voilatile
volatile byte icnt;              // var inside interrupt
volatile byte icnt1;             // var inside interrupt
volatile byte c4ms;              // counter incremented all 4ms
volatile unsigned long phaccu;   // pahse accumulator
volatile unsigned long tword_m;  // dds tuning word m

void setup()
{
  pinMode(ledPin, OUTPUT);      // sets the digital pin as output
  Serial.begin(115200);        // connect to the serial port
  Serial.println("DDS Test");

  pinMode(6, OUTPUT);      // sets the digital pin as output
  pinMode(7, OUTPUT);      // sets the digital pin as output
  pinMode(11, OUTPUT);     // pin11= PWM  output / frequency output

  Setup_timer2();

  // disable interrupts to avoid timing distortion
  cbi (TIMSK0,TOIE0);              // disable Timer0 !!! delay() is now not available
  sbi (TIMSK2,TOIE2);              // enable Timer2 Interrupt

  dfreq=100.0;                    // initial output frequency = 1000.o Hz
  tword_m=pow(2,32)*dfreq/refclk;  // calulate DDS new tuning word 

}
void loop()
{
  while(1) {
     if (c4ms > 250) {                 // timer / wait fou a full second
      c4ms=0;
      dfreq=analogRead(2
#include <avr/interrupt.h>



#define SAMPLE_RATE 8000
#define BUFFER_SIZE 1024

unsigned long sounddata_length=0;
unsigned long sample=0;
unsigned long BytesReceived=0;

unsigned long Temp=0;
unsigned long NewTemp=0;

int ledPin = 13;
int speakerPin = 11;
int Playing = 0;

//Interrupt Service Routine (ISR)
// This is called at 8000 Hz to load the next sample.
ISR(TIMER1_COMPA_vect)
{
    //If not at the end of audio
    if (sample < sounddata_length)  
    {
	  //Set the PWM Freq.

	  OCR2A = Serial.read();
	  BytesReceived++;
	  sample++;

		//if the Serial port buffer has room
    if ((BytesReceived % BUFFER_SIZE) == 0)
    {
	//Tell the remote PC how much space you have.
	Serial.println(BUFFER_SIZE);
    }//End if

    }//End if
    else //We are at the end of audio
    {
	  //Stop playing.
	  stopPlayback();
    }//End Else

}//End Interrupt






void startPlayback()
{
    //Set pin for OUTPUT mode.
    pinMode(speakerPin, OUTPUT);

    //---------------TIMER 2-------------------------------------
    // Set up Timer 2 to do pulse width modulation on the speaker
    // pin.  
    //This plays the music at the frequency of the audio sample.

    // Use internal clock (datasheet p.160)
    //ASSR = Asynchronous Status Register
    ASSR &= ~(_BV(EXCLK) | _BV(AS2));

    // Set fast PWM mode  (p.157)
    //Timer/Counter Control Register A/B for Timer 2
    TCCR2A |= _BV(WGM21) | _BV(WGM20);
    TCCR2B &= ~_BV(WGM22);

    // Do non-inverting PWM on pin OC2A (p.155)
    // On the Arduino this is pin 11.
    TCCR2A = (TCCR2A | _BV(COM2A1)) & ~_BV(COM2A0);
    TCCR2A &= ~(_BV(COM2B1) | _BV(COM2B0));

    // No prescaler (p.158)
    TCCR2B = (TCCR2B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);

    //16000000 cycles	 1 increment    2000000 increments
    //--------	  *  ----		= -------
    //	 1 second	 8 cycles		 1 second

    //Continued...
    //2000000 increments     1 overflow	7812 overflows
    //-------		* ---		= -----
    //	1 second	 256 increments	 1 second




    // Set PWM Freq to the sample at the end of the buffer.
    OCR2A = Serial.read();
    BytesReceived++;


    //--------TIMER 1----------------------------------
    // Set up Timer 1 to send a sample every interrupt.
    // This will interrupt at the sample rate (8000 hz)
    //

    cli();

    // Set CTC mode (Clear Timer on Compare Match) (p.133)
    // Have to set OCR1A *after*, otherwise it gets reset to 0!
    TCCR1B = (TCCR1B & ~_BV(WGM13)) | _BV(WGM12);
    TCCR1A = TCCR1A & ~(_BV(WGM11) | _BV(WGM10));

    // No prescaler (p.134)
    TCCR1B = (TCCR1B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);

    // Set the compare register (OCR1A).
    // OCR1A is a 16-bit register, so we have to do this with
    // interrupts disabled to be safe.
    OCR1A = F_CPU / SAMPLE_RATE;    // 16e6 / 8000 = 2000

    //Timer/Counter Interrupt Mask Register
    // Enable interrupt when TCNT1 == OCR1A (p.136)
    TIMSK1 |= _BV(OCIE1A);


    //Init Sample.  Start from the beginning of audio.
    sample = 0;
    
    //Enable Interrupts
    sei();  
}//End StartPlayback





void stopPlayback()
{
    // Disable playback per-sample interrupt.
    TIMSK1 &= ~_BV(OCIE1A);

    // Disable the per-sample timer completely.
    TCCR1B &= ~_BV(CS10);

    // Disable the PWM timer.
    TCCR2B &= ~_BV(CS10);

    digitalWrite(speakerPin, LOW);
    
    reset();
}//End StopPlayback



    //Use the custom powlong() function because the standard
    //pow() function uses floats and has rounding errors.
    //This powlong() function does only integer powers.
    //Be careful not to use powers that are too large, otherwise
    //this function could take a really long time.
long powlong(long x, long y)
{
  //Base case for recursion
  if (y==0)
  {
    return(1);
  }//End if
  else
  {
    //Do recursive call.
    return(powlong(x,y-1)*x);
  }//End Else
}

void reset()
{
	//PC sends audio length as 10-digit ASCII
    //While audio length hasn't arrived yet
    while (Serial.available()<10)
    {
    //Blink the LED on pin 13.
    digitalWrite(ledPin,!digitalRead(ledPin));
    delay(100);
    }
    
    //Init number of audio samples.
    sounddata_length=0;
    
    //Convert 10 ASCII digits to an unsigned long.
    for (int i=0;i<10;i++)
    {
    //Convert from ASCII to int
    Temp=Serial.read()-48;  
    //Shift the digit the correct location.
    NewTemp = Temp * powlong(10,9-i);  
    //Add the current digit to the total.
    sounddata_length = sounddata_length + NewTemp;
    }//End for

    //Tell the remote PC/device that the Arduino is ready
    //to begin receiving samples.
    Serial.println(128);
    
    while (Serial.available() < 64)
    {
    //Blink the LED on pin 13.
    digitalWrite(ledPin,!digitalRead(ledPin));
    delay(100);
    }
    
    Playing =0;
    BytesReceived=0;
    sample=0;
}//End Reset

void setup()
{
    //Set LED for OUTPUT mode
    pinMode(ledPin, OUTPUT);
    
    //Start Serial port.  If your application can handle a
    //faster baud rate, that would increase your bandwidth
    //115200 only allows for 14,400 Bytes/sec.  Audio will
    //require 8000 bytes / sec to play at the correct speed.
    //This only leaves 44% of the time free for processing
    //bytes.
    Serial.begin(115200);
    
    reset();

}//End Setup

void loop()
{

  //If audio not started yet...
  if (Playing == 0)
  {
    //There's data now, so start playing.
    Playing=1;
    startPlayback();
  }//End if
  
}//End Loop 



#include <avr/interrupt.h>



#define SAMPLE_RATE 8000
#define BUFFER_SIZE 1024

unsigned long sounddata_length=0;
unsigned long sample=0;
unsigned long BytesReceived=0;

unsigned long Temp=0;
unsigned long NewTemp=0;

int ledPin = 13;
int speakerPin = 11;
int Playing = 0;

//Interrupt Service Routine (ISR)
// This is called at 8000 Hz to load the next sample.
ISR(TIMER1_COMPA_vect)
{
    //If not at the end of audio
    if (sample < sounddata_length)  
    {
	  //Set the PWM Freq.

	  OCR2A = Serial.read();
	  BytesReceived++;
	  sample++;

		//if the Serial port buffer has room
    if ((BytesReceived % BUFFER_SIZE) == 0)
    {
	//Tell the remote PC how much space you have.
	Serial.println(BUFFER_SIZE);
    }//End if

    }//End if
    else //We are at the end of audio
    {
	  //Stop playing.
	  stopPlayback();
    }//End Else

}//End Interrupt






void startPlayback()
{
    //Set pin for OUTPUT mode.
    pinMode(speakerPin, OUTPUT);

    //---------------TIMER 2-------------------------------------
    // Set up Timer 2 to do pulse width modulation on the speaker
    // pin.  
    //This plays the music at the frequency of the audio sample.

    // Use internal clock (datasheet p.160)
    //ASSR = Asynchronous Status Register
    ASSR &= ~(_BV(EXCLK) | _BV(AS2));

    // Set fast PWM mode  (p.157)
    //Timer/Counter Control Register A/B for Timer 2
    TCCR2A |= _BV(WGM21) | _BV(WGM20);
    TCCR2B &= ~_BV(WGM22);

    // Do non-inverting PWM on pin OC2A (p.155)
    // On the Arduino this is pin 11.
    TCCR2A = (TCCR2A | _BV(COM2A1)) & ~_BV(COM2A0);
    TCCR2A &= ~(_BV(COM2B1) | _BV(COM2B0));

    // No prescaler (p.158)
    TCCR2B = (TCCR2B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);

    //16000000 cycles	 1 increment    2000000 increments
    //--------	  *  ----		= -------
    //	 1 second	 8 cycles		 1 second

    //Continued...
    //2000000 increments     1 overflow	7812 overflows
    //-------		* ---		= -----
    //	1 second	 256 increments	 1 second




    // Set PWM Freq to the sample at the end of the buffer.
    OCR2A = Serial.read();
    BytesReceived++;


    //--------TIMER 1----------------------------------
    // Set up Timer 1 to send a sample every interrupt.
    // This will interrupt at the sample rate (8000 hz)
    //

    cli();

    // Set CTC mode (Clear Timer on Compare Match) (p.133)
    // Have to set OCR1A *after*, otherwise it gets reset to 0!
    TCCR1B = (TCCR1B & ~_BV(WGM13)) | _BV(WGM12);
    TCCR1A = TCCR1A & ~(_BV(WGM11) | _BV(WGM10));

    // No prescaler (p.134)
    TCCR1B = (TCCR1B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);

    // Set the compare register (OCR1A).
    // OCR1A is a 16-bit register, so we have to do this with
    // interrupts disabled to be safe.
    OCR1A = F_CPU / SAMPLE_RATE;    // 16e6 / 8000 = 2000

    //Timer/Counter Interrupt Mask Register
    // Enable interrupt when TCNT1 == OCR1A (p.136)
    TIMSK1 |= _BV(OCIE1A);


    //Init Sample.  Start from the beginning of audio.
    sample = 0;
    
    //Enable Interrupts
    sei();  
}//End StartPlayback





void stopPlayback()
{
    // Disable playback per-sample interrupt.
    TIMSK1 &= ~_BV(OCIE1A);

    // Disable the per-sample timer completely.
    TCCR1B &= ~_BV(CS10);

    // Disable the PWM timer.
    TCCR2B &= ~_BV(CS10);

    digitalWrite(speakerPin, LOW);
    
    reset();
}//End StopPlayback



    //Use the custom powlong() function because the standard
    //pow() function uses floats and has rounding errors.
    //This powlong() function does only integer powers.
    //Be careful not to use powers that are too large, otherwise
    //this function could take a really long time.
long powlong(long x, long y)
{
  //Base case for recursion
  if (y==0)
  {
    return(1);
  }//End if
  else
  {
    //Do recursive call.
    return(powlong(x,y-1)*x);
  }//End Else
}

void reset()
{
	//PC sends audio length as 10-digit ASCII
    //While audio length hasn't arrived yet
    while (Serial.available()<10)
    {
    //Blink the LED on pin 13.
    digitalWrite(ledPin,!digitalRead(ledPin));
    delay(100);
    }
    
    //Init number of audio samples.
    sounddata_length=0;
    
    //Convert 10 ASCII digits to an unsigned long.
    for (int i=0;i<10;i++)
    {
    //Convert from ASCII to int
    Temp=Serial.read()-48;  
    //Shift the digit the correct location.
    NewTemp = Temp * powlong(10,9-i);  
    //Add the current digit to the total.
    sounddata_length = sounddata_length + NewTemp;
    }//End for

    //Tell the remote PC/device that the Arduino is ready
    //to begin receiving samples.
    Serial.println(128);
    
    while (Serial.available() < 64)
    {
    //Blink the LED on pin 13.
    digitalWrite(ledPin,!digitalRead(ledPin));
    delay(100);
    }
    
    Playing =0;
    BytesReceived=0;
    sample=0;
}//End Reset

void setup()
{
    //Set LED for OUTPUT mode
    pinMode(ledPin, OUTPUT);
    
    //Start Serial port.  If your application can handle a
    //faster baud rate, that would increase your bandwidth
    //115200 only allows for 14,400 Bytes/sec.  Audio will
    //require 8000 bytes / sec to play at the correct speed.
    //This only leaves 44% of the time free for processing
    //bytes.
    Serial.begin(115200);
    
    reset();

}//End Setup

void loop()
{

  //If audio not started yet...
  if (Playing == 0)
  {
    //There's data now, so start playing.
    Playing=1;
    startPlayback();
  }//End if
  
}//End Loop 


);             // read Poti on analog pin 0 to adjust output frequency from 0..1023 Hz

      cbi (TIMSK2,TOIE2);              // disble Timer2 Interrupt
      tword_m=pow(2,32)*dfreq/refclk;  // calulate DDS new tuning word
      sbi (TIMSK2,TOIE2);              // enable Timer2 Interrupt 

      Serial.print(dfreq);
      Serial.print("  ");
      Serial.println(tword_m);
    }

   sbi(PORTD,6); // Test / set PORTD,7 high to observe timing with a scope
   cbi(PORTD,6); // Test /reset PORTD,7 high to observe timing with a scope
  }
 }
//******************************************************************
// timer2 setup
// set prscaler to 1, PWM mode to phase correct PWM,  16000000/510 = 31372.55 Hz clock
void Setup_timer2() {

// Timer2 Clock Prescaler to : 1
  sbi (TCCR2B, CS20);
  cbi (TCCR2B, CS21);
  cbi (TCCR2B, CS22);

  // Timer2 PWM Mode set to Phase Correct PWM
  cbi (TCCR2A, COM2A0);  // clear Compare Match
  sbi (TCCR2A, COM2A1);

  sbi (TCCR2A, WGM20);  // Mode 1  / Phase Correct PWM
  cbi (TCCR2A, WGM21);
  cbi (TCCR2B, WGM22);
}

//******************************************************************
// Timer2 Interrupt Service at 31372,550 KHz = 32uSec
// this is the timebase REFCLOCK for the DDS generator
// FOUT = (M (REFCLK)) / (2 exp 32)
// runtime : 8 microseconds ( inclusive push and pop)
ISR(TIMER2_OVF_vect) {

  sbi(PORTD,7);          // Test / set PORTD,7 high to observe timing with a oscope

  phaccu=phaccu+tword_m; // soft DDS, phase accu with 32 bits
  icnt=phaccu >> 24;     // use upper 8 bits for phase accu as frequency information
                         // read value fron ROM sine table and send to PWM DAC
  OCR2A=pgm_read_byte_near(sine256 + icnt);    

  if(icnt1++ == 125) {  // increment variable c4ms all 4 milliseconds
    c4ms++;
    icnt1=0;
   }   

 cbi(PORTD,7);            // reset PORTD,7
}

