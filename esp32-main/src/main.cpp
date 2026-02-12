/**
 * ESP32 Main Controller - Burglary Alert System
 * 
 * Functionality:
 * - Monitor 3 PIR sensors for human detection
 * - Trigger ESP32-CAM for image capture
 * - Send alert metadata to backend via WiFi
 * - Fallback to SMS via SIM800L when WiFi unavailable
 * - Activate buzzer on detection
 */

#include <Arduino.h>
#include <WiFi.h>
#include "include/config.h"
#include "src/pir_detector.h"
#include "src/gsm_handler.h"
#include "src/buzzer.h"
#include "src/http_client.h"

// Global objects
PIRDetector pirDetector(PIR_LEFT_PIN, PIR_MIDDLE_PIN, PIR_RIGHT_PIN);
HardwareSerial gsmSerial(1);  // Use Serial1 for GSM
GSMHandler gsm(&gsmSerial);
Buzzer buzzer(BUZZER_PIN);
BackendClient backend(BACKEND_URL, API_KEY);

// State variables
unsigned long lastDetectionTime = 0;
unsigned long lastHeartbeatTime = 0;
const unsigned long DETECTION_COOLDOWN = 10000;  // 10 seconds between detections

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("==================================");
    Serial.println("ESP32 Burglary Alert System");
    Serial.println("==================================");
    
    // Initialize status LED
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, LOW);
    
    // Initialize ESP32-CAM trigger pin
    pinMode(CAM_TRIGGER_PIN, OUTPUT);
    digitalWrite(CAM_TRIGGER_PIN, LOW);
    
    // Initialize PIR detector
    pirDetector.begin();
    
    // Initialize buzzer
    buzzer.begin();
    
    // Play startup beep
    buzzer.beep(100);
    delay(100);
    buzzer.beep(100);
    
    // Connect to WiFi
    Serial.println("\n--- WiFi Setup ---");
    if (backend.connectWiFi()) {
        digitalWrite(STATUS_LED_PIN, HIGH);  // LED on = WiFi connected
    } else {
        Serial.println("WiFi connection failed, will use SMS fallback");
    }
    
    // Initialize GSM module
    Serial.println("\n--- GSM Setup ---");
    if (gsm.begin()) {
        Serial.println("GSM ready for SMS fallback");
    } else {
        Serial.println("GSM initialization failed, SMS fallback unavailable");
    }
    
    Serial.println("\n--- System Ready ---");
    Serial.println("Monitoring for intrusions...\n");
    
    // Final startup signal
    buzzer.beep(200);
}

void loop() {
    // Update PIR detector
    pirDetector.update();
    
    // Check for detection cooldown
    unsigned long now = millis();
    if (now - lastDetectionTime < DETECTION_COOLDOWN) {
        delay(50);
        return;
    }
    
    // Check for human detection
    HumanDetectionResult detection = pirDetector.detectHuman();
    
    if (detection.detected) {
        Serial.println("\n========== INTRUDER DETECTED ==========");
        Serial.printf("Confidence: %.2f%%\n", detection.confidence * 100);
        Serial.printf("PIR Sensors - Left: %d, Middle: %d, Right: %d\n",
                     detection.pir_left, detection.pir_middle, detection.pir_right);
        
        lastDetectionTime = now;
        
        // Step 1: Trigger ESP32-CAM immediately
        Serial.println("\n[1] Triggering ESP32-CAM...");
        digitalWrite(CAM_TRIGGER_PIN, HIGH);
        delay(TRIGGER_PULSE_MS);
        digitalWrite(CAM_TRIGGER_PIN, LOW);
        Serial.println("Camera trigger sent");
        
        // Step 2: Activate buzzer
        Serial.println("\n[2] Activating alarm buzzer...");
        buzzer.playAlertPattern();
        
        // Step 3: Send alert to backend or SMS
        Serial.println("\n[3] Sending alert...");
        
        bool alertSent = false;
        
        if (backend.isConnected()) {
            Serial.println("WiFi available - posting to backend");
            alertSent = backend.postAlert(detection, "online");
            
            if (alertSent) {
                Serial.println("✓ Alert posted to backend successfully");
                
                // Blink LED to confirm
                for (int i = 0; i < 3; i++) {
                    digitalWrite(STATUS_LED_PIN, LOW);
                    delay(100);
                    digitalWrite(STATUS_LED_PIN, HIGH);
                    delay(100);
                }
            } else {
                Serial.println("✗ Backend post failed, falling back to SMS");
            }
        } else {
            Serial.println("WiFi unavailable - using SMS fallback");
        }
        
        // SMS fallback if WiFi failed or unavailable
        if (!alertSent) {
            if (gsm.canSendSMS()) {
                char message[160];
                sprintf(message, 
                        "INTRUDER ALERT! Motion detected at %lu. "
                        "Confidence: %.0f%%. Image may follow.",
                        now / 1000, 
                        detection.confidence * 100);
                
                if (gsm.sendSMS(EMERGENCY_PHONE, message)) {
                    Serial.println("✓ SMS alert sent successfully");
                    buzzer.beep(100);  // Confirmation beep
                } else {
                    Serial.println("✗ SMS send failed");
                    
                    // Critical failure - play error pattern
                    for (int i = 0; i < 5; i++) {
                        buzzer.beep(50);
                        delay(50);
                    }
                }
            } else {
                Serial.println("SMS rate limited");
            }
        }
        
        // Step 4: Reset detector for next detection
        Serial.println("\n[4] Resetting detector");
        pirDetector.reset();
        
        Serial.println("======================================\n");
    }
    
    // Periodic heartbeat (optional - for debugging)
    if (now - lastHeartbeatTime > HEARTBEAT_INTERVAL_MS) {
        lastHeartbeatTime = now;
        
        Serial.println("--- System Heartbeat ---");
        Serial.printf("WiFi: %s\n", backend.isConnected() ? "Connected" : "Disconnected");
        Serial.printf("Uptime: %lu seconds\n", now / 1000);
        
        if (!backend.isConnected()) {
            Serial.println("Attempting WiFi reconnect...");
            backend.reconnect();
        }
    }
    
    delay(50);  // Small delay to prevent tight loop
}
