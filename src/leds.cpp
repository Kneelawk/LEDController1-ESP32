//
// Created by jedidiah on 9/14/21.
//

#include "leds.h"

#include <Arduino.h>

#include "util.h"

namespace leds {

const int DATA_PIN = 4;

const uint8_t DEFAULT_BRIGHTNESS = 96;
const uint32_t DEFAULT_FRAME_DURATION = 20;
const uint8_t DEFAULT_HUE_PER_FRAME = 1;
const uint8_t DEFAULT_HUE_PER_PIXEL = 3;
const uint8_t DEFAULT_SATURATION = 240;

uint8_t hue = 0;
CRGB leds[NUM_LEDS];

uint8_t brightness = DEFAULT_BRIGHTNESS;

uint8_t huePerFrame = DEFAULT_HUE_PER_FRAME;

uint8_t huePerPixel = DEFAULT_HUE_PER_PIXEL;

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

void fill_every_n(struct CRGB *pFirstLED, int len, int stride, CRGB color) {
    for (int i = 0; i < len; i += stride) {
        pFirstLED[i] = color;
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
    ArRequestHandlerFunction brightnessGet = [](
            AsyncWebServerRequest *request) {
        String brightnessStr = String(brightness);
        request->send_P(200, "text/plain", brightnessStr.c_str());
    };
    server->on("/brightness", HTTP_GET, brightnessGet);

    // Setup brightness PUT endpoint
    AsyncCallbackWebHandler *brightnessPut = new AsyncCallbackWebHandler();
    brightnessPut->setUri("/brightness");
    brightnessPut->setMethod(HTTP_PUT);
    brightnessPut->onBody([](AsyncWebServerRequest *request, uint8_t *data,
                             size_t len, size_t index, size_t total) {
        // Construct a null-terminated string out of the data
        char *dataStr = util::nullTerminate(data, len);

        // Convert data string to a number and assign it to brightness
        brightness = (uint8_t) strtol(dataStr, nullptr, 10);

        delete[] dataStr;
    });
    brightnessPut->onRequest(brightnessGet);
    server->addHandler(brightnessPut);

    // Setup frame-duration GET endpoint
    ArRequestHandlerFunction frameDurationGet = [](
            AsyncWebServerRequest *request) {
        String frameDurationStr = String(frameDuration);
        request->send_P(200, "text/plain", frameDurationStr.c_str());
    };
    server->on("/frame-duration", HTTP_GET, frameDurationGet);

    // Setup frame-duration PUT endpoint
    AsyncCallbackWebHandler *frameDurationPut = new AsyncCallbackWebHandler();
    frameDurationPut->setUri("/frame-duration");
    frameDurationPut->setMethod(HTTP_PUT);
    frameDurationPut->onBody([](AsyncWebServerRequest *request, uint8_t *data,
                                size_t len, size_t index, size_t total) {
        // Construct a null-terminated string out of the data
        char *dataStr = util::nullTerminate(data, len);

        frameDuration = (uint32_t) strtol(dataStr, nullptr, 10);

        delete[] dataStr;
    });
    frameDurationPut->onRequest(frameDurationGet);
    server->addHandler(frameDurationPut);

    // Setup hue-per-pixel GET endpoint
    ArRequestHandlerFunction huePerPixelGet = [](
            AsyncWebServerRequest *request) {
        String huePerPixelStr = String(huePerPixel);
        request->send_P(200, "text/plain", huePerPixelStr.c_str());
    };
    server->on("/hue-per-pixel", HTTP_GET, huePerPixelGet);

    // Setup hue-per-pixel PUT endpoint
    AsyncCallbackWebHandler *huePerPixelPut = new AsyncCallbackWebHandler();
    huePerPixelPut->setUri("/hue-per-pixel");
    huePerPixelPut->setMethod(HTTP_PUT);
    huePerPixelPut->onBody([](AsyncWebServerRequest *request, uint8_t *data,
                              size_t len, size_t index, size_t total) {
        char *dataStr = util::nullTerminate(data, len);
        huePerPixel = (uint8_t) strtol(dataStr, nullptr, 10);
        delete[] dataStr;
    });
    huePerPixelPut->onRequest(huePerPixelGet);
    server->addHandler(huePerPixelPut);

    // Setup hue-per-frame GET endpoint
    ArRequestHandlerFunction huePerFrameGet = [](
            AsyncWebServerRequest *request) {
        String huePerFrameStr = String(huePerFrame);
        request->send_P(200, "text/plain", huePerFrameStr.c_str());
    };
    server->on("/hue-per-frame", HTTP_GET, huePerFrameGet);

    // Setup hue-per-frame PUT endpoint
    AsyncCallbackWebHandler *huePerFramePut = new AsyncCallbackWebHandler();
    huePerFramePut->setUri("/hue-per-frame");
    huePerFramePut->setMethod(HTTP_PUT);
    huePerFramePut->onBody([](AsyncWebServerRequest *request, uint8_t *data,
                              size_t len, size_t index, size_t total) {
        char *dataStr = util::nullTerminate(data, len);
        huePerFrame = (uint8_t) strtol(dataStr, nullptr, 10);
        delete[] dataStr;
    });
    huePerFramePut->onRequest(huePerFrameGet);
    server->addHandler(huePerFramePut);
}

void update() {
    uint32_t now = millis();
    if (now < lastFrame || now >= lastFrame + frameDuration) {
        lastFrame = now;

        custom_fill_rainbow(leds, NUM_LEDS, hue, huePerPixel, brightness,
                            DEFAULT_SATURATION);

        if (WiFiClass::status() != WL_CONNECTED) {
            fill_every_n(leds, NUM_LEDS, 5, CRGB{ 0, 0, brightness });
        }

        FastLED.show();

        hue += huePerFrame;
    }
}

}
