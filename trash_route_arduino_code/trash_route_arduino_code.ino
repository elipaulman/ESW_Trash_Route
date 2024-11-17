#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <TFLI2C.h>

// WiFi credentials
const char* SSID = "Ye Olde Wifi";
// "Registered4OSU";
const char* PASS = "141ENorwichAve!";
// "dSDfe5jvfGVV7yg5";

// REST API Endpoint
const char* SERVER_NAME = "https://esw-trash-route.onrender.com/api/trash-data";

// TFLuna Variables
TFLI2C tflI2C;
int16_t tfDist;  // Distance in centimeters
int16_t tfFlux;  // Signal quality/strength
int16_t tfTemp;  // Chip temperature in hundredths of *C
int16_t tfAddr = TFL_DEF_ADR; // Default TFLuna address

// Sleep duration: 1 hour (in microseconds)
#define SLEEP_DURATION_US (60ULL * 60 * 1000000)

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  WiFi.begin(SSID, PASS);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" Connected!");
}

void sendToServer(int16_t distance, int16_t flux, int16_t temperature) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(SERVER_NAME);
    http.addHeader("Content-Type", "application/json");

    // Create JSON payload
    String jsonPayload = "{\"distance\": " + String(distance) +
                         ", \"flux\": " + String(flux) +
                         ", \"temperature\": " + String(temperature) + "}";

    // Send POST request
    int httpResponseCode = http.POST(jsonPayload);
    if (httpResponseCode > 0) {
      Serial.print("Server response: ");
      Serial.println(http.getString());
    } else {
      Serial.print("Error sending data: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi not connected. Skipping data send.");
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  connectWiFi();

  // Take measurement
  if (tflI2C.getData(tfDist, tfFlux, tfTemp, tfAddr)) {
    Serial.println("Measurement taken successfully!");
    sendToServer(tfDist, tfFlux, tfTemp);
  } else {
    Serial.println("Failed to read sensor data.");
    tflI2C.printStatus();
  }

  // Prepare for deep sleep
  Serial.println("Going to sleep for 1 hour...");
  esp_sleep_enable_timer_wakeup(SLEEP_DURATION_US);
  esp_deep_sleep_start();
}

void loop() {
  // Not used; ESP32 is in deep sleep between wakeups
}
