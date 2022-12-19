#include <WiFi.h>

//ESP WI-FI ACCESS POINT
#define ssid "BEACON_SSID" // place your beacon ssid here
#define password "BEACON_PASSWORD" // place your beacon password here

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_AP);

  WiFi.softAP(ssid, password, 1, 0, 4, true);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  String macAddress = WiFi.macAddress();
  Serial.print("Beacon MacAddress: ");
  Serial.println(macAddress);

  Serial.print("Network SSID: ")
  Serial.println(ssid);

}

void loop() {

}
