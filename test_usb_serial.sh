#!/bin/bash

# ESP32 USB Serial Test Script
# This script demonstrates how to control the ESP32 music visualizer via USB serial

# Find the ESP32 device (usually /dev/ttyUSB0 on Linux, /dev/cu.usbserial-* on Mac)
DEVICE="/dev/ttyUSB0"  # Change this to match your system

# Check if device exists
if [ ! -e "$DEVICE" ]; then
    echo "ESP32 device not found at $DEVICE"
    echo "Try running: ls /dev/tty* | grep -E '(USB|ACM)'"
    exit 1
fi

echo "=== ESP32 Music Visualizer USB Serial Test ==="
echo "Device: $DEVICE"
echo ""

# Function to send command and wait
send_command() {
    echo "Sending: $1"
    echo "$1" > "$DEVICE"
    sleep 2
}

# Test sequence
echo "Starting test sequence..."
echo ""

send_command "status"
send_command "rainbow"
send_command "solid"
send_command "red"
send_command "green"
send_command "blue"
send_command "yellow"
send_command "white"
send_command "brightness:64"
send_command "music"
send_command "music:75"
send_command "music:50"
send_command "music:100"
send_command "off"

echo ""
echo "Test sequence complete!"
echo ""
echo "To manually send commands:"
echo "  echo 'rainbow' > $DEVICE"
echo "  echo 'red' > $DEVICE"
echo "  echo 'brightness:128' > $DEVICE"
echo ""
echo "To monitor responses:"
echo "  cat $DEVICE"
echo "  # or use: pio device monitor"
