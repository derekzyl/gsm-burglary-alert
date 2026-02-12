/**
 * PIR Detector Implementation
 * Analyzes PIR sensor patterns to detect human presence
 */

#include "pir_detector.h"
#include "../include/config.h"

PIRDetector::PIRDetector(int left, int middle, int right) 
    : pinLeft(left), pinMiddle(middle), pinRight(right),
      lastTriggerTime(0), triggerCount(0), windowStart(0) {
    triggered[0] = false;
    triggered[1] = false;
    triggered[2] = false;
}

void PIRDetector::begin() {
    pinMode(pinLeft, INPUT_PULLDOWN);
    pinMode(pinMiddle, INPUT_PULLDOWN);
    pinMode(pinRight, INPUT_PULLDOWN);
    
    Serial.println("PIR Detector initialized");
}

void PIRDetector::update() {
    // Read PIR sensors
    bool left = digitalRead(pinLeft);
    bool middle = digitalRead(pinMiddle);
    bool right = digitalRead(pinRight);
    
    unsigned long now = millis();
    
    // Start new window if first trigger or window expired
    if (windowStart == 0 || (now - windowStart) > DETECTION_WINDOW_MS) {
        windowStart = now;
        triggerCount = 0;
        triggered[0] = false;
        triggered[1] = false;
        triggered[2] = false;
    }
    
    // Update trigger states within window
    if (left && !triggered[0]) {
        triggered[0] = true;
        triggerCount++;
        Serial.println("PIR Left triggered");
    }
    
    if (middle && !triggered[1]) {
        triggered[1] = true;
        triggerCount++;
        Serial.println("PIR Middle triggered");
    }
    
    if (right && !triggered[2]) {
        triggered[2] = true;
        triggerCount++;
        Serial.println("PIR Right triggered");
    }
}

HumanDetectionResult PIRDetector::detectHuman() {
    HumanDetectionResult result;
    result.pir_left = triggered[0];
    result.pir_middle = triggered[1];
    result.pir_right = triggered[2];
    result.timestamp = millis();
    
    // Human detection logic
    // Require at least 2 PIR sensors triggered within window
    if (triggerCount >= MIN_PIR_TRIGGERS) {
        result.detected = true;
        
        // Calculate confidence based on number of sensors
        if (triggerCount == 3) {
            result.confidence = 0.95;  // Very high confidence
        } else if (triggerCount == 2) {
            result.confidence = 0.80;  // High confidence
        } else {
            result.confidence = 0.60;  // Medium confidence
        }
        
        Serial.printf("Human detected! Confidence: %.2f, Sensors: %d\n", 
                     result.confidence, triggerCount);
    } else {
        result.detected = false;
        result.confidence = 0.0;
    }
    
    return result;
}

void PIRDetector::reset() {
    windowStart = 0;
    triggerCount = 0;
    triggered[0] = false;
    triggered[1] = false;
    triggered[2] = false;
}
