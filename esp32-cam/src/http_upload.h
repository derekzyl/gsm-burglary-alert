/**
 * HTTP Upload Module
 * Direct image upload to backend server
 */

#ifndef HTTP_UPLOAD_H
#define HTTP_UPLOAD_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "esp_camera.h"

class HTTPUploader {
private:
    String apiKey;
    String serverUrl;
    HTTPClient http;
    
    String createMultipartBoundary();
    
public:
    HTTPUploader(const char* url, const char* key);
    
    bool uploadImage(camera_fb_t* fb, unsigned long timestamp);
    bool uploadImageFromBuffer(uint8_t* buffer, size_t size, unsigned long timestamp);
    bool connectWiFi();
    bool isConnected();
    int getSignalStrength();
    bool sendHeartbeat(const char* deviceId, const char* status, const char* ip, const char* version);
};

#endif // HTTP_UPLOAD_H
