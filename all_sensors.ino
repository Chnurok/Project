#include "soc/rtc_cntl_reg.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "driver/adc.h"
#include "esp32/ulp.h"
#include "esp_sleep.h"
#include "ulp_main.h"
#include "ulptool.h"
#include <WEMOS_SHT3X.h>
#include <Wire.h>


#define SHT_30_ADRR 0x45
#define MAX_44009_ADRR 0x4A


// Unlike the esp-idf always use these binary blob names
extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[]   asm("_binary_ulp_main_bin_end");

const gpio_num_t SENSORS_KEY = GPIO_NUM_33;

static void init_ulp_program() {  // init ULP program

  esp_err_t err = ulptool_load_binary(0, ulp_main_bin_start, (ulp_main_bin_end - ulp_main_bin_start) / sizeof(uint32_t));
  ESP_ERROR_CHECK(err);

  ulp_adc_1_4r = 0;   // raw adc values
  ulp_adc_1_4q = 0;   // r - reminder after devision
  ulp_adc_1_7r = 0;   // q - quotient
  ulp_adc_1_7q = 0;
  ulp_adc_2_4r = 0;
  ulp_adc_2_4q = 0;
  ulp_adc_2_6r = 0;
  ulp_adc_2_6q = 0;
  ulp_analog_measurements_taken = 0; // set if analog measurements in deeps sleep were taken

  // init adc1
  adc1_config_width(ADC_WIDTH_BIT_12);                         // set bit width of ADC1
  adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_6);   // GPIO 32
  adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_6);   // GPIO 35
  adc1_ulp_enable();                                           // enable ADC1 for ULP

  // init adc2
  //adc2_config_width(ADC_WIDTH_BIT_12);                       // not purposed for ADC2
  adc2_config_channel_atten(ADC2_CHANNEL_4, ADC_ATTEN_DB_6);   // GPIO 13
  adc2_config_channel_atten(ADC2_CHANNEL_6, ADC_ATTEN_DB_6);   // GPIO 14
  //adc2_ulp_enable();                                         // not supported and not needed for ADC2

  rtc_gpio_init(SENSORS_KEY);  // GPIO_33 connected to sensors key
  rtc_gpio_set_direction(SENSORS_KEY, RTC_GPIO_MODE_OUTPUT_ONLY);  // GPIO_33 is output
}

double calculate_volts(double raw, uint32_t adc) {  // calculate voltage from raw ADC bits
  if (adc == 1) {
    return (28507 * raw + 32768) / 65536.0 + 107;
  }
  else {
    return (28560 * raw + 32768) / 65536.0 + 89;
  }
}

float get_air_hum() {    // get air humidity from SHT30
  SHT3X sensor(SHT_30_ADRR);
  if (sensor.get() == 0) {  // if sensor is available
    return sensor.humidity;
  }
  else {
    ets_printf("Warning, SHT30 is unavailable!\n");
    return 0.0;
  }
}

float get_air_temp() {    // get air temperature from SHT30
  SHT3X sensor(SHT_30_ADRR);
  if (sensor.get() == 0) {  // if sensor is available
    return sensor.cTemp;
  }
  else {
    ets_printf("Warning, SHT30 is unavailable!\n");
    return 0.0;
  }
}

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

double volt_to_hum(double volt) {  // convert voltage to soil humidity
  double hum = pow(volt/1000, -2.152) * 69.943;  // calculated for specific designed analog sensor
  if (hum > 100) {
    hum = 100.0;
  }
  if (hum < 0) {
    hum = 0.0;
  }
  return hum;
}

double volt_to_battery_percent(double volt) {  // convert voltage to percent of battery charge
  double battery_percent = (volt - 1600) / 500 * 100; // calculated for specific designed analog sensor
  if (battery_percent > 100) {
    battery_percent = 100.0;
  }
  if (battery_percent < 0) {
    battery_percent = 0.0;
  }
  return battery_percent;
}

bool volt_to_water_level(double volt) {
  if (volt > 1650) {
    return false;
  }
  else {
    return true;
  }
}

void setup() {
  delay(1000);
  pinMode(SENSORS_KEY, OUTPUT);    // set GPIO_33 as output
  Serial.begin(115200);

  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  if (cause != ESP_SLEEP_WAKEUP_ULP) {
    ets_printf("Not ULP wakeup, initializing ULP\n");
    init_ulp_program();
  }
  else {
    ets_printf("--------------------------------------------------------------------------\n");
    ets_printf("ULP wakeup, printing values\n");

    // calculate voltage from raw bits
    double voltage14 = calculate_volts((ulp_adc_1_4q & 0xFFFF) + (ulp_adc_1_4r & 0xFFFF) / 16.0, 1);
    double voltage17 = calculate_volts((ulp_adc_1_7q & 0xFFFF) + (ulp_adc_1_7r & 0xFFFF) / 16.0, 1);
    double voltage24 = calculate_volts((ulp_adc_2_4q & 0xFFFF) + (ulp_adc_2_4r & 0xFFFF) / 16.0, 2);
    double voltage26 = calculate_volts((ulp_adc_2_6q & 0xFFFF) + (ulp_adc_2_6r & 0xFFFF) / 16.0, 2);

    Serial.printf("Value @ ADC1 CH 4 (humidity sensor): %f (%f mV)\n", (ulp_adc_1_4q & 0xFFFF) + (ulp_adc_1_4r & 0xFFFF) / 16.0, voltage14);
    Serial.printf("Value @ ADC1 CH 7 (fertility sensor): %f (%f mV)\n", (ulp_adc_1_7q & 0xFFFF) + (ulp_adc_1_7r & 0xFFFF) / 16.0, voltage17);
    Serial.printf("Value @ ADC2 CH 4 (water level sensor): %f (%f mV)\n", (ulp_adc_2_4q & 0xFFFF) + (ulp_adc_2_4r & 0xFFFF) / 16.0, voltage24);
    Serial.printf("Value @ ADC2 CH 6 (battery charge sensor): %f (%f mV)\n", (ulp_adc_2_6q & 0xFFFF) + (ulp_adc_2_6r & 0xFFFF) / 16.0, voltage26);

    // convert voltage to values
    double soil_humidity = volt_to_hum(voltage14);  // calculate soil humidity
    double battery_percent = volt_to_battery_percent(voltage26);  // calculate battery percent
    bool water_level_ok = volt_to_water_level(voltage24); // calculate if water level is fine
    //double soil_fertility = volt_to_fert(voltage17);  // calculate soil fertility
    double soil_fertility = 11.8;  // just decoy


    if ((ulp_analog_measurements_taken & 0xFFFF) == 2905) { // if analog measurements were taken
      ets_printf("Analog measurements were taken, turn on i2c sensors\n");
      ets_printf("Lock the sensors key\n");
      digitalWrite(SENSORS_KEY, HIGH); // set GPIO_33 HIGH to lock the sensors key
      delay(1200);

      float luminance = get_luminance();  // get luminance from I2C sensor
      float air_temperature = get_air_temp();  // get air temperature from I2C sensor
      float air_humidity = get_air_hum();// get air humidity from I2C sensor

      ets_printf("Unlock the sensors key\n");
      digitalWrite(SENSORS_KEY, LOW); // set GPIO_33 LOW to unlock the sensors key

      Serial.printf("\nLuminance: %f lux\n", luminance);
      Serial.printf("Air temperature: %f C\n", air_temperature);
      Serial.printf("Air humidity: %f %%\n", air_humidity);
      Serial.printf("__\n");
      Serial.printf("Soil humidity: %f %%\n", soil_humidity);
      Serial.printf("Soil fertility: %f S\n", soil_fertility);
      Serial.printf("Battery charge: %f %%\n", battery_percent);
      const char* water_level = (water_level_ok) ? "fine" : "low";
      Serial.printf("Waterlevel: %s\n", water_level);

      ulp_analog_measurements_taken = 0;  // clear flag to take analog measurements again
      rtc_gpio_deinit(SENSORS_KEY);    // to be able to use it again by ulp
      rtc_gpio_init(SENSORS_KEY);      // init GPIO_33
      rtc_gpio_set_direction(SENSORS_KEY, RTC_GPIO_MODE_OUTPUT_ONLY);  // GPIO_33 is output
    }
  }

  ets_printf("Entering deep sleep\n\n");
  /* Start the ULP program */
  ESP_ERROR_CHECK( ulp_run((&ulp_entry - RTC_SLOW_MEM) / sizeof(uint32_t)));
  ESP_ERROR_CHECK( esp_sleep_enable_ulp_wakeup() );
  esp_deep_sleep_start();
}

void loop() {
  // not used due to deep sleep wakeup shenanigans
}
