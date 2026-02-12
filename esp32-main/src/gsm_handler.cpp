/**
 * GSM Handler Implementation
 * SIM800L communication and SMS sending
 */

#include "gsm_handler.h"
#include "../include/config.h"

GSMHandler::GSMHandler(HardwareSerial* serial) 
    : gsmSerial(serial), lastSMSTime(0), initialized(false) {
}

bool GSMHandler::begin() {
    gsmSerial->begin(9600, SERIAL_8N1, GSM_RX_PIN, GSM_TX_PIN);
    delay(3000);  // Give module time to start
    
    Serial.println("Initializing GSM module...");
    
    // Test communication
    if (!sendATCommand("AT", "OK", 2000)) {
        Serial.println("GSM: No response to AT");
        return false;
    }
    
    // Disable echo
    sendATCommand("ATE0");
    
    // Set SMS mode to text
    if (!sendATCommand("AT+CMGF=1", "OK")) {
        Serial.println("GSM: Failed to set SMS text mode");
        return false;
    }
    
    // Wait for network registration
    Serial.println("Waiting for network registration...");
    for (int i = 0; i < 30; i++) {  // Try for 30 seconds
        if (isNetworkRegistered()) {
            initialized = true;
            Serial.println("GSM initialized successfully!");
            return true;
        }
        delay(1000);
    }
    
    Serial.println("GSM: Network registration failed");
    return false;
}

bool GSMHandler::sendATCommand(const char* command, const char* expectedResponse, unsigned long timeout) {
    gsmSerial->println(command);
    return waitForResponse(expectedResponse, timeout);
}

bool GSMHandler::waitForResponse(const char* expected, unsigned long timeout) {
    unsigned long start = millis();
    String response = "";
    
    while (millis() - start < timeout) {
        if (gsmSerial->available()) {
            char c = gsmSerial->read();
            response += c;
            
            if (response.indexOf(expected) >= 0) {
                return true;
            }
            
            if (response.indexOf("ERROR") >= 0) {
                return false;
            }
        }
        delay(10);
    }
    
    return false;
}

bool GSMHandler::isNetworkRegistered() {
    gsmSerial->println("AT+CREG?");
    
    unsigned long start = millis();
    String response = "";
    
    while (millis() - start < 3000) {
        if (gsmSerial->available()) {
            response += (char)gsmSerial->read();
            
            // Check for registration status
            // +CREG: 0,1 = registered on home network
            // +CREG: 0,5 = registered on roaming network
            if (response.indexOf("+CREG: 0,1") >= 0 || 
                response.indexOf("+CREG: 0,5") >= 0) {
                return true;
            }
        }
    }
    
    return false;
}

int GSMHandler::getSignalStrength() {
    gsmSerial->println("AT+CSQ");
    
    unsigned long start = millis();
    String response = "";
    
    while (millis() - start < 2000) {
        if (gsmSerial->available()) {
            char c = gsmSerial->read();
            response += c;
        }
    }
    
    // Parse response: +CSQ: <rssi>,<ber>
    int idx = response.indexOf("+CSQ:");
    if (idx >= 0) {
        int rssi = response.substring(idx + 6).toInt();
        return rssi;
    }
    
    return 0;
}

bool GSMHandler::canSendSMS() {
    // Rate limit: 1 SMS per 5 minutes
    unsigned long now = millis();
    if (lastSMSTime > 0 && (now - lastSMSTime) < SMS_RATE_LIMIT_MS) {
        Serial.println("SMS rate limit active");
        return false;
    }
    return true;
}

bool GSMHandler::sendSMS(const char* phoneNumber, const char* message) {
    if (!initialized) {
        Serial.println("GSM not initialized");
        return false;
    }
    
    if (!canSendSMS()) {
        return false;
    }
    
    Serial.printf("Sending SMS to %s\n", phoneNumber);
    
    // Set phone number
    char cmd[64];
    sprintf(cmd, "AT+CMGS=\"%s\"", phoneNumber);
    
    gsmSerial->println(cmd);
    delay(500);
    
    // Wait for '>'
    unsigned long start = millis();
    bool gotPrompt = false;
    
    while (millis() - start < 5000) {
        if (gsmSerial->available()) {
            char c = gsmSerial->read();
            if (c == '>') {
                gotPrompt = true;
                break;
            }
        }
    }
    
    if (!gotPrompt) {
        Serial.println("SMS: No prompt received");
        return false;
    }
    
    // Send message
    gsmSerial->print(message);
    delay(100);
    
    // Send Ctrl+Z to end message
    gsmSerial->write(26);
    
    // Wait for confirmation
    if (waitForResponse("+CMGS:", 10000)) {
        lastSMSTime = millis();
        Serial.println("SMS sent successfully!");
        return true;
    }
    
    Serial.println("SMS send failed");
    return false;
}
