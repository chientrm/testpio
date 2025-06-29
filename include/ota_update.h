#pragma once
#include <Arduino.h>

// OTA Update Functions
void setupOTA();

// OTA State Variables
extern bool otaInProgress;
extern String otaStatus;
