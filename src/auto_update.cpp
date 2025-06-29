#include "auto_update.h"
#include "config.h"
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <ArduinoJson.h>
#include <WiFi.h>

// Auto-Update State Variables
unsigned long lastUpdateCheck = 0;
bool autoUpdateEnabled = true;
bool updateInProgress = false;
String latestVersion = "";
String updateStatus = "Ready";

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
            const String CURRENT_VERSION = "v" + FIRMWARE_VERSION;

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
    // Periodic update check
    if (autoUpdateEnabled && !updateInProgress &&
        (millis() - lastUpdateCheck) > UPDATE_CHECK_INTERVAL)
    {
        lastUpdateCheck = millis();
        checkForFirmwareUpdate();
    }
}
