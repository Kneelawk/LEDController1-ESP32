#include <Arduino.h>
#include <FastLED.h>
#include <ESPAsyncWebServer.h>
#include <AsyncUDP.h>

const int NUM_LEDS = 150;
const int DATA_PIN = 4;
const uint16_t UDP_PORT = 12888;
const char *UDP_PREFIX = "ESPLEDS";

const char *SSID = "Pommert";
const char *PASSWORD = "HuwaWaHaya";

const uint8_t DEFAULT_BRIGHTNESS = 96;
const uint8_t DEFAULT_SATURATION = 240;

CRGB leds[NUM_LEDS];
uint8_t hue = 0;
uint8_t brightness = DEFAULT_BRIGHTNESS;
uint8_t *broadcastPacket;
size_t broadcastPacketLen;
int pingIndex = 0;

AsyncWebServer server(80);
AsyncUDP udp;

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

    // Create broadcast string: <prefix> <binary-length> <ip> | <name>
    String message = WiFi.localIP().toString() + "|";
    auto messageLen = (uint8_t) message.length();
    size_t prefixLen = strlen(UDP_PREFIX);
    broadcastPacket = new uint8_t[prefixLen + 1 + messageLen + 1];
    memcpy(broadcastPacket, UDP_PREFIX, prefixLen);
    broadcastPacket[prefixLen] = messageLen;
    memcpy(broadcastPacket + prefixLen + 1, message.c_str(), messageLen);
    broadcastPacketLen = prefixLen + 1 + messageLen;

    // Connect to a UDP port for broadcasting
    udp.connect(IPAddress{ 192, 168, 1, 255 }, UDP_PORT);

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

    pingIndex++;
    if (pingIndex >= 5) {
        pingIndex = 0;
        udp.broadcastTo(broadcastPacket, broadcastPacketLen, UDP_PORT);
    }

    delay(10);
}
