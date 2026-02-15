/**
 * HTTP Upload Implementation
 * Direct backend communication for image uploads
 */

#include "http_upload.h"
#include "config.h"

HTTPUploader::HTTPUploader(const char* url, const char* key) 
    : serverUrl(url), apiKey(key) {
}

bool HTTPUploader::connectWiFi() {
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }
    
    Serial.print("Connecting to WiFi: ");
    Serial.println(WIFI_SSID);
    Serial.print("Device MAC: ");
    Serial.println(WiFi.macAddress());

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false); // Disable power saving for better stability
    WiFi.disconnect();
    delay(1000); // Increased delay for stability
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    unsigned long start = millis();
    int lastStatus = -1;

    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start > WIFI_CONNECT_TIMEOUT_MS) {
            Serial.println("\nWiFi connection failed!");
            Serial.print("Final Status: ");
            Serial.println(WiFi.status());
            Serial.println("Reasons: 1=NoSSID, 4=Fail, 6=Disconnect");
            return false;
        }
        
        int currentStatus = WiFi.status();
        if (currentStatus != lastStatus) {
            Serial.printf("\nStatus changed: %d -> %d ", lastStatus, currentStatus);
            lastStatus = currentStatus;
        }
        
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.printf("Channel: %d\n", WiFi.channel());
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
    
    String boundary = createMultipartBoundary();
    
    // Construct multipart headers (but don't combine yet)
    String head = "--" + boundary + "\r\n";
    head += "Content-Disposition: form-data; name=\"file\"; filename=\"capture.jpg\"\r\n";
    head += "Content-Type: image/jpeg\r\n\r\n";
    
    String tail = "\r\n--" + boundary + "--\r\n";
    
    size_t totalLen = head.length() + size + tail.length();
    
    // Use the underlying WiFiClient from HTTPClient isn't directly exposed for writing in a standard way
    // reliably across all versions, so we'll use strict HTTPClient methods or
    // manual WiFiClientSecure if we want to stream.
    // simpler approach: The ESP32 HTTPClient library *does* support sending a stream,
    // but constructing a stream from parts is hard.
    //
    // However, since we are using HTTPS, we should be careful with manual implementation.
    // PlatformIO ESP32 usually allows `begin` then `addHeader` then... 
    // actually, let's try to verify if we can do this:
    // http.begin(url);
    // http.addHeader(...);
    // // The internal client is private. 
    //
    // OK, looking at standard valid ESP32 solutions for this:
    // The "correct" way to avoid big buffer is to not use HTTPClient for the body, 
    // OR use a custom Stream class. 
    // 
    // Let's implement a clean manual HTTPS POST using WiFiClientSecure.
    // But I need to parse the URL (host, port, path).
    // The URL is in config.h.
    
    // Parse URL
    String urlStr = serverUrl;
    String protocol = "https";
    int port = 443;
    String host;
    String path;
    
    int doubleSlash = urlStr.indexOf("//");
    if (doubleSlash != -1) {
        protocol = urlStr.substring(0, doubleSlash - 1); // http or https
        int nextSlash = urlStr.indexOf('/', doubleSlash + 2);
        if (nextSlash != -1) {
            host = urlStr.substring(doubleSlash + 2, nextSlash);
            path = urlStr.substring(nextSlash);
        } else {
            host = urlStr.substring(doubleSlash + 2);
            path = "/";
        }
    }
    
    // Handle port in host
    int colon = host.indexOf(':');
    if (colon != -1) {
        port = host.substring(colon + 1).toInt();
        host = host.substring(0, colon);
    } else {
        if (protocol == "http") port = 80;
    }
    
    WiFiClient* clientPtr;
    WiFiClientSecure secureClient;
    WiFiClient plainClient;
    
    if (protocol == "https") {
        secureClient.setInsecure(); // Skip certificate validation for simplicity/robustness
        clientPtr = &secureClient;
    } else {
        clientPtr = &plainClient;
    }
    
    Serial.printf("Connecting to %s:%d...\n", host.c_str(), port);
    
    if (!clientPtr->connect(host.c_str(), port)) {
        Serial.println("Connection failed!");
        return false;
    }
    
    Serial.println("Sending chunks...");
    
    // Send Request Line
    clientPtr->print("POST " + path + " HTTP/1.1\r\n");
    clientPtr->print("Host: " + host + "\r\n");
    clientPtr->print("User-Agent: ESP32-CAM\r\n");
    clientPtr->print("X-API-Key: " + apiKey + "\r\n");
    clientPtr->print("Content-Type: multipart/form-data; boundary=" + boundary + "\r\n");
    clientPtr->print("Content-Length: " + String(totalLen) + "\r\n");
    clientPtr->print("Connection: close\r\n\r\n");
    
    // Send Body chunks
    clientPtr->print(head);
    
    // Send image in 1KB chunks
    const size_t chunkSize = 1024;
    size_t remaining = size;
    size_t offset = 0;
    
    while (remaining > 0) {
        size_t toWrite = (remaining > chunkSize) ? chunkSize : remaining;
        clientPtr->write(buffer + offset, toWrite);
        remaining -= toWrite;
        offset += toWrite;
        
        // Use yield() to prevent WDT reset during long uploads
        if (offset % 10240 == 0) yield(); 
    }
    
    clientPtr->print(tail);
    
    // Read Response
    unsigned long timeout = millis();
    while (clientPtr->available() == 0) {
        if (millis() - timeout > 10000) {
            Serial.println("Timeout waiting for response");
            clientPtr->stop();
            return false;
        }
        delay(10);
    }
    
    Serial.println("Reading response...");
    String responseLine = clientPtr->readStringUntil('\n');
    Serial.println(responseLine); // HTTP/1.1 200 OK
    
    bool success = false;
    if (responseLine.indexOf("200") != -1 || responseLine.indexOf("201") != -1) {
        success = true;
    }
    
    // Read headers (skip)
    while (clientPtr->connected()) {
        String line = clientPtr->readStringUntil('\n');
        if (line == "\r") break;
    }
    
    // Read body (optional, print it)
    if (clientPtr->available()) {
        String body = clientPtr->readString();
        Serial.println("Response body: " + body);
    }
    
    clientPtr->stop();
    Serial.println(success ? "Upload successful" : "Upload failed (HTTP code)");
    return success;
}

bool HTTPUploader::sendHeartbeat(const char* deviceId, const char* status, const char* ip, const char* version) {
    if (!isConnected()) {
        return false;
    }
    
    // Manual JSON construction
    String payload = "{";
    payload += "\"device_id\":\"" + String(deviceId) + "\",";
    payload += "\"status\":\"" + String(status) + "\",";
    payload += "\"ip_address\":\"" + String(ip) + "\",";
    payload += "\"firmware_version\":\"" + String(version) + "\"";
    payload += "}";
    
    // Parse URL from config
    String urlStr = serverUrl;
    String protocol = "https";
    int port = 443;
    String host;
    String basePath; // The base path of the backend URL
    
    int doubleSlash = urlStr.indexOf("//");
    if (doubleSlash != -1) {
        protocol = urlStr.substring(0, doubleSlash - 1);
        int nextSlash = urlStr.indexOf('/', doubleSlash + 2);
        if (nextSlash != -1) {
            host = urlStr.substring(doubleSlash + 2, nextSlash);
            basePath = urlStr.substring(nextSlash);
        } else {
            host = urlStr.substring(doubleSlash + 2);
            basePath = "/";
        }
    }
    
    // Clean base path (remove specific endpoint if present)
    String fullPath = basePath;
    int imageIdx = fullPath.indexOf("/image/image");
    if (imageIdx != -1) {
        fullPath = fullPath.substring(0, imageIdx) + "/device/heartbeat";
    } else {
        // Fallback: try to replace last segment
         int lastSlash = fullPath.lastIndexOf('/');
         if (lastSlash != -1) {
            fullPath = fullPath.substring(0, lastSlash) + "/device/heartbeat";
         }
    }
    
    // Handle port in host
    int colon = host.indexOf(':');
    if (colon != -1) {
        port = host.substring(colon + 1).toInt();
        host = host.substring(0, colon);
    } else {
        if (protocol == "http") port = 80;
    }
    
    WiFiClient* clientPtr;
    WiFiClientSecure secureClient;
    WiFiClient plainClient;
    
    if (protocol == "https") {
        secureClient.setInsecure();
        clientPtr = &secureClient;
    } else {
        clientPtr = &plainClient;
    }
    
    if (!clientPtr->connect(host.c_str(), port)) {
        return false;
    }
    
    clientPtr->print("POST " + fullPath + " HTTP/1.1\r\n");
    clientPtr->print("Host: " + host + "\r\n");
    clientPtr->print("User-Agent: ESP32-CAM\r\n");
    clientPtr->print("X-API-Key: " + apiKey + "\r\n");
    clientPtr->print("Content-Type: application/json\r\n");
    clientPtr->print("Content-Length: " + String(payload.length()) + "\r\n");
    clientPtr->print("Connection: close\r\n\r\n");
    clientPtr->print(payload);
    
    // Read response
    unsigned long timeout = millis();
    while (clientPtr->available() == 0) {
        if (millis() - timeout > 5000) {
            clientPtr->stop();
            return false;
        }
        delay(10);
    }
    
    String responseLine = clientPtr->readStringUntil('\n');
    clientPtr->stop();
    
    return (responseLine.indexOf("200") != -1);
}
