/**
 * SPIFFS Manager Module
 * Manages offline image storage and queued uploads
 */

#ifndef SPIFFS_MANAGER_H
#define SPIFFS_MANAGER_H

#include <Arduino.h>
#include <SPIFFS.h>
#include "esp_camera.h"

struct QueuedImage {
    String filename;
    unsigned long timestamp;
    size_t size;
};

class SPIFFSManager {
private:
    bool initialized;
    
    String generateFilename(unsigned long timestamp);
    
public:
    SPIFFSManager();
    
    bool begin();
    bool saveImage(camera_fb_t* fb, unsigned long timestamp);
    int getQueuedImageCount();
    QueuedImage* getQueuedImages(int& count);
    bool deleteImage(const String& filename);
    void cleanupOldImages();
    bool readImage(const String& filename, uint8_t** buffer, size_t* size);
};

#endif // SPIFFS_MANAGER_H
