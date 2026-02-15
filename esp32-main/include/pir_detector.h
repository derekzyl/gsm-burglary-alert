/**
 * PIR Detector Module
 * Handles human detection logic using multiple PIR sensors
 */

#ifndef PIR_DETECTOR_H
#define PIR_DETECTOR_H

#include <Arduino.h>

struct HumanDetectionResult {
    bool detected;
    float confidence;
    bool pir_left;
    bool pir_middle;
    bool pir_right;
    unsigned long timestamp;
};

class PIRDetector {
private:
    int pinLeft;
    int pinMiddle;
    int pinRight;
    
    unsigned long lastTriggerTime;
    int triggerCount;
    bool triggered[3];  // left, middle, right
    
    unsigned long windowStart;
    
public:
    PIRDetector(int left, int middle, int right);
    
    void begin();
    void update();
    HumanDetectionResult detectHuman();
    void reset();
};

#endif // PIR_DETECTOR_H
