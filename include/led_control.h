#pragma once
#include <FastLED.h>

// LED Strip modes
enum LedMode
{
    MODE_OFF,
    MODE_SOLID,
    MODE_RAINBOW,
    MODE_MUSIC_READY
};

// LED Control Functions
void initializeLEDs();
void setStripColor(CRGB color);
void rainbowEffect();
void musicVisualizerDemo();
void handleLedStrip();
void handleMusicVisualization(String musicData);

// LED State Variables
extern CRGB leds[];
extern LedMode currentMode;
extern CRGB currentColor;
extern bool ledState;
