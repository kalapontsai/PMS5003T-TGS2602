#include <ESP8266WiFi.h>
// replace with your channel’s thingspeak API key,
String apiKey = "";
const char* ssid = "";
const char* password = "";
const char* server = "api.thingspeak.com";
WiFiClient client;
String readString;
void sendata(String t,String h,String p25,String p100, String v){
  digitalWrite(5, LOW);
  if (client.connect(server,80)) { // "184.106.153.149" or api.thingspeak.com
  String postStr = apiKey;
  //postStr +="&field1=";
  //postStr += String(t);
  postStr +="&field2=";
  postStr += String(h);
  postStr +="&field3=";
  postStr += String(p25);
  postStr +="&field4=";
  postStr += String(p100);
  postStr +="&field5=";
  postStr += String(v);
  postStr +="&field6=";
  postStr += String(t);
  postStr += "\r\n\r\n";
  
  client.print("POST /update HTTP/1.1\n");
  client.print("Host: api.thingspeak.com\n");
  client.print("Connection: close\n");
  client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
  client.print("Content-Type: application/x-www-form-urlencoded\n");
  client.print("Content-Length: ");
  client.print(postStr.length());
  client.print("\n\n");
  client.print(postStr);
  
  Serial.print("<");
  Serial.print(postStr);
  Serial.println("> send to Thingspeak");
  }
  client.stop();
}

void setup() {
  Serial.begin(115200);
  delay(10);
  pinMode(5, OUTPUT);
  WiFi.begin(ssid, password);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (digitalRead(5) == HIGH){
        digitalWrite(5, LOW);
    } else {
        digitalWrite(5, HIGH);   // 點亮 LED
        }
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void loop() {
  digitalWrite(5, HIGH);
  while (Serial.available()) {
    delay(2);  //delay to allow byte to arrive in input buffer
    char c = Serial.read();
    readString += c;
  }
  if ( (readString.length() == 17 ) && readString.substring(0,2) == "TH") {
    Serial.println(readString);
    sendata(readString.substring(2,5),readString.substring(5,8),readString.substring(8,11),readString.substring(11,14),readString.substring(14));
    readString="";
  } else {
          readString="";
  }
}
