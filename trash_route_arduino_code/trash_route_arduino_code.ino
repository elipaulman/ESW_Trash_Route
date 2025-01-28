#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <TFLI2C.h>

// WiFi credentials
const char* SSID = "Registered4OSU";
const char* PASS = "UpcMys375nW5nFxC";

// REST API Endpoint
const char* SERVER_NAME = "https://esw-trash-route.onrender.com/api/trash-data";

// Device Name
const char* DEVICE_NAME = "Thompson-01";

// TFLuna Variables
TFLI2C tflI2C;
int16_t tfDist;  // Distance in centimeters
int16_t tfFlux;  // Signal quality/strength
int16_t tfTemp;  // Chip temperature in hundredths of *C
int16_t tfAddr = TFL_DEF_ADR; // Default TFLuna address

// Sleep duration: 1 hour (in microseconds)
// 1 hour: 60ULL * 60 * 1000000
// currently set to 1 min
#define SLEEP_DURATION_US (60ULL * 1000000)

// Function to print the reset reason
void printResetReason() {
  esp_reset_reason_t reset_reason = esp_reset_reason();
  Serial.print("Reset reason: ");
  Serial.println(reset_reason);
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  WiFi.begin(SSID, PASS);
  Serial.print("Connecting to WiFi");
  unsigned long startAttemptTime = millis();
  
  // Timeout after 10 seconds
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    Serial.print(".");
    delay(500);  // Feed the watchdog
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" Connected!");
  } else {
    Serial.println(" WiFi connection failed!");
  }
}

void sendToServer(int16_t distance, int16_t flux, int16_t temperature) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(SERVER_NAME);
    http.addHeader("Content-Type", "application/json");

    // Create JSON payload
    String jsonPayload = String("{\"name\": \"") + DEVICE_NAME +
                         "\", \"distance\": " + String(distance) +
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

  // Print the reset reason for debugging
  printResetReason();

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
  Serial.println("Going to sleep for 1 minute...");
  esp_sleep_enable_timer_wakeup(SLEEP_DURATION_US);
  esp_deep_sleep_start();
}

void loop() {
  // Not used; ESP32 is in deep sleep between wakeups
}
