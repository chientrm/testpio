#pragma once
#include <Arduino.h>

// Device Configuration
#define LED_PIN 23
#define NUM_LEDS 60
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define BRIGHTNESS 128
#define BUILTIN_LED_PIN 2

// Serial Configuration
#define SERIAL_TIMEOUT 30000 // 30 seconds

// Auto-Update Configuration
#define UPDATE_CHECK_INTERVAL 3600000 // Check every hour (3600000ms)

// Device Info
extern const String DEVICE_NAME;
extern const String FIRMWARE_VERSION;

// Auto-Update URLs
extern const String FIRMWARE_UPDATE_URL;
extern const String FIRMWARE_BINARY_URL_BASE;
