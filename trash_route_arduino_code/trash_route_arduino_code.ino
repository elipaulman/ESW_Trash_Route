#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
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
int16_t tfDist;
int16_t tfFlux;
int16_t tfTemp;
const uint8_t TF_LUNA_ADDR = 0x10; // Default TFLuna address

// I2C Configuration (ESP32-C3 GPIO Pins)
#define I2C_SDA 4
#define I2C_SCL 5

// Sleep duration: 1 hour (in microseconds)
#define SLEEP_DURATION_US (1ULL * 60 * 60 * 1000000)

// Maximum measurement attempts
#define MAX_MEASUREMENT_ATTEMPTS 3
#define RETRY_DELAY 1000 // ms

WiFiClientSecure client;

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true); // Reset WiFi state
  delay(100);

  Serial.print("Connecting to WiFi");
  WiFi.begin(SSID, PASS);
  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) {
    Serial.print(".");
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" Connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[ERROR] WiFi connection failed after 15 seconds.");
  }
}

void sendToServer(int16_t distance, int16_t flux, int16_t temperature) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[ERROR] WiFi not connected. Data not sent.");
    return;
  }

  client.setInsecure(); // Skip SSL validation (use setCACert() for production)

  HTTPClient http;
  http.setTimeout(20000);  // 20 seconds to wait for slow servers (e.g., Render cold start)
  http.setReuse(false);    // Close connection after request
  String payload = String("{\"name\": \"") + DEVICE_NAME +
                   "\", \"distance\": " + String(distance) +
                   ", \"flux\": " + String(flux) +
                   ", \"temperature\": " + String(temperature) + "}";

  Serial.println("Sending payload: " + payload);

  http.begin(client, SERVER_NAME);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(payload);

  if (httpCode > 0) {
    Serial.printf("HTTP Response code: %d\n", httpCode);
  } else {
    Serial.printf("[ERROR] HTTP POST failed: %s\n", http.errorToString(httpCode).c_str());

    // Retry once after delay if it's a timeout
    if (httpCode == HTTPC_ERROR_READ_TIMEOUT) {
      Serial.println("[INFO] Retrying POST after 5 seconds...");
      delay(5000);
      http.end();
      http.begin(client, SERVER_NAME);
      http.addHeader("Content-Type", "application/json");
      httpCode = http.POST(payload);

      if (httpCode > 0) {
        Serial.printf("[Retry] HTTP Response code: %d\n", httpCode);
      } else {
        Serial.printf("[Retry ERROR] POST failed: %s\n", http.errorToString(httpCode).c_str());
      }
    }
  }

  http.end();
}

void scanI2C() {
  Serial.println("Scanning I2C...");
  byte found = 0;
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("I2C device at 0x%02X\n", addr);
      found++;
    }
  }
  Serial.printf("I2C scan complete. Devices found: %d\n", found);
}

bool takeMeasurement() {
  for (int i = 1; i <= MAX_MEASUREMENT_ATTEMPTS; i++) {
    Serial.printf("Measurement attempt %d/%d...\n", i, MAX_MEASUREMENT_ATTEMPTS);

    if (tflI2C.getData(tfDist, tfFlux, tfTemp, TF_LUNA_ADDR)) {
      Serial.printf("Measurement successful: %d cm, Flux: %d, Temp: %.2f°C\n", tfDist, tfFlux, tfTemp / 100.0);
      return true;
    }

    Serial.println("[WARNING] Measurement failed. Retrying...");
    delay(RETRY_DELAY);
  }

  Serial.println("[ERROR] All measurement attempts failed.");
  return false;
}

void enterDeepSleep() {
  Serial.println("Entering deep sleep...");
  delay(100);
  esp_sleep_enable_timer_wakeup(SLEEP_DURATION_US);
  esp_deep_sleep_start();
}

void setup() {
  Serial.begin(115200);
  delay(300);

  Serial.println("\n\n=== ESP32 Trash Monitor Boot ===");

  Wire.begin(I2C_SDA, I2C_SCL);
  delay(200);
  scanI2C();

  bool success = false;

  // Main logic with graceful failure handling
  do {
    if (!takeMeasurement()) break;
    connectWiFi();
    sendToServer(tfDist, tfFlux, tfTemp);
    success = true;
  } while (false);

  if (!success) {
    Serial.println("[ERROR] Critical failure occurred. Retrying after sleep.");
  }

  enterDeepSleep();
}

void loop() {
  // Not used; ESP32 sleeps between runs
}
