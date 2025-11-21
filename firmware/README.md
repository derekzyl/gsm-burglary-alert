# ESP32 Firmware - GSM Burglary Alarm System

Firmware for ESP32-based burglary alarm system with GSM (SIM800L) and WiFi connectivity.

## Features

- Multi-sensor monitoring (PIR, magnetic, shock, tamper)
- GSM SMS/call alerts via SIM800L
- WiFi HTTP notifications
- Configurable alarm delay
- Silent mode (notifications only)
- Heartbeat to backend server
- Automatic sensor registration

## Hardware Configuration

### Pin Assignments (Default)

**GSM Module (SIM800L):**
- RX Pin: GPIO 16 (ESP32)
- TX Pin: GPIO 17 (ESP32)
- Power Control: GPIO 4
- Reset: GPIO 5

**Sensors:**
- PIR Sensor 1: GPIO 27
- PIR Sensor 2: GPIO 26
- Magnetic Sensor 1 (Door): GPIO 25
- Magnetic Sensor 2 (Window): GPIO 33
- Shock Sensor: GPIO 32
- Tamper Switch: GPIO 14

**Actuators:**
- Siren Relay: GPIO 12
- Buzzer: GPIO 13
- LED Status: GPIO 2 (built-in)

### Power Requirements

**Critical:** SIM800L requires stable power supply with peak current capability of ~2A during transmission.

**Recommended Power Setup:**
1. 12V power adapter (3A+) for main power
2. 12V SLA battery (7Ah) for backup
3. 5V buck converter (LM2596) for ESP32 (12V → 5V)
4. Diode OR circuit for automatic battery backup
5. Capacitors: 1000µF + 470µF + 100nF near SIM800L

### GSM Antenna

- **Critical:** GSM antenna must be properly connected for reliable operation
- Place antenna in location with good signal strength
- Use external antenna if installing in enclosure with poor signal

## Configuration

### 1. Edit Configuration File

Edit `include/config.h`:

```cpp
// WiFi Configuration
#define WIFI_SSID "YourWiFiSSID"
#define WIFI_PASSWORD "YourWiFiPassword"

// Backend API
#define API_BASE_URL "http://your-backend-url:8000/api/v1"
#define DEVICE_ID "ESP32_001"  // Unique per device - change for each device!

// Phone Numbers for SMS Alerts
#define PHONE_NUMBER_1 "+1234567890"
#define PHONE_NUMBER_2 "+0987654321"
```

### 2. Adjust Pin Assignments (Optional)

If you need to change pin assignments, edit the pin definitions in `include/config.h`:

```cpp
#define PIR_SENSOR_1_PIN 27
#define MAGNETIC_SENSOR_1_PIN 25
// ... etc
```

### 3. Device ID

**Important:** Each ESP32 device must have a unique `DEVICE_ID`. This ID is used for authentication with the backend API.

Format: `ESP32_XXX` where XXX is a unique number or identifier.

## Building and Uploading

### Prerequisites

1. Install PlatformIO:
```bash
pip install platformio
```

2. Install PlatformIO CLI (if not already installed):
```bash
# On Linux/Mac
brew install platformio  # or
pip install -U platformio

# Or use PlatformIO IDE extension for VS Code
```

### Build

```bash
cd firmware
pio run
```

### Upload

```bash
pio run -t upload
```

### Monitor Serial Output

```bash
pio device monitor
```

Or specify baud rate:
```bash
pio device monitor -b 115200
```

## Initial Setup

### 1. Insert SIM Card

1. Power off device
2. Insert activated SIM card into SIM800L module
3. Ensure SIM card has SMS/data plan
4. Test SIM card in a phone first

### 2. Connect Antenna

- Connect GSM antenna to SIM800L module
- Ensure good connection
- Test signal strength via serial monitor

### 3. Power On

1. Connect 12V power supply
2. Power on device
3. Monitor serial output at 115200 baud
4. Watch for GSM initialization messages

### 4. Register Device

1. Register device in mobile app or backend with same `DEVICE_ID`
2. Device will automatically register sensors on first boot
3. Check backend logs to verify registration

## Operation

### System States

- **DISARMED**: System is off, no alerts
- **ARMING**: Transition state (not used in current firmware)
- **ARMED**: System is active, monitoring sensors
- **ALARM**: Alarm is triggered
- **SILENT**: Alarms trigger notifications only, no siren

### Sensor Behavior

**PIR Sensors:**
- Trigger on motion detection
- 1-second debounce to prevent false triggers
- Continuous monitoring when armed

**Magnetic Sensors:**
- Trigger when door/window opens (magnet moves away)
- Uses pull-up resistors (NO contacts)
- Debounced to prevent false triggers

**Shock Sensor:**
- Trigger on vibration/impact
- 500ms debounce
- Useful for tamper/vandalism detection

**Tamper Switch:**
- Highest priority alert
- Immediate alarm when triggered
- Mounted on enclosure

### Alarm Flow

1. **Sensor Triggered**: When armed, sensor detects event
2. **Alert Reported**: SMS and HTTP notification sent immediately
3. **Delay Period**: Configurable delay (default 30 seconds) before siren activates
4. **Alarm Activation**: Siren activates (unless silent mode)
5. **Notification**: Push notification sent to mobile app

### SMS Commands

Send SMS to SIM card number:

- **ARM** → Arms the system
- **DISARM** → Disarms the system
- **STATUS** → Returns current status

### Heartbeat

Device sends heartbeat to backend every 30 seconds (configurable via `HEARTBEAT_INTERVAL`):

- Reports device status
- Reports battery level (if implemented)
- Reports GSM signal strength
- Receives configuration updates from backend

## Troubleshooting

### GSM Module Not Initializing

1. **Check Power Supply:**
   - Ensure power supply can handle ~2A peak current
   - Check voltage at SIM800L module (should be 3.7-4.2V)
   - Verify capacitor values (1000µF + 470µF + 100nF)

2. **Check SIM Card:**
   - Verify SIM card is inserted correctly
   - Test SIM card in a phone
   - Check if SIM card is activated and has SMS/data plan

3. **Check Antenna:**
   - Ensure antenna is connected
   - Check signal strength via serial monitor
   - Try different antenna location

4. **Check Connections:**
   - Verify RX/TX connections (ESP32 RX → SIM800L TX, ESP32 TX → SIM800L RX)
   - Check power and ground connections
   - Verify serial communication via serial monitor

### WiFi Connection Issues

1. **Check Credentials:**
   - Verify SSID and password in `config.h`
   - Ensure no typos
   - Check WiFi router is operational

2. **Signal Strength:**
   - Check WiFi signal at device location
   - Consider WiFi extender if signal is weak

3. **Router Settings:**
   - Ensure router allows new devices
   - Check if MAC address filtering is enabled
   - Verify DHCP is enabled

### Sensors Not Triggering

1. **Check Connections:**
   - Verify sensor connections to correct GPIO pins
   - Check pull-up resistors for magnetic sensors
   - Test sensors individually

2. **Check Sensor Power:**
   - PIR sensors need 5V power
   - Magnetic sensors use pull-ups (no external power needed)
   - Shock sensor may need power depending on type

3. **Test Sensors:**
   - Use serial monitor to debug sensor readings
   - Check sensor outputs with multimeter
   - Verify sensor sensitivity settings

### Backend Connection Issues

1. **Check API URL:**
   - Verify `API_BASE_URL` in `config.h`
   - Ensure backend is running and accessible
   - Check CORS settings in backend

2. **Check Device ID:**
   - Verify device is registered in backend with same `DEVICE_ID`
   - Device ID in firmware must match backend registration

3. **Check Authentication:**
   - Verify `X-Device-ID` header is being sent
   - Check backend logs for authentication errors

### False Alarms

1. **Adjust Sensor Sensitivity:**
   - PIR sensors: Adjust delay/range potentiometers
   - Shock sensors: Adjust sensitivity threshold

2. **Add Debounce:**
   - Increase debounce delay in code
   - Add hardware debouncing with capacitors

3. **Check Environment:**
   - Avoid placing PIR sensors near heat sources
   - Shield sensors from direct sunlight
   - Check for air currents causing false triggers

## Serial Monitor Output

Monitor serial output at 115200 baud to debug issues:

```
[GSM] Initializing GSM module...
[GSM] SIM card ready
[GSM] Module initialized
[WiFi] Connecting to: YourWiFiSSID
[WiFi] Connected!
[WiFi] IP: 192.168.1.100
[API] Sensors registered. Status: 200
[SYSTEM] Setup complete. System is DISARMED.
[API] Heartbeat sent. Status: 200
[ALERT] Intrusion detected: Motion detected in Living Room
[GSM] Sending SMS to: +1234567890
[GSM] SMS sent successfully
```

## Power Consumption

**Typical Current Draw:**
- ESP32 (WiFi off): ~80mA
- ESP32 (WiFi on): ~150-250mA
- SIM800L (idle): ~20-30mA
- SIM800L (transmitting): ~2000mA (peak)
- Sensors: ~10-20mA each
- **Total (idle):** ~150-300mA
- **Total (peak during transmission):** ~2500mA

**Battery Backup Calculation:**
- 7Ah battery = 7000mAh
- Idle time: 7000mAh / 250mA ≈ 28 hours
- Consider lower capacity for cost savings or larger for longer backup

## Future Enhancements

- Battery level monitoring
- Deep sleep mode for battery saving
- Multiple alarm zones
- Keypad for local arming/disarming
- LCD display for local status
- Camera integration (ESP32-CAM)
- ESP-NOW wireless sensors
- MQTT integration for home automation

## License

This firmware is provided as-is for educational and personal use.

