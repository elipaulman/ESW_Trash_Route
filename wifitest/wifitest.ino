#include <WiFi.h>

const char* SSID = "Registered4OSU";
const char* PASS = "UpcMys375nW5nFxC";

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
  
}

void loop() {
  Serial.print("Registered4OSU RSSI: ")
  Serial.print(WiFi.RSSI());
  Serial.print("\n");
}
