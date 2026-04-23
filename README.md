# GSM-Based Burglary Alarm System

A comprehensive IoT burglary alarm system using ESP32, GSM module (SIM800L), multiple sensors, and wireless alert capabilities via SMS, calls, HTTP, and push notifications.

## Project Structure

```
gsm-burglary-alarm/
├── backend/          # FastAPI backend server
├── flutter-app/      # Flutter mobile application
├── esp32-main/       # Main Controller firmware (PlatformIO)
├── esp32-cam/        # ESP32-CAM firmware (PlatformIO)
└── BOM.md           # Bill of Materials
```

## Features

### Core Functionality
- **Intrusion Detection**: PIR motion sensors, magnetic door/window sensors
- **Tamper Detection**: Shock/vibration sensors, enclosure tamper switch
- **Multi-Channel Alerts**:
  - **Image Capture**: High-resolution snapshots uploaded on detection
  - GSM SMS notifications (Top Priority)
  - GSM voice calls (optional)
  - HTTP API notifications
  - Push notifications via Firebase
- **Dual Trigger System**: Wireless ESP-NOW trigger with physical wire fallback
- **Remote Control**: Arm/disarm via mobile app or SMS
- **Battery Backup**: Automatic failover to battery power
- **Status Monitoring**: Real-time device status, battery level, GSM signal strength

### Backend Features
- RESTful API for device management
- User authentication and authorization
- Alert management and history
- Push notification service (Firebase)
- Real-time device status tracking
- Device configuration management

### Mobile App Features
- User authentication (login/register)
- Device management (add, configure, arm/disarm)
- Real-time alert monitoring
- Alert acknowledgment and resolution
- Push notifications
- Device status dashboard

### Firmware Features
- **Intrusion Logic**: 3-sensor PIR array (Left, Middle, Right) for high-confidence detection
- **Image Capture**: Automatic ESP32-CAM triggering via ESP-NOW
- **Offline Storage**: Image queuing to SPIFFS when WiFi is unavailable
- **GSM module communication** (SIM800L)
- **WiFi connectivity** for HTTP notifications
- **Configurable alarm delay**
- **Silent mode** (notifications only, no siren)
- **Heartbeat** to backend server
- **Automatic sensor registration**

## Hardware Requirements

See [BOM.md](BOM.md) for complete bill of materials.

### Essential Components
- ESP32-WROOM32 development board (Main Unit)
- ESP32-CAM AI-Thinker module
- SIM800L GSM module
- HC-SR501 PIR sensors (3 units: Left, Middle, Right)
- Magnetic reed switches (2-6 units)
- 12V siren with relay
- 12V power supply (3A+) with battery backup
- Tamper switch for enclosure

## Software Setup

### Backend Setup

1. **Install Dependencies**
```bash
cd backend
pip install -e .
```

2. **Configure Environment**
```bash
cp .env.example .env
# Edit .env with your configuration
```

3. **Database Setup**
```bash
# Create PostgreSQL database
createdb burglary_alarm_db

# Run migrations
alembic upgrade head
```

4. **Run Server**
```bash
uvicorn app.main:app --reload
```

Backend API will be available at `http://localhost:8000`
API documentation: `http://localhost:8000/docs`

### Flutter App Setup

1. **Install Dependencies**
```bash
cd flutter-app
flutter pub get
```

2. **Configure API URL**
Edit `lib/core/config.dart` and set your backend API URL:
```dart
static const String apiBaseUrl = 'http://your-backend-url:8000/api/v1';
```

3. **Run App**
```bash
flutter run
```

### Firmware Setup

The system consists of two firmware projects that communicate via **ESP-NOW**.

1. **Install PlatformIO**
```bash
pip install platformio
```

2. **Setup ESP32 Main Controller**
   - Edit `esp32-main/include/config.h`
   - Set WiFi credentials and Backend URL
   - **CRITICAL**: Update `ESP32_CAM_MAC` with the MAC address of your ESP32-CAM board.
   - Run: `cd esp32-main && pio run -t upload`

3. **Setup ESP32-CAM**
   - Edit `esp32-cam/include/config.h`
   - Set WiFi credentials and Backend URL
   - Run: `cd esp32-cam && pio run -t upload`

4. **Wiring Fallback**
   - Connect GPIO 27 (Main) to GPIO 13 (CAM) for the physical trigger fallback.
   - Ensure a common GND between both boards.

## Configuration

### ESP32 Configuration

Edit `firmware/include/config.h`:

```cpp
#define WIFI_SSID "YourWiFiSSID"
#define WIFI_PASSWORD "YourWiFiPassword"
#define API_BASE_URL "http://your-backend-url:8000/api/v1"
#define DEVICE_ID "ESP32_001"  // Unique per device
#define PHONE_NUMBER_1 "+1234567890"  // Your phone number
```

### Pin Assignments

Default pin configuration (adjust in `config.h` as needed):
- **SIM800L**: RX=16, TX=17, PWR=4, RST=5
- **PIR Sensors**: Left=22, Middle=23, Right=21
- **ESP32-CAM Trigger**: Pin 27 (Physical Fallback)
- **Siren Relay**: Pin 12
- **Buzzer**: Pin 25
- **Status LED**: Pin 2
- **SIM Status LED**: Pin 4

### Backend Configuration

Key environment variables in `.env`:
- `DATABASE_URL`: PostgreSQL connection string
- `SECRET_KEY`: JWT secret key
- `FIREBASE_CREDENTIALS_PATH`: Path to Firebase credentials JSON
- `FIREBASE_PROJECT_ID`: Firebase project ID
- `CORS_ORIGINS`: Allowed CORS origins

### Mobile App Configuration

Edit `lib/core/config.dart`:
- `apiBaseUrl`: Backend API endpoint
- Connection timeouts

## Usage

### Initial Setup

1. **Register User**: Use mobile app to create an account
2. **Add Device**: Register your ESP32 device with unique device ID
3. **Configure Sensors**: ESP32 will auto-register sensors on first boot
4. **Configure Alerts**: Set phone numbers and notification preferences
5. **Test System**: Trigger sensors to verify alerts

### Arming/Disarming

**Via Mobile App:**
1. Open app and navigate to Devices
2. Select your device
3. Tap "Arm" or "Disarm" button

**Via SMS:**
- Send "ARM" to SIM card number → Arms system
- Send "DISARM" to SIM card number → Disarms system
- Send "STATUS" to SIM card number → Returns current status

### Alert Handling

1. **Alerts appear** in mobile app immediately
2. **SMS sent** to configured phone numbers
3. **HTTP notification** sent to backend (if WiFi available)
4. **Push notification** sent to mobile app (if configured)

**Acknowledging Alerts:**
- Open alert in mobile app
- Tap "Acknowledge" or "Resolve"

## API Endpoints

### Authentication
- `POST /api/v1/auth/register` - Register new user
- `POST /api/v1/auth/login` - Login user
- `GET /api/v1/auth/me` - Get current user

### Devices
- `GET /api/v1/devices` - List user's devices
- `POST /api/v1/devices` - Create new device
- `GET /api/v1/devices/{id}` - Get device details
- `POST /api/v1/devices/{id}/arm` - Arm device
- `POST /api/v1/devices/{id}/disarm` - Disarm device
- `GET /api/v1/devices/{id}/config` - Get device configuration
- `PUT /api/v1/devices/{id}/config` - Update device configuration

### ESP32 Endpoints (Device Authentication)
- `POST /api/v1/devices/esp32/heartbeat` - Device heartbeat (requires X-Device-ID header)
- `POST /api/v1/devices/esp32/alert` - Report alert (requires X-Device-ID header)
- `POST /api/v1/devices/esp32/sensors` - Register sensors (requires X-Device-ID header)

### Alerts
- `GET /api/v1/alerts` - List alerts
- `GET /api/v1/alerts/{id}` - Get alert details
- `PUT /api/v1/alerts/{id}/acknowledge` - Acknowledge alert
- `PUT /api/v1/alerts/{id}/resolve` - Resolve alert

See backend API documentation at `/docs` endpoint for complete API reference.

## Troubleshooting

### ESP32 Not Connecting to WiFi
- Check SSID and password in `config.h`
- Verify WiFi router is operational
- Check signal strength at device location

### GSM Module Not Working
- Verify SIM card is inserted correctly and activated
- Check antenna connection
- Ensure adequate power supply (SIM800L needs ~2A peak)
- Check AT commands via serial monitor

### No Alerts Received
- Verify device is armed
- Check sensor connections
- Verify phone numbers are configured correctly
- Check backend API connectivity
- Review alert logs in backend

### Power Issues
- Ensure power supply can handle peak current (ESP32 + GSM module)
- Check battery backup circuit
- Verify diode OR circuit for battery failover
- Use proper wire gauge for power connections

### Backend Connection Issues
- Verify backend is running and accessible
- Check CORS settings in backend
- Verify API URL in ESP32 config and Flutter app
- Check firewall settings

## Security Considerations

1. **Change Default Passwords**: Use strong passwords for user accounts
2. **HTTPS in Production**: Use HTTPS for API communication in production
3. **SIM Card Security**: Protect SIM card PIN
4. **Network Security**: Use secure WiFi networks
5. **API Authentication**: Device authentication via X-Device-ID header
6. **Encryption**: Consider encrypting sensitive sensor data

## License

This project is provided as-is for educational and personal use.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Support

For issues and questions:
1. Check troubleshooting section
2. Review API documentation
3. Check serial monitor output from ESP32
4. Review backend logs

---

**Version:** 1.0.0
**Last Updated:** 2026

# gsm-burglary-alert
