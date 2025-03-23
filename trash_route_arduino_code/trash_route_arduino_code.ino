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
int16_t tfDist;  // Distance in centimeters
int16_t tfFlux;  // Signal quality/strength
int16_t tfTemp;  // Chip temperature in hundredths of *C
const uint8_t TF_LUNA_ADDR = 0x10; // Default TFLuna address

// I2C Configuration (ESP32-C3 GPIO Pins)
#define I2C_SDA 4
#define I2C_SCL 5

// Sleep duration: 6 hours (in microseconds)
#define SLEEP_DURATION_US (6ULL * 60 * 60 * 1000000)

// Maximum measurement attempts before giving up
#define MAX_MEASUREMENT_ATTEMPTS 3
// Delay between measurement attempts (in milliseconds)
#define RETRY_DELAY 1000

// Static WiFi client to prevent memory fragmentation
WiFiClientSecure client;

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  WiFi.begin(SSID, PASS);
  Serial.print("Connecting to WiFi");
  
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
    Serial.print(".");
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" Connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println(" WiFi connection failed!");
  }
}

void sendToServer(int16_t distance, int16_t flux, int16_t temperature) {
  if (WiFi.status() == WL_CONNECTED) {
    client.setInsecure(); // Skip certificate verification
    HTTPClient http;
    http.begin(client, SERVER_NAME);
    
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(10000); // 10-second timeout
    http.addHeader("Content-Type", "application/json");

    String jsonPayload = String("{\"name\": \"") + DEVICE_NAME +
                         "\", \"distance\": " + String(distance) +
                         ", \"flux\": " + String(flux) +
                         ", \"temperature\": " + String(temperature) + "}";

    Serial.println("Sending payload: " + jsonPayload);
    
    int httpResponseCode = http.POST(jsonPayload);
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error sending data: ");
      Serial.println(http.errorToString(httpResponseCode));
    }

    http.end();
  } else {
    Serial.println("WiFi not connected. Skipping data send.");
  }
}

void scanI2C() {
  Serial.println("Scanning I2C devices...");
  byte count = 0;
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("Found device at 0x%02X\n", addr);
      count++;
      delay(1);
    }
  }
  Serial.printf("%d I2C devices found\n", count);
}

bool takeMeasurement() {
  for (int attempt = 1; attempt <= MAX_MEASUREMENT_ATTEMPTS; attempt++) {
    Serial.printf("Measurement attempt %d/%d\n", attempt, MAX_MEASUREMENT_ATTEMPTS);
    
    if (tflI2C.getData(tfDist, tfFlux, tfTemp, TF_LUNA_ADDR)) {
      Serial.printf("Distance: %dcm, Flux: %d, Temp: %.2f°C\n", 
                   tfDist, tfFlux, tfTemp / 100.0);
      return true;
    } else {
      Serial.println("Measurement failed.");
      delay(RETRY_DELAY);
    }
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("\n\n--- ESP32 Trash Route Monitor ---");

  Wire.begin(I2C_SDA, I2C_SCL);
  delay(500);
  
  scanI2C();

  if (takeMeasurement()) {
    connectWiFi();
    sendToServer(tfDist, tfFlux, tfTemp);
  } else {
    Serial.println("Skipping data transmission due to failed measurements");
  }

  Serial.println("Entering deep sleep...");
  delay(100);  // Ensure serial output finishes
  esp_sleep_enable_timer_wakeup(SLEEP_DURATION_US);
  esp_deep_sleep_start();
}

void loop() {
  // Not used; ESP32 is in deep sleep between wakeups
}
