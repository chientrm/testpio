#include "ota_update.h"
#include "config.h"
#include <ArduinoOTA.h>
#include <FastLED.h>

// OTA State Variables
bool otaInProgress = false;
String otaStatus = "Ready";

void setupOTA()
{
    // Set hostname for easier identification
    ArduinoOTA.setHostname(DEVICE_NAME.c_str());

    ArduinoOTA.onStart([]()
                       {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }
    
    otaInProgress = true;
    otaStatus = "Starting " + type + " update...";
    Serial.println("OTA Update Starting: " + type);
    
    // Turn off LED strip during update to save power and avoid conflicts
    FastLED.clear();
    FastLED.show();
    
    // Turn on built-in LED to indicate OTA in progress
    digitalWrite(BUILTIN_LED_PIN, HIGH); });

    ArduinoOTA.onEnd([]()
                     {
    otaInProgress = false;
    otaStatus = "Update complete! Restarting...";
    Serial.println("\nOTA Update Complete");
    digitalWrite(BUILTIN_LED_PIN, LOW); });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                          {
    int percent = (progress / (total / 100));
    otaStatus = "Progress: " + String(percent) + "%";
    Serial.printf("OTA Progress: %u%%\r", percent);
    
    // Blink built-in LED to show progress
    digitalWrite(BUILTIN_LED_PIN, (percent % 10 < 5) ? HIGH : LOW); });

    ArduinoOTA.onError([](ota_error_t error)
                       {
    otaInProgress = false;
    otaStatus = "Update failed: ";
    Serial.printf("OTA Error[%u]: ", error);
    
    if (error == OTA_AUTH_ERROR) {
      otaStatus += "Auth Failed";
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      otaStatus += "Begin Failed";
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      otaStatus += "Connect Failed";
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      otaStatus += "Receive Failed";
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      otaStatus += "End Failed";
      Serial.println("End Failed");
    }
    
    digitalWrite(BUILTIN_LED_PIN, LOW); });

    ArduinoOTA.begin();
    Serial.println("OTA update service started");
    Serial.println("Device hostname: " + String(DEVICE_NAME));
}
