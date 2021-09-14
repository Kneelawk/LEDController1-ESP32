//
// Created by jedidiah on 9/14/21.
//

#ifndef NODEMCUWS2815_LEDS_H
#define NODEMCUWS2815_LEDS_H

#include <ESPAsyncWebServer.h>

namespace leds {

const int NUM_LEDS = 150;

extern CRGB leds[NUM_LEDS];

void setupLeds();

void setupServer(AsyncWebServer *server);

void update();

}

#endif //NODEMCUWS2815_LEDS_H
