#include <Wire.h>  // Arduino IDE 內建
// LCD I2C Library，從這裡可以下載：
// https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3);//rx,tx
/* PlanTower PMS5003T
code ref from http://jaihung.blogspot.tw/search/label/pms5003t
pin list:
1:VCC (+5v)
2:GND
3:High (+3.3v) or Open is normal mode(Default), Low:Sleep mode
4:RX (+3.3v)
5:TX (+3.3v)
6:RESET (+3.3v), Low:reset enable
7:NC (eng purpose)
8:NC (eng purpose)
need to shift 5.5 to 3.3V between Arduino and PMS devices (use 1Kohm & 2Kohm R)
*/

//const int SET_Pin = 4;
//const int ECO_Pin = 5; //Eco mode manual switch, Low = disable (Default)
long pmcf10=0;
long pmcf25=0;
long pmcf100=0;
long pmat10=0;
long pmat25=0;
long pmat100=0;
long pm03PNO =0;
long pm05PNO =0;
long pm10PNO =0;
long pm25PNO =0;
long Temperature =0;
long Humidity =0;
char buf[50];
/* FIGARO TGS2602
pin list:
1:Heater (+5v)
2:Sense electrode (connect Analog pin and GND via 0.45k ohm min. )
3:Sense electrode (+5V)
4:Heater (GND or switch with Pin1)
*/
int gasSensor = 2;  // Analog pin A2
int gasval = 0;

const int debug_mode = 1;

// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // 設定 LCD I2C 位址

void lcdmsg(int Yaxis,String msg){
  lcd.setCursor(Yaxis,1);
  lcd.print("                ");
  lcd.setCursor(Yaxis,1);
  lcd.print(msg);
}

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  pinMode(ledPower,OUTPUT);
  digitalWrite(ledPower,0);
  if (ECO_Pin) pinMode(Eco_Pin,INPUT);
  if (SET_Pin) { pinMode(SET_Pin,OUTPUT);digitalWrite(SET_Pin,High);}
  lcd.begin(16, 2); // LCD initial，16 words x 2 column，backlight ON
  for(int i=0; i<3; i++) {  // shall be wait for 30 sec since internal fan power on
    lcdmsg(0,"Humidity & Temp"); // to get the stable value
    lcdmsg(1,"Dust PM2.5")
    delay(1000);
    lcd.clear();
    delay(1000);
  }

void loop() {
  int count = 0;
  unsigned char c;
  unsigned char high;
  unsigned char c_all;
  while (mySerial.available()) {
    digitalWrite(ledPower,1);
    c = mySerial.read();
    c_all += c;
    if(count > 40){
      gasval = analogRead(gasSensor);
      Serial.println("Gas");
      Serial.print("VOCs = ");
      Serial.println(gasval);
      Serial.println("--- Raw strings ---");
      Serial.println(c_all);
      Serial.println("--- Start ---");
      break;
      }
    if((count==0 && c!=0x42) || (count==1 && c!=0x4d)){
      Serial.println("check failed");
      break;
      }
    else if(count == 4 || count == 6 || count == 8 || count == 10 || count == 12 || count == 14|| count == 16|| count == 18|| count == 20|| count == 22|| count == 24|| count == 26|| count == 28 ) high = c;
    else if(count == 5){
      pmcf10 = 256*high + c;
      Serial.print("CF=1, PM1.0=");
      Serial.print(pmcf10);
      Serial.println(" ug/m3");
      }
    else if(count == 7){
      pmcf25 = 256*high + c;
      Serial.print("CF=1, PM2.5=");
      Serial.print(pmcf25);
      Serial.println(" ug/m3");
      }
    else if(count == 9){
      pmcf100 = 256*high + c;
      Serial.print("CF=1, PM10=");
      Serial.print(pmcf100);
      Serial.println(" ug/m3");
      }
    else if(count == 11){
      pmat10 = 256*high + c;
      Serial.print("atmosphere, PM1.0=");
      Serial.print(pmat10);
      Serial.println(" ug/m3");
      }
    else if(count == 13){
      pmat25 = 256*high + c;
      Serial.print("atmosphere, PM2.5=");
      Serial.print(pmat25);
      Serial.println(" ug/m3");
      }
    else if(count == 15){
      pmat100 = 256*high + c;
      Serial.print("atmosphere, PM10=");
      Serial.print(pmat100);
      Serial.println(" ug/m3");
      }
    else if(count == 17){
      pm03PNO= 256*high + c;
      Serial.print("above0.3um, no=");
      Serial.print(pm03PNO);
      Serial.println(" number");
      }
    else if(count == 19){
      pm05PNO = 256*high + c;
      Serial.print("above0.5um, no=");
      Serial.print(pm05PNO);
      Serial.println(" number");
      }
    else if(count == 21){
      pm10PNO = 256*high + c;
      Serial.print("above 1.0um, no=");
      Serial.print(pm10PNO);
      Serial.println(" number");
      }
    else if(count == 23){
      pm25PNO = 256*high + c;
      Serial.print("above 2.5um, no=");
      Serial.print(pm25PNO);
      Serial.println(" number");
      }
    else if(count == 25){
      Temperature = 256*high + c;
      Serial.print("Temperature = ");
      Serial.print(Temperature/10);
      Serial.println(" ^C");
      }
    else if(count == 27){
      Humidity = 256*high + c;
      Serial.print("Humidity = ");
      Serial.print(Humidity/10) ;
      Serial.println(" %");
      }
    count++;
    }
  lcd.backlight();
  lcdmsg(0,"Temp     :" & Temperature/10);
  lcdmsg(1,"Humidity :" & Humidity/10);
  delay(1000);
  lcdmsg(0,"PM 2.5 :" & pmat25);
  lcdmsg(1,"PM 10  :" & pmat100);
  delay(1000);
  digitalWrite(ledPower,0);
  delay(250);

  if (Eco_Pin && pmat25 < 40) {  // 若節能模式有設jump及指數在正常範圍再檢查
    int eco_mode = digitalRead(Eco_Pin);
    if (eco_mode) {
      lcd.noBacklight(); // 關閉背光
      digitalWrite(SET_Pin,Low);
      delay(60000);
      digitalWrite(SET_Pin,High);
      delay(60000);
      }
    }
}
