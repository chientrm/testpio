#pragma once
#include <WebServer.h>

// Web Server Functions
void initializeWebServer();
void handleRoot();
void handleLedOn();
void handleLedOff();
void handleLedToggle();
void handleStripMode();
void handleStripColor();
void handleAutoUpdateWeb();
void handleMusicData();

// Web Server Instance
extern WebServer server;
