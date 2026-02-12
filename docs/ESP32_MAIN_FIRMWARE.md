# ESP32 Main Controller - Firmware Flashing Guide

## Prerequisites

1. **Install PlatformIO**:
```bash
pip install platformio
```

2. **Install USB drivers** for ESP32 (CP210x or CH340)

## Configuration

1. **Edit include/config.h**:
   - Set your WiFi credentials
   - Set backend URL (your server's IP address)
   - Set emergency phone number
   - Set mobile operator APN
   - Verify API key matches backend configuration

## Flashing Firmware

1. **Connect ESP32** via USB

2. **Build and upload**:
```bash
cd esp32-main
pio run -t upload
```

3. **Monitor serial output**:
```bash
pio device monitor
```

## Testing

### Test 1: PIR Sensors
- Walk in front of sensors
- Check serial monitor for "PIR Left/Middle/Right triggered"
- Should detect human when ≥2 sensors trigger within 2 seconds

### Test 2: WiFi Connection
- Check serial monitor for "WiFi connected" message
- Verify IP address is shown

### Test 3: Backend Communication
- Trigger detection
- Check backend logs for received alert
- Verify alert appears in `/api/v1/burglary/alert/feeds`

### Test 4: SMS Fallback
- Disable WiFi on router
- Trigger detection
- Verify SMS received on emergency phone

### Test 5: Camera Trigger
- Connect oscilloscope or LED to GPIO 4
- Trigger detection
- Verify 100ms pulse sent to ESP32-CAM

## Troubleshooting

**WiFi won't connect:**
- Check SSID and password in config.h
- Verify ESP32 is in range
- Check router settings (2.4GHz only)

**GSM module not responding:**
- Check wiring (TX→RX, RX→TX)
- Verify SIM card is inserted and activated
- Check power supply (SIM800L needs 2A peak)
- Test SIM card in phone first

**Backend communication fails:**
- Verify backend URL is correct
- Check API key matches
- Ensure backend is running
- Check firewall settings

**PIR sensors always triggered:**
- Check sensor sensitivity potentiometer
- Verify pull-down resistors
- Test sensors individually
- Check for environmental interference

## Pin Connections

| Component | ESP32 Pin | Notes |
|-----------|-----------|-------|
| PIR Left | GPIO 22 | Input, pull-down |
| PIR Middle | GPIO 23 | Input, pull-down |
| PIR Right | GPIO 21 | Input, pull-down |
| SIM800L TX | GPIO 17 | ESP32 RX |
| SIM800L RX | GPIO 16 | ESP32 TX |
| Buzzer | GPIO 25 | DAC pin |
| Camera Trigger | GPIO 4 | Output to ESP32-CAM GPIO 13 |
| Status LED | GPIO 2 | Built-in LED |
