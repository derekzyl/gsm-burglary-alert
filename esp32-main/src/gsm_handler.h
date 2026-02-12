/**
 * GSM Handler Module
 * Manages SIM800L GSM module for SMS alerts
 */

#ifndef GSM_HANDLER_H
#define GSM_HANDLER_H

#include <Arduino.h>
#include <HardwareSerial.h>

class GSMHandler {
private:
    HardwareSerial* gsmSerial;
    unsigned long lastSMSTime;
    bool initialized;
    
    bool waitForResponse(const char* expected, unsigned long timeout = 5000);
    bool sendATCommand(const char* command, const char* expectedResponse = "OK", unsigned long timeout = 5000);
    
public:
    GSMHandler(HardwareSerial* serial);
    
    bool begin();
    bool sendSMS(const char* phoneNumber, const char* message);
    bool isNetworkRegistered();
    int getSignalStrength();
    bool canSendSMS();  // Rate limiting check
};

#endif // GSM_HANDLER_H
