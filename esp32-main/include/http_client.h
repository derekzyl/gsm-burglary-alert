/**
 * HTTP Client Module
 * Handles communication with backend server
 */

#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "pir_detector.h"

class BackendClient {
private:
    String apiKey;
    String baseUrl;
    HTTPClient http;
    
    int retryCount;
    unsigned long lastRetryTime;
    
public:
    BackendClient(const char* url, const char* key);
    
    bool postAlert(HumanDetectionResult& detection, const char* networkStatus);
    bool sendHeartbeat(const char* deviceId, const char* status, const char* ip, const char* version);
    bool connectWiFi();
    bool isConnected();
    void reconnect();
};

#endif // HTTP_CLIENT_H
