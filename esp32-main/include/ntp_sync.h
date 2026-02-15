/**
 * NTP Time Synchronization
 * Syncs system time with NTP server for accurate timestamps
 */

#ifndef NTP_SYNC_H
#define NTP_SYNC_H

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

class NTPSync {
private:
    bool synchronized;
    unsigned long lastSyncTime;
    
    const char* ntpServer = "pool.ntp.org";
    const long gmtOffset_sec = 3600;  // GMT+1 timezone (Nigeria)
    const int daylightOffset_sec = 0;
    
public:
    NTPSync();
    
    bool begin();
    bool syncTime();
    unsigned long getCurrentTimestamp();  // Returns epoch timestamp in seconds
    bool isSynchronized();
    void update();
};

#endif // NTP_SYNC_H
