#include <Wire.h>  // Arduino IDE 內建
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3);//rx,tx, receive PMS data and send ThinkSpeak data to wf8266r
#include <LiquidCrystal_I2C.h>
int gasSensor = 2;  // Analog pin A2
int gasval = 0;
int cycle = 0; // to switch LCD screen to show PM2.5 or VOCs

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // declare LCD I2C address

unsigned long now_time; // control upload data period to ThinkSpeak
unsigned long o_time;

String zero(int val){
  String temp = String(val);
  if (temp.length() > 0) {
    while (temp.length() < 3){
      temp = "0" + temp;
    }
  return temp;
  }
}

void espSend (int t,int h,int p25,int p100,int v){
  String check_code = "TH";
  String send_data = check_code + zero(t/10) + zero(h/10) + zero(p25) + zero(p100) + zero(v);
  if (send_data.length() != 17) {  // std data format : TH+020+080+100+100+100
    Serial.print("send data to ESP fail:");
    Serial.print(send_data);
    Serial.print("-- Length = ");
    Serial.println(send_data.length());
  } else {
    mySerial.print(send_data);  // send data to ESP8266 or wf8266r
    Serial.print("send data to ESP :");
    Serial.println(now_time - o_time);
    Serial.println(send_data);
    lcd.setCursor(15,0);
    lcd.print("X");
  }
}

char CopeSerialData(unsigned char ucData){
  static unsigned char ucRxBuffer[250];
  static unsigned char ucRxCnt = 0;
  long  pmcf10=0,pmcf25=0,pmcf100=0;
  long  pmat10=0,pmat25=0,pmat100=0;
  long  pmcount03=0,pmcount05=0,pmcount10=0;
  long  pmcount25=0,pmcount50=0,pmcount100=0;
  long  HCHO=0, temp=0, humi=0;
  ucRxBuffer[ucRxCnt++]=ucData;
  if(ucRxBuffer[0]!=0x42&&ucRxBuffer[1]!=0x4D){
    ucRxCnt=0;
    return ucRxCnt;
  }
  if (ucRxCnt<30){  // G5T
    return ucRxCnt;
  }
  else{
    pmcf10=(float)ucRxBuffer[4]*256+(float)ucRxBuffer[5];       Serial.print("PM1.0_CF1:"); Serial.print(pmcf10);    Serial.print("   ");
    pmcf25=(float)ucRxBuffer[6]*256+(float)ucRxBuffer[7];       Serial.print("PM2.5_CF1:"); Serial.print(pmcf25);    Serial.print("   ");
    pmcf100=(float)ucRxBuffer[8]*256+(float)ucRxBuffer[9];      Serial.print("PM10_CF1:");  Serial.println(pmcf100);
    pmat10=(float)ucRxBuffer[10]*256+(float)ucRxBuffer[11];     Serial.print("PM1.0_AT:");  Serial.print(pmat10);    Serial.print("   ");
    pmat25=(float)ucRxBuffer[12]*256+(float)ucRxBuffer[13];     Serial.print("PM2.5_AT:");  Serial.print(pmat25);    Serial.print("   ");
    pmat100=(float)ucRxBuffer[14]*256+(float)ucRxBuffer[15];    Serial.print("PM10_AT:");   Serial.println(pmat100);
    pmcount03=(float)ucRxBuffer[16]*256+(float)ucRxBuffer[17];  Serial.print("PMcount0.3:");Serial.println(pmcount03);
    pmcount05=(float)ucRxBuffer[18]*256+(float)ucRxBuffer[19];  Serial.print("PMcount0.5:");Serial.println(pmcount05);
    pmcount10=(float)ucRxBuffer[20]*256+(float)ucRxBuffer[21];  Serial.print("PMcount1.0:");Serial.println(pmcount10);
    pmcount25=(float)ucRxBuffer[22]*256+(float)ucRxBuffer[23];  Serial.print("PMcount2.5:");Serial.println(pmcount25);
    temp=(float)ucRxBuffer[24]*256+(float)ucRxBuffer[25];       Serial.print("Temperature:");Serial.println(temp/10);
    humi=(float)ucRxBuffer[26]*256+(float)ucRxBuffer[27];       Serial.print("Humidity:");   Serial.println(humi/10);
    //Serial.print("Version:");     Serial.println((float)ucRxBuffer[28]);
    //Serial.print("Error Code:");  Serial.println((float)ucRxBuffer[29]);
    //Serial.println("------------------------------------------------------");
    lcd.setCursor(0,0);
    lcd.print("                ");
    lcd.setCursor(0,0);
    lcd.print("Temp:");
    lcd.setCursor(5,0);
    lcd.print(temp/10);
    lcd.setCursor(8,0);
    lcd.print("Humi:");
    lcd.setCursor(13,0);
    lcd.print(humi/10);
    if (cycle == 0){
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print("PM25:");
      lcd.setCursor(5,1);
      lcd.print(pmat25);
      lcd.setCursor(8,1);
      lcd.print("PM10:");
      lcd.setCursor(13,1);
      lcd.print(pmat100);
    }
    if (cycle == 1){
      gasval = analogRead(gasSensor);
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print("Gas VOCs:");
      lcd.setCursor(9,1);
      lcd.print(gasval);
    } 
    Serial.print("GAS VOC ");
    Serial.println(gasval);
    Serial.print("Now / Old time : ");
    Serial.print(now_time);
    Serial.print(" / ");
    Serial.println(o_time);
    Serial.println("------------------------------------------------------");
    if ((now_time - o_time) > 300000) {
      espSend(temp,humi,pmat25,pmat100,gasval);
      o_time = now_time;
    }
    ucRxCnt=0;
    return ucRxCnt;
  }
}

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  lcd.begin(16, 2); // LCD initial，16 words x 2 column，backlight ON
  for(int i=0; i<3; i++) {  // shall be wait for 30 sec since internal fan power on
    lcd.setCursor(0,0);
    lcd.print("Humi & Temp");
    lcd.setCursor(0,1);
    lcd.print("PM & VOCs");
    delay(1000);
    lcd.clear();
    delay(1000);
  }
  lcd.clear();
}
void loop(){
  now_time = millis();
  while (mySerial.available()) {
    CopeSerialData(mySerial.read());
  }
  if (cycle == 0) {
    cycle = 1;
  }
    else {
      cycle = 0;
    } 
  delay(3000);  // rescreen LCD every 3 sec, but upload to ThinkSpeak is 5 min.
}
