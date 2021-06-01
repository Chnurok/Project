#include <WEMOS_SHT3X.h>
#include <Wire.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <WiFi.h>
#include <HTTPClient.h>
#include "Adafruit_NeoPixel.h"


#define PIN 2        // пин DI
#define NUM_LEDS 1   // число диодов
#define MAX_44009_ADRR 0x4A
SHT3X sht30(0x44);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);
const char* host_name = "herewego.again";
const byte cap = 34;
const byte en_cup = 13;
const byte res = 35;
const byte en_res = 27;
const byte buttonPin = 17; // pushbutton pin
const byte batterypin = 33;
const byte pumpkey = 19;
const byte i =1;

float temp = 0;
float res_datchik = 0;
float Humid = 0;
float Temperat = 0;
int ares[5];
int acap[5];


float get_luminance() {  // get luminance from MAX44009
  Wire.begin(21, 22); // init I2C: SDA = GPIO21 / SCL = GPIO22
  Wire.beginTransmission(MAX_44009_ADRR);
  Wire.write(0x02);  // add byte to queue
  Wire.write(0x40);  // add byte to queue
  Wire.endTransmission(); // send bytes from queue
  delay(300);   // wait for sensor to receive commands and
  unsigned int lum_data[2];  // array for aquired data
  Wire.beginTransmission(MAX_44009_ADRR);
  Wire.write(0x03);  // add byte to queue
  Wire.endTransmission(); // send bytes from queue
  // request 2 bytes of data
  Wire.requestFrom(MAX_44009_ADRR, 2);
  // read 2 bytes of data luminance msb, luminance lsb
  if (Wire.available() == 2) {
    lum_data[0] = Wire.read();
    lum_data[1] = Wire.read();
  }
  else {
    ets_printf("Warning, MAX44009 is unavailable!\n");
    float  luminance = random(300, 600);
    return luminance;
  }
  // convert the data to lux (all calculations based on sensor datasheet)
  int exponent = (lum_data[0] & 0xF0) >> 4;
  int mantissa = ((lum_data[0] & 0x0F) << 4) | (lum_data[1] & 0x0F);
  float luminance = pow(2, exponent) * mantissa * 0.045;
  return luminance;
}


float get_air_temp() {    // get air humidity from SHT30
  if(sht30.get()==0){
    Temperat = sht30.cTemp;
    return Temperat;
  }
  else
  {
    Temperat = random(270, 290)/10.0;
    return Temperat;
  }
}

float get_air_hum() {    // get air humidity from SHT30
  if(sht30.get()==0){
    Humid = sht30.humidity;
    return Humid;
  }
  else
  {
    Humid = random(500, 600)/10.0;
    return Humid;
  }
}


float funcen_cup(){
  float q = 0;
  digitalWrite(en_cup, HIGH);
  delay(500);
  for (int i = 0; i < 5; i++) {
  acap[i] = analogRead(cap);
  delay(200);
  q = acap[i]+q;
  }
  temp = q/5;
  if (temp > 2130){
    if (temp > 2400){
      return temp;
    }
    digitalWrite(pumpkey, HIGH);
    delay(3000);
    digitalWrite(pumpkey, LOW);
    delay(100);
  }
  digitalWrite(en_cup, LOW);
  delay(100);
  return temp;
  }

float funcen_res(){
  float w = 0;
  digitalWrite(en_res, HIGH);
  delay(500);
  for (int i = 0; i < 5; i++) {
  ares[i] = analogRead(res);
  delay(200);
  w= ares[i]+w;
  }
  float w2 = w/5;
  digitalWrite(en_res, LOW);
  delay(500);
  return w2;
  }

String post_data(const char* DeviceId, int Light, int SoilMoist, int SoilEc, int EnvHumid, int Battery, int Temp, byte WaterRemained){ 
  WiFiManager wm;
  wm.setHostname(host_name);
  delay(100);
  HTTPClient http;
  

  if (!wm.autoConnect("ESP32", "password")) {
    strip.setPixelColor(i, 0x0000ff);     // залить синим
    strip.show();                         // отправить на ленту
  }
  else {
    http.begin("http://52.224.241.170:5000/Api/Device");
    http.addHeader("Content-Type", "application/json"); 
    char request_body[200];
    sprintf(request_body, "{DeviceId: %s , Light: %d, Temp: %d, EnvHumid: %d, SoilMoist: %d, SoilEc: %d, Battery: %d, WaterRemained: %d}", DeviceId, Light, Temp, EnvHumid, SoilMoist, SoilEc, Battery, WaterRemained);
    int httpResponseCode = http.POST(request_body);
    Serial.println("Отравляем данные:");
    Serial.print(request_body);
    if (httpResponseCode == 200) {
      String response = http.getString();                  // get the response to the request
      ets_printf("Code: %d\n", httpResponseCode);          // print return code
      ets_printf("Response: %s\n", response.c_str());      // print response to request
      http.end(); // free the resources
      return response;
    }
    else if (httpResponseCode == 400) {
      ets_printf("Error, Device ID is absent from database: 400\n");
      http.end(); // free the resources
      strip.setPixelColor(i, 0x00ff00);     // залить зелёным
      strip.show();                         // отправить на ленту
      return "400";
    }
    else if (httpResponseCode == 403) {
      ets_printf("Error, device is not linked to the user: 403\n");
      http.end(); // free the resources
      strip.setPixelColor(i, 0x00ff00);     // залить зелёным
      strip.show();                         // отправить на ленту
      return "403";
    }
    else {
      ets_printf("Error while data POST: %d\n", httpResponseCode);
      http.end(); // free the resources
      strip.setPixelColor(i, 0x00ff00);     // залить зелёным
      strip.show();                         // отправить на ленту
      return String(httpResponseCode);
  }
 }
}

void button(){
  WiFiManager wm;
  wm.setHostname(host_name);
  HTTPClient http;
    delay(10);
    wm.resetSettings(); 
    delay(10);
    ESP.restart();
}



float func_check_battery(){
  float sensorValue = analogRead(batterypin);
  float sensorValue2 = (sensorValue - 1680)*100/745;
  // print out the value you read:
  if (sensorValue2 < 10) {
    int i = 0;
    strip.setPixelColor(i, 0xff0000);     // залить красным
    strip.show();                         // отправить на ленту
    delay(1000);
    if (sensorValue < 0){
      sensorValue = 5;
    }
  }
  return(sensorValue);
}  
     

void setup() {
  Serial.begin(115200);
  pinMode(en_cup, OUTPUT);
  pinMode(en_res, OUTPUT);
  pinMode(pumpkey, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPin), button, HIGH);
  const char* DeviceId = "\"df14574e-bd43-11eb-8529-0242ac130003\"";
  strip.begin();
  strip.setBrightness(200);    // яркость, от 0 до 255
  strip.clear();                          // очистить 
  strip.show();
}


void loop() {
  const char* DeviceId = "\"df14574e-bd43-11eb-8529-0242ac130003\"";
  byte WaterRemained = 80;

  float Battery = func_check_battery();

   2 
  float SoilMoist = funcen_cup();
  delay(100);
  
  float SoilEc = funcen_res();
  delay(100);
  
  float Temperature = get_air_temp(); // get air temperature from I2C sensor
  delay(100);
  
  float EnvHumid = get_air_hum(); // get air humidity from I2C sensor
  delay(100);
  
  float Light = get_luminance();  // get luminance from I2C sensor
  delay(100);
  
  String response = post_data(DeviceId, Light, SoilMoist, SoilEc, EnvHumid, Battery, Temperature, WaterRemained);
  attachInterrupt(digitalPinToInterrupt(buttonPin), button, HIGH);
  delay(10000);
}
