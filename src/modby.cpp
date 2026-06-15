#include "modby.h" 
#include <string.h>


uint16_t holding_registers[1000] = {0}; //memoria para holding registers
bool coils[256] = {false};  


//LECTURA DE DATOS
int32_t tcp_read(uint8_t* buf, uint16_t count, int32_t byte_timeout_ms, void* arg) {
    WiFiClient* client = (WiFiClient*)arg;
    uint32_t start = millis();
    uint16_t read_bytes = 0;
    
    while (read_bytes < count) { // count= cantidad de bytes modbus
        if (client->available()) {
            buf[read_bytes++] = client->read();
            start = millis(); //por cada byte recibido reinicio el cronometro
        } else {
            if (byte_timeout_ms > 0 && (millis() - start) > byte_timeout_ms) break; //tiempo limite
            if (!client->connected()) break; // si se desconecta, aborta
            delay(1); 
        }
    }
    return read_bytes; //retorno de lectura para que se validen
}

//ESCRITURA DE DATOS
int32_t tcp_write(const uint8_t* buf, uint16_t count, int32_t byte_timeout_ms, void* arg) {
    WiFiClient* client = (WiFiClient*)arg; //contexto de la conexion
    return client->write(buf, count); //escribo el bloque de memoria completo
}
//si la direccion es mayor a 1000, error
nmbs_error read_holding_registers(uint16_t address, uint16_t quantity, uint16_t* registers_out, uint8_t unit_id, void* arg) {
    if (address + quantity > 1000) return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS; // evito lectura de memoria no asignada
    for (uint16_t i = 0; i < quantity; i++) registers_out[i] = holding_registers[address + i]; // copio datos desde la memoria hacia el buffer de nmb
    return NMBS_ERROR_NONE;
}

nmbs_error write_multiple_registers(uint16_t address, uint16_t quantity, const uint16_t* registers_in, uint8_t unit_id, void* arg) {
    Serial.printf("\n[MULTIPLE] Modbus Poll escribio %d registros en la dir %d\n", quantity, address);
    if (address + quantity > 1000) return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS; // si excede el tamaño del array, rechazo.
    for (uint16_t i = 0; i < quantity; i++) holding_registers[address + i] = registers_in[i];
    return NMBS_ERROR_NONE;
}

nmbs_error write_single_register(uint16_t address, uint16_t value, uint8_t unit_id, void* arg) {
    if (address >= 1000) return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    holding_registers[address] = value;
    return NMBS_ERROR_NONE;
}
 //para coils 01 es leer, 05 escribir uno, 15 escribir varios.
nmbs_error read_coils(uint16_t address, uint16_t quantity, uint8_t* coils_out, uint8_t unit_id, void* arg) {
    if (address + quantity > 256) return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    memset(coils_out, 0, (quantity + 7) / 8); //limpio con 0 donde voy a guardar los datos
    for (uint16_t i = 0; i < quantity; i++) { //8 coils por cada byte para ahorro de ancho de banda
        if (coils[address + i]) coils_out[i / 8] |= (1 << (i % 8));  // OR y desplazamiento
    }
    return NMBS_ERROR_NONE;
}

nmbs_error write_multiple_coils(uint16_t address, uint16_t quantity, const uint8_t* coils_in, uint8_t unit_id, void* arg) {
    Serial.printf("\n[MULTIPLE] Modbus Poll escribio %d Coils en la dir %d\n", quantity, address);
    if (address + quantity > 256) return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    for (uint16_t i = 0; i < quantity; i++) {
        coils[address + i] = (coils_in[i / 8] & (1 << (i % 8))) != 0; //AND para extraer el valor del bit especifico a cambiar
    }
    return NMBS_ERROR_NONE;
}

nmbs_error write_single_coil(uint16_t address, bool value, uint8_t unit_id, void* arg) {
    if (address >= 256) return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    coils[address] = value;
    return NMBS_ERROR_NONE;
}
 //HAL e inversion de control (socket raw a protocolo modbus TCP)
void handle_modbus_client(WiFiClient* client) {
    nmbs_platform_conf platform_conf; // configuro la red
    nmbs_platform_conf_create(&platform_conf);
    platform_conf.transport = NMBS_TRANSPORT_TCP; //selecciona formato de paquete TCP
    platform_conf.read = tcp_read; // dependencia para lectura
    platform_conf.write = tcp_write; //dependencia para escritura 
    platform_conf.arg = client; //cliente del socket

    nmbs_callbacks callbacks; //registro de callbacks, codigo de funcion de c/u a c++
    nmbs_callbacks_create(&callbacks);
    callbacks.read_holding_registers = read_holding_registers;
    callbacks.write_multiple_registers = write_multiple_registers;
    callbacks.write_single_register = write_single_register;
    callbacks.read_coils = read_coils;
    callbacks.write_multiple_coils = write_multiple_coils; 
    callbacks.write_single_coil = write_single_coil; 

    nmbs_t modbus_server; // HAL, callbacks y slave id 1
    nmbs_server_create(&modbus_server, 1, &platform_conf, &callbacks);

    nmbs_set_read_timeout(&modbus_server, 1000); //evito bloqueos del esp
    nmbs_set_byte_timeout(&modbus_server, 1000);

    while (client->connected()) { // polling, escucha la red y despacha callbacks de manera automatica si el frame es valido
        nmbs_error err = nmbs_server_poll(&modbus_server);
        if (err != NMBS_ERROR_NONE && err != NMBS_ERROR_TIMEOUT) break; // se rompe el bucle if error
        delay(1);
    }
}