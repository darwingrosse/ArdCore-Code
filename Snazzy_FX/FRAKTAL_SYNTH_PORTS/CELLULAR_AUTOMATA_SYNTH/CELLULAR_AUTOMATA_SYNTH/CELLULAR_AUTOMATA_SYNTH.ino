/*CEULLULAR AUTOMATA SYNTH


PORTED TO AVR AND MODIFIED FOR ARDCORE BY DAN SNAZELLE

2008 ERIG BOGER




A0 PARAMETER=clock speed
A1 PARAMETER=number of iterations
A2 PARAMETER=rule
A3 PARAMETER=cell

DAC=output


/*

   One dimensional Cellular Automata Synthesizer

                  

   Version 0.4  

               

   Eric Boger (2008)                      

*/           



//                                                    

// global variables  

// 



// "API" Potentiometer 0..7 values

// byte API_POT[8];



// Param1 = 

// Param2 = 

// Param3 = 

// Param4 = 

// Param5 = 

// Param6 = 




#define BIT_TEST(x,n) (x & (0x01<<n))

#define CA_MAXCELLS 34

#define _mul(x,y) ((x)*(y))


#define make8(val, offset) ( ( (val)>>((offset)*8) ) & 0xff)



//                     

// global variables  

//             



// "API" Potentiometer 0..7 values:

// byte API_POT[8]={analogRead(0),analogRead(1), analogRead(2),analogRead(3), analogRead(4), analogRead(0), analogRead(1), analogRead(2)};
const int pinOffset = 5;       // the first DAC pin (from 5-12)
int digState[2] = {LOW, LOW};  // start with both set low
const int digPin[2] = {3, 4};  // the digital output pins
//byte   LED          = 0x7f;

//byte   ALGORITHM    = 0;

//byte    ALGORITHMOLD = 0xff;

//boolean   PLAY         = true;

boolean REVERSE= false;

//byte    POTLOCK      = 0;

//byte    CLOCK        = 1;


byte  CA_out       = 0;

byte  CA_tonecnt   = 1;

byte  CA_tonerate  = 0;       

byte  CA_CELL[CA_MAXCELLS+1][2];

         

   void setup () {
     
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
   }




     void loop () {
     DSP_CA_Do ();  
    DSP_CA_Init();
    //
       
       
  byte goodOut=DSP_CA_Alg_1();
 
 


dacOutput( goodOut);
} 

void DSP_CA_Init(void)                          

{                   

   byte i;

   

   for (i=0; i<CA_MAXCELLS; i++)

   {                

      CA_CELL[i][0] = 0;

      CA_CELL[i][1] = 0;

      //restart_wdt();
      

   }

   CA_CELL[17][0] = 1;

}                                  

   

                                            

void DSP_CA_Do (void)

{                     

   static int caclock=1;

   static boolean  swapnewold = true;



   byte rule;

   byte iterations;

   byte cell_hi, cell_lo;



   byte c;

   byte i;

   byte state;

   byte old, newA;

      

   caclock--;

   if (caclock!=0) return; 

   

   caclock=(~analogRead(0)>>2);

   caclock=caclock<<8;

   caclock=caclock+0xff;

  

   // do cellular automata   

   

  // rule       = analogRead(2)>>2;             // rule       0..255
 rule       = analogRead(2);             // rule       0..255

   //cell_hi    = (18 + analogRead(3)>>6);   // cell width 1..32
 cell_hi    = (18 + analogRead(3)>>2);   // cell width 1..32
   //cell_lo    = 16 - (analogRead(3)>>6);   // cell width 1..32
 cell_lo    = 16 - (analogRead(3)>>2);   // cell width 1..32
  // iterations = analogRead(1)>>6;          // iterations 1..16
  iterations = analogRead(1)>>4;          // iterations 1..16
   iterations++;

   

   for(i=0;i<iterations;i++)

   {          

      if (swapnewold) { old = 1; newA = 0; swapnewold = false; }

      else            { old = 0; newA = 1; swapnewold = true;  }

      

      for(c=0;c<34;c++)

      {

         if ((c > cell_lo) && (c < cell_hi))

         {           

            state = 0;

            if (CA_CELL[c-1][old] != 0) state += 1;

            if (CA_CELL[c]  [old] != 0) state += 2;

            if (CA_CELL[c+1][old] != 0) state += 4;

      

           // if (state==11110000)
//if (state==11110000&rule==01011100)
//if (state<=rule)
 if (BIT_TEST(rule,state))
            {

               state = CA_CELL[c][old];

               state++;

               state = state %16;

               state++;

               CA_tonerate = _mul(CA_tonerate,state);

               CA_CELL[c][newA] = state;

            }

            else

            {

               CA_CELL[c][newA] = 0;

            }   

         }

         else

         {

            CA_CELL[c][newA] = 0;

         }

      }

      

      CA_tonerate++;

   

   }

 

}



 

byte DSP_CA_Alg_1 (void)

{           

   // generate tone

   CA_tonecnt--;

   if (CA_tonecnt == 0)

   {                                              

      CA_tonecnt = CA_tonerate;               

      CA_out = ~CA_out;

   }



   return CA_out;

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






    



                        

                    


