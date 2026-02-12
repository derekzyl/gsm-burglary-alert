/**
 * NTP Synchronization Module
 * Manages time synchronization for accurate timestamps
 */

#ifndef NTP_SYNC_H
#define NTP_SYNC_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

class NTPSync {
private:
    WiFiUDP ntpUDP;
    NTPClient* timeClient;
    unsigned long lastSyncTime;
    bool synchronized;
    
public:
    NTPSync();
    ~NTPSync();
    
    bool begin();
    bool syncTime();
    unsigned long getCurrentTimestamp();
    bool isSynchronized();
    void update();
};

#endif // NTP_SYNC_H
