/**
 * Configuration for ESP32 Main Controller
 * 
 * CRITICAL: Update these values before uploading!
 */

#ifndef CONFIG_H
#define CONFIG_H

// ==================== NETWORK CONFIGURATION ====================
#define WIFI_SSID "cybergenii"
#define WIFI_PASSWORD "12341234"
#define BACKEND_URL "https://xenophobic-netta-cybergenii-1584fde7.koyeb.app"
#define API_KEY "esp32_device_key_xyz789"

// ==================== GSM CONFIGURATION ====================
#define EMERGENCY_PHONE "+1234567890"  // Phone number for SMS alerts
#define APN "your.apn.here"  // Your mobile operator's APN

// ==================== PIN DEFINITIONS ====================
// PIR Sensors
#define PIR_LEFT_PIN 22
#define PIR_MIDDLE_PIN 23
#define PIR_RIGHT_PIN 21

// Ultrasonic Sensor
#define ULTRASONIC_ECHO_PIN 18
#define ULTRASONIC_TRIG_PIN 19

// SIM800L GSM Module
#define GSM_TX_PIN 16  // ESP32 TX to SIM800L RX
#define GSM_RX_PIN 17  // ESP32 RX from SIM800L TX
#define GSM_PWR_PIN 4  // Power control (if needed)

// ESP32-CAM Trigger
#define CAM_TRIGGER_PIN 4  // Trigger signal to ESP32-CAM GPIO 13

// Buzzer
#define BUZZER_PIN 25  // DAC pin for tone generation

// Status LED
#define STATUS_LED_PIN 2  // Built-in LED

// UART Backup to ESP32-CAM
#define UART2_TX_PIN 32  // To ESP32-CAM RX backup
#define UART2_RX_PIN 33  // From ESP32-CAM TX backup

// ==================== DETECTION PARAMETERS ====================
#define DETECTION_WINDOW_MS 2000  // Time window for PIR triggers (ms)
#define MIN_PIR_TRIGGERS 2  // Minimum PIR sensors for human detection
#define TRIGGER_PULSE_MS 100  // Duration of trigger pulse to ESP32-CAM (ms)
#define DEBOUNCE_DELAY_MS 50  // PIR debounce delay

// ==================== TIMING CONFIGURATION ====================
#define WIFI_CONNECT_TIMEOUT_MS 10000  // WiFi connection timeout
#define SERVER_TIMEOUT_MS 5000  // HTTP request timeout
#define HEARTBEAT_INTERVAL_MS 60000  // Status heartbeat interval
#define SMS_RATE_LIMIT_MS 300000  // 5 minutes between SMS (cost control)

// ==================== BUZZER PATTERN ====================
#define BUZZER_BEEPS 3
#define BUZZER_ON_MS 200
#define BUZZER_OFF_MS 100
#define BUZZER_FREQUENCY 2000  // Hz

#endif // CONFIG_H
