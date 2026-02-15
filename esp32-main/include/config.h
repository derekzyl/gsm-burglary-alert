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

// ==================== ESP-NOW CONFIGURATION ====================
// REPLACE WITH YOUR ESP32-CAM MAC ADDRESS
// Example: {0x24, 0x6F, 0x28, 0xAE, 0x12, 0x34}
#define ESP32_CAM_MAC {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} 
#define USE_ESP_NOW true


// ==================== GSM CONFIGURATION ====================
// Emergency Phones
extern const char* EMERGENCY_PHONES[];
const int NUM_PHONES = 2;
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
// Wiring: ESP32 TX -> SIM800L RX,  ESP32 RX <- SIM800L TX,  GND common.
// Option A: GPIO 16/17 (if SMS works; some 38-pin boards have issues here)
// Option B: GPIO 33/32 (recommended for ESP32 38-pin - often more stable)
#define GSM_TX_PIN 33  // ESP32 TX -> SIM800L RX (use 16 for Option A)
#define GSM_RX_PIN 32  // ESP32 RX <- SIM800L TX (use 17 for Option A)
// SIM800L needs 3.4–4.4V @ 2A peak; use external supply, not ESP32 3.3V.

// ESP32-CAM Trigger
#define CAM_TRIGGER_PIN 27  // Trigger signal to ESP32-CAM GPIO 13

// Buzzer
#define BUZZER_PIN 25  // DAC pin for tone generation

// Status LED
#define STATUS_LED_PIN 2  // Built-in LED

// SIM/GSM status LED - blinks when GSM is registered, pattern when SMS sent (use same pin as STATUS_LED_PIN if no extra LED)
#define SIM_STATUS_LED_PIN 4

// UART Backup to ESP32-CAM
// RX Only - receive images/status from ESP32-CAM
#define UART2_RX_PIN 35  // From ESP32-CAM TX backup

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
