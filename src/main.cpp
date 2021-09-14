#include <Arduino.h>
#include <FastLED.h>
#include <ESPAsyncWebServer.h>

#include "broadcaster.h"

const int NUM_LEDS = 150;
const int DATA_PIN = 4;

const char *SSID = "Pommert";
const char *PASSWORD = "HuwaWaHaya";

const uint8_t DEFAULT_BRIGHTNESS = 96;
const uint8_t DEFAULT_SATURATION = 240;

CRGB leds[NUM_LEDS];
uint8_t hue = 0;
uint8_t brightness = DEFAULT_BRIGHTNESS;

AsyncWebServer server(80);

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

void setup() {
    CFastLED::addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);

    // Turn all the LEDS off
    fill_solid(leds, NUM_LEDS, CRGB{});
    FastLED.show();

    // Connect to the WiFi
    WiFi.begin(SSID, PASSWORD);

    // Show a status indicator (moving LED) while we connect
    size_t index = 0, prevIndex = 0;
    while (WiFiClass::status() != WL_CONNECTED) {
        leds[prevIndex] = CRGB{};
        leds[index] = CRGB{ 0, 0, 255 };
        FastLED.show();

        prevIndex = index;
        index = (index + 1) % NUM_LEDS;

        delay(500);
    }

    // Show a status that we've connected
    fill_solid(leds, NUM_LEDS, CRGB{ 0, 255, 0 });
    FastLED.show();
    delay(1000);

    broadcaster::setup();

    // Setup brightness GET endpoint
    server.on("/brightness", HTTP_GET, [](AsyncWebServerRequest *request) {
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
    server.addHandler(brightnessPut);

    // Start the server listening
    server.begin();
}

void loop() {
    custom_fill_rainbow(leds, NUM_LEDS, hue, 3, brightness,
                        DEFAULT_SATURATION);
    FastLED.show();

    hue++;

    broadcaster::update();

    delay(10);
}
