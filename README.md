# GSM-Based Burglary Alarm System

A comprehensive IoT burglary alarm system using ESP32, GSM module (SIM800L), multiple sensors, and wireless alert capabilities via SMS, calls, HTTP, and push notifications.

## Project Structure

```
gsm-burglary-alarm/
├── backend/          # FastAPI backend server
├── burglary_app/     # Flutter mobile application
├── firmware/         # ESP32 firmware (PlatformIO)
└── BOM.md           # Bill of Materials
```

## Features

### Core Functionality
- **Intrusion Detection**: PIR motion sensors, magnetic door/window sensors
- **Tamper Detection**: Shock/vibration sensors, enclosure tamper switch
- **Multi-Channel Alerts**:
  - GSM SMS notifications
  - GSM voice calls (optional)
  - HTTP API notifications
  - Push notifications via Firebase
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
- Multi-sensor monitoring (PIR, magnetic, shock, tamper)
- GSM module communication (SIM800L)
- WiFi connectivity for HTTP notifications
- Configurable alarm delay
- Silent mode (notifications only, no siren)
- Heartbeat to backend server
- Automatic sensor registration

## Hardware Requirements

See [BOM.md](BOM.md) for complete bill of materials.

### Essential Components
- ESP32-WROOM32 development board
- SIM800L GSM module
- HC-SR501 PIR sensors (2-4 units)
- Magnetic reed switches (2-6 units)
- Shock/vibration sensor (1-2 units)
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
cd burglary_app
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

1. **Install PlatformIO**
```bash
pip install platformio
```

2. **Configure Settings**
Edit `include/config.h`:
- WiFi SSID and password
- Backend API URL
- Device ID (unique for each device)
- Sensor pin assignments
- Phone numbers for SMS alerts

3. **Build and Upload**
```bash
cd firmware
pio run -t upload
pio device monitor  # Monitor serial output
```

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
- **PIR Sensors**: Pins 27, 26
- **Magnetic Sensors**: Pins 25, 33 (with pull-up)
- **Shock Sensor**: Pin 32
- **Tamper Switch**: Pin 14
- **Siren Relay**: Pin 12
- **Buzzer**: Pin 13
- **LED Status**: Pin 2 (built-in LED)

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
**Last Updated:** 2024

# gsm-burglary-alert
