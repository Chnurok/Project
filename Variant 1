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
    return 0.0;
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
    //Temperat = random(230, 240)/10.0;
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
    //Humid = random(500, 600)/10.0;
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
  float temp2 = 0.0033049491*pow(temp,2) - 6.3356713222*temp + 4139.5453392304 - 0.00000058*pow(temp,3);
  if (temp2 < 99){
  digitalWrite(pumpkey, HIGH);
  delay(2000);
  digitalWrite(pumpkey, LOW);
  delay(100);
  }
  if (temp2 < 0){
  temp2 = 0;
  return temp2;
  delay(100);
  }
  if (temp2 > 100){
  temp2 = 100;
  return temp2;
  delay(100);
  }
  digitalWrite(en_cup, LOW);
  delay(500);
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
  res_datchik = w/5;
  digitalWrite(en_res, LOW);
  delay(500);
  return res_datchik;
  }

 String post_data(){ 
  Serial.println("\n Starting");
  Serial.println("Opening configuration portal");
  WiFiManager wm;
  wm.setHostname(host_name);
  HTTPClient http;
  
  int DeviceId = 1;
  int SoilMoist = 75;
  int SoilEc = 60;
  int EnvHumid = 80;
  int Battery = 80;
  float Temperature = 24.4;
  int Light = 23;
  
  //it starts an access point
  //and goes into a blocking loop awaiting configuration
  if (!wm.autoConnect("ESP32", "password")) {
    Serial.println("Not connected to WiFi, better restart");
    for (int i = 0; i < NUM_LEDS; i++ ) {   // всю ленту
    strip.setPixelColor(i, 0x8B4513);     // залить красным
    strip.show();                         // отправить на ленту
    delay(10);
    }
  }
  else {
    http.begin("http://52.224.241.170:5000/Api/Device");
    http.addHeader("Content-Type", "application/json"); 
    char request_body[200];
    sprintf(request_body, "{DeviceId: , Light: , Temperature: , EnvHumid: , SoilMoist: , SoilEc: , Battery: }", DeviceId, Light, Temperature, EnvHumid, SoilMoist, SoilEc, Battery);
    int httpResponseCode = http.POST(request_body);
    if (httpResponseCode == 200) {
      String response = http.getString();                  // get the response to the request
      ets_printf("Code: %d\n", httpResponseCode);          // print return code
      ets_printf("Response: %s\n", response.c_str());      // print response to request
      http.end(); // free the resources
      return response;
      for (int i = 0; i < NUM_LEDS; i++ ) {   // всю ленту
      strip.setPixelColor(i, 0x0000ff);     // залить синим
      strip.show();                         // отправить на ленту
      delay(10);
  }
    }
    else if (httpResponseCode == 400) {
      ets_printf("Error, Device ID is absent from database: 400\n");
      http.end(); // free the resources
      return "400";
      for (int i = 0; i < NUM_LEDS; i++ ) {   // всю ленту.
      strip.setPixelColor(i, 0x00ff00);     // залить зелёным
      strip.show();                         // отправить на ленту
      delay(10);
      }
    }
    else if (httpResponseCode == 403) {
      ets_printf("Error, device is not linked to the user: 403\n");
      http.end(); // free the resources
      return "403";
      for (int i = 0; i < NUM_LEDS; i++ ) {   // всю ленту
      strip.setPixelColor(i, 0x00ff00);     // залить зелёным
      strip.show();                         // отправить на ленту
      delay(10);
      }
    }
    else {
      ets_printf("Error while data POST: %d\n", httpResponseCode);
      http.end(); // free the resources
      return String(httpResponseCode);
      for (int i = 0; i < NUM_LEDS; i++ ) {   // всю ленту
      strip.setPixelColor(i, 0x00ff00);     // залить зелёным
      strip.show();                         // отправить на ленту
      delay(10);
    }
  }
 }
}

float button(){
  WiFiManager wm;
  wm.setHostname(host_name);
  HTTPClient http;
    delay(10);
    if (digitalRead(buttonPin) == HIGH) {
    wm.resetSettings(); 
    delay(10);
    ESP.restart();
    }
}



float func_check_battery(){
  float sensorValue = analogRead(batterypin);
  // print out the value you read:
  if (sensorValue < 3000) {
    for (int i = 0; i < NUM_LEDS; i++ ) {   // всю ленту
    strip.setPixelColor(i, 0x00ff00);     // залить белым
    strip.show();                         // отправить на ленту
    delay(10);
    return(sensorValue);
    }
  }
  else {
    return(sensorValue);
  }
}  
     

void setup() {
  Serial.begin(115200);
  pinMode(en_cup, OUTPUT);
  pinMode(en_res, OUTPUT);
  pinMode(pumpkey, OUTPUT);
  strip.begin();
  strip.setBrightness(200);    // яркость, от 0 до 255
  strip.clear();                          // очистить
  strip.show();
}


void loop() {
  float en_cup2 = funcen_cup();
  Serial.print("En_CUP = ");
  Serial.println(en_cup2);
  delay(100);
  
  float en_res2 = funcen_res();
  Serial.print("En_RES = ");
  Serial.println(en_res2);
  delay(100);
  
  float air_temp = get_air_temp(); // get air temperature from I2C sensor
  Serial.print("air_temp =");
  Serial.println(air_temp);
  delay(100);
  
  float Env_Humid = get_air_hum(); // get air humidity from I2C sensor
  Serial.print("Env_Humid =");
  Serial.println(Env_Humid);
  delay(100);
  
  float luminance = get_luminance();  // get luminance from I2C sensor
  Serial.println("luminance = ", luminance);
  delay(100);
  
  float battery = func_check_battery();
  Serial.print("Battery =  ");
  Serial.println(battery); 

  
  String response = post_data();
  
  button();
  delay(100);
}
