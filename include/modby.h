#ifndef MODBY_H
#define MODBY_H

#include <Arduino.h>
#include <WiFi.h>
#include "nanomodbus.h"

extern uint16_t holding_registers[1000];
extern bool coils[256];

void handle_modbus_client(WiFiClient* client);

#endif