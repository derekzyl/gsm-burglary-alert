/**
 * Camera Handler Implementation
 * OV2640 camera initialization and control
 */

#include "camera_handler.h"
#include "config.h"

CameraHandler::CameraHandler() : initialized(false) {
}

void CameraHandler::configurePins() {
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    
    // Image settings
    config.frame_size = IMAGE_SIZE;
    config.jpeg_quality = JPEG_QUALITY;
    config.fb_count = FB_COUNT;
    
    // PSRAM settings
    if (psramFound()) {
        config.fb_location = CAMERA_FB_IN_PSRAM;
        config.grab_mode = CAMERA_GRAB_LATEST;
        Serial.println("PSRAM found - using for frame buffer");
    } else {
        config.fb_location = CAMERA_FB_IN_DRAM;
        config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
        Serial.println("No PSRAM found - using DRAM");
    }
}

bool CameraHandler::begin() {
    Serial.println("Initializing camera...");
    
    configurePins();
    
    // Initialize camera
    esp_err_t err = esp_camera_init(&config);
    
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;
    }
    
    // Get camera sensor
    sensor_t* sensor = esp_camera_sensor_get();
    
    if (sensor == nullptr) {
        Serial.println("Failed to get camera sensor");
        return false;
    }
    
    // Camera adjustments for better image quality
    sensor->set_brightness(sensor, 0);     // -2 to 2
    sensor->set_contrast(sensor, 0);       // -2 to 2
    sensor->set_saturation(sensor, 0);     // -2 to 2
    sensor->set_special_effect(sensor, 0); // 0 = No effect
    sensor->set_whitebal(sensor, 1);       // 0 = disable, 1 = enable
    sensor->set_awb_gain(sensor, 1);       // 0 = disable, 1 = enable
    sensor->set_wb_mode(sensor, 0);        // 0 to 4 - white balance mode
    sensor->set_exposure_ctrl(sensor, 1);  // 0 = disable, 1 = enable
    sensor->set_aec2(sensor, 0);           // 0 = disable, 1 = enable
    sensor->set_ae_level(sensor, 0);       // -2 to 2
    sensor->set_aec_value(sensor, 300);    // 0 to 1200
    sensor->set_gain_ctrl(sensor, 1);      // 0 = disable, 1 = enable
    sensor->set_agc_gain(sensor, 0);       // 0 to 30
    sensor->set_gainceiling(sensor, (gainceiling_t)0);  // 0 to 6
    sensor->set_bpc(sensor, 0);            // 0 = disable, 1 = enable
    sensor->set_wpc(sensor, 1);            // 0 = disable, 1 = enable
    sensor->set_raw_gma(sensor, 1);        // 0 = disable, 1 = enable
    sensor->set_lenc(sensor, 1);           // 0 = disable, 1 = enable
    sensor->set_hmirror(sensor, 0);        // 0 = disable, 1 = enable
    sensor->set_vflip(sensor, 0);          // 0 = disable, 1 = enable
    sensor->set_dcw(sensor, 1);            // 0 = disable, 1 = enable
    sensor->set_colorbar(sensor, 0);       // 0 = disable, 1 = enable
    
    initialized = true;
    Serial.println("Camera initialized successfully!");
    Serial.printf("Frame size: %dx%d, JPEG quality: %d\n", 
                 config.frame_size, config.frame_size, config.jpeg_quality);
    
    return true;
}

camera_fb_t* CameraHandler::captureImage() {
    if (!initialized) {
        Serial.println("Camera not initialized");
        return nullptr;
    }
    
    Serial.println("Capturing image...");
    
    // Capture image
    camera_fb_t* fb = esp_camera_fb_get();
    
    if (!fb) {
        Serial.println("Camera capture failed");
        return nullptr;
    }
    
    Serial.printf("Image captured: %d bytes, %dx%d\n", 
                 fb->len, fb->width, fb->height);
    
    return fb;
}

void CameraHandler::releaseFrameBuffer(camera_fb_t* fb) {
    if (fb) {
        esp_camera_fb_return(fb);
    }
}

bool CameraHandler::isInitialized() {
    return initialized;
}
