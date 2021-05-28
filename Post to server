#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <WiFi.h>
#include <HTTPClient.h>

const byte PIN_LED = 1;  // onboard LED
const char* host_name = "herewego.again";
const int buttonPin = 23;     // the number of the pushbutton pin
int buttonState = 0;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  
  delay(1000);
  Serial.begin(115200);
  Serial.println("\n Starting");

  Serial.println("Opening configuration portal");
  digitalWrite(PIN_LED, LOW); // turn the LED on by making the voltage LOW to tell us we are in configuration mode

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
  }
  else {
    Serial.println("Connected to chosen WiFi");
    Serial.println("Attempt to http GET");
    http.begin("http://52.224.241.170:5000/Api/Device");
    //http.begin("http://192.168.177.61:8080/sample?temperature=24.37");
    //http.get("/");        // specify the URL and make the request

   
    int statusCode = http.GET();
    String response = http.getString();
    Serial.println(statusCode);
    Serial.println(response); 
    if (statusCode == 200 && response != "") {
      digitalWrite(PIN_LED, HIGH);
    }
    else {
      Serial.println("No access to Internet");
    }
     http.end(); // free the resources
  }
}


   /* http.begin("http://52.224.241.170:5000/Api/Device");
    char request_body[200];
    sprintf(request_body, "{DeviceId: %s, Light: %d, Temperature: %d, EnvHumid: %d, SoilMoist: %d, SoilEc: %d, Battery: %d}", DeviceId, Light, Temperature, EnvHumid, SoilMoist, SoilEc, Battery);
    int httpResponseCode = http.POST(request_body);

    if (httpResponseCode == 200) {
      String response = http.getString();                  // get the response to the request
      ets_printf("Code: %d\n", httpResponseCode);          // print return code
      ets_printf("Response: %s\n", response.c_str());      // print response to request
      http.end(); // free the resources
      //return response;
    }
    else if (httpResponseCode == 400) {
      ets_printf("Error, Device ID is absent from database: 400\n");
      http.end(); // free the resources
      //return "400";
    }
    else if (httpResponseCode == 403) {
      ets_printf("Error, device is not linked to the user: 403\n");
      http.end(); // free the resources
      //return "403";
    }
    else {
      ets_printf("Error while data POST: %d\n", httpResponseCode);
      http.end(); // free the resources
      //return String(httpResponseCode);
    }
    }
  }*/



void loop() {
  
  /*int air_humidity = 1000;
  int luminance = 2000;
  int res_datchik = 3000;
  int temp = 36;
  HTTPClient http;
  http.begin("http://192.168.177.61:8080/sample?air_humidity=" + String(air_humidity) + "&luminance=" + String(luminance));

  int statusCode = http.GET();

  delay(5000);*/
    
  /*http.begin("http://172.20.10.8:8080/sample");
    // делаем GET запрос
  int httpCode = http.GET();
    // проверяем успешность запроса
  if (httpCode > 0) {
      
      // выводим ответ сервера
    String payload = http.getString();
    Serial.println(httpCode);
    Serial.println(payload);
    delay(10000);
  }
  else {
    Serial.println("Ошибка HTTP-запроса");
  }

  if (httpCode > 0) {
    char request_body[200];
    sprintf(request_body, "{air_humidity: %d, luminance: %d, res_datchik: %d, temp: %d}", air_humidity, luminance, res_datchik, temp);
    int httpResponseCode = http.POST(request_body);   

    delay(10000);}*/


    
 /*WiFiManager wm;
  wm.setHostname(host_name);
  HTTPClient http;
    delay(10);
    if (digitalRead(buttonPin) == HIGH) {
    wm.resetSettings(); 
    delay(10);
    ESP.restart();
    digitalWrite(PIN_LED, HIGH);
    }
    else {
    digitalWrite(PIN_LED, LOW);
    }*/
}
