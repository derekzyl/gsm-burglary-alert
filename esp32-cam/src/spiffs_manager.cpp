/**
 * SPIFFS Manager Implementation
 * Offline image storage and queue management
 */

#include "spiffs_manager.h"
#include "config.h"

SPIFFSManager::SPIFFSManager() : initialized(false) {
}

bool SPIFFSManager::begin() {
    Serial.println("Initializing SPIFFS...");
    
    if (!SPIFFS.begin(true)) {  // true = format if mount fails
        Serial.println("SPIFFS initialization failed");
        return false;
    }
    
    size_t totalBytes = SPIFFS.totalBytes();
    size_t usedBytes = SPIFFS.usedBytes();
    
    Serial.printf("SPIFFS initialized: %d/%d bytes used\n", usedBytes, totalBytes);
    
    initialized = true;
    return true;
}

String SPIFFSManager::generateFilename(unsigned long timestamp) {
    char filename[64];
    sprintf(filename, "%s%lu%s", IMAGE_PREFIX, timestamp, IMAGE_EXTENSION);
    return String(filename);
}

bool SPIFFSManager::saveImage(camera_fb_t* fb, unsigned long timestamp) {
    if (!initialized || !fb) {
        return false;
    }
    
    String filename = generateFilename(timestamp);
    
    Serial.printf("Saving image to SPIFFS: %s (%d bytes)\n", filename.c_str(), fb->len);
    
    File file = SPIFFS.open(filename, FILE_WRITE);
    
    if (!file) {
        Serial.println("Failed to open file for writing");
        return false;
    }
    
    size_t written = file.write(fb->buf, fb->len);
    file.close();
    
    if (written != fb->len) {
        Serial.printf("Write error: %d bytes written, expected %d\n", written, fb->len);
        return false;
    }
    
    Serial.println("Image saved to SPIFFS successfully");
    
    // Check if we need to cleanup
    cleanupOldImages();
    
    return true;
}

int SPIFFSManager::getQueuedImageCount() {
    if (!initialized) {
        return 0;
    }
    
    int count = 0;
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    
    while (file) {
        String filename = file.name();
        if (filename.startsWith(IMAGE_PREFIX) && filename.endsWith(IMAGE_EXTENSION)) {
            count++;
        }
        file = root.openNextFile();
    }
    
    return count;
}

QueuedImage* SPIFFSManager::getQueuedImages(int& count) {
    if (!initialized) {
        count = 0;
        return nullptr;
    }
    
    // Count images first
    int imageCount = getQueuedImageCount();
    
    if (imageCount == 0) {
        count = 0;
        return nullptr;
    }
    
    // Allocate array
    QueuedImage* images = new QueuedImage[imageCount];
    int index = 0;
    
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    
    while (file && index < imageCount) {
        String filename = file.name();
        
        if (filename.startsWith(IMAGE_PREFIX) && filename.endsWith(IMAGE_EXTENSION)) {
            images[index].filename = filename;
            images[index].size = file.size();
            
            // Extract timestamp from filename
            String timestampStr = filename.substring(
                strlen(IMAGE_PREFIX), 
                filename.length() - strlen(IMAGE_EXTENSION)
            );
            images[index].timestamp = timestampStr.toInt();
            
            index++;
        }
        
        file = root.openNextFile();
    }
    
    count = index;
    return images;
}

bool SPIFFSManager::deleteImage(const String& filename) {
    if (!initialized) {
        return false;
    }
    
    if (!SPIFFS.exists(filename)) {
        return false;
    }
    
    bool deleted = SPIFFS.remove(filename);
    
    if (deleted) {
        Serial.printf("Deleted: %s\n", filename.c_str());
    }
    
    return deleted;
}

void SPIFFSManager::cleanupOldImages() {
    int count = getQueuedImageCount();
    
    if (count <= SPIFFS_MAX_IMAGES) {
        return;  // No cleanup needed
    }
    
    Serial.printf("Cleanup: %d images, max %d - deleting oldest\n", count, SPIFFS_MAX_IMAGES);
    
    int arrayCount = 0;
    QueuedImage* images = getQueuedImages(arrayCount);
    
    if (!images) {
        return;
    }
    
    // Sort by timestamp (oldest first)
    for (int i = 0; i < arrayCount - 1; i++) {
        for (int j = i + 1; j < arrayCount; j++) {
            if (images[i].timestamp > images[j].timestamp) {
                QueuedImage temp = images[i];
                images[i] = images[j];
                images[j] = temp;
            }
        }
    }
    
    // Delete oldest images until we're under the limit
    int toDelete = count - SPIFFS_MAX_IMAGES;
    for (int i = 0; i < toDelete && i < arrayCount; i++) {
        deleteImage(images[i].filename);
    }
    
    delete[] images;
}

bool SPIFFSManager::readImage(const String& filename, uint8_t** buffer, size_t* size) {
    if (!initialized) {
        return false;
    }
    
    if (!SPIFFS.exists(filename)) {
        Serial.printf("File not found: %s\n", filename.c_str());
        return false;
    }
    
    File file = SPIFFS.open(filename, FILE_READ);
    
    if (!file) {
        Serial.println("Failed to open file for reading");
        return false;
    }
    
    *size = file.size();
    *buffer = (uint8_t*)malloc(*size);
    
    if (!*buffer) {
        Serial.println("Failed to allocate memory for image");
        file.close();
        return false;
    }
    
    size_t bytesRead = file.read(*buffer, *size);
    file.close();
    
    if (bytesRead != *size) {
        Serial.printf("Read error: %d bytes read, expected %d\n", bytesRead, *size);
        free(*buffer);
        *buffer = nullptr;
        return false;
    }
    
    return true;
}
