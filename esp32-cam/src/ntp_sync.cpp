/**
 * NTP Synchronization Implementation
 */

#include "ntp_sync.h"
#include "config.h"

NTPSync::NTPSync() : timeClient(nullptr), lastSyncTime(0), synchronized(false) {
}

NTPSync::~NTPSync() {
    if (timeClient) {
        delete timeClient;
    }
}

bool NTPSync::begin() {
    Serial.println("Initializing NTP client...");
    
    timeClient = new NTPClient(ntpUDP, NTP_SERVER, GMT_OFFSET_SEC, 60000);
    
    if (!timeClient) {
        Serial.println("Failed to create NTP client");
        return false;
    }
    
    timeClient->begin();
    
    return syncTime();
}

bool NTPSync::syncTime() {
    if (!timeClient) {
        return false;
    }
    
    Serial.println("Synchronizing time with NTP server...");
    
    for (int i = 0; i < 5; i++) {  // Try 5 times
        if (timeClient->update()) {
            synchronized = true;
            lastSyncTime = millis();
            
            unsigned long epoch = timeClient->getEpochTime();
            Serial.printf("Time synchronized: %lu (epoch)\n", epoch);
            Serial.printf("Formatted: %s\n", timeClient->getFormattedTime().c_str());
            
            return true;
        }
        
        delay(1000);
    }
    
    Serial.println("Failed to sync time");
    return false;
}

unsigned long NTPSync::getCurrentTimestamp() {
    if (!timeClient || !synchronized) {
        return 0;
    }
    
    return timeClient->getEpochTime();
}

bool NTPSync::isSynchronized() {
    return synchronized;
}

void NTPSync::update() {
    if (!timeClient) {
        return;
    }
    
    // Update time client
    timeClient->update();
    
    // Periodic re-sync
    unsigned long now = millis();
    if (synchronized && (now - lastSyncTime > NTP_SYNC_INTERVAL_MS)) {
        syncTime();
    }
}
