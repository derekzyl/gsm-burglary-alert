/**
 * NTP Time Synchronization Implementation
 */

#include "ntp_sync.h"
#include "config.h"

NTPSync::NTPSync() : synchronized(false), lastSyncTime(0) {
}

bool NTPSync::begin() {
    if (!WiFi.isConnected()) {
        Serial.println("WiFi not connected - cannot sync NTP");
        return false;
    }
    
    Serial.println("Initializing NTP time sync...");
    
    // Configure time with NTP
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer, "time.nist.gov");
    
    return syncTime();
}

bool NTPSync::syncTime() {
    if (!WiFi.isConnected()) {
        Serial.println("WiFi not connected - cannot sync NTP");
        return false;
    }
    
    Serial.println("Synchronizing time with NTP server...");
    
    time_t now = time(nullptr);
    int attempts = 0;
    
    // Wait for NTP to sync (max 20 seconds)
    while (now < 24 * 3600 * 2 && attempts < 40) {
        delay(500);
        Serial.print(".");
        now = time(nullptr);
        attempts++;
    }
    
    Serial.println();
    
    if (now > 24 * 3600 * 2) {
        synchronized = true;
        lastSyncTime = millis();
        
        struct tm timeinfo;
        localtime_r(&now, &timeinfo);
        
        Serial.print("Time synchronized: ");
        Serial.println(asctime(&timeinfo));
        Serial.printf("Epoch: %lu\n", (unsigned long)now);
        
        return true;
    } else {
        Serial.println("Failed to sync time with NTP");
        return false;
    }
}

unsigned long NTPSync::getCurrentTimestamp() {
    if (!synchronized) {
        return 0;
    }
    
    return (unsigned long)time(nullptr);
}

bool NTPSync::isSynchronized() {
    return synchronized;
}

void NTPSync::update() {
    // Optional: Periodic re-sync every 24 hours
    if (synchronized && (millis() - lastSyncTime > 86400000)) {
        syncTime();
    }
}
