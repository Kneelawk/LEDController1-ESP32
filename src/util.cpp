//
// Created by jedidiah on 9/15/21.
//

#include "util.h"

namespace util {

char *nullTerminate(uint8_t *data, size_t len) {
    char *dataStr = new char[len + 1];
    memcpy(dataStr, data, len);
    dataStr[len] = 0;

    return dataStr;
}

char *truncate(const char *data, size_t len) {
    char *dataStr = new char[len + 1];
    memcpy(dataStr, data, len);
    dataStr[len] = 0;

    return dataStr;
}

}
