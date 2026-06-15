#ifndef MODBY_H
#define MODBY_H

#include <Arduino.h>
#include <WiFi.h>
#include "nanomodbus.h"

// Memoria compartida
extern uint16_t holding_registers[1000];
extern bool coils[256];

// Declaración de la función para que el main.cpp la encuentre
void handle_modbus_client(WiFiClient* client);

#endif