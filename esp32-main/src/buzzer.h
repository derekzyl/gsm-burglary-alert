/**
 * Buzzer Control Module
 * Generates alert beep patterns
 */

#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

class Buzzer {
private:
    int pin;
    
public:
    Buzzer(int buzzerPin);
    
    void begin();
    void playAlertPattern();
    void beep(int durationMs);
    void tone(int frequency, int durationMs);
};

#endif // BUZZER_H
