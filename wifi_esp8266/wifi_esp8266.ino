#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#include <WiFiManager.h>
#include <SoftwareSerial.h>


#define BUZZER_PIN D5    
SoftwareSerial Uno(D7, D8);  // D7 : RX , D8 : TX

#define ON_Board_LED D4        
const char* host = "script.google.com";
const int httpsPort = 443;
WiFiClientSecure client;

bool buttonPressed = false; 

const int arraySize = 5;  
double values[arraySize];  
int currentIndex = 0;      

String GAS_ID = "AKfycbx89OfUGH3m942fnMz4mwcKt6lp75Sd-eEJOISU_HFRkyc2WKqoyp-lbkA-cXItSKtg";

void setup() {
  Serial.begin(9600);
  Uno.begin(115200);         
  delay(500);
  pinMode(ON_Board_LED, OUTPUT);
  digitalWrite(ON_Board_LED, HIGH);
  pinMode(BUZZER_PIN, OUTPUT);

  WiFiManager wifiManager;
  wifiManager.setConnectTimeout(30);
  wifiManager.setDebugOutput(true);
  wifiManager.resetSettings();
  wifiManager.setConfigPortalTimeout(180);

  if (!wifiManager.autoConnect("TALLO")) {  
    Serial.println("เชื่อมต่อไม่สำเร็จ กำลังเข้าสู่โหมด AP");
    ESP.restart();
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("เชื่อมต่อกับ WiFi สำเร็จ!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }

  client.setInsecure();  
}

void connectToWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFiManager wifiManager;
    wifiManager.autoConnect("TALLO");

    unsigned long startAttemptTime = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {  
      delay(500);
      Serial.print(".");
    }

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("\nเชื่อมต่อ Wi-Fi ล้มเหลว กำลังรีบูต...");
      ESP.restart();  
    } else {
      Serial.println("\nเชื่อมต่อกับ Wi-Fi สำเร็จ!");
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
    }
  }
}

void loop() {
  connectToWiFi();

  if (Uno.available() > 0) {  
    String input = Uno.readString();   
    double value = input.toFloat();    

      // แปลงค่าต่างๆ โดยใช้ค่าเฉลี่ย
      double cm = value;
      double m = value / 100;
      double inch = value / 2.54;
      double foot = value / 30;

      sendData(cm, m, inch, foot);  
  }
}


void sendData(float value, float value2, float value3, float value4) {
  Serial.print("กำลังเชื่อมต่อกับ ");
  Serial.println(host);

  if (!client.connect(host, httpsPort)) {
    Serial.println("การเชื่อมต่อล้มเหลว กำลังเชื่อมต่อ WiFi ใหม่...");
    connectToWiFi();
    return;
  }

  String url = "/macros/s/" + GAS_ID + "/exec?cm=" + String(value, 3) + "&m=" + String(value2, 3) + "&inch=" + String(value3, 3) + "&foot=" + String(value4, 3);
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: ESP8266\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("ส่งคำขอแล้ว");

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("รับ Headers สำเร็จ"); 
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("ส่งข้อมูลสำเร็จ!");
  } else {
    Serial.println("การส่งข้อมูลล้มเหลว");
    tone(BUZZER_PIN, 1000);             
    delay(1000);                        
    noTone(BUZZER_PIN);                 
    delay(500);
  }
  client.stop();
}