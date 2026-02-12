/**
 * Camera Handler Module
 * Manages OV2640 camera operations
 */

#ifndef CAMERA_HANDLER_H
#define CAMERA_HANDLER_H

#include <Arduino.h>
#include "esp_camera.h"

class CameraHandler {
private:
    bool initialized;
    camera_config_t config;
    
    void configurePins();
    
public:
    CameraHandler();
    
    bool begin();
    camera_fb_t* captureImage();
    void releaseFrameBuffer(camera_fb_t* fb);
    bool isInitialized();
};

#endif // CAMERA_HANDLER_H
