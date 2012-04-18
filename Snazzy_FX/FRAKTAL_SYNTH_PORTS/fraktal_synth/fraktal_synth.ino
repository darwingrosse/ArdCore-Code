//FRACTAL  SYNTH
/*
PORTED TO AVR AND MODIFIED FOR ARDCORE BY DAN SNAZELLE

2008 ERIG BOGER




A0 PARAMETER
A1 PARAMETER
A2 PARAMETER
A3 PARAMETER

CLOCK INPUT-SWITCHES FROM ALGORITHM ONE TO ALGORITHM TWO DEPENDING ON CLOCK STATE
  
  DAC OUTPUT: OUTPUT

   *************Fractal Synthesizer**************************

   

   Version 0.4 

               

   Eric Boger (2008)                      

*/                     
#define _mul(x,y) ((x)*(y))


#define make8(val, offset) ( ( (val)>>((offset)*8) ) & 0xff)

//  variables for interrupt handling of the clock input
volatile int clkState = LOW;

//                     

// global variables  

//             



// "API" Potentiometer 0..7 values:

//  variables used to control the current DIO output states
int digState[2] = {LOW, LOW};  // start with both set low
const int clkIn = 2;           // the digital (clock) input
const int digPin[2] = {3, 4};  // the digital output pins
const int pinOffset = 5;       // the first DAC pin (from 5-12)
byte   LED          = 0x7f;

byte   ALGORITHM    = 0;

byte    ALGORITHMOLD = 0xff;

boolean   PLAY         = true;

boolean REVERSE= false;

byte    POTLOCK      = 0;

byte    CLOCK        = 1;

byte  FS_out = 0x7f;

int FS_cnt=0;   

byte  FS_cnthi = 0;

byte  FS_cntlo  = 0;


void setup () {

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
  
  
  
   // Note: Interrupt 0 is for pin 2 (clkIn)

   }





void DSP_FS_Init (void)

{

   FS_out        = 0x7f;

   FS_cnt        = 0;

}





byte DSP_FS_Alg_1 (boolean reverse)

{    

   // Param1 = hi nibble AND mod HI 

   // Param2 = lo nibble AND mod HI

   // Param3 = hi nibble MUL mod

   // Param4 = lo nibble MUL mod 

   // Param5 = output AND mod

   // Param6 = not used

      

   byte param1;

   byte param2;

   byte param3;

   

   param1 = (analogRead(0)&0xf0>>2) + (analogRead(2)>>6);

   param2 = (analogRead(3)&0xf0>>2) + (analogRead(1)>>6);

   param3 =  analogRead(2)>>2;

   if (param3==0) param3=0x80;

   

   FS_cnthi = make8(FS_cnt,1) & param1;

   FS_cntlo = make8(FS_cnt,0);       

  // FS_out = _mul(FS_cnthi, FS_cntlo);   
    FS_out = FS_cnthi* FS_cntlo;   

   //FS_out = _mul(FS_out,   param2);        
   FS_out=FS_out* param2; 


   if (param3 != 255)

   {

      if (FS_out & param3) FS_out=255;

      else                 FS_out=0;  

   }

   

   // pitch down shift register



   if (reverse) FS_cnt--;

   else         FS_cnt++;



   return FS_out;

}





byte DSP_FS_Alg_2 (boolean reverse)

{

   // Param1 = hi nibble AND mod HI 

   // Param2 = lo nibble AND mod HI

   // Param3 = hi nibble AND mod LO

   // Param4 = lo nibble AND mod LO

   // Param5 = output AND mod

   // Param6 = not used

   

   byte param1;

   byte param2;

   byte param3;



   param1 = (analogRead(0)>>2&0xf0) + (analogRead(2)>>6);

   param2 = (analogRead(1)>>2&0xf0) + (analogRead(1)>>6);

   param3 =  analogRead(2)>>2; 

   if (param3==0) param3=0x80;

   

   FS_cnthi = make8(FS_cnt,1) & param1;

   FS_cntlo = make8(FS_cnt,0) & param2;                                    

   FS_out   = FS_cnthi* FS_cntlo;           

   

   if (param3 != 255)

   {   

      if (FS_out & param3) FS_out=255;

      else                 FS_out=0;    

   }

   

   if (reverse) FS_cnt--;

   else         FS_cnt++;    

  

   return FS_out;

}


void loop () {
  int call;
  int value;
  value =digitalRead(2) ;
  
  if (value==HIGH)
  
    call=DSP_FS_Alg_2(REVERSE);
    
    
    
else  {
    
     
    call=DSP_FS_Alg_1(REVERSE);

}
 
 
 
dacOutput( call);

}





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







