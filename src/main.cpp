#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// WiFi credentials
const char *ssid = "xincamon";
const char *password = "xincamon";

// Create web server on port 80
WebServer server(80);

// LED pin (built-in LED on most ESP32 boards)
const int LED_PIN = 2;
bool ledState = false;

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
  html += "<h1>🔆 ESP32 LED Controller</h1>";
  html += "<div class='status " + String(ledState ? "on" : "off") + "'>";
  html += "LED Status: <strong>" + String(ledState ? "ON 💡" : "OFF 🌚") + "</strong></div>";
  html += "<button class='on' onclick=\"controlLED('on')\">Turn ON</button>";
  html += "<button class='off' onclick=\"controlLED('off')\">Turn OFF</button>";
  html += "<br><button class='toggle' onclick=\"controlLED('toggle')\">Toggle LED</button>";
  html += "<div class='ip'>Device IP: " + WiFi.localIP().toString() + "</div>";
  html += "</div>";
  html += "<script>";
  html += "function controlLED(action){";
  html += "fetch('/led/'+action).then(()=>{setTimeout(()=>location.reload(),500);});";
  html += "}";
  html += "setInterval(()=>location.reload(),5000);";
  html += "</script>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

// Function to turn LED on
void handleLedOn()
{
  ledState = true;
  digitalWrite(LED_PIN, HIGH);
  server.send(200, "text/plain", "LED ON");
  Serial.println("LED turned ON");
}

// Function to turn LED off
void handleLedOff()
{
  ledState = false;
  digitalWrite(LED_PIN, LOW);
  server.send(200, "text/plain", "LED OFF");
  Serial.println("LED turned OFF");
}

// Function to toggle LED
void handleLedToggle()
{
  ledState = !ledState;
  digitalWrite(LED_PIN, ledState);
  server.send(200, "text/plain", ledState ? "LED ON" : "LED OFF");
  Serial.println("LED toggled to: " + String(ledState ? "ON" : "OFF"));
}

void setup()
{
  // Initialize serial communication
  Serial.begin(115200);
  delay(1000); // Give serial time to initialize

  Serial.println("\n=== ESP32 LED Controller Starting ===");

  // Initialize LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.println("LED pin initialized");

  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  Serial.print("SSID: ");
  Serial.println(ssid);

  // Configure WiFi for better compatibility
  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, password);

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
      WiFi.begin(ssid, password);
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

  // Add a small delay to prevent watchdog timer issues
  delay(10);
}