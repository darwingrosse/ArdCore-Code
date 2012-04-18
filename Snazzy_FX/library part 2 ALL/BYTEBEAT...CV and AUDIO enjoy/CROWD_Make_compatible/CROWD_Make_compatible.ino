include <TVout.h>
#include <fontALL.h>
#include "wiring_private.h"
#undef round

TVout TV;

long t = 0;
char i = 0;

const int buffer_log_size = 7;
static unsigned char buffer[1 << buffer_log_size];
typedef unsigned char bufidx;
static bufidx front = 0;
static volatile bufidx rear = 0;

//static inline bufidx next(bufidx cur)
static inline unsigned char next(unsigned char cur)
{
  return (cur + 1) & ((1 << buffer_log_size) - 1);
}

static inline char put(char v)
{
  if (rear == next(front)) return 0;
  buffer[unsigned(front)] = v;
  front = next(front);
  return 1;
}

static inline char get(char *where)
{
  register bufidx r = rear;
  if (r == front) return 0;
  *where = buffer[unsigned(r)];
  rear = next(r);
  return 1;
}

// a place to stick a number to debug with
static unsigned char samples_spat_out_by_asm;

void our_hbi_hook() {
  if (++i != 2) return;
  i = 0;
  char c;
  if (get(&c)) {
    OCR2A = c; // set PWM duty cycle
  } else {
    samples_spat_out_by_asm++;
  }
}

// This stays interesting for ten minutes or so at least,
// and repeats about every twenty:
static inline char crowd()
{
  // unoptimized formula:
  // ((t<<1)^((t<<1)+(t>>7)&t>>12))|t>>(4-(1^7&(t>>19)))|t>>7
  unsigned ut = unsigned(t);
  char t1 = char(ut) << 1;
  unsigned t7 = ut >> 7;
  long t12 = t >> 12;
  return (t1 ^ (t1 + t7 & t12)) | ut >> (4 - (1 ^ 7 & char(t12 >> 7))) | t7;
}

static inline char triangle_bells()
{
  char f = (unsigned char)((unsigned char)((unsigned char)(t >> 11) % (unsigned char)63 ^ (0x15 + t >> 12)) %10*2)%13*4;
  // unoptimized formula:
  // return (f = ((t >> 11) % 63 ^ (0x15 + t >> 12)) %10*2%13*4, ((((t * f & 256) == 0) - 1 ^ t * f) & 255) >> ((t >> 7 + (t >> 13 & 1)) & 7));
  unsigned short ust = t;
  // Some notes on frequency.
  // ust >> 7 & 7 uses the last 10 bits of ust. (Well, the top three of them, anyway.)
  // So it should repeat every 1024 samples.
  // ust >> 8 & 7 should repeat every 2048 samples.
  // ust >> 13 & 1 should, by the same logic, repeat every 16384 samples.
  // So the beat structure should be eight beats of 1024 samples followed by four beats of 2048 samples.
  // Audacity tells me that one of these 16384-sample beats runs from 1.000s in my recording to 3.341s.
  // So it's taking 2.341 seconds instead of the 2.048 seconds it should take, which is really fairly large.
  // I mean, that's 14%, about 2.3 semitones. I'm only getting 6999 samples per second.
  // I'm not sure what to do about that. Maybe skip calculating 14% of the samples? Play 28% of the samples
  // for only a single scan line instead of two scan lines?
  // Audacity also shows a disturbing amount of high-frequency energy, including a peak at 12726Hz (0.55 samples)
  // at -37dB, louder than any of the easily audible sounds!
  // I see what looks like a regular oscillation with a period of about 3.3 of my 44.1kHz samples in Audacity,
  // which would be about 13kHz; I had chalked it up to PWM but I think it's too low-frequency.
  // And I may be imagining things but I think I can hear the 13kHz whine on the TV as well.
  return (((((ust * f & 256) == 0) - 1 ^ ust * f) & 255) >> ((ust >> 7 + (ust >> 13 & 1)) & 7));
}

void generate_samples()
{
  for (;;) {
    char ft;
    char sample = //t*(((t>>12)|(t>>8))&(63&(t>>4)));
      // This works okay and has an interesting rhythm, but I
      // wonder if it may be better off without the second line
      // and corresponding mixing:
      // (((((t>>6|t<<1)+(t>>5|t<<3|t>>3)|t>>2|t<<1) & t>>12) ^ t>>16) & 255)
      // >> 1 | ((t<<1&t>>9|t+1023>>9) & 255) >> 1
      // ;
      // This innocent-looking formula used to be really slow until I added char().
      // Should repeat after about 4 hours. Too bad it sounds terrible.
      // char(char(t)<<(7&(t>>12)))+(t<<1)&t>>9|t+(t>>10)>>9;
      // too slow, except when t was a 16-bit int:
      // t^t%255;
      // This is a microcontroller-friendly way to do t^t%255:
      // t ^ (char(t) + char(int(t) >> 8));
      // and a little more interesting:
      // t ^ (char(t) + char(int(t) >> 8)) | short(t) >> 6 | t >> 9;
      // and more interesting still:
      // crowd();
      // skurk-raer: one of these two branches is too slow with longs:
      // ((t&4096)?((t*(t^t%255)|(t>>4))>>1):(t>>3)|((t&8192)?t<<2:t));
      // optimized for microcontroller:
      // ((t&4096)?((t*(t^(char(t)+char(int(t)>>8)))|(t>>4))>>1):(t>>3)|((t&8192)?t<<2:t));
      // this one ("laser boots") is still too slow:
      // 255-((1L<<28)/(1+(t^0x5800)%0x8000) ^ t | t >> 4 | -t >> 10);
      // This bowdlerized version supposedly worked with the Arduino compiler on the NAML
      // machine, but on inexorable it's too slow:
      // 255-((1<<28)/(1+(t^0x5800)%0x8000) ^ t | t >> 4 | -t >> 10);
      // Explore the space of all possible 8-beat rhythms in less than an hour.
      // It had some minor vsync problems.
      // (t<<1 ^ (t + (t >> 8))) | t >> 2 | (char(t>>15^t>>16^0x75)>>(7&t>>10)&1?0:-1);
      // The same rhythm generator, with Ryg's Chaos Theory melody instead:
      // t*2*(char((t>>10)^(t>>10)-2)%11) | t >> 2 | (char(t>>15^t>>16^0x75)>>(7&t>>10)&1?0:-1);
      // a sort of salsa beat; this one has some kind of incompatibility with JS:
      // (t*t>>(4-((t>>14)&7)))*t|(t&t-(2047&~(t>>7)))>>5|t>>3;
      // A triangle wave!
      // 8*t & 0x100 ? -8*t-1 : 8*t;
      // A simple square wave!
      // (t & 32) << 2;
      // t & (t >> 8);
      // t*(((t>>12)|(t>>8))&(63&(t>>4)));
      // (t*5&t>>7)|(t*3&t>>10);
      // Nice and rhythmic and interesting:
      // (t>>6|t<<1)+(t>>5|t<<3|t>>3)|t>>2|t<<1;
      // triangle_bells();
      // Simple reflected binary Gray code:
      // t ^ t >> 1;
      // pseudo-sinewave:
      // ((t & 15) * (-t & 15) ^ !(t & 16) - 1) + 128;
      // pseudo-sinewave bells:
      (ft = t * (1 + (t >> 13 & 3 ^ t >> 18 & 3 ^ 1) << (t >> 15 & 1)), (((ft & 15) * (-ft & 15) ^ !(ft & 16) - 1) >> (t >> (7 + (t >> 13 & 3 ^ 1)) & 7)) + 128);
    if (!put(sample)) break;
    t++;
  }
  //if (BUFFER_SIZE < 128) t += 128 - BUFFER_SIZE;
}

const int height = 86;
const int width = 120;

void setup()
{
  // audio setup
  pinMode(11, OUTPUT);
  TV.set_hbi_hook(&our_hbi_hook);

  // connect pwm to pin on timer 2
  sbi(TCCR2A, COM2A1);
  TCCR2B = TCCR2B & 0xf8 | 0x01; // no prescaling on clock select

  /////////////////////////
  TV.begin(NTSC, width, height);
  TV.select_font(font6x8);
}

void loop()
{
  //TV.clear_screen();

  //TV.print(int((unsigned char)buffer[0]), 16);
  //TV.print(int(samples_spat_out_by_asm));
  //TV.print(' ');

  for (unsigned char i = 10; i < height; i++) {
    generate_samples();
    unsigned char sample = buffer[i];
    TV.fill_line(i, 0, width, 0);
    TV.screen[i * width/8] = sample;
    TV.fill_line(i,
width/2 - sample*5/32,
width/2 + sample*5/32,
1);
  }
}


