/**
 * Configuration for ESP32-CAM
 * 
 * CRITICAL: Update these values before uploading!
 */

#ifndef CONFIG_H
#define CONFIG_H

// ==================== NETWORK CONFIGURATION ====================
#define WIFI_SSID "DAVIES"
#define WIFI_PASSWORD "DAVIES222"
#define BACKEND_URL "https://xenophobic-netta-cybergenii-1584fde7.koyeb.app/api/v1/burglary/image/image"
#define API_KEY "esp32_device_key_xyz789"

// ==================== NTP CONFIGURATION ====================
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 0  // UTC offset in seconds (e.g., 3600 for UTC+1)
#define DAYLIGHT_OFFSET_SEC 3600  // Daylight saving offset

// ==================== PIN DEFINITIONS ====================
// Trigger Input from ESP32 Main
#define TRIGGER_PIN 13  // From ESP32 main GPIO 4

// UART Output to ESP32 Main
#define UART_TX_PIN 14   // U0TXD (Standard Serial TX) -> Connect to ESP32 Main GPIO 33

// Status LED
#define STATUS_LED_PIN 33  // Built-in LED (usually GPIO 33 on AI Thinker)

// Flash LED (optional, can be used for night vision)
#define FLASH_LED_PIN 4

// ==================== CAMERA CONFIGURATION ====================
// Camera model: AI Thinker ESP32-CAM
// Pin definitions for AI Thinker board (do not change unless using different board)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Camera settings
#define IMAGE_SIZE FRAMESIZE_UXGA  // 1600x1200 pixels
#define JPEG_QUALITY 12  // 0-63, lower is higher quality (12 = ~150KB)
#define FB_COUNT 1  // Number of frame buffers

// ==================== SPIFFS CONFIGURATION ====================
#define SPIFFS_MAX_IMAGES 20  // Maximum queued images before deletion
#define IMAGE_PREFIX "/capture_"
#define IMAGE_EXTENSION ".jpg"

// ==================== TIMING CONFIGURATION ====================
#define WIFI_CONNECT_TIMEOUT_MS 10000  // WiFi connection timeout
#define SERVER_TIMEOUT_MS 10000  // HTTP request timeout (images are large)
#define NTP_SYNC_INTERVAL_MS 3600000  // Re-sync NTP every hour
#define TRIGGER_DEBOUNCE_MS 100  // Debounce trigger input
#define MIN_SIGNAL_STRENGTH -70  // Minimum WiFi RSSI for upload attempt
#define HEARTBEAT_INTERVAL_MS 60000  // 1 minute heartbeat

// ==================== STATUS LED PATTERNS ====================
#define LED_BLINK_FAST 100  // Fast blink for activity
#define LED_BLINK_SLOW 500  // Slow blink for standby
#define LED_BLINK_ERROR 50  // Very fast blink for errors

#endif // CONFIG_H
