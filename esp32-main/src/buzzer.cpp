/**
 * Buzzer Control Implementation
 */

#include "buzzer.h"
#include "config.h"

Buzzer::Buzzer(int buzzerPin) : pin(buzzerPin) {
}

void Buzzer::begin() {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

void Buzzer::tone(int frequency, int durationMs) {
    // Use ledcWrite for DAC pin (GPIO 25)
    ledcSetup(0, frequency, 8);  // Channel 0, frequency, 8-bit resolution
    ledcAttachPin(pin, 0);
    ledcWrite(0, 128);  // 50% duty cycle
    
    delay(durationMs);
    
    ledcWrite(0, 0);  // Stop
}

void Buzzer::beep(int durationMs) {
    tone(BUZZER_FREQUENCY, durationMs);
}

void Buzzer::playAlertPattern() {
    Serial.println("Playing alert pattern");
    
    for (int i = 0; i < BUZZER_BEEPS; i++) {
        beep(BUZZER_ON_MS);
        
        if (i < BUZZER_BEEPS - 1) {  // No delay after last beep
            delay(BUZZER_OFF_MS);
        }
    }
}
