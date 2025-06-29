#include "web_server.h"
#include "config.h"
#include "led_control.h"
#include "ota_update.h"
#include "auto_update.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Web Server Instance
WebServer server(80);

void initializeWebServer()
{
    // Setup web server routes
    server.on("/", handleRoot);
    server.on("/led/on", handleLedOn);
    server.on("/led/off", handleLedOff);
    server.on("/led/toggle", handleLedToggle);
    server.on("/strip/mode/*", handleStripMode);
    server.on("/strip/color/*", handleStripColor);
    server.on("/auto-update/*", handleAutoUpdateWeb);
    server.on("/api/music", HTTP_POST, handleMusicData);
    server.on("/music/data", handleMusicData);

    server.begin();
    Serial.println("Web server started!");
}

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
    html += "</style></head>";
    html += "<body><div class='container'>";
    html += "<h1>🎵 ESP32 Music Visualizer</h1>";
    html += "<div class='status " + String(ledState ? "on" : "off") + "'>";
    html += "Built-in LED: <strong>" + String(ledState ? "ON 💡" : "OFF 🌚") + "</strong></div>";

    // Device info
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

    // Auto-update controls
    html += "<div style='background:#fff3cd;padding:10px;border-radius:8px;margin:15px 0;font-size:14px;'>";
    html += "<strong>🔄 Auto Updates:</strong><br>";
    html += "<button style='background:#17a2b8;color:white;margin:5px;padding:8px 16px;border:none;border-radius:5px;' onclick=\"autoUpdate('check')\">Check Updates</button>";
    html += "<button style='background:" + String(autoUpdateEnabled ? "#dc3545" : "#28a745") + ";color:white;margin:5px;padding:8px 16px;border:none;border-radius:5px;' onclick=\"autoUpdate('" + String(autoUpdateEnabled ? "disable" : "enable") + "')\">" + String(autoUpdateEnabled ? "Disable" : "Enable") + "</button>";
    if (latestVersion != "" && latestVersion != ("v" + FIRMWARE_VERSION))
    {
        html += "<br><button style='background:#ff6b35;color:white;margin:5px;padding:8px 16px;border:none;border-radius:5px;' onclick=\"autoUpdate('now')\">Update Now</button>";
    }
    html += "</div>";

    // LED Controls
    html += "<h3>LED Strip Control</h3>";
    html += "<button class='off' onclick=\"setMode('off')\">Strip OFF</button>";
    html += "<button class='on' onclick=\"setMode('solid')\">Solid Color</button>";
    html += "<button class='toggle' onclick=\"setMode('rainbow')\">Rainbow</button>";
    html += "<button style='background:#ff6b35;color:white;' onclick=\"setMode('visualizer')\">Visualizer</button>";

    html += "<h3>Colors</h3>";
    html += "<button style='background:#ff0000;color:white;' onclick=\"setColor('red')\">Red</button>";
    html += "<button style='background:#00ff00;color:black;' onclick=\"setColor('green')\">Green</button>";
    html += "<button style='background:#0000ff;color:white;' onclick=\"setColor('blue')\">Blue</button>";
    html += "<button style='background:#ffff00;color:black;' onclick=\"setColor('yellow')\">Yellow</button>";

    html += "<h3>Built-in LED</h3>";
    html += "<button class='on' onclick=\"controlLED('on')\">Turn ON</button>";
    html += "<button class='off' onclick=\"controlLED('off')\">Turn OFF</button>";
    html += "<br><button class='toggle' onclick=\"controlLED('toggle')\">Toggle LED</button>";

    html += "<div style='margin-top:20px;font-size:14px;color:#666;'>Device IP: " + WiFi.localIP().toString() + "</div>";
    html += "<div style='margin-top:10px;font-size:12px;color:#888;'>Standalone LED visualizer ready!</div>";

    // GitHub repository link
    html += "<div style='margin-top:15px;padding:10px;background:#f8f9fa;border-radius:8px;text-align:center;'>";
    html += "<a href='" + REPOSITORY_URL + "' target='_blank' style='color:#0366d6;text-decoration:none;font-size:14px;'>";
    html += "📂 View Source Code on GitHub</a>";
    html += "</div>";

    html += "</div>";
    html += "<script>";
    html += "function controlLED(action){ fetch('/led/'+action); setTimeout(() => location.reload(), 500); }";
    html += "function setMode(mode){ fetch('/strip/mode/'+mode); }";
    html += "function setColor(color){ fetch('/strip/color/'+color); }";
    html += "function autoUpdate(action){ fetch('/auto-update/'+action); setTimeout(() => location.reload(), 2000); }";
    html += "setInterval(() => location.reload(), 15000);";
    html += "</script>";
    html += "</body></html>";

    server.send(200, "text/html", html);
}

void handleLedOn()
{
    ledState = true;
    digitalWrite(BUILTIN_LED_PIN, HIGH);
    server.send(200, "text/plain", "LED ON");
    Serial.println("LED turned ON");
}

void handleLedOff()
{
    ledState = false;
    digitalWrite(BUILTIN_LED_PIN, LOW);
    server.send(200, "text/plain", "LED OFF");
    Serial.println("LED turned OFF");
}

void handleLedToggle()
{
    ledState = !ledState;
    digitalWrite(BUILTIN_LED_PIN, ledState);
    server.send(200, "text/plain", ledState ? "LED ON" : "LED OFF");
    Serial.println("LED toggled to: " + String(ledState ? "ON" : "OFF"));
}

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
    else if (mode == "visualizer")
    {
        currentMode = MODE_VISUALIZER;
        server.send(200, "text/plain", "Strip Visualizer Mode");
        Serial.println("LED Strip: Visualizer Mode");
    }
    else
    {
        server.send(400, "text/plain", "Invalid mode");
    }
}

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

void handleAutoUpdateWeb()
{
    String action = server.pathArg(0);

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
            checkForFirmwareUpdate(); // This will trigger the update
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

void handleMusicData()
{
    if (server.method() == HTTP_POST)
    {
        String body = server.arg("plain");
        Serial.println("Music data received: " + body);
        server.send(200, "application/json", "{\"status\":\"ok\"}");
    }
    else
    {
        server.send(405, "text/plain", "Method not allowed");
    }
}
