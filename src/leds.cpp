//
// Created by jedidiah on 9/14/21.
//

#include <Arduino.h>
#include <FastLED.h>

#include "leds.h"

namespace leds {

const int DATA_PIN = 4;

const uint8_t DEFAULT_BRIGHTNESS = 96;
const uint8_t DEFAULT_SATURATION = 240;
const uint32_t DEFAULT_FRAME_DURATION = 20;

uint8_t brightness = DEFAULT_BRIGHTNESS;
uint8_t hue = 0;
CRGB leds[NUM_LEDS];

uint32_t frameDuration = DEFAULT_FRAME_DURATION;
uint32_t lastFrame = 0;

void custom_fill_rainbow(struct CRGB *pFirstLED, int numToFill,
                         uint8_t initialHue, uint8_t deltaHue,
                         uint8_t brightness, uint8_t saturation) {
    CHSV hsv{};
    hsv.hue = initialHue;
    hsv.val = brightness;
    hsv.sat = saturation;
    for (int i = 0; i < numToFill; ++i) {
        pFirstLED[i] = hsv;
        hsv.hue += deltaHue;
    }
}

void setupLeds() {
    CFastLED::addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);

    // Turn all the LEDS off
    fill_solid(leds, NUM_LEDS, CRGB{});
    FastLED.show();
}

void setupServer(AsyncWebServer *server) {

    // Setup brightness GET endpoint
    server->on("/brightness", HTTP_GET, [](AsyncWebServerRequest *request) {
        String brightnessStr = String(brightness);
        request->send_P(200, "text/plain", brightnessStr.c_str());
    });

    // Setup brightness PUT endpoint
    AsyncCallbackWebHandler *brightnessPut = new AsyncCallbackWebHandler();
    brightnessPut->setUri("/brightness");
    brightnessPut->setMethod(HTTP_PUT);
    brightnessPut->onBody([](AsyncWebServerRequest *request, uint8_t *data,
                             size_t len, size_t index, size_t total) {
        // Construct a null-terminated string out of the data
        char *dataStr = new char[len + 1];
        memcpy(dataStr, data, len);
        dataStr[len] = 0;

        // Convert data string to a number and assign it to brightness
        brightness = atoi(dataStr);

        delete[] dataStr;
    });
    brightnessPut->onRequest([](AsyncWebServerRequest *request) {
        String brightnessStr = String(brightness);
        request->send_P(200, "text/plain", brightnessStr.c_str());
    });
    server->addHandler(brightnessPut);
}

void update() {
    uint32_t now = millis();
    if (now < lastFrame || now >= lastFrame + frameDuration) {
        lastFrame = now;

        custom_fill_rainbow(leds, NUM_LEDS, hue, 3, brightness,
                            DEFAULT_SATURATION);
        FastLED.show();

        hue++;
    }
}

}
