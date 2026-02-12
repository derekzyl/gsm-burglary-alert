/**
 * HTTP Upload Implementation
 * Direct backend communication for image uploads
 */

#include "http_upload.h"
#include "../include/config.h"

HTTPUploader::HTTPUploader(const char* url, const char* key) 
    : serverUrl(url), apiKey(key) {
}

bool HTTPUploader::connectWiFi() {
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }
    
    Serial.print("Connecting to WiFi: ");
    Serial.println(WIFI_SSID);
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start > WIFI_CONNECT_TIMEOUT_MS) {
            Serial.println("\nWiFi connection failed!");
            return false;
        }
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.printf("Signal strength: %d dBm\n", WiFi.RSSI());
    
    return true;
}

bool HTTPUploader::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

int HTTPUploader::getSignalStrength() {
    return WiFi.RSSI();
}

String HTTPUploader::createMultipartBoundary() {
    return "----ESP32CAMBoundary" + String(random(100000, 999999));
}

bool HTTPUploader::uploadImage(camera_fb_t* fb, unsigned long timestamp) {
    if (!fb) {
        Serial.println("No frame buffer provided");
        return false;
    }
    
    return uploadImageFromBuffer(fb->buf, fb->len, timestamp);
}

bool HTTPUploader::uploadImageFromBuffer(uint8_t* buffer, size_t size, unsigned long timestamp) {
    if (!isConnected()) {
        Serial.println("WiFi not connected");
        return false;
    }
    
    // Check signal strength
    int rssi = getSignalStrength();
    if (rssi < MIN_SIGNAL_STRENGTH) {
        Serial.printf("Weak signal (%d dBm), upload may fail\n", rssi);
    }
    
    Serial.printf("Uploading image to backend: %d bytes\n", size);
    Serial.printf("URL: %s\n", serverUrl.c_str());
    
    String boundary = createMultipartBoundary();
    
    // Construct multipart/form-data payload
    String head = "--" + boundary + "\r\n";
    head += "Content-Disposition: form-data; name=\"file\"; filename=\"capture.jpg\"\r\n";
    head += "Content-Type: image/jpeg\r\n\r\n";
    
    String tail = "\r\n--" + boundary + "--\r\n";
    
    uint32_t totalLen = head.length() + size + tail.length();
    
    http.begin(serverUrl);
    http.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);
    http.addHeader("Content-Length", String(totalLen));
    http.addHeader("X-API-Key", apiKey);
    http.setTimeout(SERVER_TIMEOUT_MS);
    
    // Create payload
    uint8_t* payload = (uint8_t*)malloc(totalLen);
    if (!payload) {
        Serial.println("Failed to allocate memory for payload");
        http.end();
        return false;
    }
    
    // Copy header
    memcpy(payload, head.c_str(), head.length());
    
    // Copy image data
    memcpy(payload + head.length(), buffer, size);
    
    // Copy tail
    memcpy(payload + head.length() + size, tail.c_str(), tail.length());
    
    // Send POST request
    Serial.println("Sending HTTP POST...");
    int httpCode = http.POST(payload, totalLen);
    
    free(payload);
    
    if (httpCode > 0) {
        Serial.printf("HTTP Response code: %d\n", httpCode);
        
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
            String response = http.getString();
            Serial.println("Server response:");
            Serial.println(response);
            http.end();
            return true;
        } else {
            String response = http.getString();
            Serial.println("Server error response:");
            Serial.println(response);
        }
    } else {
        Serial.printf("HTTP POST failed: %s\n", http.errorToString(httpCode).c_str());
    }
    
    http.end();
    return false;
}
