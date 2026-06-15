#include <Arduino.h>
#include <WiFi.h>
#include "Timers.h"
#include <LinkedList.h>
#include "nanomodbus.h"
#include "modby.h"

WiFiServer server(502);
String sta_SSID = "Romero";
String sta_PASS = "Francina0602";

void setup() {
    Serial.begin(9600);
    WiFi.begin(sta_SSID.c_str(), sta_PASS.c_str());

    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\nConnected IP: " + WiFi.localIP().toString());
    server.begin();
}

void loop() {
    WiFiClient client = server.available();

    if (client) {
        Serial.println("\n¡Maestro Modbus conectado!");
        
        // IMPORTANTE
        handle_modbus_client(&client);
        
        client.stop();
        Serial.println("¡Maestro Modbus desconectado!");
    }
}