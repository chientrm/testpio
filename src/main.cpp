#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>

// Project Headers
#include "config.h"
#include "led_control.h"
#include "serial_control.h"
#include "ota_update.h"
#include "auto_update.h"
#include "web_server.h"
#include "wifi_credentials.h"

void setup()
{
  // Initialize serial communication
  initializeSerial();

  // Initialize LED strip and built-in LED
  initializeLEDs();
  Serial.println("LED strip initialized (60 LEDs)");

  // WiFi Connection
  Serial.println("\n=== Attempting WiFi Connection ===");
  Serial.println("Note: WiFi is optional - USB serial control always available");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30)
  {
    delay(1000);
    Serial.print(".");
    attempts++;

    if (attempts % 10 == 0)
    {
      Serial.println();
      Serial.println("Retrying WiFi connection...");
      WiFi.disconnect();
      delay(1000);
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("");
    Serial.println("=== WiFi connected successfully! ===");
    Serial.print("IPv4 address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal strength (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

    // Initialize mDNS
    if (MDNS.begin(DEVICE_NAME.c_str()))
    {
      Serial.println("mDNS responder started: " + DEVICE_NAME + ".local");
      MDNS.addService("http", "tcp", 80);
    }

    // Setup OTA and Auto-updates
    setupOTA();

    Serial.println("=================================");
    Serial.println("🌐 Open your browser and go to:");
    Serial.print("   http://");
    Serial.println(WiFi.localIP());
    Serial.println("=================================");
  }
  else
  {
    Serial.println("");
    Serial.println("=== WiFi connection failed! ===");
    Serial.println("Continuing with USB serial control only...");
  }

  // Initialize web server
  initializeWebServer();

  // Print available commands
  Serial.println("\n=== USB Serial Control Ready ===");
  Serial.println("Available commands:");
  Serial.println("- ping (test connection)");
  Serial.println("- off, solid, rainbow, visualizer");
  Serial.println("- red, green, blue, yellow, white");
  Serial.println("- ledon, ledoff, toggle, status, info");
  Serial.println("- brightness:0-255");
  Serial.println("- music:data (for real-time music sync)");
  Serial.println("- update:check, update:enable, update:disable, update:now");
  Serial.println("=================================");

  Serial.println("RESPONSE:READY");
  lastSerialActivity = millis();
}

void loop()
{
  // Handle OTA updates (highest priority)
  if (WiFi.status() == WL_CONNECTED)
  {
    ArduinoOTA.handle();
  }

  // Handle auto-updates (check periodically)
  if (WiFi.status() == WL_CONNECTED && !updateInProgress && !otaInProgress)
  {
    handleAutoUpdate();
  }

  // Check for USB serial commands
  checkSerialInput();

  // Handle web server requests
  server.handleClient();

  // Update LED strip (skip during updates)
  if (!otaInProgress && !updateInProgress)
  {
    handleLedStrip();
  }

  // Send periodic heartbeat if USB connected
  static unsigned long lastHeartbeat = 0;
  if (serialConnected && (millis() - lastHeartbeat) > 10000)
  {
    Serial.println("RESPONSE:HEARTBEAT");
    lastHeartbeat = millis();
  }

  // Small delay to prevent watchdog issues
  delay(10);
}
