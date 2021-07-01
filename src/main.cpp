#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <config.hpp>

WiFiUDP Udp;

static void constructHostname(const char *prefix, char *hostnameBuffer,
                              size_t bufferSize) {
  strlcpy(hostnameBuffer, prefix, bufferSize);
  uint32_t chipId = ESP.getChipId();

  char chipIdAsHex[10];
  snprintf(chipIdAsHex, sizeof(chipIdAsHex), "%0X", chipId);

  strlcat(hostnameBuffer, " ", bufferSize);
  strlcat(hostnameBuffer, chipIdAsHex, bufferSize);
}

void setup() {
  Serial.begin(115200);
  Serial.print(F("\n\n\n"));

  char hostname[32];
  constructHostname(basename, hostname, sizeof(hostname));
  Serial.print(F("Hostname:"));
  Serial.println(hostname);
  Serial.print(F("MAC: "));
  Serial.println(WiFi.macAddress());
  Serial.print(F("Connecting to Wifi "));
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("Not connected, trying again\n"));
  }
  Serial.print(F("Connected!\n"));
  Serial.print(F("IP: "));
  Serial.println(WiFi.localIP());

  Serial.print(F("Listening for Datagrams on Port "));
  Serial.println(udpPort);
  Udp.begin(udpPort);
}

void loop() {
  uint8_t packetBuffer[0xFFFF];

  int packetSize = Udp.parsePacket();
  if (packetSize > 0) {
    Serial.print(F("Received Datagram of"));
    Serial.print(packetSize);
    Serial.println(F(" bytes"));

    // read the packet into packetBufffer
    packetBuffer[0] = 0x00;
    int len = Udp.read(packetBuffer, sizeof(packetBuffer));
    if (len >= 0) {
      packetBuffer[len] = 0x00;
    }

    Serial.print(F("Contents:"));
    for (int i = 0; i < len; i++) {
      Serial.printf(" %02X", packetBuffer[i]);
    }
    Serial.println(F("."));
  }
}
