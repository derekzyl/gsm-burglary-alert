/**
 * HTTP Client Implementation
 * Backend API communication
 */

#include "http_client.h"
#include "../include/config.h"
#include <ArduinoJson.h>

BackendClient::BackendClient(const char* url, const char* key) 
    : baseUrl(url), apiKey(key), retryCount(0), lastRetryTime(0) {
}

bool BackendClient::connectWiFi() {
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
    
    return true;
}

bool BackendClient::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void BackendClient::reconnect() {
    WiFi.disconnect();
    delay(1000);
    connectWiFi();
}

bool BackendClient::postAlert(HumanDetectionResult& detection, const char* networkStatus) {
    if (!isConnected()) {
        Serial.println("WiFi not connected, cannot post alert");
        return false;
    }
    
    // Prepare JSON payload
    StaticJsonDocument<512> doc;
    doc["timestamp"] = millis();  // Will be converted to epoch time on backend
    doc["detection_confidence"] = detection.confidence;
    doc["pir_left"] = detection.pir_left;
    doc["pir_middle"] = detection.pir_middle;
    doc["pir_right"] = detection.pir_right;
    doc["network_status"] = networkStatus;
    
    String payload;
    serializeJson(doc, payload);
    
    // Prepare HTTP request
    String url = String(baseUrl) + "/api/v1/burglary/alert/alert";
    
    Serial.println("Posting alert to backend:");
    Serial.println(url);
    Serial.println(payload);
    
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-API-Key", apiKey);
    http.setTimeout(SERVER_TIMEOUT_MS);
    
    int httpCode = http.POST(payload);
    
    if (httpCode > 0) {
        Serial.printf("HTTP Response code: %d\n", httpCode);
        
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
            String response = http.getString();
            Serial.println("Server response:");
            Serial.println(response);
            http.end();
            return true;
        }
    } else {
        Serial.printf("HTTP POST failed: %s\n", http.errorToString(httpCode).c_str());
    }
    
    http.end();
    return false;
}
