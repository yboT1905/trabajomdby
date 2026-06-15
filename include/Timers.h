#pragma once
#include <Arduino.h>
#include "td3_timer.h"

//wrapper fuction 

uint32_t arduino_millis();
uint32_t arduino_micros();

//C
//typedef mcu::Timer<uint32_t,1,1000,arduino_millis> Tim32_ms;

//C++
using Tim32_ms = mcu::Timer<uint32_t,1,1000,    arduino_millis>;
using Tim32_us = mcu::Timer<uint32_t,1,1000000, arduino_micros>;
//dt = num/den seconds
