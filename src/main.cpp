#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <FastLED.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <ArduinoJson.h>
#include "wifi_credentials.h"

// LED Strip Configuration
#define LED_PIN 23
#define NUM_LEDS 60
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define BRIGHTNESS 128 // 0-255, start at 50% for battery life

CRGB leds[NUM_LEDS];

// Create web server on port 80
WebServer server(80);

// Built-in LED pin (keep for testing)
const int BUILTIN_LED_PIN = 2;
bool ledState = false;

// LED Strip modes
enum LedMode
{
  MODE_OFF,
  MODE_SOLID,
  MODE_RAINBOW,
  MODE_MUSIC_READY
};

LedMode currentMode = MODE_OFF;
CRGB currentColor = CRGB::Blue;

// USB Serial Command Processing
String serialBuffer = "";
bool serialCommandReady = false;
unsigned long lastSerialActivity = 0;
const unsigned long SERIAL_TIMEOUT = 30000; // 30 seconds timeout
bool serialConnected = false;

// OTA Update variables
bool otaInProgress = false;
String otaStatus = "Ready";

// Auto-Update Configuration
const String FIRMWARE_UPDATE_URL = "https://api.github.com/repos/chientrm/testpio/releases/latest";
const String FIRMWARE_BINARY_URL_BASE = "https://github.com/chientrm/testpio/releases/download/";
unsigned long lastUpdateCheck = 0;
const unsigned long UPDATE_CHECK_INTERVAL = 3600000; // Check every hour (3600000ms)
bool autoUpdateEnabled = true;
bool updateInProgress = false;
String latestVersion = "";
String updateStatus = "Ready";

// Device Info
const String DEVICE_NAME = "ESP32-MusicViz";
const String FIRMWARE_VERSION = "1.3.0";

// Function declarations
void checkForFirmwareUpdate();
void performAutoUpdate(String firmwareUrl);
void handleAutoUpdate();

// LED Strip Control Functions
void setStripColor(CRGB color)
{
  fill_solid(leds, NUM_LEDS, color);
  FastLED.show();
}

void rainbowEffect()
{
  static uint8_t hue = 0;
  fill_rainbow(leds, NUM_LEDS, hue, 255 / NUM_LEDS);
  FastLED.show();
  hue += 3;
}

void musicVisualizerDemo()
{
  // Demo effect until Android app connects
  static uint8_t beat = 0;
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CHSV(beat + (i * 4), 255,
                   beatsin8(60 + (i * 2), 0, 255));
  }
  FastLED.show();
  beat += 2;
}

void handleLedStrip()
{
  // Don't update LEDs during OTA to avoid conflicts
  if (otaInProgress)
    return;

  switch (currentMode)
  {
  case MODE_OFF:
    FastLED.clear();
    FastLED.show();
    break;
  case MODE_SOLID:
    setStripColor(currentColor);
    break;
  case MODE_RAINBOW:
    rainbowEffect();
    break;
  case MODE_MUSIC_READY:
    musicVisualizerDemo();
    break;
  }
}

// Auto-Update Functions
void checkForFirmwareUpdate()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Not connected to WiFi, skipping update check");
    return;
  }

  HTTPClient http;
  http.begin(FIRMWARE_UPDATE_URL);
  http.addHeader("Accept", "application/json");

  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK)
  {
    String payload = http.getString();

    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, payload);

    if (!error)
    {
      latestVersion = doc["tag_name"].as<String>();
      Serial.println("Latest version: " + latestVersion);

      // Compare with current version using FIRMWARE_VERSION
      const String CURRENT_VERSION = "v" + FIRMWARE_VERSION; // Construct version string

      if (latestVersion != CURRENT_VERSION)
      {
        Serial.println("New version available: " + latestVersion);
        updateStatus = "Update available: " + latestVersion;

        if (autoUpdateEnabled)
        {
          String downloadUrl = doc["assets"][0]["browser_download_url"];
          performAutoUpdate(downloadUrl);
        }
      }
      else
      {
        Serial.println("Firmware is up to date");
        updateStatus = "Up to date";
      }
    }
    else
    {
      Serial.println("Error parsing update response");
      updateStatus = "Check failed";
    }
  }
  else
  {
    Serial.println("Error checking for updates: " + String(httpCode));
    updateStatus = "Check failed";
  }

  http.end();
}

void performAutoUpdate(String updateUrl)
{
  if (updateInProgress)
  {
    Serial.println("Update already in progress");
    return;
  }

  updateInProgress = true;
  updateStatus = "Downloading...";
  Serial.println("Starting firmware update from: " + updateUrl);

  WiFiClient client;
  HTTPUpdate httpUpdate;

  httpUpdate.onStart([]()
                     {
    Serial.println("Update started");
    updateStatus = "Installing..."; });

  httpUpdate.onEnd([]()
                   {
    Serial.println("Update finished");
    updateStatus = "Complete - Restarting..."; });

  httpUpdate.onProgress([](int cur, int total)
                        {
    Serial.printf("Update progress: %d%%\n", (cur * 100) / total);
    updateStatus = "Installing... " + String((cur * 100) / total) + "%"; });

  httpUpdate.onError([](int error)
                     {
    Serial.printf("Update error: %d\n", error);
    updateStatus = "Update failed";
    updateInProgress = false; });

  t_httpUpdate_return ret = httpUpdate.update(client, updateUrl);

  switch (ret)
  {
  case HTTP_UPDATE_FAILED:
    Serial.printf("Update failed: %s\n", httpUpdate.getLastErrorString().c_str());
    updateStatus = "Failed: " + httpUpdate.getLastErrorString();
    break;
  case HTTP_UPDATE_NO_UPDATES:
    Serial.println("No update needed");
    updateStatus = "No update needed";
    break;
  case HTTP_UPDATE_OK:
    Serial.println("Update successful");
    updateStatus = "Update successful";
    ESP.restart();
    break;
  }

  updateInProgress = false;
}

void handleAutoUpdate()
{
  String action = server.arg("action");

  if (action == "check")
  {
    checkForFirmwareUpdate();
    server.send(200, "text/plain", "Update check initiated");
  }
  else if (action == "enable")
  {
    autoUpdateEnabled = true;
    server.send(200, "text/plain", "Auto-update enabled");
    Serial.println("Auto-update enabled via web");
  }
  else if (action == "disable")
  {
    autoUpdateEnabled = false;
    server.send(200, "text/plain", "Auto-update disabled");
    Serial.println("Auto-update disabled via web");
  }
  else if (action == "now")
  {
    if (latestVersion != "" && latestVersion != ("v" + FIRMWARE_VERSION))
    {
      // Perform immediate update
      HTTPClient http;
      http.begin(FIRMWARE_UPDATE_URL);
      int httpCode = http.GET();
      if (httpCode == HTTP_CODE_OK)
      {
        String payload = http.getString();
        DynamicJsonDocument doc(4096);
        if (deserializeJson(doc, payload) == DeserializationError::Ok)
        {
          String downloadUrl = doc["assets"][0]["browser_download_url"];
          performAutoUpdate(downloadUrl);
        }
      }
      http.end();
      server.send(200, "text/plain", "Update started");
    }
    else
    {
      server.send(200, "text/plain", "No update available");
    }
  }
  else
  {
    server.send(400, "text/plain", "Invalid action");
  }
}

void setupOTA()
{
  // Set hostname for easier identification
  ArduinoOTA.setHostname(DEVICE_NAME.c_str());

  // Optional: Set password for OTA updates (uncomment if needed)
  // ArduinoOTA.setPassword("your_ota_password");

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

// Function to handle root page
void handleRoot()
{
  String html = "<!DOCTYPE html><html>";
  html += "<head><meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>ESP32 LED Control</title>";
  html += "<style>";
  html += "body{font-family:Arial,sans-serif;text-align:center;margin:0;padding:20px;background:#f0f0f0;}";
  html += ".container{max-width:400px;margin:0 auto;background:white;padding:30px;border-radius:15px;box-shadow:0 4px 6px rgba(0,0,0,0.1);}";
  html += "h1{color:#333;margin-bottom:30px;}";
  html += ".status{font-size:20px;margin:20px 0;padding:15px;border-radius:10px;}";
  html += ".status.on{background:#d4edda;color:#155724;border:1px solid #c3e6cb;}";
  html += ".status.off{background:#f8d7da;color:#721c24;border:1px solid #f5c6cb;}";
  html += "button{padding:15px 30px;font-size:18px;margin:10px;border:none;border-radius:10px;cursor:pointer;min-width:120px;}";
  html += ".on{background-color:#28a745;color:white;}.on:hover{background-color:#218838;}";
  html += ".off{background-color:#dc3545;color:white;}.off:hover{background-color:#c82333;}";
  html += ".toggle{background-color:#007bff;color:white;}.toggle:hover{background-color:#0056b3;}";
  html += ".ip{margin-top:20px;font-size:14px;color:#666;}";
  html += "</style></head>";
  html += "<body><div class='container'>";
  html += "<h1>🎵 ESP32 Music Visualizer</h1>";
  html += "<div class='status " + String(ledState ? "on" : "off") + "'>";
  html += "Built-in LED: <strong>" + String(ledState ? "ON 💡" : "OFF 🌚") + "</strong></div>";

  // Add device info and OTA status
  html += "<div style='background:#e9ecef;padding:10px;border-radius:8px;margin:15px 0;font-size:14px;'>";
  html += "<strong>Device Info:</strong><br>";
  html += "Name: " + DEVICE_NAME + "<br>";
  html += "Version: " + FIRMWARE_VERSION + "<br>";
  html += "OTA Status: <span style='color:" + String(otaInProgress ? "#dc3545" : "#28a745") + ";'>" + otaStatus + "</span><br>";
  html += "Auto-Update: <span style='color:" + String(autoUpdateEnabled ? "#28a745" : "#dc3545") + ";'>" + String(autoUpdateEnabled ? "Enabled" : "Disabled") + "</span>";
  if (latestVersion != "")
  {
    html += "<br>Latest: " + latestVersion;
  }
  html += "</div>";

  // Add auto-update controls
  html += "<div style='background:#fff3cd;padding:10px;border-radius:8px;margin:15px 0;font-size:14px;'>";
  html += "<strong>🔄 Auto Updates:</strong><br>";
  html += "<button style='background:#17a2b8;color:white;margin:5px;padding:8px 16px;border:none;border-radius:5px;' onclick=\"autoUpdate('check')\">Check Updates</button>";
  html += "<button style='background:" + String(autoUpdateEnabled ? "#dc3545" : "#28a745") + ";color:white;margin:5px;padding:8px 16px;border:none;border-radius:5px;' onclick=\"autoUpdate('" + String(autoUpdateEnabled ? "disable" : "enable") + "')\">" + String(autoUpdateEnabled ? "Disable" : "Enable") + "</button>";
  if (latestVersion != "" && latestVersion != ("v" + FIRMWARE_VERSION))
  {
    html += "<br><button style='background:#ff6b35;color:white;margin:5px;padding:8px 16px;border:none;border-radius:5px;' onclick=\"autoUpdate('now')\">Update Now</button>";
  }
  html += "</div>";

  html += "<h3>LED Strip Control</h3>";
  html += "<button class='off' onclick=\"setMode('off')\">Strip OFF</button>";
  html += "<button class='on' onclick=\"setMode('solid')\">Solid Color</button>";
  html += "<button class='toggle' onclick=\"setMode('rainbow')\">Rainbow</button>";
  html += "<button style='background:#ff6b35;color:white;' onclick=\"setMode('music')\">Music Mode</button>";

  html += "<h3>Colors</h3>";
  html += "<button style='background:#ff0000;color:white;' onclick=\"setColor('red')\">Red</button>";
  html += "<button style='background:#00ff00;color:black;' onclick=\"setColor('green')\">Green</button>";
  html += "<button style='background:#0000ff;color:white;' onclick=\"setColor('blue')\">Blue</button>";
  html += "<button style='background:#ffff00;color:black;' onclick=\"setColor('yellow')\">Yellow</button>";

  html += "<h3>Built-in LED</h3>";
  html += "<button class='on' onclick=\"controlLED('on')\">Turn ON</button>";
  html += "<button class='off' onclick=\"controlLED('off')\">Turn OFF</button>";
  html += "<br><button class='toggle' onclick=\"controlLED('toggle')\">Toggle LED</button>";
  html += "<div class='ip'>Device IP: " + WiFi.localIP().toString() + "</div>";
  html += "<div style='margin-top:10px;font-size:12px;color:#888;'>Ready for Android app connection!</div>";
  html += "<div style='margin-top:15px;padding:10px;background:#fff3cd;border-radius:5px;font-size:12px;'>";
  html += "<strong>📡 OTA Updates:</strong><br>";
  html += "Use Arduino IDE or PlatformIO to upload wirelessly<br>";
  html += "Network Port: " + DEVICE_NAME + ".local";
  html += "</div>";
  html += "</div>";
  html += "<script>";
  html += "function controlLED(action){";
  html += "  if(action==='on'){";
  html += "    document.querySelector('.status').className='status on';";
  html += "    document.querySelector('.status strong').textContent='ON 💡';";
  html += "  } else if(action==='off'){";
  html += "    document.querySelector('.status').className='status off';";
  html += "    document.querySelector('.status strong').textContent='OFF 🌚';";
  html += "  } else if(action==='toggle'){";
  html += "    const isOn = document.querySelector('.status').classList.contains('on');";
  html += "    if(isOn){";
  html += "      document.querySelector('.status').className='status off';";
  html += "      document.querySelector('.status strong').textContent='OFF 🌚';";
  html += "    } else {";
  html += "      document.querySelector('.status').className='status on';";
  html += "      document.querySelector('.status strong').textContent='ON 💡';";
  html += "    }";
  html += "  }";
  html += "  fetch('/led/'+action);";
  html += "}";
  html += "function setMode(mode){fetch('/strip/mode/'+mode);}";
  html += "function setColor(color){fetch('/strip/color/'+color);}";
  html += "function autoUpdate(action){";
  html += "  fetch('/auto-update/'+action).then(response => response.json()).then(data => {";
  html += "    console.log('Auto-update response:', data);";
  html += "    if(action === 'now') alert('Update started! Device will restart when complete.');";
  html += "    setTimeout(() => location.reload(), 2000);";
  html += "  }).catch(err => console.error('Auto-update error:', err));";
  html += "}";
  html += "setInterval(function(){location.reload();},15000);";
  html += "</script>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

// Function to turn LED on
void handleLedOn()
{
  ledState = true;
  digitalWrite(BUILTIN_LED_PIN, HIGH);
  server.send(200, "text/plain", "LED ON");
  Serial.println("LED turned ON");
}

// Function to turn LED off
void handleLedOff()
{
  ledState = false;
  digitalWrite(BUILTIN_LED_PIN, LOW);
  server.send(200, "text/plain", "LED OFF");
  Serial.println("LED turned OFF");
}

// Function to toggle LED
void handleLedToggle()
{
  ledState = !ledState;
  digitalWrite(BUILTIN_LED_PIN, ledState);
  server.send(200, "text/plain", ledState ? "LED ON" : "LED OFF");
  Serial.println("LED toggled to: " + String(ledState ? "ON" : "OFF"));
}

// Function to handle LED strip mode changes
void handleStripMode()
{
  String mode = server.pathArg(0);

  if (mode == "off")
  {
    currentMode = MODE_OFF;
    server.send(200, "text/plain", "Strip OFF");
    Serial.println("LED Strip: OFF");
  }
  else if (mode == "solid")
  {
    currentMode = MODE_SOLID;
    server.send(200, "text/plain", "Strip Solid Color");
    Serial.println("LED Strip: Solid Color");
  }
  else if (mode == "rainbow")
  {
    currentMode = MODE_RAINBOW;
    server.send(200, "text/plain", "Strip Rainbow");
    Serial.println("LED Strip: Rainbow Mode");
  }
  else if (mode == "music")
  {
    currentMode = MODE_MUSIC_READY;
    server.send(200, "text/plain", "Strip Music Mode");
    Serial.println("LED Strip: Music Visualizer Ready");
  }
  else
  {
    server.send(400, "text/plain", "Invalid mode");
  }
}

// Function to handle LED strip color changes
void handleStripColor()
{
  String color = server.pathArg(0);

  if (color == "red")
  {
    currentColor = CRGB::Red;
  }
  else if (color == "green")
  {
    currentColor = CRGB::Green;
  }
  else if (color == "blue")
  {
    currentColor = CRGB::Blue;
  }
  else if (color == "yellow")
  {
    currentColor = CRGB::Yellow;
  }
  else if (color == "white")
  {
    currentColor = CRGB::White;
  }
  else
  {
    server.send(400, "text/plain", "Invalid color");
    return;
  }

  if (currentMode == MODE_SOLID)
  {
    setStripColor(currentColor);
  }

  server.send(200, "text/plain", "Color set to " + color);
  Serial.println("LED Strip color: " + color);
}

// Android App API endpoint for music data
void handleMusicData()
{
  if (server.method() == HTTP_POST)
  {
    String body = server.arg("plain");
    Serial.println("Music data received: " + body);

    // Parse JSON music data here (frequencies, beat, etc.)
    // For now, just acknowledge receipt
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  }
  else
  {
    server.send(405, "text/plain", "Method not allowed");
  }
}

// USB Serial Command Processing Functions
void handleMusicVisualization(String musicData)
{
  // Parse music data and create visualization
  // Format: "freq1,freq2,freq3,beat" or JSON-like data
  // For now, create a simple beat-responsive effect

  if (currentMode == MODE_MUSIC_READY)
  {
    // Simple example: flash intensity based on beat
    int beatValue = musicData.toInt(); // Simple parsing for demo
    int intensity = map(constrain(beatValue, 0, 100), 0, 100, 50, 255);

    // Create beat-responsive effect
    for (int i = 0; i < NUM_LEDS; i++)
    {
      leds[i] = CHSV(160 + (beatValue % 60), 255, intensity);
    }
    FastLED.show();
  }
}

void processSerialCommand(String command)
{
  command.trim();
  command.toLowerCase();

  // Update connection status
  lastSerialActivity = millis();
  if (!serialConnected)
  {
    serialConnected = true;
    Serial.println("RESPONSE:USB_CONNECTED");
  }

  Serial.println("USB Command received: " + command);

  // Handle ping/keepalive command
  if (command == "ping")
  {
    Serial.println("RESPONSE:PONG");
    return;
  }

  if (command == "off")
  {
    currentMode = MODE_OFF;
    Serial.println("RESPONSE:Strip OFF");
  }
  else if (command == "solid")
  {
    currentMode = MODE_SOLID;
    Serial.println("RESPONSE:Strip Solid Color");
  }
  else if (command == "rainbow")
  {
    currentMode = MODE_RAINBOW;
    Serial.println("RESPONSE:Strip Rainbow");
  }
  else if (command == "music")
  {
    currentMode = MODE_MUSIC_READY;
    Serial.println("RESPONSE:Strip Music Mode");
  }
  else if (command == "red")
  {
    currentColor = CRGB::Red;
    if (currentMode == MODE_SOLID)
      setStripColor(currentColor);
    Serial.println("RESPONSE:Color Red");
  }
  else if (command == "green")
  {
    currentColor = CRGB::Green;
    if (currentMode == MODE_SOLID)
      setStripColor(currentColor);
    Serial.println("RESPONSE:Color Green");
  }
  else if (command == "blue")
  {
    currentColor = CRGB::Blue;
    if (currentMode == MODE_SOLID)
      setStripColor(currentColor);
    Serial.println("RESPONSE:Color Blue");
  }
  else if (command == "yellow")
  {
    currentColor = CRGB::Yellow;
    if (currentMode == MODE_SOLID)
      setStripColor(currentColor);
    Serial.println("RESPONSE:Color Yellow");
  }
  else if (command == "white")
  {
    currentColor = CRGB::White;
    if (currentMode == MODE_SOLID)
      setStripColor(currentColor);
    Serial.println("RESPONSE:Color White");
  }
  else if (command == "ledon")
  {
    ledState = true;
    digitalWrite(BUILTIN_LED_PIN, HIGH);
    Serial.println("RESPONSE:LED ON");
  }
  else if (command == "ledoff")
  {
    ledState = false;
    digitalWrite(BUILTIN_LED_PIN, LOW);
    Serial.println("RESPONSE:LED OFF");
  }
  else if (command == "toggle")
  {
    ledState = !ledState;
    digitalWrite(BUILTIN_LED_PIN, ledState);
    Serial.println("RESPONSE:LED " + String(ledState ? "ON" : "OFF"));
  }
  else if (command == "status")
  {
    String wifiStatus = (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : "disconnected";
    Serial.println("RESPONSE:Mode=" + String(currentMode) + ",LED=" + String(ledState) + ",WiFi=" + wifiStatus + ",USB=connected,Version=" + FIRMWARE_VERSION + ",OTA=" + otaStatus);
  }
  else if (command == "info")
  {
    Serial.println("RESPONSE:Device=" + DEVICE_NAME + ",Version=" + FIRMWARE_VERSION + ",IP=" + WiFi.localIP().toString() + ",AutoUpdate=" + String(autoUpdateEnabled));
  }
  else if (command == "update:check")
  {
    checkForFirmwareUpdate();
    Serial.println("RESPONSE:Update check initiated");
  }
  else if (command == "update:enable")
  {
    autoUpdateEnabled = true;
    Serial.println("RESPONSE:Auto-update enabled");
  }
  else if (command == "update:disable")
  {
    autoUpdateEnabled = false;
    Serial.println("RESPONSE:Auto-update disabled");
  }
  else if (command == "update:now")
  {
    if (latestVersion != "")
    {
      String downloadUrl = FIRMWARE_BINARY_URL_BASE + latestVersion + "/firmware.bin";
      performAutoUpdate(downloadUrl);
      Serial.println("RESPONSE:Manual update started");
    }
    else
    {
      Serial.println("RESPONSE:ERROR No update available");
    }
  }
  else if (command.startsWith("brightness:"))
  {
    int brightness = command.substring(11).toInt();
    if (brightness >= 0 && brightness <= 255)
    {
      FastLED.setBrightness(brightness);
      FastLED.show();
      Serial.println("RESPONSE:Brightness set to " + String(brightness));
    }
    else
    {
      Serial.println("RESPONSE:ERROR Invalid brightness (0-255)");
    }
  }
  else if (command.startsWith("music:"))
  {
    // Real-time music data: "music:freq1,freq2,freq3,beat"
    String musicData = command.substring(6);
    handleMusicVisualization(musicData);
    Serial.println("RESPONSE:Music data processed");
  }
  else if (command.startsWith("update:"))
  {
    String updateCmd = command.substring(7);
    if (updateCmd == "check")
    {
      checkForFirmwareUpdate();
      Serial.println("RESPONSE:Update check initiated");
    }
    else if (updateCmd == "enable")
    {
      autoUpdateEnabled = true;
      Serial.println("RESPONSE:Auto-update enabled");
    }
    else if (updateCmd == "disable")
    {
      autoUpdateEnabled = false;
      Serial.println("RESPONSE:Auto-update disabled");
    }
    else if (updateCmd == "now")
    {
      if (latestVersion != "" && latestVersion != ("v" + FIRMWARE_VERSION))
      {
        HTTPClient http;
        http.begin(FIRMWARE_UPDATE_URL);
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK)
        {
          String payload = http.getString();
          DynamicJsonDocument doc(4096);
          if (deserializeJson(doc, payload) == DeserializationError::Ok)
          {
            String downloadUrl = doc["assets"][0]["browser_download_url"];
            performAutoUpdate(downloadUrl);
            Serial.println("RESPONSE:Update started");
          }
          else
          {
            Serial.println("RESPONSE:ERROR Failed to parse update info");
          }
        }
        else
        {
          Serial.println("RESPONSE:ERROR Failed to get update info");
        }
        http.end();
      }
      else
      {
        Serial.println("RESPONSE:No update available");
      }
    }
    else
    {
      Serial.println("RESPONSE:ERROR Invalid update command");
    }
  }
  else
  {
    Serial.println("RESPONSE:ERROR Unknown command: " + command);
    Serial.println("Available commands: ping, off, solid, rainbow, music, red, green, blue, yellow, white, ledon, ledoff, toggle, status, info, brightness:0-255, music:data, update:check/enable/disable/now");
  }
}

void checkSerialInput()
{
  while (Serial.available())
  {
    char incoming = Serial.read();

    if (incoming == '\n' || incoming == '\r')
    {
      if (serialBuffer.length() > 0)
      {
        serialCommandReady = true;
      }
    }
    else if (incoming >= 32 && incoming <= 126) // Only printable ASCII characters
    {
      serialBuffer += incoming;

      // Prevent buffer overflow
      if (serialBuffer.length() > 100)
      {
        Serial.println("RESPONSE:ERROR Command too long");
        serialBuffer = "";
      }
    }
  }

  if (serialCommandReady)
  {
    processSerialCommand(serialBuffer);
    serialBuffer = "";
    serialCommandReady = false;
  }

  // Check for USB disconnection
  if (serialConnected && (millis() - lastSerialActivity) > SERIAL_TIMEOUT)
  {
    serialConnected = false;
    Serial.println("RESPONSE:USB_TIMEOUT");
  }
}

void setup()
{
  // Initialize serial communication
  Serial.begin(115200);
  delay(1000); // Give serial time to initialize

  Serial.println("\n=== ESP32 Music Visualizer Starting ===");

  // Initialize LED strip
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();
  Serial.println("LED strip initialized (60 LEDs)");

  // Initialize built-in LED pin
  pinMode(BUILTIN_LED_PIN, OUTPUT);
  digitalWrite(BUILTIN_LED_PIN, LOW);
  Serial.println("Built-in LED pin initialized");

  Serial.println("\n=== Attempting WiFi Connection ===");
  Serial.println("Note: WiFi is optional - USB serial control always available");

  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  Serial.print("SSID: ");
  Serial.println(WIFI_SSID);

  // Configure WiFi for better compatibility
  WiFi.mode(WIFI_STA);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30)
  {
    delay(1000);
    Serial.print(".");
    attempts++;

    // Try to reconnect every 10 attempts
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
    Serial.print("Subnet mask: ");
    Serial.println(WiFi.subnetMask());
    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("DNS: ");
    Serial.println(WiFi.dnsIP());
    Serial.print("MAC address: ");
    Serial.println(WiFi.macAddress());
    Serial.print("Signal strength (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

    // Initialize mDNS for easier network discovery
    if (MDNS.begin(DEVICE_NAME.c_str()))
    {
      Serial.println("mDNS responder started: " + DEVICE_NAME + ".local");
      MDNS.addService("http", "tcp", 80);
    }

    // Setup OTA updates
    setupOTA();
  }
  else
  {
    Serial.println("");
    Serial.println("=== WiFi connection failed! ===");
    Serial.println("Check your WiFi credentials and try again");
    Serial.println("Continuing with USB serial control only...");
  }

  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/led/on", handleLedOn);
  server.on("/led/off", handleLedOff);
  server.on("/led/toggle", handleLedToggle);

  // LED Strip routes
  server.on("/strip/mode/*", handleStripMode);
  server.on("/strip/color/*", handleStripColor);

  // Auto-update routes
  server.on("/auto-update/*", handleAutoUpdate);

  // Android App API
  server.on("/api/music", HTTP_POST, handleMusicData);
  server.on("/strip/mode/*", handleStripMode);
  server.on("/strip/color/*", handleStripColor);
  server.on("/music/data", handleMusicData);

  // Start server
  server.begin();
  Serial.println("Web server started!");

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("=================================");
    Serial.println("🌐 Open your browser and go to:");
    Serial.print("   http://");
    Serial.println(WiFi.localIP());
    Serial.println("=================================");
  }

  // Print USB serial control info
  Serial.println("\n=== USB Serial Control Ready ===");
  Serial.println("Available commands:");
  Serial.println("- ping (test connection)");
  Serial.println("- off, solid, rainbow, music");
  Serial.println("- red, green, blue, yellow, white");
  Serial.println("- ledon, ledoff, toggle, status, info");
  Serial.println("- brightness:0-255");
  Serial.println("- music:data (for real-time music sync)");
  Serial.println("- update:check, update:enable, update:disable, update:now");
  Serial.println("=================================");

  // Send initial status
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

  // Check for USB serial commands (high priority for low latency)
  checkSerialInput();

  // Handle web server requests
  server.handleClient();

  // Update LED strip based on current mode (skip during OTA/updates)
  if (!otaInProgress && !updateInProgress)
  {
    handleLedStrip();
  }

  // Send periodic heartbeat if connected
  static unsigned long lastHeartbeat = 0;
  if (serialConnected && (millis() - lastHeartbeat) > 10000)
  { // Every 10 seconds
    Serial.println("RESPONSE:HEARTBEAT");
    lastHeartbeat = millis();
  }

  // Periodic update check (every 24 hours)
  if (WiFi.isConnected() && !otaInProgress &&
      autoUpdateEnabled && (millis() - lastUpdateCheck) > UPDATE_CHECK_INTERVAL)
  {
    checkForFirmwareUpdate();
  }

  // Add a small delay to prevent watchdog timer issues
  delay(10);
}
