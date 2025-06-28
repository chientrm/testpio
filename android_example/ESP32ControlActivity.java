// Example Android Activity for ESP32 USB Serial Control
// Add to app/build.gradle: implementation 'com.github.mik3y:usb-serial-for-android:3.7.3'

package com.yourpackage.esp32musicvisualizer;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;

import com.hoho.android.usbserial.driver.UsbSerialDriver;
import com.hoho.android.usbserial.driver.UsbSerialPort;
import com.hoho.android.usbserial.driver.UsbSerialProber;

import java.io.IOException;
import java.util.List;

public class ESP32ControlActivity extends Activity {
    private static final String TAG = "ESP32Control";
    private static final String ACTION_USB_PERMISSION = "com.yourpackage.esp32musicvisualizer.USB_PERMISSION";
    
    private UsbSerialPort serialPort;
    private TextView statusText;
    private UsbManager usbManager;
    
    private final BroadcastReceiver usbReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (ACTION_USB_PERMISSION.equals(action)) {
                synchronized (this) {
                    UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                    if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                        if(device != null){
                            connectToESP32();
                        }
                    } else {
                        updateStatus("USB permission denied");
                    }
                }
            }
        }
    };
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_esp32_control);
        
        usbManager = (UsbManager) getSystemService(Context.USB_SERVICE);
        statusText = findViewById(R.id.statusText);
        setupButtons();
        setupBrightnessControl();
        
        // Register USB permission receiver
        IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
        registerReceiver(usbReceiver, filter);
        
        connectToESP32();
    }
    
    private void connectToESP32() {
        List<UsbSerialDriver> availableDrivers = UsbSerialProber.getDefaultProber().findAllDrivers(usbManager);
        
        if (availableDrivers.isEmpty()) {
            updateStatus("No ESP32 device found");
            return;
        }
        
        UsbSerialDriver driver = availableDrivers.get(0);
        UsbDevice device = driver.getDevice();
        
        if (!usbManager.hasPermission(device)) {
            PendingIntent permissionIntent = PendingIntent.getBroadcast(this, 0, 
                new Intent(ACTION_USB_PERMISSION), PendingIntent.FLAG_IMMUTABLE);
            usbManager.requestPermission(device, permissionIntent);
            updateStatus("Requesting USB permission...");
            return;
        }
        
        UsbDeviceConnection connection = usbManager.openDevice(device);
        
        if (connection == null) {
            updateStatus("Failed to open device");
            return;
        }
        
        try {
            serialPort = driver.getPorts().get(0);
            serialPort.open(connection);
            serialPort.setParameters(115200, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE);
            updateStatus("Connected to ESP32");
            
            // Request initial status
            sendCommand("status");
            
        } catch (IOException e) {
            updateStatus("Error: " + e.getMessage());
        }
    }
    
    private void setupButtons() {
        findViewById(R.id.btnOff).setOnClickListener(v -> sendCommand("off"));
        findViewById(R.id.btnRainbow).setOnClickListener(v -> sendCommand("rainbow"));
        findViewById(R.id.btnSolid).setOnClickListener(v -> sendCommand("solid"));
        findViewById(R.id.btnMusic).setOnClickListener(v -> sendCommand("music"));
        
        findViewById(R.id.btnRed).setOnClickListener(v -> {
            sendCommand("solid");
            sendCommand("red");
        });
        findViewById(R.id.btnGreen).setOnClickListener(v -> {
            sendCommand("solid");
            sendCommand("green");
        });
        findViewById(R.id.btnBlue).setOnClickListener(v -> {
            sendCommand("solid");
            sendCommand("blue");
        });
        findViewById(R.id.btnYellow).setOnClickListener(v -> {
            sendCommand("solid");
            sendCommand("yellow");
        });
        
        // Built-in LED controls
        findViewById(R.id.btnLedOn).setOnClickListener(v -> sendCommand("ledon"));
        findViewById(R.id.btnLedOff).setOnClickListener(v -> sendCommand("ledoff"));
        findViewById(R.id.btnLedToggle).setOnClickListener(v -> sendCommand("toggle"));
    }
    
    private void setupBrightnessControl() {
        SeekBar brightnessSeekBar = findViewById(R.id.brightnessSeekBar);
        brightnessSeekBar.setMax(255);
        brightnessSeekBar.setProgress(128); // 50% default
        
        brightnessSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser) {
                    sendCommand("brightness:" + progress);
                }
            }
            
            @Override public void onStartTrackingTouch(SeekBar seekBar) {}
            @Override public void onStopTrackingTouch(SeekBar seekBar) {}
        });
    }
    
    private void sendCommand(String command) {
        if (serialPort == null) {
            updateStatus("Not connected");
            return;
        }
        
        try {
            String fullCommand = command + "\n";
            serialPort.write(fullCommand.getBytes(), 1000);
            Log.d(TAG, "Sent: " + command);
            
            // Read response (optional)
            byte[] buffer = new byte[256];
            int len = serialPort.read(buffer, 1000);
            if (len > 0) {
                String response = new String(buffer, 0, len).trim();
                Log.d(TAG, "Response: " + response);
                if (response.startsWith("RESPONSE:")) {
                    updateStatus(response.substring(9));
                }
            }
            
        } catch (IOException e) {
            updateStatus("Send error: " + e.getMessage());
        }
    }
    
    private void updateStatus(String message) {
        runOnUiThread(() -> {
            statusText.setText(message);
            Log.d(TAG, "Status: " + message);
        });
    }
    
    // Music visualization example
    private void startMusicVisualization() {
        // This would integrate with Android's audio analysis
        // For demo purposes, send random beat data
        new Thread(() -> {
            while (!isDestroyed()) {
                try {
                    // Simulate beat detection (0-100)
                    int beatIntensity = (int)(Math.random() * 100);
                    sendCommand("music:" + beatIntensity);
                    Thread.sleep(50); // 20 FPS
                } catch (InterruptedException e) {
                    break;
                }
            }
        }).start();
    }
    
    @Override
    protected void onDestroy() {
        super.onDestroy();
        unregisterReceiver(usbReceiver);
        if (serialPort != null) {
            try {
                serialPort.close();
            } catch (IOException e) {
                Log.e(TAG, "Error closing port", e);
            }
        }
    }
}

/* Layout file: res/layout/activity_esp32_control.xml

<?xml version="1.0" encoding="utf-8"?>
<LinearLayout xmlns:android="http://schemas.android.com/apk/res/android"
    android:layout_width="match_parent"
    android:layout_height="match_parent"
    android:orientation="vertical"
    android:padding="16dp">
    
    <TextView
        android:id="@+id/statusText"
        android:layout_width="match_parent"
        android:layout_height="wrap_content"
        android:text="Connecting..."
        android:textSize="16sp"
        android:padding="8dp"
        android:background="#E0E0E0" />
    
    <TextView
        android:layout_width="match_parent"
        android:layout_height="wrap_content"
        android:text="LED Strip Modes"
        android:textSize="18sp"
        android:textStyle="bold"
        android:layout_marginTop="16dp" />
    
    <LinearLayout
        android:layout_width="match_parent"
        android:layout_height="wrap_content"
        android:orientation="horizontal">
        
        <Button android:id="@+id/btnOff"
            android:layout_width="0dp"
            android:layout_weight="1"
            android:layout_height="wrap_content"
            android:text="OFF" />
            
        <Button android:id="@+id/btnRainbow"
            android:layout_width="0dp"
            android:layout_weight="1"
            android:layout_height="wrap_content"
            android:text="Rainbow" />
            
        <Button android:id="@+id/btnSolid"
            android:layout_width="0dp"
            android:layout_weight="1"
            android:layout_height="wrap_content"
            android:text="Solid" />
            
        <Button android:id="@+id/btnMusic"
            android:layout_width="0dp"
            android:layout_weight="1"
            android:layout_height="wrap_content"
            android:text="Music" />
    </LinearLayout>
    
    <TextView
        android:layout_width="match_parent"
        android:layout_height="wrap_content"
        android:text="Colors"
        android:textSize="18sp"
        android:textStyle="bold"
        android:layout_marginTop="16dp" />
    
    <LinearLayout
        android:layout_width="match_parent"
        android:layout_height="wrap_content"
        android:orientation="horizontal">
        
        <Button android:id="@+id/btnRed"
            android:layout_width="0dp"
            android:layout_weight="1"
            android:layout_height="wrap_content"
            android:text="Red"
            android:backgroundTint="#FF0000" />
            
        <Button android:id="@+id/btnGreen"
            android:layout_width="0dp"
            android:layout_weight="1"
            android:layout_height="wrap_content"
            android:text="Green"
            android:backgroundTint="#00FF00" />
            
        <Button android:id="@+id/btnBlue"
            android:layout_width="0dp"
            android:layout_weight="1"
            android:layout_height="wrap_content"
            android:text="Blue"
            android:backgroundTint="#0000FF" />
            
        <Button android:id="@+id/btnYellow"
            android:layout_width="0dp"
            android:layout_weight="1"
            android:layout_height="wrap_content"
            android:text="Yellow"
            android:backgroundTint="#FFFF00" />
    </LinearLayout>
    
    <TextView
        android:layout_width="match_parent"
        android:layout_height="wrap_content"
        android:text="Brightness"
        android:textSize="18sp"
        android:textStyle="bold"
        android:layout_marginTop="16dp" />
    
    <SeekBar
        android:id="@+id/brightnessSeekBar"
        android:layout_width="match_parent"
        android:layout_height="wrap_content"
        android:layout_marginTop="8dp" />
    
    <TextView
        android:layout_width="match_parent"
        android:layout_height="wrap_content"
        android:text="Built-in LED"
        android:textSize="18sp"
        android:textStyle="bold"
        android:layout_marginTop="16dp" />
    
    <LinearLayout
        android:layout_width="match_parent"
        android:layout_height="wrap_content"
        android:orientation="horizontal">
        
        <Button android:id="@+id/btnLedOn"
            android:layout_width="0dp"
            android:layout_weight="1"
            android:layout_height="wrap_content"
            android:text="LED ON"
            android:backgroundTint="#28a745" />
            
        <Button android:id="@+id/btnLedOff"
            android:layout_width="0dp"
            android:layout_weight="1"
            android:layout_height="wrap_content"
            android:text="LED OFF"
            android:backgroundTint="#dc3545" />
            
        <Button android:id="@+id/btnLedToggle"
            android:layout_width="0dp"
            android:layout_weight="1"
            android:layout_height="wrap_content"
            android:text="TOGGLE"
            android:backgroundTint="#007bff" />
    </LinearLayout>
        
</LinearLayout>

*/
