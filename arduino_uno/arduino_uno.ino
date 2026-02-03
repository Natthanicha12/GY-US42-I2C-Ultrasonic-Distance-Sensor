#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); 
//LiquidCrystal_I2C lcd(0x3F, 16, 2); 
#include<SoftwareSerial.h>
SoftwareSerial Uno(10,11); //(rx,tx)

#define SENSOR_ADDRESS 0x70  // I2C address
#define NUM_READINGS 5 // Number of readings to store
#define BUTTON_PIN 8
#define BUTTON1_PIN 9
#define BUTTON2_PIN 12 // Pin connected to the button (not the RESET pin)
#define BUTTON3_PIN 7

double average;

int readings[NUM_READINGS]; // Array to store distance values
int index1 = 0; 
bool validReadings = false; // Check if enough data is available
String unit ;
double tf ; // Store variables
int x = 0;
int y = 0;
double distance2;

const int arraySize = 5;   // Define the array size
double values[arraySize];  // Array for storing values
int currentIndex = 0;     

void setup() {
  lcd.init();
  lcd.backlight();

  Serial.begin(115200); // Initialize serial communication
  Uno.begin(115200); // Initialize I2C communication
  Wire.begin();

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);

}

void loop() {
    double distance = readDistance(); // Read distance from the sensor

  if (digitalRead(BUTTON1_PIN) == 0){
    x++;
    delay(100);
  }

  if (distance > 0) { // ถ้าได้ค่าที่ถูกต้อง
    lcd.clear();

      if(distance >= 720 || distance <= 20){
      Serial.println("error !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
      lcd.setCursor(1, 0); 
      lcd.print("!!! error !!!");

     }else{

      if(digitalRead(BUTTON3_PIN) == 0){
       distance2 = distance + 11;
       lcd.setCursor(0, 1);
       lcd.print("^");
      }else{
       distance2 = distance;
       lcd.setCursor(0, 1);
       lcd.print("v");
      }

       //calibrate จากการทดลอง 
      if(distance2 >= 100 && distance2 < 120 ){
        distance2 += (distance2 * 0.020) ; 
      }else if(distance2 >= 120 && distance2 < 180 ){
        distance2 += (distance2 * 0.025) ; 
      }else if (distance2 >= 180 && distance2 < 240){
        distance2 += (distance2 * 0.019) ;
      }else if (distance2 >= 240 && distance2 < 300){
        distance2 += (distance2 * 0.018) ;
      }

      Serial.print("Distance : ");
      Serial.print(distance2);
      Serial.println(" cm");

      if(x == 0){
        tf = distance2;
        unit = "cm";
      }else if (x == 1){
        tf = distance2 / 100;
        unit = "m";
      }else if (x == 2){
        tf = distance2 / 2.54;
        unit = "in";
      }else if(x == 3){
        tf = distance2 / 30;
        unit = "f";
      }else{
        x = 0;
      }

      lcd.setCursor(0, 0);
      lcd.print("dist :");
      lcd.setCursor(7, 0);
      lcd.print(tf,3);
      lcd.setCursor(14, 0);
      lcd.print(unit);

      if (digitalRead(BUTTON_PIN) == 0) { 

     
    if (currentIndex > 0) { 
      double previousValue = values[currentIndex - 1];
      if (abs(distance2 - previousValue) > 15) {
        Serial.println("Difference exceeds ±15, resetting values...");
        currentIndex = 0; 
        return; 
      }
    }

     
    values[currentIndex] = distance2;
    currentIndex++;

    if (currentIndex >= arraySize) {
      double sum = 0;
      for (int i = 0; i < arraySize; i++) {
        sum += values[i];  
      }

     average = sum / arraySize;  

     if(x == 0){
        tf = average;
        unit = "cm";
      }else if (x == 1){
        tf = average / 100;
        unit = "m";
      }else if (x == 2){
        tf = average / 2.54;
        unit = "in";
      }else if(x == 3){
        tf = average / 30;
        unit = "f";
      }else{
        x = 0;
      }

      lcd.setCursor(0, 0); 
      lcd.print("dist :");
      lcd.setCursor(7, 0);
      lcd.print(tf,3); 
      lcd.setCursor(14, 0);
      lcd.print(unit);
      lcd.setCursor(2, 1);
      lcd.print("Successful");

      Uno.print(average,3);

      delay(7000);

    }
  }
      
    }

  } else {
    Serial.println("Error reading sensor");
  }

  delay(100); 
  //lcd.clear();

  if (digitalRead(BUTTON2_PIN) == 0) {
    Serial.println("Resetting...");
    delay(100);  /

    
    asm volatile ("  jmp 0"); 
  }

  delay(200);
}

// Function to acquire distance data from the sensor
int readDistance() {
  Wire.beginTransmission(SENSOR_ADDRESS);// Initialize communication with the sensor
  Wire.write(0x51); // Send a command to initiate distance measurement
  Wire.endTransmission(); // End data transmission

  delay(70); 

  Wire.requestFrom(SENSOR_ADDRESS, 2);

  if (Wire.available() == 2) { 
    byte highByte = Wire.read(); 
    byte lowByte = Wire.read();

    return (highByte << 8) + lowByte; 
  }

  return 0; 
}


