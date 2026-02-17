/**
 * ESP32 Main Controller - Burglary Alert System
 * 
 * Functionality:
 * - Monitor 3 PIR sensors for human detection
 * - Trigger ESP32-CAM for image capture
 * - Send alert metadata to backend via WiFi
 * - Send SMS first (top priority), then backend via WiFi
 * - Activate buzzer on detection
 */


#include <esp_now.h>
#include <WiFi.h>

// Data structure for ESP-NOW
#include "config.h"
#include "ntp_sync.h"
#include "gsm_handler.h"
#include "pir_detector.h"
#include "buzzer.h"
#include "http_client.h"

// Emergency Phones
const char* EMERGENCY_PHONES[] = {
    "+2349164936378", 
    "+2347059011222"
};

PIRDetector pirDetector(PIR_LEFT_PIN, PIR_MIDDLE_PIN, PIR_RIGHT_PIN);
HardwareSerial gsmSerial(1);  // Use Serial1 for GSM
GSMHandler gsm(&gsmSerial);
Buzzer buzzer(BUZZER_PIN);
BackendClient backend(BACKEND_URL, API_KEY);
NTPSync ntpSync;

// State variables
unsigned long lastDetectionTime = 0;
unsigned long lastHeartbeatTime = 0;
unsigned long lastSimLedTime = 0;
bool simLedOn = false;
const unsigned long DETECTION_COOLDOWN = 10000;  // 10 seconds between detections
const unsigned long SIM_LED_HEARTBEAT_MS = 2000;  // SIM status LED blink interval when GSM ready

// Data structure for ESP-NOW

typedef struct struct_message {
  char a[32];
  int command; // 1 = Trigger
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;
uint8_t broadcastAddress[] = ESP32_CAM_MAC;

// Callback when data is sent (debug purposes)
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("==================================");
    Serial.println("ESP32 Burglary Alert System");
    Serial.println("==================================");
    
    // Initialize status LED
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, LOW);
    // SIM status LED - shows GSM registered (heartbeat) and SMS sent (blink pattern)
    pinMode(SIM_STATUS_LED_PIN, OUTPUT);
    digitalWrite(SIM_STATUS_LED_PIN, LOW);
    
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
    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);
    
    if (backend.connectWiFi()) {
        digitalWrite(STATUS_LED_PIN, HIGH);  // LED on = WiFi connected
    } else {
        Serial.println("WiFi connection failed, will use SMS fallback");
        // Ensure channel 1 if not connected, though connectWiFi might leave it wherever
    }

    // Initialize NTP time sync
    Serial.println("\n--- NTP Time Synchronization ---");
    if (ntpSync.begin()) {
        Serial.println("✓ NTP time synchronized successfully");
    } else {
        Serial.println("✗ NTP time sync failed - using system time");
    }

    // Init ESP-NOW
#ifdef USE_ESP_NOW
    Serial.println("\n--- ESP-NOW Setup ---");
    if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
    } else {
      Serial.println("ESP-NOW Initialized");
      
      // Once ESPNow is successfully Init, we will register for Send CB to
      // get the status of Trasnmitted packet
      esp_now_register_send_cb(OnDataSent);
      
      // Register peer
      memcpy(peerInfo.peer_addr, broadcastAddress, 6);
      peerInfo.channel = 0;  
      peerInfo.encrypt = false;
      
      if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
      } else {
        Serial.println("ESP-NOW Peer Added");
      }
    }
#endif
    
    // Initialize GSM module
    Serial.println("\n--- GSM Setup ---");
    if (gsm.begin()) {
        Serial.println("GSM ready for SMS fallback");
        // Brief blink on SIM LED to show it's working
        for (int i = 0; i < 3; i++) {
            digitalWrite(SIM_STATUS_LED_PIN, HIGH);
            delay(100);
            digitalWrite(SIM_STATUS_LED_PIN, LOW);
            delay(100);
        }
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
        
        // Step 1: Trigger ESP32-CAM
        Serial.println("\n[1] Triggering ESP32-CAM...");
        
        bool espNowSuccess = false;
        
#ifdef USE_ESP_NOW
        // Send ESP-NOW message
        myData.command = 1;
        strcpy(myData.a, "TRIGGER");
        
        esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
        
        if (result == ESP_OK) {
            Serial.println("Sent with success");
            espNowSuccess = true;
        } else {
            Serial.println("Error sending the data");
        }
#endif
        
        if (!espNowSuccess) {
            Serial.println("Using Physical Wire Fallback...");
            digitalWrite(CAM_TRIGGER_PIN, HIGH);
            delay(TRIGGER_PULSE_MS);
            digitalWrite(CAM_TRIGGER_PIN, LOW);
            Serial.println("Physical trigger pulse sent");
        }
        
        // Step 2: Activate buzzer
        Serial.println("\n[2] Activating alarm buzzer...");
        buzzer.playAlertPattern();
        
        // Step 3: Send alert - SMS is TOP PRIORITY, backend secondary
        Serial.println("\n[3] Sending alert...");
        
        // --- SMS FIRST (top priority - direct, reliable, works without WiFi) ---
        if (gsm.canSendSMS()) {
                // Get proper timestamp
                unsigned long timestamp = ntpSync.isSynchronized() 
                    ? ntpSync.getCurrentTimestamp() 
                    : now / 1000;
                
                char message[160];
                
                // Format time string
                struct tm timeinfo;
                time_t ts = timestamp;
                localtime_r(&ts, &timeinfo);
                char timeStr[20];
                strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);

                sprintf(message, 
                        "INTRUDER ALERT! Motion detected at %s. "
                        "Confidence: %.0f%%.",
                        timeStr,
                        detection.confidence * 100);
                
                // Send to all configured numbers (force=true so we try even if GSM init failed e.g. network reg)
                bool anySuccess = false;
                for (int i = 0; i < NUM_PHONES; i++) {
                    if (gsm.sendSMS(EMERGENCY_PHONES[i], message, true)) {
                        Serial.printf("✓ SMS sent to %s\n", EMERGENCY_PHONES[i]);
                        anySuccess = true;
                    } else {
                        Serial.printf("✗ SMS failed to %s\n", EMERGENCY_PHONES[i]);
                    }
                    delay(3000); // Increased delay between multiple SMS to be safe
                }

                if (anySuccess) {
                    Serial.println("✓ SMS alert process completed");
                    // SIM LED: 5 quick blinks = SMS sent successfully (visible feedback)
                    for (int b = 0; b < 5; b++) {
                        digitalWrite(SIM_STATUS_LED_PIN, HIGH);
                        delay(80);
                        digitalWrite(SIM_STATUS_LED_PIN, LOW);
                        delay(80);
                    }
                }
        } else {
            Serial.println("SMS rate limited - skipping SMS");
        }
        
        // --- Backend SECOND (secondary - dashboard, images, logging) ---
        if (backend.isConnected()) {
            Serial.println("Posting to backend...");
            if (backend.postAlert(detection, "online")) {
                Serial.println("✓ Alert posted to backend successfully");
                for (int i = 0; i < 3; i++) {
                    digitalWrite(STATUS_LED_PIN, LOW);
                    delay(100);
                    digitalWrite(STATUS_LED_PIN, HIGH);
                    delay(100);
                }
            } else {
                Serial.println("✗ Backend post failed");
            }
        } else {
            Serial.println("WiFi unavailable - backend skip");
        }
        
        // Step 4: Reset detector for next detection
        Serial.println("\n[4] Resetting detector");
        pirDetector.reset();
        
        Serial.println("======================================\n");
    }
    
    // SIM status LED heartbeat - blink when GSM is ready (shows "SIM is working")
    if (gsm.isReady() && (now - lastSimLedTime >= SIM_LED_HEARTBEAT_MS)) {
        lastSimLedTime = now;
        simLedOn = !simLedOn;
        digitalWrite(SIM_STATUS_LED_PIN, simLedOn ? HIGH : LOW);
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
        } else {
            // Send heartbeat
            backend.sendHeartbeat("ESP32_MAIN", "online", WiFi.localIP().toString().c_str(), "v2.0");
        }
    }
    
    // Check for debug commands from Serial Monitor
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        
        if (command == "TEST_SMS") {
            Serial.println("\n[DEBUG] FORCE SENDING SMS...");
            bool sent = false;
            for (int i = 0; i < NUM_PHONES; i++) {
                if (gsm.sendSMS(EMERGENCY_PHONES[i], "TEST SMS: System is active and GSM is working.", true)) {
                    Serial.printf("✓ Test SMS sent to %s\n", EMERGENCY_PHONES[i]);
                    sent = true;
                } else {
                    Serial.printf("✗ Failed to send to %s\n", EMERGENCY_PHONES[i]);
                }
                delay(1000);
            }
            if (sent) {
                buzzer.beep(200);
                for (int b = 0; b < 5; b++) {
                    digitalWrite(SIM_STATUS_LED_PIN, HIGH);
                    delay(80);
                    digitalWrite(SIM_STATUS_LED_PIN, LOW);
                    delay(80);
                }
            }
        } else if (command == "GET_SIGNAL") {
            int csq = gsm.getSignalStrength();
            Serial.printf("GSM Signal Strength: %d\n", csq);
        }
    }
    
    delay(50);  // Small delay to prevent tight loop
}
