#include <U8x8lib.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "TinyIRReceiver.cpp.h"

//IR receiver stuff
#define IR_INPUT_PIN    2
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   //initialise display in full buffer mode (uses buttloads of memory but we aren't using it for much else);

boolean channelChangeFlag = 0;

const uint8_t sw1 = 3;
const uint8_t sw2 = 4;
const uint8_t cnt1 = 5;
const uint8_t cnt2 = 6;
const uint8_t zcen = 9;

const int CS_pin = 10;
const int SDI_pin = 12;   //oops, wrong pin, looks like we're bit banging
const int SCLK_pin = 13;

uint8_t gainVal = 0;        //8 bit value for PGA2311
uint8_t channel = 3;        //default channel
uint8_t maxGain = 30;       //number of gain steps
uint8_t gain = 0;           //human readable
uint8_t gainChangeFlag = 0; //set by buttons or IR, 1 = gain--, 2 = gain++


void setup() 
{
  pinMode(sw1,INPUT_PULLUP);
  pinMode(sw2,INPUT_PULLUP);
  pinMode(cnt1,OUTPUT);
  pinMode(cnt2,OUTPUT);

  //Serial.begin(115200);

  PGA2311_init();                       //Initialize the ports for the volume control
  initPCIInterruptForTinyReceiver();    //initialise IR
  u8g2.begin(); 

  selectChannel(channel);
}

void loop() 
{
  boolean sw1Val = digitalRead(sw1);
  boolean sw2Val = digitalRead(sw2);

  //read front panel switches
  if((sw1Val == LOW) || (sw2Val == LOW))
  {
    if(sw1Val == LOW)                         //left button volume down
    {
      gainChangeFlag = 1;
    }
    if(sw2Val == LOW)                         //right button volume up
    {
      gainChangeFlag = 2;
    }
    if((sw2Val == LOW) && (sw1Val == LOW))    //both buttons change channel
    {
      channelChangeFlag = 1;
    }
  }
  
  if(gainChangeFlag == 1 && gain > 0)
  {
    gainChangeFlag = 0;
    gain--;
    gainVal = map(gain, 0, maxGain, 0, 256);  //map human readable steps to 8 bit volume level
    PGA2311_write(gainVal, gainVal);          //send volumes
    delay(100);                               //we have to wait a little before the next volume update or the chip glitches
  }

  if(gainChangeFlag == 2 && gain < maxGain)
  {
    gainChangeFlag = 0;
    gain++;
    gainVal = map(gain, 0, maxGain, 0, 256);
    PGA2311_write(gainVal, gainVal);
    delay(100);
  }
  
  if(channelChangeFlag == 1)
  {
    channel++;
    if(channel > 3)
    {
      channel = 1;
    }
    selectChannel(channel);
    channelChangeFlag = 0;
  }
  updateScreen();
}

void PGA2311_init() 
{
  pinMode(CS_pin, OUTPUT);        //Make chip select line an output
  digitalWrite(CS_pin, HIGH);     //CS = off
  pinMode(SDI_pin, OUTPUT);       //Make SDI line an output
  digitalWrite(SDI_pin, LOW);     //SDI =  = 0
  pinMode(SCLK_pin, OUTPUT);      //Make SCLK line an output
  digitalWrite(SCLK_pin, LOW);    //Clock = 0
  pinMode(zcen, OUTPUT);        
  digitalWrite(zcen, HIGH);     
}

void PGA2311_write(byte vol_left, byte vol_right) 
{
  digitalWrite(CS_pin, LOW);     //CS = on
  PGA2311_byteout(vol_right);
  PGA2311_byteout(vol_left);
  digitalWrite(CS_pin, HIGH);    //CS = off
}

void PGA2311_byteout(byte vol) 
{
  byte mask = 0x80;
  for (int i = 0; i < 8; i++) {
    if (vol & mask)  digitalWrite(SDI_pin, HIGH);
    else digitalWrite(SDI_pin, LOW);
    digitalWrite(SCLK_pin, HIGH);
    mask >>= 1;                      //delay and prepare for next bit    
    digitalWrite(SCLK_pin, LOW);
  }
}

void selectChannel(byte channel)
{
  switch(channel)
  {
    case 1:
      digitalWrite(cnt1,LOW);
      digitalWrite(cnt2,LOW);
      break;
    case 2:
      digitalWrite(cnt1,HIGH);
      digitalWrite(cnt2,LOW);
      break;
    case 3:
      digitalWrite(cnt1,LOW);
      digitalWrite(cnt2,HIGH);
      break;
  }
}

void updateScreen()
{   
    u8g2.clearBuffer();                             //Clear the buffer
    u8g2.setFont(u8g2_font_VCR_OSD_tu);             //sexy VCR font
    u8g2.setCursor(0, 16);
    u8g2.print("CHANNEL: "); u8g2.print(channel);
    u8g2.setFont(u8g2_font_inb24_mf);               //big ol' chonky font
    u8g2.setCursor(36, 64);
    u8g2.print(u8x8_u16toa((gain), 2));             //right justify the volume so <10 shows as 01, 02 etc
    u8g2.sendBuffer();                              //send data to device
}

//uses the first NEC codes frome the Chromecast remote that were recognised by the IR receiver
//I don't bother with the repeat codes because the first code is always received fine. 
void handleReceivedTinyIRData(uint16_t aAddress, uint8_t aCommand, bool isRepeat) 
{
    if(aAddress == 4)
    {
      if(aCommand == 2 && isRepeat == 0)
      {
        gainChangeFlag = 2; //voldown
      }
      if(aCommand == 3 && isRepeat == 0)
      {
        gainChangeFlag = 1; //volup
      }
      if(aCommand == 11 && isRepeat == 0)
      {
        channelChangeFlag = 1;
      }
      else
      return;
    }
}
