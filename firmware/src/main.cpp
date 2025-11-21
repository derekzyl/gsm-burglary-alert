#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"

// Forward declarations
class GSMController;
class SensorManager;
class AlarmSystem;

// Global instances
GSMController* gsmController = nullptr;
SensorManager* sensorManager = nullptr;
AlarmSystem* alarmSystem = nullptr;

// System state
SystemStatus systemStatus = STATUS_DISARMED;
unsigned long lastHeartbeat = 0;
unsigned long alarmTriggerTime = 0;
bool alarmActive = false;
int alarmDelaySeconds = 30;

// WiFi and HTTP
bool wifiConnected = false;
String deviceConfig = "";

// GSM Controller Class
class GSMController {
private:
  HardwareSerial* simSerial;
  bool initialized = false;
  String lastSMS = "";
  
public:
  GSMController() {
    simSerial = new HardwareSerial(2);
    simSerial->begin(9600, SERIAL_8N1, SIM800L_RX_PIN, SIM800L_TX_PIN);
    pinMode(SIM800L_PWR_PIN, OUTPUT);
    pinMode(SIM800L_RST_PIN, OUTPUT);
  }
  
  bool init() {
    Serial.println("[GSM] Initializing GSM module...");
    digitalWrite(SIM800L_PWR_PIN, HIGH);
    delay(2000);
    digitalWrite(SIM800L_RST_PIN, LOW);
    delay(1000);
    digitalWrite(SIM800L_RST_PIN, HIGH);
    delay(3000);
    
    sendATCommand("AT");
    delay(1000);
    sendATCommand("ATE0");  // Echo off
    delay(1000);
    
    if (sendATCommand("AT+CPIN?") && waitForResponse("READY", 5000)) {
      Serial.println("[GSM] SIM card ready");
    } else {
      Serial.println("[GSM] SIM card not ready!");
      return false;
    }
    
    sendATCommand("AT+CMGF=1");  // SMS text mode
    delay(1000);
    sendATCommand("AT+CNMI=2,2,0,0,0");  // New SMS indication
    delay(1000);
    
    initialized = true;
    Serial.println("[GSM] Module initialized");
    return true;
  }
  
  bool sendATCommand(const String& cmd) {
    simSerial->println(cmd);
    delay(100);
    return true;
  }
  
  bool waitForResponse(const String& expected, unsigned long timeout = 2000) {
    unsigned long startTime = millis();
    String response = "";
    
    while (millis() - startTime < timeout) {
      if (simSerial->available()) {
        char c = simSerial->read();
        response += c;
        if (response.indexOf(expected) >= 0) {
          return true;
        }
      }
    }
    return false;
  }
  
  bool sendSMS(const String& phoneNumber, const String& message) {
    if (!initialized) return false;
    
    Serial.println("[GSM] Sending SMS to: " + phoneNumber);
    sendATCommand("AT+CMGS=\"" + phoneNumber + "\"");
    delay(1000);
    simSerial->print(message);
    simSerial->write(26);  // Ctrl+Z
    delay(5000);
    
    if (waitForResponse("OK", 10000)) {
      Serial.println("[GSM] SMS sent successfully");
      return true;
    } else {
      Serial.println("[GSM] SMS failed to send");
      return false;
    }
  }
  
  bool makeCall(const String& phoneNumber) {
    if (!initialized) return false;
    
    Serial.println("[GSM] Making call to: " + phoneNumber);
    sendATCommand("ATD" + phoneNumber + ";");
    delay(5000);
    
    // Hang up after 10 seconds
    delay(10000);
    sendATCommand("ATH");
    return true;
  }
  
  int getSignalStrength() {
    if (!initialized) return 0;
    
    sendATCommand("AT+CSQ");
    delay(1000);
    
    String response = "";
    unsigned long startTime = millis();
    while (millis() - startTime < 2000) {
      if (simSerial->available()) {
        char c = simSerial->read();
        response += c;
      }
    }
    
    int rssiIndex = response.indexOf("+CSQ: ");
    if (rssiIndex >= 0) {
      int rssi = response.substring(rssiIndex + 6, rssiIndex + 8).toInt();
      return rssi;
    }
    return 0;
  }
  
  void checkSMS() {
    // Check for incoming SMS commands
    if (simSerial->available()) {
      String response = simSerial->readString();
      if (response.indexOf("ARM") >= 0) {
        systemStatus = STATUS_ARMED;
        Serial.println("[GSM] Received ARM command via SMS");
      } else if (response.indexOf("DISARM") >= 0) {
        systemStatus = STATUS_DISARMED;
        alarmActive = false;
        Serial.println("[GSM] Received DISARM command via SMS");
      } else if (response.indexOf("STATUS") >= 0) {
        String statusMsg = "Status: " + String(systemStatus == STATUS_ARMED ? "ARMED" : "DISARMED");
        sendSMS(PHONE_NUMBER_1, statusMsg);
      }
    }
  }
  
  void loop() {
    checkSMS();
  }
};

// Sensor Manager Class
class SensorManager {
private:
  bool pir1State = false;
  bool pir2State = false;
  bool magnetic1State = false;
  bool magnetic2State = false;
  bool shockState = false;
  bool tamperState = false;
  
  unsigned long lastPIR1Time = 0;
  unsigned long lastPIR2Time = 0;
  unsigned long lastShockTime = 0;
  
  bool readDigitalSensor(int pin, bool& state) {
    bool currentState = digitalRead(pin);
    if (currentState != state) {
      delay(DEBOUNCE_DELAY);
      currentState = digitalRead(pin);
      if (currentState != state) {
        state = currentState;
        return true;
      }
    }
    return false;
  }
  
public:
  void init() {
    pinMode(PIR_SENSOR_1_PIN, INPUT);
    pinMode(PIR_SENSOR_2_PIN, INPUT);
    pinMode(MAGNETIC_SENSOR_1_PIN, INPUT_PULLUP);
    pinMode(MAGNETIC_SENSOR_2_PIN, INPUT_PULLUP);
    pinMode(SHOCK_SENSOR_PIN, INPUT);
    pinMode(TAMPER_SWITCH_PIN, INPUT_PULLUP);
    
    // Read initial states
    magnetic1State = !digitalRead(MAGNETIC_SENSOR_1_PIN);  // Reed switch is NO
    magnetic2State = !digitalRead(MAGNETIC_SENSOR_2_PIN);
    tamperState = digitalRead(TAMPER_SWITCH_PIN);
  }
  
  bool checkPIR1() {
    bool newState = digitalRead(PIR_SENSOR_1_PIN);
    if (newState && !pir1State) {
      lastPIR1Time = millis();
      pir1State = true;
      return true;
    }
    if (!newState && pir1State && (millis() - lastPIR1Time > 1000)) {
      pir1State = false;
    }
    return false;
  }
  
  bool checkPIR2() {
    bool newState = digitalRead(PIR_SENSOR_2_PIN);
    if (newState && !pir2State) {
      lastPIR2Time = millis();
      pir2State = true;
      return true;
    }
    if (!newState && pir2State && (millis() - lastPIR2Time > 1000)) {
      pir2State = false;
    }
    return false;
  }
  
  bool checkMagnetic1() {
    return readDigitalSensor(MAGNETIC_SENSOR_1_PIN, magnetic1State);
  }
  
  bool checkMagnetic2() {
    return readDigitalSensor(MAGNETIC_SENSOR_2_PIN, magnetic2State);
  }
  
  bool checkShock() {
    bool newState = digitalRead(SHOCK_SENSOR_PIN);
    if (newState && !shockState) {
      lastShockTime = millis();
      shockState = true;
      return true;
    }
    if (!newState && shockState && (millis() - lastShockTime > 500)) {
      shockState = false;
    }
    return false;
  }
  
  bool checkTamper() {
    bool newState = digitalRead(TAMPER_SWITCH_PIN);
    if (newState != tamperState) {
      tamperState = newState;
      return true;
    }
    return false;
  }
  
  void loop() {
    // Sensors are checked individually in main loop
  }
};

// Alarm System Class
class AlarmSystem {
private:
  bool sirenState = false;
  
public:
  void init() {
    pinMode(SIREN_RELAY_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_STATUS_PIN, OUTPUT);
    digitalWrite(SIREN_RELAY_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  }
  
  void triggerAlarm(bool silent = false) {
    if (!silent && !sirenState) {
      digitalWrite(SIREN_RELAY_PIN, HIGH);  // Activate siren relay
      sirenState = true;
      Serial.println("[ALARM] Siren activated");
    }
    
    // Blink LED rapidly
    digitalWrite(LED_STATUS_PIN, HIGH);
    delay(100);
    digitalWrite(LED_STATUS_PIN, LOW);
    delay(100);
    
    // Beep buzzer
    tone(BUZZER_PIN, 2000, 100);
  }
  
  void stopAlarm() {
    digitalWrite(SIREN_RELAY_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_STATUS_PIN, LOW);
    sirenState = false;
    Serial.println("[ALARM] Alarm stopped");
  }
  
  void blinkStatus(int times, int delayMs = 200) {
    for (int i = 0; i < times; i++) {
      digitalWrite(LED_STATUS_PIN, HIGH);
      delay(delayMs);
      digitalWrite(LED_STATUS_PIN, LOW);
      delay(delayMs);
    }
  }
};

// WiFi and HTTP Functions
void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    return;
  }
  
  Serial.print("[WiFi] Connecting to: ");
  Serial.println(WIFI_SSID);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("\n[WiFi] Connected!");
    Serial.print("[WiFi] IP: ");
    Serial.println(WiFi.localIP());
  } else {
    wifiConnected = false;
    Serial.println("\n[WiFi] Connection failed!");
  }
}

void sendHeartbeat() {
  if (!wifiConnected) return;
  
  HTTPClient http;
  http.begin(String(API_BASE_URL) + "/devices/esp32/heartbeat");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-Device-ID", DEVICE_ID);
  
  DynamicJsonDocument doc(1024);
  doc["status"] = systemStatus == STATUS_ARMED ? "armed" : "disarmed";
  doc["firmware_version"] = "1.0.0";
  doc["gsm_signal_strength"] = gsmController->getSignalStrength();
  doc["battery_level"] = 100.0;  // Implement battery monitoring if needed
  
  String payload;
  serializeJson(doc, payload);
  
  int httpCode = http.POST(payload);
  if (httpCode > 0) {
    Serial.print("[API] Heartbeat sent. Status: ");
    Serial.println(httpCode);
    
    if (httpCode == 200) {
      String response = http.getString();
      DynamicJsonDocument responseDoc(1024);
      deserializeJson(responseDoc, response);
      
      if (responseDoc.containsKey("device_config")) {
        alarmDelaySeconds = responseDoc["device_config"]["alarm_delay_seconds"] | 30;
        bool silentMode = responseDoc["device_config"]["silent_mode"] | false;
        Serial.println("[API] Config updated: delay=" + String(alarmDelaySeconds) + "s, silent=" + String(silentMode));
      }
    }
  } else {
    Serial.println("[API] Heartbeat failed");
  }
  
  http.end();
}

void reportAlert(const String& alertType, const String& sensorId, const String& message) {
  // Send via GSM SMS
  if (gsmController) {
    gsmController->sendSMS(PHONE_NUMBER_1, "ALERT: " + message);
    delay(2000);
  }
  
  // Send via HTTP if WiFi available
  if (wifiConnected) {
    HTTPClient http;
    http.begin(String(API_BASE_URL) + "/devices/esp32/alert");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Device-ID", DEVICE_ID);
    
    DynamicJsonDocument doc(512);
    doc["alert_type"] = alertType;
    doc["sensor_id"] = sensorId;
    doc["message"] = message;
    
    String payload;
    serializeJson(doc, payload);
    
    int httpCode = http.POST(payload);
    if (httpCode > 0) {
      Serial.println("[API] Alert reported. Status: " + String(httpCode));
    }
    http.end();
  }
}

void registerSensors() {
  if (!wifiConnected) return;
  
  HTTPClient http;
  http.begin(String(API_BASE_URL) + "/devices/esp32/sensors");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-Device-ID", DEVICE_ID);
  
  DynamicJsonDocument doc(2048);
  doc["sensors"][0]["sensor_id"] = "PIR1";
  doc["sensors"][0]["sensor_type"] = "pir";
  doc["sensors"][0]["name"] = "PIR Sensor 1";
  doc["sensors"][0]["location"] = "Living Room";
  
  doc["sensors"][1]["sensor_id"] = "PIR2";
  doc["sensors"][1]["sensor_type"] = "pir";
  doc["sensors"][1]["name"] = "PIR Sensor 2";
  doc["sensors"][1]["location"] = "Bedroom";
  
  doc["sensors"][2]["sensor_id"] = "MAG1";
  doc["sensors"][2]["sensor_type"] = "magnetic";
  doc["sensors"][2]["name"] = "Door Sensor";
  doc["sensors"][2]["location"] = "Front Door";
  
  doc["sensors"][3]["sensor_id"] = "MAG2";
  doc["sensors"][3]["sensor_type"] = "magnetic";
  doc["sensors"][3]["name"] = "Window Sensor";
  doc["sensors"][3]["location"] = "Window 1";
  
  doc["sensors"][4]["sensor_id"] = "SHOCK1";
  doc["sensors"][4]["sensor_type"] = "shock";
  doc["sensors"][4]["name"] = "Shock Sensor";
  doc["sensors"][4]["location"] = "Front Door";
  
  doc["sensors"][5]["sensor_id"] = "TAMPER1";
  doc["sensors"][5]["sensor_type"] = "tamper";
  doc["sensors"][5]["name"] = "Tamper Switch";
  doc["sensors"][5]["location"] = "Enclosure";
  
  String payload;
  serializeJson(doc, payload);
  
  int httpCode = http.POST(payload);
  if (httpCode > 0) {
    Serial.println("[API] Sensors registered. Status: " + String(httpCode));
  }
  http.end();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=== GSM Burglary Alarm System ===");
  Serial.println("Initializing...");
  
  // Initialize components
  alarmSystem = new AlarmSystem();
  alarmSystem->init();
  
  sensorManager = new SensorManager();
  sensorManager->init();
  
  gsmController = new GSMController();
  if (gsmController->init()) {
    Serial.println("[GSM] GSM module ready");
  } else {
    Serial.println("[GSM] Warning: GSM module initialization failed");
  }
  
  // Connect WiFi
  connectWiFi();
  
  // Register sensors with backend
  if (wifiConnected) {
    delay(2000);
    registerSensors();
  }
  
  // Status LED blink
  alarmSystem->blinkStatus(3, 100);
  
  Serial.println("[SYSTEM] Setup complete. System is DISARMED.");
}

void loop() {
  // Check tamper switch (highest priority)
  if (sensorManager->checkTamper()) {
    alarmActive = true;
    alarmTriggerTime = millis();
    systemStatus = STATUS_ALARM;
    reportAlert("tamper", "TAMPER1", "Tamper switch triggered!");
    alarmSystem->triggerAlarm(false);
    Serial.println("[ALERT] Tamper detected!");
  }
  
  // Main alarm logic (only if armed)
  if (systemStatus == STATUS_ARMED && !alarmActive) {
    // Check sensors
    bool intrusionDetected = false;
    String alertSensor = "";
    String alertMessage = "";
    
    if (sensorManager->checkPIR1()) {
      intrusionDetected = true;
      alertSensor = "PIR1";
      alertMessage = "Motion detected in Living Room";
    } else if (sensorManager->checkPIR2()) {
      intrusionDetected = true;
      alertSensor = "PIR2";
      alertMessage = "Motion detected in Bedroom";
    } else if (sensorManager->checkMagnetic1()) {
      intrusionDetected = true;
      alertSensor = "MAG1";
      alertMessage = "Front door opened";
    } else if (sensorManager->checkMagnetic2()) {
      intrusionDetected = true;
      alertSensor = "MAG2";
      alertMessage = "Window opened";
    } else if (sensorManager->checkShock()) {
      intrusionDetected = true;
      alertSensor = "SHOCK1";
      alertMessage = "Shock/vibration detected";
    }
    
    if (intrusionDetected && !alarmActive) {
      alarmTriggerTime = millis();
      Serial.println("[ALERT] Intrusion detected: " + alertMessage);
      
      // Report immediately but delay alarm sound
      reportAlert("intrusion", alertSensor, alertMessage);
      
      // Start alarm delay countdown
      alarmActive = true;
    }
  }
  
  // Trigger alarm after delay
  if (alarmActive && systemStatus == STATUS_ARMED) {
    unsigned long elapsed = millis() - alarmTriggerTime;
    if (elapsed >= (alarmDelaySeconds * 1000UL)) {
      systemStatus = STATUS_ALARM;
      alarmSystem->triggerAlarm(false);
    } else {
      // Blink LED during delay
      alarmSystem->blinkStatus(1, 500);
    }
  }
  
  // GSM operations
  if (gsmController) {
    gsmController->loop();
  }
  
  // Heartbeat to backend
  if (millis() - lastHeartbeat >= HEARTBEAT_INTERVAL) {
    lastHeartbeat = millis();
    sendHeartbeat();
    
    // Reconnect WiFi if lost
    if (!wifiConnected) {
      connectWiFi();
    }
  }
  
  // Small delay to prevent watchdog issues
  delay(10);
}

