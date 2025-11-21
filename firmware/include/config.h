#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration (for HTTP notifications)
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Backend API Configuration
#define API_BASE_URL "http://your-backend-url:8000/api/v1"
#define DEVICE_ID "ESP32_001"  // Unique device ID - change for each device

// GSM Module Configuration (SIM800L)
#define SIM800L_RX_PIN 16  // Connect to SIM800L TX
#define SIM800L_TX_PIN 17  // Connect to SIM800L RX
#define SIM800L_PWR_PIN 4  // Power control pin
#define SIM800L_RST_PIN 5  // Reset pin

// Sensor Pin Definitions
#define PIR_SENSOR_1_PIN 27
#define PIR_SENSOR_2_PIN 26
#define MAGNETIC_SENSOR_1_PIN 25  // Door sensor
#define MAGNETIC_SENSOR_2_PIN 33  // Window sensor
#define SHOCK_SENSOR_PIN 32
#define TAMPER_SWITCH_PIN 14

// Actuator Pin Definitions
#define SIREN_RELAY_PIN 12
#define BUZZER_PIN 13
#define LED_STATUS_PIN 2  // Built-in LED

// Configuration
#define HEARTBEAT_INTERVAL 30000  // 30 seconds
#define ALARM_DELAY_MS 30000      // 30 seconds default delay before alarm
#define SMS_CHECK_INTERVAL 5000   // 5 seconds
#define DEBOUNCE_DELAY 50         // 50ms debounce for sensors

// Phone numbers for SMS alerts (add your numbers)
#define PHONE_NUMBER_1 "+1234567890"
#define PHONE_NUMBER_2 "+0987654321"

// System Status
enum SystemStatus {
  STATUS_DISARMED = 0,
  STATUS_ARMING = 1,
  STATUS_ARMED = 2,
  STATUS_ALARM = 3,
  STATUS_SILENT = 4
};

#endif

