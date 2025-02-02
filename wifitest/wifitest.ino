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

char* getStrengthDescriptor(long rssi) {
  if (rssi >= -79) return "Excellent";
  else if (rssi < -79 && rssi >= -89) return "Good";
  else if (rssi < -89 && rssi >= -100) return "Fair";
  else if (rssi < -100 && rssi > -110) return "Poor";
  else if (rssi <= -110) return "No Signal";
}

void setup() {
  Serial.begin(115200);
  connectWifi();
}

void loop() {
  long rssi = WiFi.RSSI();
  Serial.print("Registered4OSU RSSI: ")
  Serial.print(rssi);
  Serial.print("dBm (");
  Serial.print(getStrengthDescriptor(rssi));
  Serial.print(")\n");
  delay(100);
}
