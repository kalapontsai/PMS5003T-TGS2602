#include <Wire.h>  // Arduino IDE 內建
// LCD I2C Library，從這裡可以下載：
// https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads
#include <LiquidCrystal_I2C.h>
const int buttonPin = 2;
const int lcdtimer = 10;
int buttonState = 0;  //開啟LCD螢幕燈的開關 A3
int buttonAlive = 10;  //倒數10次
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3);
// PMS5003T
// http://jaihung.blogspot.tw/search/label/pms5003t
#define PMS_SET 4   
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

// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // 設定 LCD I2C 位址

void setup() {
// put your setup code here, to run once:
  Serial.begin(9600);
  mySerial.begin(9600);
  pinMode(ledPower,OUTPUT);
  lcd.begin(16, 2); // 初始化 LCD，一行 16 的字元，共 2 行，預設開啟背光
    // 閃爍三次
  for(int i=0; i<3; i++) {
    lcd.backlight(); // 開啟背光
    delay(250);
    lcd.noBacklight(); // 關閉背光
    delay(250);
  }
  lcd.backlight();
  // 輸出初始化文字
  lcd.setCursor(0, 0); // 設定游標位置在第一行行首
  lcd.print("Humidity & Temp");
  delay(1000);
  lcd.setCursor(0, 1); // 設定游標位置在第二行行首
  lcd.print("Dust PM2.5");
  delay(500);
  lcd.clear();
}

void loop() {
  int count = 0;
  unsigned char c;
  unsigned char high;
  while (mySerial.available()) {
  c = mySerial.read();
  if((count==0 && c!=0x42) || (count==1 && c!=0x4d)){
  Serial.println("check failed");
  break;
  }
  if(count > 40){
  Serial.println("complete");
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
  Serial.println(" ^C ");
  }
  else if(count == 27){
  Humidity = 256*high + c;
  Serial.print("Humidity = ");
  Serial.print(Humidity/10) ;
  Serial.println(" % ");
  }
  count++;
  }
  while(mySerial.available()) mySerial.read();
  Serial.println();
  u8g.firstPage();  
      do {
        draw();
      } while( u8g.nextPage() );
  delay(4000);
}
