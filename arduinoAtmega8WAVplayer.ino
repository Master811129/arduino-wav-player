// https://www.telegram.org/ArduinoKaraneJavan
//written by Ali Najafian
//sucsess
#include "PetitFS.h"
#define TRIGGER1 230
#define TRIGGER2 20
#ifndef __AVR__
#error "this sketch only compatible with avr MCUs."
#endif

FATFS fs;     /* File system object */
volatile uint16_t i = 0;
volatile bool i_need_data1 = true;
volatile bool i_need_data2 = true;
volatile uint8_t buf1[256];
volatile uint8_t buf2[256];
volatile bool one_or_two=1;
ISR(TIMER2_COMP_vect)
{
  if(one_or_two)OCR1AL = buf1[i];
  else OCR1AL = buf2[i];
  i++;
  if (i >= sizeof(buf1))
  {
    i = 0;
    one_or_two = !one_or_two;
  }
  if (i == TRIGGER1&&one_or_two)i_need_data2 = true;
  if (i == TRIGGER2&&!one_or_two)i_need_data1 = true;
}

void errorHalt(char* msg) {
  Serial.print(F("Error: "));
  Serial.println(msg);
  while (1);
}

void beginSD()
{
  if (pf_mount(&fs)) errorHalt("pf_mount");
  else Serial.println(F("sucsess mount"));
  _delay_ms(1000);
  if (pf_open("TITLE2.WAV")) errorHalt("pf_open");//WARNING: very important it MUST be an upper case string
  else Serial.println(F("sucsess open"));
}

void setup()
{
  Serial.begin(9600);//initialize Serial for handle errors
  //timer 2 initialize
  ASSR = 0 << AS2;   //disable the async timer 2 clock source
  TCCR2 = 0b00001010;//set timer 2 in CTC mode
  TCNT2 = 0x00;      //it is time 2 register. it automaticly counts up until it reach OCR2 value and it goes back to zero
  OCR2 = 0xF9;       //this is our time 2 value you change it if you want to use lower quality wav file
  pinMode(9, OUTPUT);//very important if you want to use PWM
  //begin time 1 in fast PWM mode
  //in fact we use this timer as an software DAC.
  TCCR1A = (1 << COM1A1) | (0 << COM1A0) | (0 << COM1B1) | (0 << COM1B0) | (0 << WGM11) | (1 << WGM10);
  TCCR1B = (0 << ICNC1) | (0 << ICES1) | (0 << WGM13) | (1 << WGM12) | (0 << CS12) | (0 << CS11) | (1 << CS10);
  TCNT1H = 0x00;
  TCNT1L = 0x00;
  ICR1H = 0x00;
  ICR1L = 0x00;
  OCR1AH = 0x00;
  OCR1AL = 0xFF;
  OCR1BH = 0x00;
  OCR1BL = 0x00;
  TIMSK = 0x00;
  TIMSK = (1 << OCIE2);
  OCR1AL = 0xff;
  //ok it has begun
  //PWM testing section
  _delay_ms(1000);
  OCR1AL = 0x7f;
  _delay_ms(1000);
  OCR1AL = 0x00;
  _delay_ms(1000);
  pinMode(SS, OUTPUT);//very important if you want to use SPI hardware
  Serial.println(F("hello world"));//say hello to every one
  beginSD();// ...
  Serial.println(F("\nDone!"));
}
void loop()
{
  if(Serial.available())//Serial handling
  {
    switch(Serial.read())
    {
      case 'p':TCCR2 = 0;Serial.println(F("Paused!"));break;//disable timer 2
      case 'c':TCCR2 = 0b00001010;Serial.println(F("Countinue"));break;//re-enable timer 2
      default :Serial.println(F("Unknown command"));// :(
    }
  }
  unsigned int nr;// store available bytes
  if (i_need_data1)
  {
    i_need_data1 = false;
    pf_read((void*)(buf1), sizeof(buf1), &nr);//read SD and store it to buffer number 1
  }
  if (i_need_data2)
  {
    i_need_data2=false;
    pf_read((void*)(buf2),sizeof(buf2),&nr);//read SD data an store it to buffer number 2 
  }
  if (nr == 0)// if we don't have any data
  {
    TCCR2 = 0;//dasable timer2 and timer 1
    TCCR1A = 0;
    TCCR1B = 0;
  }
}
