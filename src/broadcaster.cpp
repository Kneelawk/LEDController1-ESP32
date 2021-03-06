//
// Created by jedidiah on 9/14/21.
//

#include "broadcaster.h"

#include <Arduino.h>
#include <AsyncUDP.h>
#include <WiFi.h>

#include "storage.h"

namespace broadcaster {

const uint32_t DURATION_PER_UPDATE = 1000;
const uint16_t UDP_PORT = 12888;
const char *UDP_PREFIX = "ESPLEDS";

uint8_t *broadcastPacket = nullptr;
size_t broadcastPacketLen;
uint32_t lastUpdate = 0;

AsyncUDP udp;

void rebuildPacket() {
    if (broadcastPacket) {
        delete[] broadcastPacket;
    }

    // Create broadcast string: <prefix> <binary-length> <ip> | <name>
    String message = WiFi.localIP().toString() + "|" + storage::getName();
    auto messageLen = (uint8_t) message.length();
    size_t prefixLen = strlen(UDP_PREFIX);
    broadcastPacket = new uint8_t[prefixLen + 1 + messageLen + 1];
    memcpy(broadcastPacket, UDP_PREFIX, prefixLen);
    broadcastPacket[prefixLen] = messageLen;
    memcpy(broadcastPacket + prefixLen + 1, message.c_str(), messageLen);
    broadcastPacketLen = prefixLen + 1 + messageLen;
}

void setup() {
    rebuildPacket();

    // Connect to a UDP port for broadcasting
    udp.connect(IPAddress{ 192, 168, 1, 255 }, UDP_PORT);
}

void update() {
    uint32_t now = millis();

    if (now < lastUpdate || now >= lastUpdate + DURATION_PER_UPDATE) {
        lastUpdate = now;
        udp.broadcastTo(broadcastPacket, broadcastPacketLen, UDP_PORT);
    }
}

}
