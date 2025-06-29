#include "led_control.h"
#include "config.h"
#include "ota_update.h"

// LED State Variables
CRGB leds[NUM_LEDS];
LedMode currentMode = MODE_OFF;
CRGB currentColor = CRGB::Blue;
bool ledState = false;

void initializeLEDs()
{
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear();
    FastLED.show();

    pinMode(BUILTIN_LED_PIN, OUTPUT);
    digitalWrite(BUILTIN_LED_PIN, LOW);
}

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
