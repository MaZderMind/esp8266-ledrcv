#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <config.hpp>

WiFiUDP Udp;

uint8_t packetSize = 0;
uint8_t packetBuffer[0x0FF];

static const int16_t ledPin = 13;
static const int16_t indicatorPin = 2;

Adafruit_NeoPixel pixels =
    Adafruit_NeoPixel(numLeds, ledPin, NEO_GRB + NEO_KHZ800);

static void setupIndicator() {
  pinMode(indicatorPin, OUTPUT);
  digitalWrite(indicatorPin, LOW);
}

static void toggleIndicator() {
  static bool indicatorPinState = false;
  digitalWrite(indicatorPin, indicatorPinState);
  indicatorPinState = !indicatorPinState;
}

static void constructHostname(const char *prefix, char *hostnameBuffer,
                              size_t bufferSize) {
  strlcpy(hostnameBuffer, prefix, bufferSize);
  uint32_t chipId = ESP.getChipId();

  char chipIdAsHex[10];
  snprintf(chipIdAsHex, sizeof(chipIdAsHex), "%0X", chipId);

  strlcat(hostnameBuffer, " ", bufferSize);
  strlcat(hostnameBuffer, chipIdAsHex, bufferSize);
}

void setupWifi() {
  char hostname[32];
  constructHostname(devicename, hostname, sizeof(hostname));
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
    toggleIndicator();
    Serial.print(F("Not connected, trying again\n"));
    delay(500);
  }
  Serial.print(F("Connected!\n"));
  Serial.print(F("IP: "));
  Serial.println(WiFi.localIP());
}

void setupUdpSocket() {
  Serial.print(F("Listening for Datagrams on Port "));
  Serial.println(udpPort);
  Udp.begin(udpPort);
}

bool readUdpPacket() {
  packetSize = Udp.parsePacket();
  if (packetSize > 0) {
    if (debugCommunication) {
      Serial.print(F("Received Datagram of "));
      Serial.print(packetSize);
      Serial.println(F(" bytes"));
    }

    // read the packet into packetBufffer
    packetSize = Udp.read(packetBuffer, sizeof(packetBuffer));

    if (debugCommunication) {
      Serial.print(F("Contents:"));
      for (int i = 0; i < packetSize; i++) {
        Serial.printf(" %02X", packetBuffer[i]);
      }
      Serial.println(F("."));
    }
  }

  return packetSize > 0;
}

void setColors() {
  uint8_t numPixelsInPacket = (packetSize - 1) / 3;
  uint8_t numPixels = numPixelsInPacket < numLeds ? numPixelsInPacket : numLeds;
  if (debugCommunication) {
    Serial.printf("numPixelsInPacket = %u\n", numPixelsInPacket);
    Serial.printf("numLeds = %u\n", numLeds);
    Serial.printf("numPixels = %u\n", numPixels);
  }

  // TODO check if buffer size does match
  for (size_t i = 0; i < numPixels; i++) {
    uint8_t offset = i * 3 + 1;
    if (debugCommunication) {
      Serial.printf("Pixel %u\n", i);
      Serial.printf("  Offset = %u\n", offset);
    }

    uint8_t r = packetBuffer[offset + 0];
    uint8_t g = packetBuffer[offset + 1];
    uint8_t b = packetBuffer[offset + 2];

    if (debugCommunication) {
      Serial.printf("  RGB %X %X %X\n", r, g, b);
    }
    pixels.setPixelColor(i, r, g, b);
  }
}

void processCommand() {
  Serial.print(F("Executing Command "));
  Serial.printf("%02X\n", packetBuffer[0]);
  switch (packetBuffer[0]) {
  case 0x00: // Black Out
    Serial.println(F("Black Out"));
    pixels.clear();
    pixels.show();
    break;

  case 0x01: // Set Colors
    Serial.println(F("Set Colors"));
    setColors();
    break;

  case 0x11: // Set Colors Immediate
    Serial.println(F("Set Colors Immediate"));
    setColors();
    pixels.show();
    break;

  case 0xFF: // Show
    Serial.println(F("Show"));
    pixels.show();
    break;
  }
}

void setupLeds() {
  Serial.print(F("Setting up LED-Strip"));
  pixels.begin();
}

void setup() {
  Serial.begin(115200);
  Serial.print(F("\n\n\n"));

  setupIndicator();
  setupLeds();
  setupWifi();
  setupUdpSocket();
}

void loop() {
  if (readUdpPacket()) {
    toggleIndicator();
    processCommand();
  }
}
