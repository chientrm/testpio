#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <FastLED.h>
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
  html += "setInterval(function(){location.reload();},10000);";
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

// Power bank keep-alive
unsigned long lastKeepAlive = 0;
const unsigned long KEEP_ALIVE_INTERVAL = 10000; // 10 seconds

void keepPowerBankActive()
{
  unsigned long currentTime = millis();
  if (currentTime - lastKeepAlive > KEEP_ALIVE_INTERVAL)
  {
    // Brief high-frequency WiFi scan to increase current draw
    WiFi.scanNetworks(true);

    // Flash built-in LED rapidly to create current spikes
    for (int i = 0; i < 10; i++)
    {
      digitalWrite(BUILTIN_LED_PIN, HIGH);
      delay(50);
      digitalWrite(BUILTIN_LED_PIN, LOW);
      delay(50);
    }

    lastKeepAlive = currentTime;
    Serial.println("Power bank keep-alive burst");
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

  // Initialize keep-alive timer
  lastKeepAlive = millis();
  Serial.println("Power bank keep-alive initialized");

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
  }
  else
  {
    Serial.println("");
    Serial.println("=== WiFi connection failed! ===");
    Serial.println("Check your WiFi credentials and try again");
    Serial.println("Continuing without WiFi...");
  }

  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/led/on", handleLedOn);
  server.on("/led/off", handleLedOff);
  server.on("/led/toggle", handleLedToggle);

  // LED Strip routes
  server.on("/strip/mode/*", handleStripMode);
  server.on("/strip/color/*", handleStripColor);

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
}

void loop()
{
  // Handle web server requests
  server.handleClient();

  // Update LED strip based on current mode
  handleLedStrip();

  // Keep power bank alive
  keepPowerBankActive();

  // Add a small delay to prevent watchdog timer issues
  delay(10);
}