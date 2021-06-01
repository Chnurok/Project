#include <WEMOS_SHT3X.h>
#include <Wire.h>


#define MAX_44009_ADRR 0x4A
SHT3X sht30(0x44);

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


float get_air_hum() {    // get air humidity from SHT30
  if(sht30.get()==0){
    Serial.print("Temperature in Celsius : ");
    Serial.println(sht30.cTemp);
    Serial.print("Temperature in Fahrenheit : ");
    Serial.println(sht30.fTemp);
    Serial.print("Relative Humidity : ");
    Serial.println(sht30.humidity);
    Serial.println();
  }
  else
  {
    Serial.println("Error!");
  }
}


void setup() {

  Serial.begin(115200);

}

void loop() {
  delay(1000);
  float air_humidity = get_air_hum();// get air humidity from I2C sensor
  float luminance = get_luminance();  // get luminance from I2C sensor

  Serial.printf("\nLuminance: %f lux\n", luminance);
  delay(1000);

}
