#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
// TFLuna-I2C Library v0.2.0
#include <TFLI2C.h>

const char* SSID = "Registered4OSU";
const char* PASS = "dSDfe5jvfGVV7yg5";

TFLI2C tflI2C;

void setup() {
  // Initialize
  Serial.begin(115200);
  Wire.begin();
  Serial.println("Hello, World!");

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

  Serial.println("\nConnected successfully! Setup complete!");
}

void loop() {
  // put your main code here, to run repeatedly:

}
