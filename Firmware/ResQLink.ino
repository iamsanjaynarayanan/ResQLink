/*
===============================================================================
ResQLink - Intelligent Vehicular Emergency Communication System
===============================================================================

Features:
- Automatic Accident Detection using MPU6050
- Rollover Detection using Roll/Pitch Analysis
- Manual SOS Emergency Trigger
- GPS Location Tracking using NEO-6M
- GSM SMS Alert using SIM900A
- Backend API Integration over WiFi
- Real-Time Vehicle Location Reporting
- Last Known Location Recovery

Hardware Used:
- ESP32 Dev Board
- MPU6050 Accelerometer + Gyroscope
- NEO-6M GPS Module
- SIM900A GSM Module
- Push Button (SOS Trigger)

Communication:
- I2C : MPU6050
- UART1 : SIM900A
- UART2 : NEO-6M
- WiFi : Backend Communication

Author:
Project developed as an academic prototype for
Vehicle Safety and Emergency Response Applications.

===============================================================================
*/

#include <Wire.h>
#include <HardwareSerial.h>
#include <math.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// ----------------- WiFi -----------------
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Backend API endpoint
const char* serverUrl = "https://YOUR_BACKEND_URL/api/accident";

// Unique vehicle/device identifier
String DEVICE_ID = "YOUR_DEVICE_ID";

// Emergency contact number
const char PHONE_NUMBER[] = "+91XXXXXXXXXX";

// ----------------- MPU6050 -----------------
const int MPU_ADDR = 0x68;
const float ACCEL_SCALE = 8192.0; // ±4g
const float GYRO_SCALE  = 131.0;  // ±250°/s

// Thresholds (prototype tuned)
const float G_THRESH      = 2.0f;
const float GYRO_THRESH   = 150.0f;
const float ROLL_THRESH   = 45.0f;
const unsigned long ROLL_SUSTAIN_MS = 1000;

// ----------------- Peripherals -----------------
HardwareSerial SIM900(1);    // UART1
HardwareSerial GPSserial(2); // UART2

// Pin mapping
const int SIM900_RX_PIN = 27;
const int SIM900_TX_PIN = 26;

const int GPS_RX_PIN    = 17;
const int GPS_TX_PIN    = 16;

// ----------------- Button -----------------
const int BUTTON_PIN = 15;
bool lastButtonState = HIGH;

// ----------------- System flags -----------------
bool systemArmed = true;
bool accidentDetected = false;
bool sosSent = false;

// GPS cache
String lastLatStr = "0.000000";
String lastLngStr = "0.000000";

// Print timer
unsigned long lastPrintMillis = 0;

// Rollover sustain timer
unsigned long rollExceededAt = 0;

// GPS NMEA buffer
String nmeaLine = "";

// ----------------- SIM900 Helper -----------------
void sendAT(const char *cmd, unsigned long waitMs = 500) {
  SIM900.println(cmd);
  delay(waitMs);

  while (SIM900.available()) {
    Serial.write(SIM900.read());
  }
}

void sendSMSWithText(const String &text) {
  SIM900.print("AT+CMGS=\"");
  SIM900.print(PHONE_NUMBER);
  SIM900.println("\"");
  delay(500);

  SIM900.print(text);
  delay(200);

  SIM900.write(26);  // CTRL+Z
  delay(6000);
}

void initSIM900() {
  Serial.println("Initializing SIM900A...");
  sendAT("AT", 800);
  sendAT("ATE0", 500);
  sendAT("AT+CMGF=1", 800);
  sendAT("AT+CSCS=\"GSM\"", 500);
  Serial.println("SIM900A initialized.");
}

// ----------------- MPU6050 -----------------
void initMPU() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  delay(10);
  Serial.println("MPU6050 initialized.");
}

void readMPURaw(int16_t &ax, int16_t &ay, int16_t &az, int16_t &gx, int16_t &gy, int16_t &gz) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);

  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();

  Wire.read(); Wire.read(); // skip temp

  gx = Wire.read() << 8 | Wire.read();
  gy = Wire.read() << 8 | Wire.read();
  gz = Wire.read() << 8 | Wire.read();
}

// ----------------- GPS Parsing (NO LIBRARY) -----------------
String getField(String data, int index) {
  int commaCount = 0;
  String field = "";

  for (int i = 0; i < data.length(); i++) {
    char c = data[i];

    if (c == ',') {
      commaCount++;
      continue;
    }

    if (commaCount == index) {
      field += c;
    }

    if (commaCount > index) break;
  }
  return field;
}

// Convert ddmm.mmmm to decimal degrees
float convertToDecimal(String raw, String dir) {
  if (raw.length() < 6) return 0.0;

  float val = raw.toFloat();
  int deg = (int)(val / 100);
  float mins = val - (deg * 100);

  float dec = deg + (mins / 60.0);

  if (dir == "S" || dir == "W") dec *= -1;
  return dec;
}

void updateGPSCache() {
  while (GPSserial.available()) {
    char c = GPSserial.read();

    if (c == '\n') {
      if (nmeaLine.startsWith("$GNRMC") || nmeaLine.startsWith("$GPRMC")) {

        String status = getField(nmeaLine, 2);

        if (status == "A") {
          String latRaw = getField(nmeaLine, 3);
          String latDir = getField(nmeaLine, 4);
          String lngRaw = getField(nmeaLine, 5);
          String lngDir = getField(nmeaLine, 6);

          float lat = convertToDecimal(latRaw, latDir);
          float lng = convertToDecimal(lngRaw, lngDir);

          if (lat != 0.0 && lng != 0.0) {
            lastLatStr = String(lat, 6);
            lastLngStr = String(lng, 6);
          }
        }
      }
      nmeaLine = "";
    }
    else if (c != '\r') {
      nmeaLine += c;
    }
  }
}

String buildGMapsLink() {
  return "https://maps.google.com/?q=" + lastLatStr + "," + lastLngStr;
}

// ----------------- WiFi + Backend API -----------------
void connectWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  unsigned long startAttempt = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 15000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi Connection Failed (continuing with GSM only).");
  }
}

void sendAccidentToServer(String deviceId, float lat, float lng, String type) {

  if (WiFi.status() == WL_CONNECTED) {

    WiFiClientSecure client;
    client.setInsecure();  // Required for ngrok HTTPS

    HTTPClient http;
    http.begin(client, serverUrl);
    http.addHeader("Content-Type", "application/json");

    String json = "{";
    json += "\"deviceId\":\"" + deviceId + "\",";
    json += "\"type\":\"" + type + "\",";
    json += "\"latitude\":" + String(lat, 6) + ",";
    json += "\"longitude\":" + String(lng, 6);
    json += "}";

    Serial.println("Sending JSON to backend:");
    Serial.println(json);

    int httpResponseCode = http.POST(json);

    Serial.print("HTTP Response Code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Server Response:");
      Serial.println(response);
    } else {
      Serial.print("Error sending POST: ");
      Serial.println(httpResponseCode);
    }

    http.end();

  } else {
    Serial.println("WiFi not connected, backend API skipped.");
  }
}

// ----------------- Alerts -----------------
void sendAlertSMS(const char *title) {
  String msg = String(title) + "\n";

  if (lastLatStr != "0.000000" && lastLngStr != "0.000000") {
    msg += "Location: " + lastLatStr + "," + lastLngStr + "\n";
    msg += buildGMapsLink();
  } else {
    msg += "GPS not fixed. Location unavailable.";
  }

  sendSMSWithText(msg);
  Serial.println("SMS sent successfully.");
}

// ----------------- Setup -----------------
void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  SIM900.begin(9600, SERIAL_8N1, SIM900_RX_PIN, SIM900_TX_PIN);
  GPSserial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

  delay(500);

  initMPU();
  initSIM900();

  connectWiFi();

  Serial.println();
  Serial.println("====== SYSTEM READY ======");
  Serial.println("MPU + GPS + SIM900A + WiFi running");
  Serial.println("Button press = SOS / Rearm");
  Serial.println("==========================");

  lastPrintMillis = millis();
}

// ----------------- Main loop -----------------
void loop() {

  // GPS update always
  updateGPSCache();

  // Read button
  bool currentButton = digitalRead(BUTTON_PIN);

  // SOS Trigger (only when armed)
  if (systemArmed && lastButtonState == HIGH && currentButton == LOW && !accidentDetected) {
    delay(150);

    if (digitalRead(BUTTON_PIN) == LOW) {
      Serial.println(">> SOS Button Pressed!");

      sendAlertSMS("SOS ALERT!");

      // Send to backend also (optional SOS trigger)
      sendAccidentToServer(
        DEVICE_ID,
        lastLatStr.toFloat(),
        lastLngStr.toFloat(),
        "SOS"
      );

      systemArmed = false;
      sosSent = true;
      Serial.println("System paused - waiting for rearm.");
    }
  }

  // Rearm Trigger (when paused)
  if (!systemArmed && lastButtonState == HIGH && currentButton == LOW) {
    delay(150);

    if (digitalRead(BUTTON_PIN) == LOW) {
      systemArmed = true;
      accidentDetected = false;
      sosSent = false;
      rollExceededAt = 0;

      Serial.println(">> System Rearmed. Monitoring resumed.");
    }
  }

  lastButtonState = currentButton;

  // If not armed, stop monitoring
  if (!systemArmed) {
    delay(200);
    return;
  }

  // Read MPU
  int16_t ax_raw, ay_raw, az_raw, gx_raw, gy_raw, gz_raw;
  readMPURaw(ax_raw, ay_raw, az_raw, gx_raw, gy_raw, gz_raw);

  // Convert raw to g and dps
  float ax_g = (float)ax_raw / ACCEL_SCALE;
  float ay_g = (float)ay_raw / ACCEL_SCALE;
  float az_g = (float)az_raw / ACCEL_SCALE;

  float gx_dps = (float)gx_raw / GYRO_SCALE;
  float gy_dps = (float)gy_raw / GYRO_SCALE;
  float gz_dps = (float)gz_raw / GYRO_SCALE;

  // Magnitudes
  float accelMag = sqrt(ax_g * ax_g + ay_g * ay_g + az_g * az_g);
  float deltaG = fabs(accelMag - 1.0f);
  float gyroMag = sqrt(gx_dps * gx_dps + gy_dps * gy_dps + gz_dps * gz_dps);

  // Roll + Pitch
  float roll  = atan2(ay_g, az_g) * 180.0f / PI;
  float pitch = atan2(-ax_g, sqrt(ay_g * ay_g + az_g * az_g)) * 180.0f / PI;

  // Print every 1 second
  if (millis() - lastPrintMillis >= 1000) {
    Serial.print("Accel(g): "); Serial.print(accelMag, 3);
    Serial.print(" | dG: "); Serial.print(deltaG, 3);
    Serial.print(" | Gyro: "); Serial.print(gyroMag, 1);
    Serial.print(" | Roll: "); Serial.print(roll, 1);
    Serial.print(" | Pitch: "); Serial.print(pitch, 1);
    Serial.print(" | GPS: "); Serial.print(lastLatStr);
    Serial.print(", "); Serial.println(lastLngStr);

    lastPrintMillis = millis();
  }

  // Rollover detection
  bool bigTilt = (fabs(roll) >= ROLL_THRESH) || (fabs(pitch) >= ROLL_THRESH);

  if (bigTilt) {
    if (rollExceededAt == 0) rollExceededAt = millis();
    else if (millis() - rollExceededAt >= ROLL_SUSTAIN_MS) {
      Serial.println(">> ROLLOVER detected!");
      accidentDetected = true;
    }
  } else {
    rollExceededAt = 0;
  }

  // Impact detection
  bool impact = false;

  if ((deltaG >= G_THRESH && gyroMag >= (GYRO_THRESH * 0.6f)) ||
      (deltaG >= (G_THRESH * 1.2f) && gyroMag >= (GYRO_THRESH * 0.4f))) {
    impact = true;
  }

  // Accident trigger
  if (impact || accidentDetected) {

    Serial.println(">> ACCIDENT detected! Sending SMS + Backend API...");

    // GSM SMS (as usual)
    sendAlertSMS("ACCIDENT ALERT!");

    // Backend trigger (for calling / website / tracking)
    sendAccidentToServer(
      DEVICE_ID,
      lastLatStr.toFloat(),
      lastLngStr.toFloat(),
      "ACCIDENT"
    );

    systemArmed = false;
    Serial.println("System paused - waiting for rearm.");
  }

  delay(50);
}