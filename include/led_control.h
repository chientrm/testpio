#pragma once
#include <FastLED.h>

// LED Strip modes
enum LedMode
{
    MODE_OFF,
    MODE_SOLID,
    MODE_RAINBOW,
    MODE_VISUALIZER
};

// LED Control Functions
void initializeLEDs();
void setStripColor(CRGB color);
void rainbowEffect();
void musicVisualizerEffect();
void handleLedStrip();
void handleMusicVisualization(String musicData);

// LED State Variables
extern CRGB leds[];
extern LedMode currentMode;
extern CRGB currentColor;
extern bool ledState;
