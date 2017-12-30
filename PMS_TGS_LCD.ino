#include <Wire.h>  // Arduino IDE 內建
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3);//rx,tx
#include <LiquidCrystal_I2C.h>
int gasSensor = 2;  // Analog pin A2
int gasval = 0;
int cycle = 0;
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // 設定 LCD I2C 位址

void lcdmsg(int Yaxis,String type,int value){
  lcd.setCursor(0,Yaxis);
  lcd.print("                ");
  lcd.setCursor(0,Yaxis);
  lcd.print(type);
  lcd.print(":");
  lcd.print(value);
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
    temp=(float)ucRxBuffer[24]*256+(float)ucRxBuffer[25];       Serial.print("Temperature:");Serial.println(temp);
    humi=(float)ucRxBuffer[26]*256+(float)ucRxBuffer[27];       Serial.print("Humidity:");   Serial.println(humi);
    //Serial.print("Version:");     Serial.println((float)ucRxBuffer[28]);
    //Serial.print("Error Code:");  Serial.println((float)ucRxBuffer[29]);
    //Serial.println("------------------------------------------------------");
    //if (temp/10 > 99) break;  // 指數異常, 不顯示
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
      lcd.print("PM25:");
      lcd.setCursor(5,1);
      lcd.print(pmat25);
      lcd.setCursor(8,1);
      lcd.print("PM100:");
      lcd.setCursor(14,1);
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
    Serial.println("------------------------------------------------------");
    //delay(1000);
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
  while (mySerial.available()) {
    CopeSerialData(mySerial.read());
  }
  if (cycle == 0) {
    cycle = 1;
  }
    else {
      cycle = 0;
    }  
}
