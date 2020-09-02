//20181212 rev 1.0
//20181220 rev1.1 add alert led to highlight the air statuspdatre
//20190613 update the sleeping timing to 3600sec

#include <Wire.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(5, 14);//GPIO5,GPIO14 => rx,tx, need Rx to receive PMS data only

const char* ssid = ";
const char* password = "";
const char* url = "api.thingspeak.com";
String apiKey = "";
 
// ESP-12 LED = GPIO2 & LED_BUILTIN = GPIO16
// onboard led : LOW is ON, HIGH is OFF
int PMS_SET=0; //GPIO0=D3 for PMS Pin3, Default/High : Normal , Low : Sleep mode
int ledConnect = 2; //GPIO2 = D4 or LED_BUILTIN for debug mode
int ledAlert = 4;

unsigned long last_time;

long pmat10=0,pmat25=0,pmat100=0;
long temp=0, humi=0;
long c3=0, c4=0; // checksum c3=0, c4=28

//WiFiServer server(80);
WiFiClient client;

void led_change(int times = 1,int on = 1000,int off = 1000){
  for(int i=0; i<times*2; i++) { 
    if (digitalRead(ledConnect) == LOW) {
      digitalWrite(ledConnect, HIGH);
      delay(on);
    }
    else {
      digitalWrite(ledConnect, LOW);
      delay(off);
    }
  }
}

void wifi_connect(){
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ...");
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    led_change(1,200,200);
  }
  Serial.println("WiFi connected");
  Serial.print("Local IP : ");
  Serial.println(WiFi.localIP());
  digitalWrite(ledConnect, HIGH);
}

void ESP_send(String t,String h,String p25,String p100){
  // checksum c3=0, c4=28
  if (c3!=0x00 || c4!=0x1c){
    Serial.println("Checksum fail !!");
    return false;
  }
  Serial.printf("\n[Connecting to host %s ... ", url);
  if (client.connect(url,80)) { // "184.106.153.149" or api.thingspeak.com
    Serial.println("connected]");
    String postStr = apiKey;
    postStr +="&field2=";
    postStr += String(h);
    postStr +="&field3=";
    postStr += String(p25);
    postStr +="&field4=";
    postStr += String(p100);
    postStr +="&field6=";
    postStr += String(t);
    postStr += "\r\n\r\n";
    
    Serial.println("[Sending a request]");
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
    
    Serial.println("[Response:]");
    while (client.connected() || client.available())
    {
      if (client.available())
      {
        String line = client.readStringUntil('\n');
        Serial.println(line);
      }
    }

    client.stop();
    Serial.println("\n[Host Disconnected]");
    return true;
  }
  else  {
    Serial.println("connection failed!]");
    client.stop();
    while(WiFi.status() != WL_CONNECTED){
      Serial.println("WiFi disconnect !!");
      wifi_connect();
    }
    return false;
  }
}

void catchPMS(){
  unsigned char c;
  unsigned char high;
  int count = 1;

  while (mySerial.available()) { 
    c = mySerial.read();
    //Serial.println(c);
    //Serial.print("count:");Serial.print(count);Serial.print(":");Serial.println(c);
    // can  add (count==3 && c!=0x00) || (count==4 && c!=0x1c)
    if((count==1 && c!=0x42) || (count==2 && c!=0x4d)){
      break;
    }
    if(count >= 29){
      Serial.println("--- Complete ---");
      break;
    }
    else if(count == 3){
      c3 = c;
      Serial.print("C3=");
      Serial.println(c3);
    }
    else if(count == 4){
      c4 = c;
      Serial.print("C4=");
      Serial.println(c4);
    }
    else if(count == 11 || count == 13 || count == 15 || count == 25 || count == 27) {
      high = c;
    }
     else if(count == 12){
      pmat10 = 256*high + c;
      Serial.print("P10=");
      Serial.println(pmat10);
    }
    else if(count == 14){
      pmat25 = 256*high + c;
      Serial.print("P25=");
      Serial.println(pmat25);
    }
    else if(count == 16){
      pmat100 = 256*high + c;
      Serial.print("P100=");
      Serial.println(pmat100);
    }
     else if(count == 26){        
      temp = (256*high + c)/10;
      Serial.print("temp=");
      Serial.println(temp);
     }
    else if(count == 28){            
      humi = (256*high + c)/10;
      Serial.print("humi=");
      Serial.println(humi);
    }
    count++;
    delay(1);
  }
  while(mySerial.available()) {
    mySerial.read();
  }  
}

void eco_Mode(int cmd){
  if (cmd == 1){
    digitalWrite(PMS_SET, HIGH);
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("Suspend Mode Disabled !!");
  }
  else if (cmd == 0){
    digitalWrite(PMS_SET, LOW);
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("Suspend Mode Enabled !!");
  }
}

void setup() {
  Serial.begin(115200);
  //Serial1.begin(9600); // backup plan; GPIO2 can use TX pin only
  mySerial.begin(9600);
  delay(100);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ledConnect, OUTPUT);
  pinMode(ledAlert, OUTPUT);
  pinMode(PMS_SET, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(PMS_SET, HIGH);
  digitalWrite(ledAlert, LOW);
  wifi_connect();
  // PMS need 30 sec to steady
  for (int i=0; i<10; i++){
    catchPMS();
    delay(3000);
  }
  ESP_send(String(temp),String(humi),String(pmat25),String(pmat100));
  delay(5);
  last_time = millis();
}

void loop(){
  bool send = false;
  catchPMS();
  if ((millis() - last_time) > 3600000) {
    send = ESP_send(String(temp),String(humi),String(pmat25),String(pmat100));
    if (send == true){
      eco_Mode(0);
      last_time = millis();
    }
  }
  if ((pmat25 > 75) || (pmat100 > 100)) {
    digitalWrite(ledAlert, HIGH);
  } else {
    digitalWrite(ledAlert, LOW);
  }
  if ((digitalRead(PMS_SET) == LOW) && ((millis() - last_time) > 3550000)){
    eco_Mode(1);
  }
  led_change(1,2000,1000);
}
