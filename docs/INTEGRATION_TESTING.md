# Integration Testing Guide

This guide covers end-to-end testing of the GSM Burglary Alert System.

## Prerequisites

- Backend server running and accessible
- ESP32 main controller flashed and powered
- ESP32-CAM flashed and powered
- Both devices connected to same WiFi network
- SIM800L module installed on ESP32 main (for SMS testing)
- Telegram bot configured (optional)

## Test Environment Setup

### 1. Backend Startup

```bash
cd backend
source venv/bin/activate  # If using virtualenv
uvicorn main:app --reload --host 0.0.0.0 --port 8000
```

Verify at: http://localhost:8000/docs

### 2. Configure Devices

**ESP32 Main (include/config.h)**:
```cpp
#define WIFI_SSID "YourWiFi"
#define WIFI_PASSWORD "password"
#define BACKEND_URL "http://192.168.1.100:8000"
#define API_KEY "esp32_device_key_xyz789"
#define EMERGENCY_PHONE "+1234567890"
```

**ESP32-CAM (include/config.h)**:
```cpp
#define WIFI_SSID "YourWiFi"  // Same as main
#define WIFI_PASSWORD "password"
#define BACKEND_URL "http://192.168.1.100:8000/api/v1/burglary/image/image"
#define API_KEY "esp32_device_key_xyz789"  // Same key
```

### 3. Hardware Connections

```
ESP32 Main GPIO 4  →  ESP32-CAM GPIO 13  (Trigger signal)
ESP32 Main GND     →  ESP32-CAM GND      (Common ground)
```

Optional UART backup:
```
ESP32 Main GPIO 32 →  ESP32-CAM RX (GPIO 1)
ESP32 Main GPIO 33 →  ESP32-CAM TX (GPIO 3)
```

## Test Scenarios

### Test 1: Basic Motion Detection (WiFi Online)

**Objective**: Verify complete flow from motion → backend

**Steps**:
1. Power on both ESP32 devices
2. Monitor serial outputs on both
3. Wave hand in front of 2+ PIR sensors
4. Observe sequence:
   - ESP32 main: "Human detected!"
   - ESP32 main: "Triggering ESP32-CAM..."
   - ESP32-CAM: "CAPTURE TRIGGERED"
   - ESP32 main: "Alert posted to backend successfully"
   - ESP32-CAM: "Image uploaded to backend successfully!"

**Verification**:
```bash
# Check alert endpoint
curl -X POST http://localhost:8000/api/v1/burglary/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username":"admin","password":"admin123"}' | jq -r '.access_token'

# Use token to get feeds
curl -X GET http://localhost:8000/api/v1/burglary/alert/feeds \
  -H "Authorization: Bearer YOUR_TOKEN"
```

**Expected Result**:
- Alert appears in feeds with `correlated: true`
- Image linked to alert (`image_id` not null)
- Timestamp within ±5 seconds
- **Total time**: <10 seconds from motion to image in backend

---

### Test 2: Telegram Integration

**Objective**: Verify automatic image forwarding to Telegram

**Prerequisites**:
1. Create Telegram bot via @BotFather
2. Get chat ID (send message to bot, check https://api.telegram.org/bot<TOKEN>/getUpdates)
3. Configure via backend API:

```bash
curl -X POST http://localhost:8000/api/v1/burglary/telegram/config \
  -H "Authorization: Bearer YOUR_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "bot_token": "YOUR_BOT_TOKEN",
    "chat_id": "YOUR_CHAT_ID",
    "active": true
  }'
```

**Steps**:
1. Trigger motion detection
2. Wait for image upload
3. Check Telegram chat

**Expected Result**:
- Telegram message received with image
- Caption includes: timestamp, confidence, sensors triggered

---

### Test 3: SMS Fallback (WiFi Offline)

**Objective**: Verify SMS alert when network unavailable

**Steps**:
1. Disable WiFi router OR set wrong WiFi credentials
2. Reflash ESP32 main
3. Wait for "WiFi connection failed" message
4. Trigger motion detection
5. Check ESP32 main serial: "Sending SMS..."

**Expected Result**:
- SMS received on emergency phone
- Format: "INTRUDER ALERT! Motion detected at [time]. Image may follow."
- Alert stored in EEPROM (optional - check if implemented)

**Note**: SIM800L requires:
- Active SIM card with credit
- Correct APN setting
- Good GSM signal strength
- 5V 2A power supply

---

### Test 4: Image Correlation Accuracy

**Objective**: Test correlation algorithm precision

**Steps**:
1. Manually send alert without image:
```bash
curl -X POST http://localhost:8000/api/v1/burglary/alert/alert \
  -H "X-API-Key: esp32_device_key_xyz789" \
  -H "Content-Type: application/json" \
  -d '{
    "timestamp": '$(date +%s000)',
    "detection_confidence": 0.85,
    "pir_left": true,
    "pir_middle": true,
    "pir_right": false,
    "network_status": "online"
  }'
```

2. Within 5 seconds, upload test image:
```bash
curl -X POST http://localhost:8000/api/v1/burglary/image/image \
  -H "X-API-Key: esp32_device_key_xyz789" \
  -F "file=@test_image.jpg"
```

3. Check feeds - alert should have `correlated: true`

**Expected Result**:
- Images within ±5 second window correlate correctly
- Images >5 seconds apart don't correlate
- Oldest un correlated alert matched first

---

### Test 5: Offline Queue Recovery (ESP32-CAM)

**Objective**: Verify SPIFFS queuing and auto-upload

**Steps**:
1. Disable WiFi on ESP32-CAM (change SSID to wrong value)
2. Reflash ESP32-CAM
3. Trigger 3 captures (via GPIO 13 or ESP32 main)
4. Check serial: "Image queued in SPIFFS for later upload"
5. Fix WiFi credentials and reflash
6. Wait up to 30 seconds

**Expected Result**:
- All 3 images uploaded automatically
- SPIFFS queue emptied
- Serial shows "Uploading queued image..."

---

### Test 6: Rate Limiting

**Objective**: Verify SMS rate limiting works

**Steps**:
1. Trigger motion detection (SMS sent)
2. Immediately trigger again within 5 minutes
3. Check serial output

**Expected Result**:
- First SMS sent successfully
- Second SMS blocked: "SMS rate limit active"
- No duplicate SMS received

---

### Test 7: Buzzer Pattern

**Objective**: Verify audible alert

**Steps**:
1. Connect buzzer to GPIO 25 on ESP32 main
2. Trigger motion detection

**Expected Result**:
- 3 beeps at 2kHz frequency
- Pattern: 200ms ON, 100ms OFF, repeat 3x

---

### Test 8: Weak WiFi Signal

**Objective**: Test behavior with poor connectivity

**Steps**:
1. Move ESP devices far from router (RSSI < -70 dBm)
2. Trigger motion detection

**Expected Result**:
- Upload may be slower but succeeds
- OR falls back to SPIFFS queue if signal too weak
- No crashes or freezes

---

### Test 9: Power Cycle Recovery

**Objective**: Verify devices recover gracefully

**Steps**:
1. Power off both devices
2. Wait 10 seconds
3. Power on
4. Check serial for successful initialization

**Expected Result**:
- Both devices reconnect to WiFi
- NTP re-synchronized
- Queued images uploaded (if any)
- System ready within 30 seconds

---

### Test 10: Concurrent Triggers

**Objective**: Test rapid multiple detections

**Steps**:
1. Trigger motion 3 times rapidly
2. Observe cooldown behavior

**Expected Result**:
- First detection processed
- Subsequent detections within 10s cooldown ignored
- Message: "Trigger ignored - cooldown active"

---

## Performance Benchmarks

| Metric | Target | Acceptable |
|--------|--------|------------|
| Motion → Camera Trigger | <100ms | <500ms |
| Camera Capture | ~2s | <5s |
| Image Upload (WiFi) | 3-5s | <10s |
| Total Flow (Motion → Backend) | <10s | <15s |
| Correlation Accuracy | >95% | >90% |
| SMS Delivery | <15s | <30s |
| Telegram Delivery | <5s | <10s |

## Common Issues

**Correlation fails**:
- Check system clocks are synchronized
- Verify NTP is working on ESP32-CAM
- Check correlation window size (5s default)

**Images not received**:
- Verify multipart/form-data format
- Check API key header
- Ensure backend accepts JPEG format
- Test with smaller image (reduce JPEG_QUALITY)

**SMS not sent**:
- Check SIM card balance
- Verify APN settings
- Test SIM in phone first
- Check GSM signal strength (AT+CSQ)

**Slow upload times**:
- Check WiFi signal strength
- Reduce image quality (increase JPEG_QUALITY value)
- Test network bandwidth

## Success Criteria

✅ Complete Phase 1 when:
- [ ] Motion detection triggers camera 100% of time
- [ ] Images upload successfully >90% of attempts (good WiFi)
- [ ] Correlation works within ±5 second window
- [ ] SMS fallback functions when WiFi down
- [ ] Telegram forwarding works (if configured)
- [ ] SPIFFS queue uploads on reconnection
- [ ] No memory leaks after 24-hour runtime
- [ ] All LEDs and buzzer patterns work as expected

## Next Phase

After integration testing passes, proceed to Phase 2:
- Backend refinements (pagination, filtering)
- ESP-NOW coordination (optional)
- UART backup communication
- Flutter mobile app development
