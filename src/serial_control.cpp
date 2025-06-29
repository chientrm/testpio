#include "serial_control.h"
#include "config.h"
#include "led_control.h"
#include "auto_update.h"
#include <WiFi.h>

// Serial State Variables
String serialBuffer = "";
bool serialCommandReady = false;
unsigned long lastSerialActivity = 0;
bool serialConnected = false;

void initializeSerial()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n=== ESP32 Music Visualizer Starting ===");
    Serial.println("USB Serial initialized");
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
        Serial.println("RESPONSE:Mode=" + String(currentMode) + ",LED=" + String(ledState) + ",WiFi=" + wifiStatus + ",USB=connected,Version=" + FIRMWARE_VERSION);
    }
    else if (command == "info")
    {
        Serial.println("RESPONSE:Device=" + DEVICE_NAME + ",Version=" + FIRMWARE_VERSION + ",IP=" + WiFi.localIP().toString() + ",AutoUpdate=" + String(autoUpdateEnabled));
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
                checkForFirmwareUpdate(); // This will trigger the update if available
                Serial.println("RESPONSE:Update started");
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
