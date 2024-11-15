#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <TFLI2C.h>

const char* SSID = "Ye Olde Wifi";
//"Registered4OSU";
const char* PASS = "141ENorwichAve!";
//"dSDfe5jvfGVV7yg5";

// REST API Endpoint
const char* SERVER_NAME = "http://192.168.1.77:3000/api/trash-data";
//"https://your-backend-server.com/api/trash-data";

// TFLuna Variables
TFLI2C tflI2C;

// Distance in centimeters
int16_t tfDist;
// Signal quality/strength
int16_t tfFlux;
// Chip temperature in hundredths of *C
int16_t tfTemp;

int16_t tfAddr = TFL_DEF_ADR; // Default TFLuna address

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
  Serial.println("Connected!");
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
}

void loop() {
  // Take measurement
  if (tflI2C.getData(tfDist, tfFlux, tfTemp, tfAddr)) {
    Serial.println("Measurement taken successfully!");
    sendToServer(tfDist, tfFlux, tfTemp);
  } else {
    Serial.println("Failed to read sensor data.");
    tflI2C.printStatus();
  }

  // Wait 30 seconds before taking another measurement
  delay(30000); // 30 seconds
}
