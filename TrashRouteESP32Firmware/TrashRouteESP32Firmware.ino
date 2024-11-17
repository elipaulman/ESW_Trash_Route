#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
// TFLuna-I2C Library v0.2.0
#include <TFLI2C.h>

// 24 hours in microseconds
const int64_t TIME = 86400000000;
// const int64_t TIME = 10000000;

const char* SSID = "Registered4OSU";
const char* PASS = "dSDfe5jvfGVV7yg5";

TFLI2C tflI2C;

// Distance in centimeters
int16_t tfDist;
// Signal quality/strength
int16_t tfFlux;
// Chip temperature in hundreths of *C
int16_t tfTemp;

int16_t tfAddr = TFL_DEF_ADR;

void connectWiFi() {
  // Station WiFi and disconnect from any networks we might already be on.
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Connect to WiFi
  WiFi.begin(SSID, PASS);
  Serial.print("Connecting to Registered4OSU");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.print("done!");
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Serial.println("\n\n\nGood morning!");

  connectWiFi();

  Serial.print("\n\nTaking reading...");

  if (tflI2C.getData(tfDist, tfFlux, tfTemp, tfAddr)) {
    Serial.println("OK\n");
    Serial.print("Dist: ");
    Serial.println(tfDist);
    Serial.print("Flux: ");
    Serial.println(tfFlux);
    Serial.print("Temp: ");
    Serial.println(tfTemp);
  } else {
    Serial.println("***FAILED***");
    Serial.println("\ntflI2C STATUS:");
    tflI2C.printStatus();
  }

  Serial.println("Goodnight!\n\n\n");
  Serial.flush();
  esp_sleep_enable_timer_wakeup(TIME);
  esp_deep_sleep_start();
}

void loop () { }