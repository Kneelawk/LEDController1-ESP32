//
// Created by jedidiah on 9/15/21.
//

#ifndef NODEMCUWS2815_STORAGE_H
#define NODEMCUWS2815_STORAGE_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

namespace storage {

void setup();

void setupServer(AsyncWebServer *server);

const char *getName();

void setName(const char *newName);

}

#endif //NODEMCUWS2815_STORAGE_H
