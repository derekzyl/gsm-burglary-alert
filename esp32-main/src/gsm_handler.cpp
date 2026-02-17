/**
 * GSM Handler Implementation
 * SIM800L communication and SMS sending
 */

#include "gsm_handler.h"
#include "config.h"

GSMHandler::GSMHandler(HardwareSerial* serial) 
    : gsmSerial(serial), lastSMSTime(0), initialized(false) {
}

bool GSMHandler::begin() {
    gsmSerial->begin(9600, SERIAL_8N1, GSM_RX_PIN, GSM_TX_PIN);
    delay(3000);  // Give module time to start

    // Drain any garbage on the line (helps on 38-pin / noisy boards)
    while (gsmSerial->available()) gsmSerial->read();
    delay(200);

    Serial.println("Initializing GSM module...");

    // Test communication (retry once in case of cold start)
    if (!sendATCommand("AT", "OK", 2000)) {
        delay(500);
        while (gsmSerial->available()) gsmSerial->read();
        if (!sendATCommand("AT", "OK", 3000)) {
            Serial.println("GSM: No response to AT - check wiring (TX/RX crossed?) and power (2A supply)");
            return false;
        }
    }
    
    // Disable echo
    sendATCommand("ATE0");
    
    // Set SMS mode to text
    if (!sendATCommand("AT+CMGF=1", "OK")) {
        Serial.println("GSM: Failed to set SMS text mode");
        return false;
    }
    
    // Wait for network registration (up to 30 s)
    Serial.println("Waiting for network registration...");
    for (int i = 0; i < 30; i++) {
        if (isNetworkRegistered()) {
            initialized = true;
            sendATCommand("AT+CNETLIGHT=1");
            Serial.println("GSM initialized successfully! (Netlight LED enabled)");
            return true;
        }
        delay(1000);
    }
    
    // No network yet - still mark ready so we attempt SMS on alert (may work if signal appears later)
    initialized = true;
    Serial.println("GSM: No network in 30s - module ready, SMS will be attempted on alert");
    return true;
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

bool GSMHandler::sendSMS(const char* phoneNumber, const char* message, bool force) {
    if (!initialized && !force) {
        Serial.println("GSM not initialized");
        return false;
    }
    
    if (!canSendSMS() && !force) {
        Serial.println("SMS rate limit active (use force to bypass)");
        return false;
    }
    
    Serial.printf("Sending SMS to %s\n", phoneNumber);
    
    char cmd[64];
    sprintf(cmd, "AT+CMGS=\"%s\"", phoneNumber);
    bool gotPrompt = false;
    const int maxTries = 2;

    for (int tryNum = 0; tryNum < maxTries && !gotPrompt; tryNum++) {
        if (tryNum > 0) {
            Serial.println("SMS: Retrying...");
            delay(500);
        }
        // Drain RX so we don't see stale data
        while (gsmSerial->available()) gsmSerial->read();
        delay(50);

        gsmSerial->println(cmd);
        delay(800);  // Give module time to respond

        unsigned long start = millis();
        String line;
        while (millis() - start < 8000) {
            if (gsmSerial->available()) {
                char c = gsmSerial->read();
                if (c == '>') {
                    gotPrompt = true;
                    break;
                }
                line += c;
                if (line.length() > 80) line = line.substring(line.length() - 80);
                if (line.indexOf("ERROR") >= 0 || line.indexOf("CMS ERROR") >= 0) {
                    Serial.println("SMS: Module error (check number/signal)");
                    return false;
                }
            }
            delay(10);
        }
    }

    if (!gotPrompt) {
        Serial.println("SMS: No prompt received (try power/signal/wiring)");
        return false;
    }

    // Send message body then Ctrl+Z
    gsmSerial->print(message);
    delay(150);
    gsmSerial->write(26);

    if (waitForResponse("+CMGS:", 15000)) {
        lastSMSTime = millis();
        Serial.println("SMS sent successfully!");
        return true;
    }
    Serial.println("SMS send failed");
    return false;
}
