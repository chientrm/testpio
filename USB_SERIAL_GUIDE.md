# USB Serial Control Guide

Control your ESP32 Music Visualizer via USB serial connection for direct, low-latency command access.

## 🔌 Connection Setup

### Hardware

- USB cable (USB-A to USB-C or USB-C to USB-C)
- Connect ESP32 to computer or any device with USB serial capability

### Serial Settings

- **Baud Rate**: 115200
- **Data Bits**: 8
- **Stop Bits**: 1
- **Parity**: None

## 📟 Available Commands

### LED Strip Control

```
off          - Turn off LED strip
solid        - Set to solid color mode
rainbow      - Rainbow animation mode
visualizer   - Animated visualizer mode
```

### Colors (for solid mode)

```
red          - Set color to red
green        - Set color to green
blue         - Set color to blue
yellow       - Set color to yellow
white        - Set color to white
```

### Built-in LED Control

```
ledon        - Turn on built-in LED
ledoff       - Turn off built-in LED
toggle       - Toggle built-in LED
```

### System Commands

```
ping         - Test connection (returns PONG)
status       - Get current status
info         - Get device information
brightness:N - Set brightness (0-255)
```

### Auto-Update Commands

```
update:check    - Check for firmware updates
update:enable   - Enable automatic updates
update:disable  - Disable automatic updates
update:now      - Force update now
```

## 💬 Response Format

All commands return responses prefixed with `RESPONSE:`:

```
> ping
RESPONSE:PONG

> solid
RESPONSE:Strip Solid Color

> red
RESPONSE:Color Red

> status
RESPONSE:Mode=1,LED=false,WiFi=192.168.1.100,USB=connected,Version=1.0.0
```

## 🖥️ Platform-Specific Tools

### Windows

- **PuTTY** - Popular terminal emulator
- **Arduino IDE Serial Monitor**
- **Windows Terminal** with PowerShell
- **Tera Term**

### macOS/Linux

- **Screen**: `screen /dev/ttyUSB0 115200`
- **Minicom**: `minicom -D /dev/ttyUSB0 -b 115200`
- **Arduino IDE Serial Monitor**
- **PlatformIO Serial Monitor**: `pio device monitor`

### Programming Languages

- **Python**: `pyserial` library
- **Node.js**: `serialport` package
- **C++**: Platform-specific serial libraries
- **Any language** with serial communication support

## 🐍 Python Example

```python
import serial
import time

# Connect to ESP32
ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)
time.sleep(2)  # Wait for connection

# Send commands
ser.write(b'ping\\n')
response = ser.readline().decode().strip()
print(f"Response: {response}")

ser.write(b'rainbow\\n')
response = ser.readline().decode().strip()
print(f"Response: {response}")

ser.close()
```

## 🔧 Testing Commands

1. **Connect via Serial Monitor**

   ```bash
   pio device monitor
   ```

2. **Test Basic Commands**

   ```
   ping
   status
   off
   rainbow
   solid
   red
   ```

3. **Check Responses**
   All commands should return `RESPONSE:` messages

## 🎯 Use Cases

- **Development**: Quick testing during firmware development
- **Automation**: Script-based LED control
- **Integration**: Embed ESP32 control in larger systems
- **Debugging**: Direct access for troubleshooting
- **Custom Applications**: Build your own control software

## 🚀 Benefits

- ✅ **No WiFi required** - Works offline
- ✅ **Low latency** - Direct USB communication
- ✅ **Universal** - Works with any device that supports USB serial
- ✅ **Simple protocol** - Easy to implement in any language
- ✅ **Reliable** - Wired connection, no wireless interference

The USB serial interface provides a robust, platform-independent way to control your ESP32 Music Visualizer! 🎵✨
