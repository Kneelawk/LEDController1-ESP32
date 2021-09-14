#include <Arduino.h>
#include <FastLED.h>
#include <ESPAsyncWebServer.h>

#include "broadcaster.h"
#include "leds.h"

const char *SSID = "Pommert";
const char *PASSWORD = "HuwaWaHaya";

AsyncWebServer server(80);

void setup() {
    leds::setupLeds();

    // Connect to the WiFi
    WiFi.begin(SSID, PASSWORD);

    // Show a status indicator (moving LED) while we connect
    size_t index = 0, prevIndex = 0;
    while (WiFiClass::status() != WL_CONNECTED) {
        leds::leds[prevIndex] = CRGB{};
        leds::leds[index] = CRGB{ 0, 0, 255 };
        FastLED.show();

        prevIndex = index;
        index = (index + 1) % leds::NUM_LEDS;

        delay(500);
    }

    // Show a status that we've connected
    fill_solid(leds::leds, leds::NUM_LEDS, CRGB{ 0, 255, 0 });
    FastLED.show();
    delay(1000);

    broadcaster::setup();

    leds::setupServer(&server);

    // Start the server listening
    server.begin();
}

void loop() {
    leds::update();
    broadcaster::update();
}
