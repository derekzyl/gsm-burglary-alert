/**
 * ESP32-CAM Main Firmware - Burglary Alert System
 * 
 * Functionality:
 * - Listen for trigger signal from ESP32 main controller
 * - Capture high-resolution image (UXGA 1600x1200)
 * - Upload directly to backend via WiFi
 * - Queue to SPIFFS when offline
 * - Auto-upload queued images on reconnection
 */

#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "camera_handler.h"
#include "ntp_sync.h"
#include "spiffs_manager.h"
#include "http_upload.h"

// Global objects
CameraHandler camera;
NTPSync ntpSync;
SPIFFSManager spiffsManager;
HTTPUploader uploader(BACKEND_URL, API_KEY);

// State variables
volatile bool triggerReceived = false;
unsigned long lastTriggerTime = 0;
unsigned long lastCaptureTime = 0; // Added for cooldown
unsigned long lastQueueCheckTime = 0;
unsigned long lastHeartbeatTime = 0;
const unsigned long TRIGGER_COOLDOWN = 5000;  // 5 seconds between captures
const unsigned long TRIGGER_COOLDOWN = 5000;  // 5 seconds between captures
const unsigned long QUEUE_CHECK_INTERVAL = 30000;  // Check queue every 30 seconds

#include <esp_now.h>

// Data structure for ESP-NOW
typedef struct struct_message {
  char a[32];
  int command; // 1 = Trigger
} struct_message;

struct_message myData;

// Interrupt handler for trigger signal
void IRAM_ATTR onTriggerReceived() {
    unsigned long now = millis();
    
    // Debounce
    if (now - lastTriggerTime > TRIGGER_DEBOUNCE_MS) {
        triggerReceived = true;
        lastTriggerTime = now;
    }
}

// Callback when data is received via ESP-NOW
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  if (myData.command == 1) {
    triggerReceived = true; // Use same flag as physical trigger
  }
}


void blinkLED(int pin, int times, int delayMs) {
    for (int i = 0; i < times; i++) {
        digitalWrite(pin, HIGH);
        delay(delayMs);
        digitalWrite(pin, LOW);
        
        if (i < times - 1) {
            delay(delayMs);
        }
    }
}

void uploadQueuedImages() {
    if (!uploader.isConnected()) {
        return;
    }
    
    int count = 0;
    QueuedImage* images = spiffsManager.getQueuedImages(count);
    
    if (count == 0) {
        Serial.println("No queued images to upload");
        return;
    }
    
    Serial.printf("Found %d queued images, attempting upload...\n", count);
    
    for (int i = 0; i < count; i++) {
        Serial.printf("Uploading queued image: %s (%d bytes)\n", 
                     images[i].filename.c_str(), images[i].size);
        
        uint8_t* buffer = nullptr;
        size_t size = 0;
        
        if (spiffsManager.readImage(images[i].filename, &buffer, &size)) {
            if (uploader.uploadImageFromBuffer(buffer, size, images[i].timestamp)) {
                Serial.println("✓ Queued image uploaded successfully");
                spiffsManager.deleteImage(images[i].filename);
                
                // Blink to confirm
                blinkLED(STATUS_LED_PIN, 2, 100);
            } else {
                Serial.println("✗ Failed to upload queued image");
            }
            
            free(buffer);
        }
        
        delay(1000);  // Small delay between uploads
    }
    
    delete[] images;
}

void captureAndUpload() {
    Serial.println("\n========== CAPTURE TRIGGERED ==========");
    
    // Blink LED rapidly during capture
    blinkLED(STATUS_LED_PIN, 3, 50);
    
    // Get current timestamp
    unsigned long timestamp = ntpSync.getCurrentTimestamp();
    if (timestamp == 0) {
        timestamp = millis() / 1000;  // Fallback to uptime
        Serial.println("Warning: Using millis() as timestamp (NTP not synced)");
    }
    
    Serial.printf("Timestamp: %lu\n", timestamp);
    
    // Capture image
    camera_fb_t* fb = camera.captureImage();
    
    if (!fb) {
        Serial.println("✗ Image capture failed!");
        
        // Error blink pattern
        blinkLED(STATUS_LED_PIN, 5, 50);
        return;
    }
    
    Serial.printf("Image captured: %d bytes\n", fb->len);
    
    // Attempt upload to backend
    bool uploaded = false;
    
    if (uploader.isConnected()) {
        Serial.println("WiFi connected - uploading to backend...");
        
        uploaded = uploader.uploadImage(fb, timestamp);
        
        if (uploaded) {
            Serial.println("✓ Image uploaded to backend successfully!");
            
            // Success blink pattern
            blinkLED(STATUS_LED_PIN, 2, 200);
        } else {
            Serial.println("✗ Backend upload failed");
        }
    } else {
        Serial.println("WiFi not connected - saving to SPIFFS");
    }
    
    // Save to SPIFFS if upload failed or offline
    if (!uploaded) {
        if (spiffsManager.saveImage(fb, timestamp)) {
            Serial.println("✓ Image queued in SPIFFS for later upload");
            
            // Offline mode blink pattern (3 rapid blinks)
            blinkLED(STATUS_LED_PIN, 3, 50);
        } else {
            Serial.println("✗ Failed to save image to SPIFFS!");
            
            // Error pattern
            blinkLED(STATUS_LED_PIN, 5, 50);
        }
    }
    
    // Release frame buffer
    camera.releaseFrameBuffer(fb);
    
    Serial.println("======================================\n");
}

void setup() {
    Serial.begin(115200);
    
    // Add delay to allow Serial Monitor to catch up
    delay(3000);
    
    Serial.println("\n\n\n"); // Clear some garbage
    Serial.println("==================================");
    Serial.println("BOOTING ESP32-CAM...");
    Serial.println("ESP32-CAM Burglary Alert System");
    Serial.println("==================================");
    
    // Initialize status LED
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, LOW);

    // Initialize Flash LED
    pinMode(FLASH_LED_PIN, OUTPUT);
    digitalWrite(FLASH_LED_PIN, LOW);

    // Flash LED Test on Boot (User Request)
    Serial.println("Testing Flash LED...");
    blinkLED(FLASH_LED_PIN, 5, 50); // 5 rapid flashes
    delay(500);
    
    // Initialize trigger input
    pinMode(TRIGGER_PIN, INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(TRIGGER_PIN), onTriggerReceived, RISING);
    Serial.println("Trigger input configured on GPIO 13");
    
    // Initialize SPIFFS first (for offline storage)
    if (spiffsManager.begin()) {
        Serial.println("✓ SPIFFS ready");
    } else {
        Serial.println("✗ SPIFFS initialization failed - offline mode unavailable");
    }
    
    // Initialize camera
    if (camera.begin()) {
        Serial.println("✓ Camera ready");
        
        // Test capture
        Serial.println("Taking test photo...");
        camera_fb_t* testFb = camera.captureImage();
        if (testFb) {
            Serial.printf("✓ Test photo successful: %d bytes\n", testFb->len);
            camera.releaseFrameBuffer(testFb);
            
            // Success beep
            blinkLED(STATUS_LED_PIN, 2, 100);
        } else {
            Serial.println("✗ Test photo failed");
        }
    } else {
        Serial.println("✗ Camera initialization failed!");
        
        // Error pattern - loop forever
        while (true) {
            blinkLED(STATUS_LED_PIN, 10, 50);
            delay(1000);
        }
    }
    
    // Connect to WiFi
    Serial.println("\n--- WiFi Setup ---");
    
    int retryCount = 0;
    while (!uploader.connectWiFi() && retryCount < 3) {
        Serial.println("Retrying WiFi connection...");
        retryCount++;
        delay(2000);
    }

    if (uploader.isConnected()) {
        digitalWrite(STATUS_LED_PIN, HIGH);  // LED on = connected
        
        // Initialize NTP
        Serial.println("\n--- NTP Time Sync ---");
        if (ntpSync.begin()) {
            Serial.println("✓ NTP synchronized");
        } else {
            Serial.println("✗ NTP sync failed - timestamps may be inaccurate");
        }
        
        // Check for queued images
        Serial.println("\n--- Checking Image Queue ---");
        uploadQueuedImages();
        
    } else {
        Serial.println("WiFi connection failed after retries - offline mode");
        digitalWrite(STATUS_LED_PIN, LOW);
    }

    // Init ESP-NOW (Works best if WiFi is active, but we try anyway)
    if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
    } else {
      Serial.println("ESP-NOW Initialized");
      esp_now_register_recv_cb(OnDataRecv);
    }

    
    Serial.println("\n--- System Ready ---");
    Serial.println("Waiting for trigger signal...\n");
    
    // Final ready signal
    blinkLED(STATUS_LED_PIN, 3, 100);
}

void loop() {
    // Update NTP time
    ntpSync.update();
    
    // Check WiFi status (slow blink when online)
    static unsigned long lastBlinkTime = 0;
    if (uploader.isConnected() && millis() - lastBlinkTime > LED_BLINK_SLOW) {
        lastBlinkTime = millis();
        digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
    }
    
    // Handle trigger
    if (triggerReceived) {
        triggerReceived = false;  // Reset flag
        
        unsigned long now = millis();
        
        // Check cooldown
        if (now - lastCaptureTime >= TRIGGER_COOLDOWN) {
            captureAndUpload();
            lastCaptureTime = now;
        } else {
            Serial.println("Trigger ignored - cooldown active");
        }
    }
    
    // Periodic queue upload check
    unsigned long now = millis();
    if (now - lastQueueCheckTime > QUEUE_CHECK_INTERVAL) {
        lastQueueCheckTime = now;
        
        if (uploader.isConnected()) {
            int queuedCount = spiffsManager.getQueuedImageCount();
            
            if (queuedCount > 0) {
                Serial.printf("Periodic check: %d images in queue\n", queuedCount);
                uploadQueuedImages();
            }
        }
    }
    
    // Periodic Heartbeat
    if (now - lastHeartbeatTime > HEARTBEAT_INTERVAL_MS) {
        lastHeartbeatTime = now;
        if (uploader.isConnected()) {
            Serial.println("Sending heartbeat...");
            if (uploader.sendHeartbeat("ESP32_CAM", "online", WiFi.localIP().toString().c_str(), "v2.0")) {
                Serial.println("✓ Heartbeat sent");
            } else {
                Serial.println("✗ Heartbeat failed");
            }
        }
    }

    // Reconnect WiFi if disconnected
    if (!uploader.isConnected()) {
        static unsigned long lastReconnectAttempt = 0;
        
        if (now - lastReconnectAttempt > 60000) {  // Try every minute
            lastReconnectAttempt = now;
            Serial.println("Attempting WiFi reconnect...");
            
            if (uploader.connectWiFi()) {
                Serial.println("✓ Reconnected to WiFi");
                digitalWrite(STATUS_LED_PIN, HIGH);
                
                // Sync time
                ntpSync.syncTime();
                
                // Upload queue
                uploadQueuedImages();
            }
        }
    }
    
    delay(50);  // Small delay to prevent tight loop
}
