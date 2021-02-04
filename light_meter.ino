#include <AE_TSL2572.h>
#include <Wire.h>
#include <math.h>
#include <U8g2lib.h>//Change uxu8_d_ssd1306_64x48.c's I2Caddress(autoset) to 0x3D

//OLED setup
U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R3, U8X8_PIN_NONE ,5 ,4);//Change uxu8_d_ssd1306_64x48.c's I2Caddress(autoset) to 0x3D
byte address = 0x3D;

//Light sensor vars
//ゲイン　0～ 5 (x0.167 , x1.0 , x1.33 , x8 , x16 , x120)
int gain_step = 3;

//積分時間のカウンタ(0xFFから減るごとに+2.73ms)
//0xFF：  1サイクル(約 2.73ms)
//0xDB： 37サイクル(約  101ms)
//0xC0： 64サイクル(約  175ms)
//0x00：256サイクル(約  699ms)
byte atime_cnt = 0xF5;
AE_TSL2572 TSL2572;

//Vars
int lux;
int btnState = 0;
int arrowPos = 0;
const int cmup = 10;
const int cmmd = 8;
const int cmdn = 9;

//Bitmap for meter
#define meter_width 7
#define meter_height 64
static const unsigned char meter_bits[] PROGMEM = {
 0x80,0x80,0x80,0x80,0x80,0x80,0xfe,0x80,0x80,0x80,0xf8,0x80,
 0x80,0x80,0xf8,0x80,0x80,0x80,0xfe,0x80,0x80,0x80,0xf8,0x80,
 0x80,0x80,0xf8,0x80,0x80,0x80,0xfc,0xff,0xfc,0x80,0x80,0x80,
 0xf8,0x80,0x80,0x80,0xf8,0x80,0x80,0x80,0xfe,0x80,0x80,0x80,
 0xf8,0x80,0x80,0x80,0xf8,0x80,0x80,0x80,0xfe,0x80,0x80,0x80,
 0x80,0x80,0x80,0x80 };

//Bitmap for arrow
#define arrow_width  3
#define arrow_height 5
static const unsigned char arrow_bits[] PROGMEM = {
 0xf9,0xfb,0xff,0xfb,0xf9};


void setup() {
  Serial.begin(115200);//init serial 
  Serial.println("Serial started.");

  Wire.begin();//begin i2c
  u8g2.begin();

  TSL2572.SetGain(gain_step);
  TSL2572.SetIntegralTime(atime_cnt);
  Serial.println("TSL Setup Done.");
  TSL2572.Reset();
  delay(100);
    
  pinMode(cmup,INPUT);
  pinMode(cmmd,INPUT);
  pinMode(cmdn,INPUT);
  Serial.println("Setup done!");
}

void loop() {
  readBtnStates(cmup,cmmd,cmdn);
  setGainStep();
  TSL2572.SetGain(gain_step);
  TSL2572.Reset();

  lux = TSL2572.GetLux16();
  

  u8g2.clearBuffer();
  meter(btnState);
  u8g2.setFont(u8g2_font_profont11_tr);

  u8g2.setCursor(0, 15);
  u8g2.print(lux);u8g2.print("Lux");

  u8g2.setCursor(0, 25);
  u8g2.print(luxToEv(lux,100,12.5));u8g2.print("EV");

  u8g2.setCursor(0, 35);
  u8g2.print("gain:");
  u8g2.print(gain_step);
  u8g2.sendBuffer();
  delay(100);
}

void meter(int pos){
  u8g2.drawXBM(41,0,meter_width,meter_height,meter_bits);
  arrowPos = map(pos, 0, 3, 0, 12);
  u8g2.drawXBM(37,arrowPos,arrow_width,arrow_height,arrow_bits);
}

void readBtnStates(int pin1, int pin2, int pin3){
  if(digitalRead(pin1) == HIGH){
    btnState = 1;
  }
  else if(digitalRead(pin2) == HIGH){
    btnState = 2;
  }
  else if(digitalRead(pin3) == HIGH){
    btnState = 3;
  }
  else{
    btnState = 0;
  }
}



float luxToEv(int lux, int iso, float calConst){
  float ev;
  ev = log2( ((lux * 0.16) * iso) / calConst ) - 1.5;
  return ev;
}
  
/*
int callog2(float v){
  int c;         // 32-bit int c gets the result;

  c = *(const int *) &v;  // OR, for portability:  memcpy(&c, &v, sizeof c);
  c = (c >> 23) - 127;
  return c;
}*/