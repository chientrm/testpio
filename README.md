# ESP32 Music Visualizer with Auto-Update

A feature-rich ESP32-based LED strip music visualizer with automatic firmware updates, web interface, and USB serial control.

## 🎵 Features

- **60 LED WS2812B strip control** with multiple visualization modes
- **Auto-update functionality** - automatically downloads and installs firmware updates from GitHub
- **Web interface** - beautiful responsive UI for remote control
- **USB Serial control** - full command-line interface for real-time control
- **OTA updates** - wireless firmware updates via Arduino IDE/PlatformIO
- **WiFi connectivity** with fallback to USB-only mode
- **Multiple LED modes**: Off, Solid Color, Rainbow, Music Visualizer
- **Modular codebase** - clean, maintainable architecture

## 🚀 Quick Start

### Hardware Requirements

- ESP32 development board (ESP32 Pico Kit or similar)
- WS2812B LED strip (60 LEDs)
- 5V power supply for LED strip
- Jumper wires

### Wiring

- LED Strip Data Pin → GPIO 23
- LED Strip GND → ESP32 GND
- LED Strip VCC → 5V power supply
- ESP32 GND → Power supply GND

### Software Setup

1. Clone this repository
2. Install [PlatformIO](https://platformio.org/)
3. Copy `include/wifi_credentials.h.example` to `include/wifi_credentials.h`
4. Edit WiFi credentials in `include/wifi_credentials.h`
5. Build and upload: `pio run --target upload`

## 🌐 Web Interface

After connecting to WiFi, open your browser and navigate to the ESP32's IP address. The web interface provides:

- LED strip control (modes and colors)
- Auto-update management
- Device status and information
- OTA update status

## 📱 Serial Commands

Connect via USB serial (115200 baud) and use these commands:

- `ping` - Test connection
- `off/solid/rainbow/visualizer` - LED modes
- `red/green/blue/yellow/white` - Colors
- `ledon/ledoff/toggle` - Built-in LED
- `brightness:0-255` - Set brightness
- `status/info` - Device information
- `update:check/enable/disable/now` - Auto-update control

## 🔄 Auto-Update System

The device automatically checks for firmware updates from GitHub releases:

- Checks every hour when connected to WiFi
- Downloads and installs updates automatically (if enabled)
- Web and serial interfaces for manual control
- Robust error handling and rollback protection

## 📁 Project Structure

```
├── src/
│   ├── main.cpp           # Main setup and loop
│   ├── config.cpp         # Configuration and constants
│   ├── led_control.cpp    # LED strip effects and control
│   ├── web_server.cpp     # Web interface and API endpoints
│   ├── serial_control.cpp # USB serial command processing
│   ├── ota_update.cpp     # Over-the-air update functionality
│   └── auto_update.cpp    # GitHub auto-update system
├── include/
│   ├── *.h               # Header files for each module
│   └── wifi_credentials.h # WiFi configuration (create from .example)
├── platformio.ini        # PlatformIO configuration
└── AUTO_UPDATE_GUIDE.md  # Detailed auto-update documentation
```

## 🎨 LED Modes

1. **Off** - All LEDs turned off
2. **Solid Color** - Single color across all LEDs
3. **Rainbow** - Animated rainbow effect
4. **Visualizer** - Beautiful animated light show effect

## 🔧 Configuration

Edit `src/config.cpp` to modify:

- Device name and version
- Auto-update URLs and intervals
- LED strip settings

## 📖 Documentation

- [AUTO_UPDATE_GUIDE.md](AUTO_UPDATE_GUIDE.md) - Complete auto-update setup and usage
- [OTA_UPDATE_GUIDE.md](OTA_UPDATE_GUIDE.md) - Over-the-air update instructions

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

## 📄 License

This project is open source. Feel free to use, modify, and distribute.

## 🔗 Links

- **Live Demo**: Connect to your ESP32's IP address
- **GitHub Releases**: Check for firmware updates
- **Issues**: Report bugs or request features

---

**Version**: 1.0.0  
**Author**: chientrm  
**Last Updated**: June 2025
