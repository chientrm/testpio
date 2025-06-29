# ESP32 Over-The-Air (OTA) Update Guide

Your ESP32 Music Visualizer now supports wireless firmware updates! No need to physically connect the device for updates.

## 🌟 Features Added

- **Wireless Updates**: Update firmware over WiFi
- **Progress Monitoring**: Real-time update progress with LED indicators
- **Safety Features**: Automatic LED strip shutdown during updates
- **Device Discovery**: mDNS support for easy network discovery
- **Status Reporting**: OTA status visible in web interface and serial commands

## 🔧 How to Update Firmware

### Method 1: Arduino IDE

1. **Connect to same WiFi network** as your ESP32
2. **Open Arduino IDE**
3. **Go to Tools > Port**
4. **Select Network Port**: You should see `ESP32-MusicViz at [IP_ADDRESS]`
5. **Upload normally**: Click the upload button as usual
6. **Monitor progress**: The ESP32's built-in LED will blink during update

### Method 2: PlatformIO

1. **Make sure ESP32 is connected to WiFi**
2. **Open terminal in project directory**
3. **Run OTA upload**:
   ```bash
   pio run -t upload --upload-port ESP32-MusicViz.local
   ```
   Or using IP address:
   ```bash
   pio run -t upload --upload-port 192.168.1.XXX
   ```

### Method 3: Manual OTA (Advanced)

Using `espota.py` script directly:

```bash
python ~/.platformio/packages/framework-arduinoespressif32/tools/espota.py -i 192.168.1.XXX -f .pio/build/esp32dev/firmware.bin
```

## 🔍 Finding Your Device

### Web Interface Method

1. Open the web interface: `http://[ESP32_IP]`
2. Look for device info section showing:
   - Device Name: `ESP32-MusicViz`
   - Firmware Version
   - OTA Status

### Network Discovery

- **mDNS Name**: `ESP32-MusicViz.local`
- **Ping test**: `ping ESP32-MusicViz.local`
- **Web access**: `http://ESP32-MusicViz.local`

### Serial Command

Send `info` command via USB serial to get device information:

```
info
```

Response includes device name, version, and IP address.

## 🚨 Safety Features

### During OTA Update:

- ✅ Built-in LED indicates update progress (blinking)
- ✅ LED strip is turned off to prevent conflicts
- ✅ Web interface shows update status
- ✅ Serial interface remains responsive
- ✅ Automatic recovery on failure

### Update States:

- **Ready**: Normal operation, ready for updates
- **Starting**: Update beginning, LEDs turning off
- **Progress X%**: Update in progress with percentage
- **Complete**: Update finished, device restarting
- **Failed**: Update failed with error details

## 🔧 Troubleshooting

### Device Not Found in Arduino IDE

1. **Check WiFi**: Ensure ESP32 is connected to WiFi
2. **Check network**: Ensure computer and ESP32 are on same network
3. **Restart Arduino IDE**: Refresh network ports
4. **Use IP directly**: Use `192.168.1.XXX:3232` as port

### OTA Update Fails

1. **Check network stability**: Ensure strong WiFi signal
2. **Power supply**: Ensure stable power during update
3. **Firewall**: Check if firewall blocks port 3232
4. **Memory**: Ensure sufficient flash memory for new firmware

### Can't Connect After Update

1. **Wait for reboot**: ESP32 restarts after successful update
2. **Check serial**: Use USB serial to check status
3. **Recovery mode**: Hold boot button while powering on for manual flash

## 🛡️ Security Notes

### Optional Password Protection

Uncomment this line in `main.cpp` to enable OTA password:

```cpp
ArduinoOTA.setPassword("your_ota_password");
```

### Network Security

- OTA updates work only on local network
- Device must be connected to WiFi
- Consider using separate IoT VLAN for smart devices

## 📊 Monitoring Updates

### Web Interface

- Real-time OTA status display
- Device information panel
- Current firmware version

### Serial Interface

- `status` command shows OTA status
- `info` command shows device details
- Automatic progress updates during OTA

### LED Indicators

- **Solid ON**: OTA starting
- **Blinking**: Update in progress
- **OFF**: Normal operation or update complete

## 🚀 Version Management

Current firmware version: **1.2.0**

To update version number, edit `FIRMWARE_VERSION` in `main.cpp`:

```cpp
const String FIRMWARE_VERSION = "1.3.0";
```

## 📝 Update Log

### v1.2.0 (Current)

- ✅ Added OTA update support
- ✅ Added mDNS device discovery
- ✅ Enhanced web interface with device info
- ✅ Added serial `info` command
- ✅ Improved update safety features

### Previous Versions

- v1.1.0: USB serial control, Android app support
- v1.0.0: Basic WiFi control, LED strip support

---

**Ready to update wirelessly! 🚀**

Your ESP32 Music Visualizer can now be updated remotely without physical access. Perfect for installations in hard-to-reach places!
