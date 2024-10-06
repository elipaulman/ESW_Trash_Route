#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
// TFLuna-I2C Library v0.2.0
#include <TFLI2C.h>

// 24 hours in microseconds
const int64_t TIME = 86400000000;

const char* SSID = "Registered4OSU";
const char* PASS = "dSDfe5jvfGVV7yg5";

TFLI2C tflI2C;

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
  // Initialize
  Serial.begin(115200);
  Wire.begin();
  Serial.println("Good morning!");

  connectWiFi();

  Serial.println("\nTaking reading...");

  // SENSOR AND BATTERY STUFF HERE

  Serial.println("Goodnight!");
  // Not sure if we need to free/close the sensor here, I know that deep sleep will straight up disable the wifi chip though so that's all cleaned up fine
  Serial.flush();
  esp_sleep_enable_timer_wakeup(TIME);
  esp_deep_sleep_start();
}

void loop () { }