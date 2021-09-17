//
// Created by jedidiah on 9/15/21.
//

#include "storage.h"

#include <nvs.h>

#include "util.h"
#include "broadcaster.h"

namespace storage {

const char *STORAGE_NAMESPACE = "ESPLEDS";
const char *NAME_KEY = "name";

const char *DEFAULT_NAME = "";
const size_t MAX_NAME_SIZE = 32;

nvs_handle handle;
bool nameError = false;
char *name = nullptr;
char *putName = nullptr;

void setup() {
    esp_err_t res = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (res != ESP_OK) {
        // Unable to get a partition, so we just do a stop-gap measure
        nameError = true;
        name = new char[strlen(DEFAULT_NAME)];
        strcpy(name, DEFAULT_NAME);

        return;
    }

    // load name
    size_t nameSize = 0;
    res = nvs_get_str(handle, NAME_KEY, nullptr, &nameSize);
    if (res != ESP_OK && res != ESP_ERR_NVS_NOT_FOUND) {
        nameError = true;
    } else {
        if (res == ESP_ERR_NVS_NOT_FOUND) {
            // put a new name that is the default name
            name = new char[strlen(DEFAULT_NAME)];
            strcpy(name, DEFAULT_NAME);
            nvs_set_str(handle, NAME_KEY, name);
        } else {
            // load the existing name
            name = new char[nameSize];
            nvs_get_str(handle, NAME_KEY, name, &nameSize);

            // truncate the existing name if necessary
            if (nameSize > MAX_NAME_SIZE) {
                char *newName = util::truncate(name, MAX_NAME_SIZE);
                delete[] name;
                name = newName;

                nvs_set_str(handle, NAME_KEY, name);
            }
        }
    }

    nvs_commit(handle);
}

void setupServer(AsyncWebServer *server) {
    // Setup name GET endpoint
    server->on("/name", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/plain", getName());
    });

    // Setup name PUT endpoint
    AsyncCallbackWebHandler *namePut = new AsyncCallbackWebHandler();
    namePut->setUri("/name");
    namePut->setMethod(HTTP_PUT);
    namePut->onBody([](AsyncWebServerRequest *request, uint8_t *data,
                       size_t len, size_t index, size_t total) {
        putName = util::nullTerminate(data, len);
    });
    namePut->onRequest([](AsyncWebServerRequest *request) {
        if (putName) {
            setName(putName);
            delete[] putName;
            putName = nullptr;
        } else {
            setName("");
        }

        // update the broadcaster that the name has changed
        broadcaster::rebuildPacket();

        request->send_P(200, "text/plain", getName());
    });
    server->addHandler(namePut);

    // Setup name DELETE endpoint
    server->on("/name", HTTP_DELETE, [](AsyncWebServerRequest *request) {
        setName("");

        // update the broadcaster that the name has changed
        broadcaster::rebuildPacket();

        request->send_P(200, "text/plain", "");
    });
}

const char *getName() {
    return name;
}

void setName(const char *newName) {
    // free the old name
    if (name) {
        delete[] name;
    }

    // copy in the new name
    size_t newLen = strlen(newName);
    if (newLen <= MAX_NAME_SIZE) {
        name = new char[newLen + 1];
        strcpy(name, newName);
    } else {
        name = util::truncate(newName, MAX_NAME_SIZE);
    }

    if (!nameError) {
        nvs_set_str(handle, NAME_KEY, name);
        nvs_commit(handle);
    }
}

}
