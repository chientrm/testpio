#pragma once
#include <Arduino.h>

// Auto-Update Functions
void checkForFirmwareUpdate();
void performAutoUpdate(String firmwareUrl);
void handleAutoUpdate();

// Auto-Update State Variables
extern unsigned long lastUpdateCheck;
extern bool autoUpdateEnabled;
extern bool updateInProgress;
extern String latestVersion;
extern String updateStatus;
