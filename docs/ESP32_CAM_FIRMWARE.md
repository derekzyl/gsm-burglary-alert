# ESP32-CAM Firmware - Flashing and Testing Guide

## Prerequisites

1. **Install PlatformIO**:
```bash
pip install platformio
```

2. **ESP32-CAM Board**: AI Thinker ESP32-CAM module
3. **USB to Serial Programmer** (FTDI or CP2102)

## Wiring for Programming

| ESP32-CAM Pin | USB Programmer |
|---------------|----------------|
| 5V | 5V |
| GND | GND |
| U0R (GPIO 3) | TX |
| U0T (GPIO 1) | RX |
| IO0 | GND (for flashing only) |

**IMPORTANT**: Connect IO0 to GND before powering on to enter flash mode. Disconnect after upload.

## Configuration

1. **Edit include/config.h**:
   - Set WiFi credentials (same as ESP32 main)
   - Set backend URL
   - Verify API key matches backend
   - Adjust NTP server and timezone offset

## Flashing Firmware

1. **Connect ESP32-CAM** with IO0 grounded

2. **Build and upload**:
```bash
cd esp32-cam
pio run -t upload
```

3. **Remove IO0 ground connection**

4. **Press reset button** on ESP32-CAM

5. **Monitor serial output**:
```bash
pio device monitor
```

## Expected Boot Sequence

```
==================================
ESP32-CAM Burglary Alert System
==================================
Trigger input configured on GPIO 13
✓ SPIFFS ready
Initializing camera...
Camera initialized successfully!
Frame size: 1600x1200, JPEG quality: 12
Taking test photo...
✓ Test photo successful: 156234 bytes

--- WiFi Setup ---
Connecting to WiFi: YourSSID
........
WiFi connected!
IP address: 192.168.1.101
Signal strength: -52 dBm

--- NTP Time Sync ---
Synchronizing time with NTP server...
Time synchronized: 1707847200 (epoch)
Formatted: 12:00:00
✓ NTP synchronized

--- Checking Image Queue ---
No queued images to upload

--- System Ready ---
Waiting for trigger signal...
```

## Testing

### Test 1: Camera Functionality
- Power on ESP32-CAM
- Check for test photo success in serial monitor
- Verify image size (~150KB for UXGA JPEG quality 12)

### Test 2: Trigger Signal
**Hardware setup:**
- Connect GPIO 13 to 3.3V momentarily
- OR connect to ESP32 main GPIO 4

**Expected behavior:**
1. LED blinks 3 times rapidly
2. Serial shows "CAPTURE TRIGGERED"
3. Image capture confirmation
4. Upload attempt (if WiFi connected)

### Test 3: WiFi Upload
- Ensure backend is running and accessible
- Trigger capture
- Check serial for "Image uploaded to backend successfully!"
- Verify image appears in backend `/api/v1/burglary/image/image` endpoint
- Check backend logs for received image
- Verify Telegram forwarding (if configured)

### Test 4: Offline Mode (SPIFFS)
1. Disconnect WiFi router or disable WiFi in config
2. Flash firmware
3. Trigger capture
4. Verify "Image queued in SPIFFS for later upload"
5. Re-enable WiFi connection
6. Wait up to 30 seconds
7. Should see "Uploading queued image..."

### Test 5: Integration with ESP32 Main
1. Flash both ESP32 main and ESP32-CAM
2. Connect GPIO 4 (main) to GPIO 13 (CAM)
3. Trigger motion detection on ESP32 main
4. Verify camera trigger pulse received
5. Check both serial monitors for coordinated action
6. Verify image appears in backend within 10 seconds

## LED Status Indicators

| Pattern | Meaning |
|---------|---------|
| Slow blink (500ms) | WiFi connected, standby |
| 3 rapid blinks | Capturing image |
| 2 long blinks (200ms) | Upload successful |
| 3 rapid blinks | Saved to SPIFFS (offline) |
| 5 very fast blinks | Error occurred |

## Troubleshooting

**Camera init failed:**
- Check camera ribbon cable connection
- Verify AI Thinker board model
- Try power cycle (reset button)
- Check power supply (needs stable 5V 2A)

**WiFi connection failed:**
- Verify SSID and password
- Check 2.4GHz WiFi availability (ESP32 doesn't support 5GHz)
- Move closer to router
- Check router MAC filtering

**Upload failed:**
- Verify backend URL is correct and accessible
- Check API key matches backend config
- Ensure backend is running
- Check firewall rules
- Test backend endpoint with curl

**NTP sync failed:**
- Check internet connectivity
- Try different NTP server (e.g., "time.google.com")
- Verify router allows NTP (UDP port 123)

**SPIFFS errors:**
- Reflash with "Erase Flash" option in PlatformIO
- Check partition table in platformio.ini
- Verify filesystem is mounted

**Trigger not detected:**
- Check GPIO 13 wiring
- Verify interrupt attachment in serial monitor
- Test with multimeter (should see 0V→3.3V pulse)
- Check trigger signal duration (needs >100ms)

## Pin Connections

| Function | ESP32-CAM Pin | Notes |
|----------|---------------|-------|
| Trigger Input | GPIO 13 | From ESP32 main GPIO 4 |
| Status LED | GPIO 33 | Built-in LED |
| Flash LED | GPIO 4 | Optional, not used |
| Camera | Multiple | Pre-wired on AI Thinker |

## SPIFFS Image Queue

- Maximum 20 images stored offline
- Oldest images deleted when limit reached
- Queue checked every 30 seconds when WiFi available
- Manual queue trigger on WiFi reconnection

## Performance

- Image capture: ~2 seconds
- JPEG compression: ~1 second
- Upload time: 3-5 seconds (depends on WiFi speed)
- **Total time (trigger → backend)**: ~6-8 seconds
- SPIFFS write: ~500ms
- SPIFFS read: ~300ms

## Next Steps

After successful testing:
1. Mount ESP32-CAM in weatherproof enclosure
2. Position for optimal coverage
3. Add IR LED for night vision (optional)
4. Consider external antenna for better WiFi range
5. Proceed to backend correlation testing
