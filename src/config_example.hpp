#include <Arduino.h>

static const int16_t udpPort = 5555;

Adafruit_NeoPixel ledStrips[] = {
    /* Switch-Schrank */
    Adafruit_NeoPixel(25, 13, NEO_GRB + NEO_KHZ800),

    /* Fernseher */
    Adafruit_NeoPixel(30, 12, NEO_GRB + NEO_KHZ800)

    /* EOF */
};
static const uint8_t numStrips = sizeof(ledStrips) / sizeof(*ledStrips);

static const bool debugCommunication = false;

static const char devicename[] = "Blinky";
static const char ssid[] = "Your";
static const char password[] = "WLan";
