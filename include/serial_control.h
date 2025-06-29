#pragma once
#include <Arduino.h>

// Serial Communication Functions
void initializeSerial();
void processSerialCommand(String command);
void checkSerialInput();

// Serial State Variables
extern String serialBuffer;
extern bool serialCommandReady;
extern unsigned long lastSerialActivity;
extern bool serialConnected;
