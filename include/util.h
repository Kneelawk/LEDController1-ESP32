//
// Created by jedidiah on 9/15/21.
//

#ifndef NODEMCUWS2815_UTIL_H
#define NODEMCUWS2815_UTIL_H

#include <Arduino.h>

namespace util {

char *nullTerminate(uint8_t *data, size_t len);
char *truncate(const char *data, size_t len);

}

#endif //NODEMCUWS2815_UTIL_H
