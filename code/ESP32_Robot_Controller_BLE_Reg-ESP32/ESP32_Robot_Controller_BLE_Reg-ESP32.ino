#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// BLE UUIDs
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define THROTTLE_CHAR_UUID  "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define ROTATION_CHAR_UUID  "1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e"
#define DEBUG_CHAR_UUID     "8d53dc1d-1db7-4cd3-868b-8a527460aa84"
#define COMMAND_CHAR_UUID   "7c9e6a8f-3b2d-4e1f-9a5c-8d4b7e3f2a1c"

// Motor pins
const int ENA = 25;
const int IN1 = 26;
const int IN2 = 27;
const int ENB = 32;
const int IN3 = 33;
const int IN4 = 14;

// RGB LED
#define RGB_PIN 13
#define NUM_PIXELS 1
Adafruit_NeoPixel pixels(NUM_PIXELS, RGB_PIN, NEO_GRB + NEO_KHZ800);

// PWM settings
const int pwmFreq = 20000;
const int pwmResolution = 8;

// BLE objects
BLEServer *pServer = NULL;
BLECharacteristic *throttleCharacteristic = NULL;
BLECharacteristic *rotationCharacteristic = NULL;
BLECharacteristic *debugCharacteristic = NULL;
BLECharacteristic *commandCharacteristic = NULL;
bool deviceConnected = false;

// Control values
int throttle = 0;
int rotation = 0;

// IMU
Adafruit_MPU6050 mpu;
float currentHeading = 0;
float targetHeading = 0;
unsigned long lastIMUUpdate = 0;

// Gravity reference vector (calibrated at startup)
float gravityX = 0;
float gravityY = 0;
float gravityZ = 0;

// Gyro bias (drift compensation)
float gyroBiasX = 0;
float gyroBiasY = 0;
float gyroBiasZ = 0;

// PID values
float Kp = 15.0;
float Ki = 0.5;
float Kd = 2.0;

float integralError = 0;
float lastError = 0;

// Watchdog timer
unsigned long lastCommandTime = 0;
const unsigned long COMMAND_TIMEOUT = 500;

void setLED(int r, int g, int b) {
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
}

void updateLED() {
  int r = 0, g = 0, b = 0;
  
  if (throttle > 10) { 
    r = 255; g = 255; b = 255;
  } else if (throttle < -10) { 
    r = 255; g = 0; b = 0;
  }
  
  if (rotation < -10) {
    if (throttle > 10) { 
      r = 0; g = 255; b = 255;
    } else if (throttle < -10) { 
      r = 255; g = 255; b = 0;
    } else { 
      r = 0; g = 255; b = 0;
    }
  } else if (rotation > 10) {
    if (throttle > 10) { 
      r = 128; g = 128; b = 255;
    } else if (throttle < -10) { 
      r = 255; g = 0; b = 255;
    } else { 
      r = 0; g = 0; b = 255;
    }
  }
  
  setLED(r, g, b);
}

void setMotor(int pwmPin, int inA, int inB, int speed) {
  speed = constrain(speed, -100, 100);
  
  if (speed > 0) {
    digitalWrite(inA, HIGH);
    digitalWrite(inB, LOW);
  } else if (speed < 0) {
    digitalWrite(inA, LOW);
    digitalWrite(inB, HIGH);
  } else {
    digitalWrite(inA, LOW);
    digitalWrite(inB, LOW);
  }
  
  int pwm = map(abs(speed), 0, 100, 0, 255);
  ledcWrite(pwmPin, pwm);
}

void sendDebugMessage(const char* message) {
  if (deviceConnected && debugCharacteristic != NULL) {
    debugCharacteristic->setValue(message);
    debugCharacteristic->notify();
  }
  Serial.println(message);
}

void calibrateGyroBias() {
  sendDebugMessage("CAL_GYRO_BIAS_STARTING");
  Serial.println("===========================================");
  Serial.println("GYRO BIAS CALIBRATION - DO NOT MOVE ROBOT!");
  Serial.println("===========================================");
  
  // Flash LED as countdown
  for (int i = 0; i < 3; i++) {
    setLED(255, 0, 0);
    delay(300);
    setLED(0, 0, 0);
    delay(300);
  }
  
  // Discard first 50 samples to let things settle
  for (int i = 0; i < 50; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    delay(10);
  }
  
  gyroBiasX = 0;
  gyroBiasY = 0;
  gyroBiasZ = 0;
  
  // Take 200 samples over 2 seconds (more samples = more accurate)
  setLED(255, 255, 0); // Yellow during measurement
  for (int i = 0; i < 200; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    gyroBiasX += g.gyro.x;
    gyroBiasY += g.gyro.y;
    gyroBiasZ += g.gyro.z;
    delay(10);
  }
  
  gyroBiasX /= 200.0;
  gyroBiasY /= 200.0;
  gyroBiasZ /= 200.0;
  
  // Check if bias is reasonable
  float totalBias = sqrt(gyroBiasX*gyroBiasX + gyroBiasY*gyroBiasY + gyroBiasZ*gyroBiasZ);
  
  char msg[150];
  snprintf(msg, sizeof(msg), "GYRO_BIAS|X:%.5f|Y:%.5f|Z:%.5f|MAG:%.5f", 
           gyroBiasX, gyroBiasY, gyroBiasZ, totalBias);
  sendDebugMessage(msg);
  
  Serial.printf("Gyro bias: X=%.5f, Y=%.5f, Z=%.5f rad/s\n", 
                gyroBiasX, gyroBiasY, gyroBiasZ);
  Serial.printf("Total bias magnitude: %.5f rad/s (%.2f deg/s)\n", 
                totalBias, totalBias * 180.0 / PI);
  
  // Warn if bias seems too high
  if (totalBias > 0.05) {
    Serial.println("WARNING: Gyro bias unusually high! Robot may have moved during calibration.");
    sendDebugMessage("WARN:HIGH_BIAS");
    setLED(255, 0, 0); // Red warning
    delay(1000);
  } else {
    Serial.println("Gyro bias calibration successful");
    setLED(0, 255, 0); // Green success
    delay(500);
  }
}
void calibrateIMU() {
  sendDebugMessage("CALIBRATING...");
  Serial.println("");
  Serial.println("======================================");
  Serial.println("FULL IMU CALIBRATION STARTING");
  Serial.println("KEEP ROBOT COMPLETELY STILL!");
  Serial.println("======================================");
  
  // Stop motors during calibration
  int savedThrottle = throttle;
  int savedRotation = rotation;
  throttle = 0;
  rotation = 0;
  setMotor(ENA, IN1, IN2, 0);
  setMotor(ENB, IN3, IN4, 0);
  
  // Flash LED to indicate calibration starting - 3 second countdown
  for (int i = 3; i > 0; i--) {
    Serial.printf("Starting in %d...\n", i);
    setLED(255, 255, 0);
    delay(500);
    setLED(0, 0, 0);
    delay(500);
  }
  
  // Calibrate gravity reference
  Serial.println("Measuring gravity vector...");
  gravityX = 0;
  gravityY = 0;
  gravityZ = 0;
  
  setLED(0, 255, 255); // Cyan during gravity cal
  for (int i = 0; i < 100; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    gravityX += a.acceleration.x;
    gravityY += a.acceleration.y;
    gravityZ += a.acceleration.z;
    delay(10);
  }
  
  gravityX /= 100.0;
  gravityY /= 100.0;
  gravityZ /= 100.0;
  
  float gravMag = sqrt(gravityX*gravityX + gravityY*gravityY + gravityZ*gravityZ);
  
  char calibMsg[120];
  snprintf(calibMsg, sizeof(calibMsg), 
           "GRAV_CAL|X:%.2f|Y:%.2f|Z:%.2f|M:%.2f", 
           gravityX, gravityY, gravityZ, gravMag);
  sendDebugMessage(calibMsg);
  
  Serial.printf("Gravity: X=%.2f, Y=%.2f, Z=%.2f (mag=%.2f m/s²)\n",
                gravityX, gravityY, gravityZ, gravMag);
  
  // Check if gravity measurement is reasonable
  if (gravMag < 8.5 || gravMag > 11.0) {
    Serial.println("WARNING: Gravity measurement unusual! Expected ~9.8 m/s²");
    sendDebugMessage("WARN:BAD_GRAVITY");
  }
  
  delay(500);
  
  // Calibrate gyro bias
  calibrateGyroBias();
  
  // Reset heading
  currentHeading = 0;
  targetHeading = 0;
  integralError = 0;
  lastError = 0;
  lastIMUUpdate = millis();
  
  // Restore previous values
  throttle = savedThrottle;
  rotation = savedRotation;
  
  sendDebugMessage("CAL_COMPLETE");
  Serial.println("======================================");
  Serial.println("Calibration complete!");
  Serial.println("======================================");
  Serial.println("");
}
// Project gyro rotation onto gravity vector to get vertical rotation component
float getVerticalRotation(float gx, float gy, float gz, float ax, float ay, float az) {
  // Normalize gravity vector
  float gravMag = sqrt(ax*ax + ay*ay + az*az);
  if (gravMag < 0.1) return 0; // Sanity check
  
  ax /= gravMag;
  ay /= gravMag;
  az /= gravMag;
  
  // Dot product of gyro with normalized gravity gives rotation around vertical axis
  float verticalRotation = gx*ax + gy*ay + gz*az;
  
  return verticalRotation;
}

void updateIMU() {
  unsigned long currentTime = millis();
  float dt = (currentTime - lastIMUUpdate) / 1000.0;
  
  if (dt > 0.1) dt = 0.01;
  
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  // Subtract gyro bias to compensate for drift
  float correctedGX = g.gyro.x - gyroBiasX;
  float correctedGY = g.gyro.y - gyroBiasY;
  float correctedGZ = g.gyro.z - gyroBiasZ;
  
  // Get vertical rotation component using bias-corrected gyro
  float verticalGyro = getVerticalRotation(
    correctedGX, correctedGY, correctedGZ,
    a.acceleration.x, a.acceleration.y, a.acceleration.z
  );
  
  // Integrate to get heading
  currentHeading += verticalGyro * dt * (180.0 / PI);
  
  // Keep heading in reasonable range
  if (currentHeading > 360) currentHeading -= 360;
  if (currentHeading < -360) currentHeading += 360;
  
  lastIMUUpdate = currentTime;
}

void updateMotors() {
  updateIMU();
  
  int left, right;
  
  // Check if robot is stopped - with debug output
  bool isStopped = (abs(throttle) < 10 && abs(rotation) < 10);
  
  if (isStopped) {
    // Robot is stopped - reset target to prevent drift
    static unsigned long lastResetMsg = 0;
    if (millis() - lastResetMsg > 2000) {  // Message every 2 seconds
      char msg[80];
      snprintf(msg, sizeof(msg), "STOPPED|T:%d|R:%d|Resetting", throttle, rotation);
      sendDebugMessage(msg);
      lastResetMsg = millis();
    }
    
    targetHeading = currentHeading;
    integralError = 0;
    lastError = 0;
  }
  
  // Apply drift correction when going mostly straight
  if (abs(rotation) < 15 && abs(throttle) > 15) {
    float error = targetHeading - currentHeading;
    
    while (error > 180) error -= 360;
    while (error < -180) error += 360;
    
    integralError += error * ((millis() - lastIMUUpdate) / 1000.0);
    integralError = constrain(integralError, -50, 50);
    
    float derivative = error - lastError;
    
    float correction = (Kp * error) + (Ki * integralError) + (Kd * derivative);
    correction = constrain(correction, -30, 30);
    
    lastError = error;
    
    int correctedRotation = rotation + (int)correction;
    
    left = constrain(throttle - correctedRotation, -100, 100);
    right = constrain(throttle + correctedRotation, -100, 100);
    
    // Send debug data every 500ms
    static unsigned long lastDebugSend = 0;
    if (millis() - lastDebugSend >= 500) {
      char debugMsg[150];
      snprintf(debugMsg, sizeof(debugMsg), 
               "H:%.1f|T:%.1f|E:%.1f|C:%.1f|L:%d|R:%d", 
               currentHeading, targetHeading, error, correction, left, right);
      sendDebugMessage(debugMsg);
      lastDebugSend = millis();
    }
  } else {
    // Not applying correction - use normal tank drive
    left = constrain(throttle - rotation, -100, 100);
    right = constrain(throttle + rotation, -100, 100);
  }
  
  setMotor(ENA, IN1, IN2, left);
  setMotor(ENB, IN3, IN4, right);
  updateLED();
}
void checkCommandTimeout() {
  if (deviceConnected && (millis() - lastCommandTime > COMMAND_TIMEOUT)) {
    if (throttle != 0 || rotation != 0) {
      throttle = 0;
      rotation = 0;
      updateMotors();
      sendDebugMessage("CMD_TIMEOUT");
    }
  }
}

class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Device connected");
      sendDebugMessage("CONNECTED");
      setLED(0, 255, 0);
      delay(500);
      updateLED();
    }

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Device disconnected");
      throttle = 0;
      rotation = 0;
      updateMotors();
      setLED(255, 0, 0);
      BLEDevice::startAdvertising();
      Serial.println("Waiting for connection...");
    }
};

class ControlCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        String value = pCharacteristic->getValue().c_str();
        
        if (value.length() > 0) {
            int val = value.toInt();
            val = constrain(val, -100, 100);
            
            if (pCharacteristic->getUUID().toString() == THROTTLE_CHAR_UUID) {
                throttle = val;
            } else if (pCharacteristic->getUUID().toString() == ROTATION_CHAR_UUID) {
                rotation = val;
            }
            
            lastCommandTime = millis();
            updateMotors();
        }
    }
};

class CommandCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        String value = pCharacteristic->getValue().c_str();
        
        if (value.length() > 0) {
            Serial.print("Command received: ");
            Serial.println(value);
            
            if (value == "CALIBRATE") {
                calibrateIMU();
            } else if (value == "RESET_HEADING") {
                currentHeading = 0;
                targetHeading = 0;
                integralError = 0;
                lastError = 0;
                sendDebugMessage("HEADING_RESET");
                Serial.println("Heading reset to 0");
            } else if (value == "RECAL_GYRO") {
                // Just recalibrate gyro bias without full calibration
                calibrateGyroBias();
            }
        }
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE Robot Control with IMU Drift Compensation...");
  
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  ledcAttach(ENA, pwmFreq, pwmResolution);
  ledcAttach(ENB, pwmFreq, pwmResolution);

  pixels.begin();
  pixels.setBrightness(50);
  setLED(255, 165, 0);

  Wire.begin(21, 22);
  
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050!");
    setLED(255, 0, 0);
    while (1) {
      delay(10);
    }
  }
  
  Serial.println("MPU6050 Found!");
  
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
  delay(100);
  
  // Initial calibration
  Serial.println("Initial calibration...");
  Serial.println("*** KEEP ROBOT STILL ON FLAT SURFACE ***");
  
  // Calibrate gravity reference
  gravityX = 0;
  gravityY = 0;
  gravityZ = 0;
  
  for (int i = 0; i < 50; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    gravityX += a.acceleration.x;
    gravityY += a.acceleration.y;
    gravityZ += a.acceleration.z;
    delay(20);
  }
  
  gravityX /= 50.0;
  gravityY /= 50.0;
  gravityZ /= 50.0;
  
  float gravMag = sqrt(gravityX*gravityX + gravityY*gravityY + gravityZ*gravityZ);
  
  Serial.printf("Gravity calibrated: X=%.2f Y=%.2f Z=%.2f (mag=%.2f m/s²)\n", 
                gravityX, gravityY, gravityZ, gravMag);
  
  // Calibrate gyro bias
  calibrateGyroBias();
  
  currentHeading = 0;
  targetHeading = 0;
  lastIMUUpdate = millis();
  
  Serial.println("IMU ready with drift compensation!");

  // Initialize BLE
  BLEDevice::init("Robot-ESP32");
  
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Throttle Characteristic
  throttleCharacteristic = pService->createCharacteristic(
                              THROTTLE_CHAR_UUID,
                              BLECharacteristic::PROPERTY_READ |
                              BLECharacteristic::PROPERTY_WRITE
                            );
  throttleCharacteristic->setCallbacks(new ControlCallbacks());
  throttleCharacteristic->setValue("0");

  // Rotation Characteristic
  rotationCharacteristic = pService->createCharacteristic(
                              ROTATION_CHAR_UUID,
                              BLECharacteristic::PROPERTY_READ |
                              BLECharacteristic::PROPERTY_WRITE
                            );
  rotationCharacteristic->setCallbacks(new ControlCallbacks());
  rotationCharacteristic->setValue("0");

  // Debug Characteristic (READ + NOTIFY)
  debugCharacteristic = pService->createCharacteristic(
                           DEBUG_CHAR_UUID,
                           BLECharacteristic::PROPERTY_READ |
                           BLECharacteristic::PROPERTY_NOTIFY
                         );
  debugCharacteristic->addDescriptor(new BLE2902());
  debugCharacteristic->setValue("READY");

  // Command Characteristic (WRITE)
  commandCharacteristic = pService->createCharacteristic(
                             COMMAND_CHAR_UUID,
                             BLECharacteristic::PROPERTY_WRITE
                           );
  commandCharacteristic->setCallbacks(new CommandCallbacks());
  commandCharacteristic->setValue("NONE");

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.println("BLE Robot ready!");
  Serial.println("Waiting for connection...");
  setLED(0, 0, 255);
  
  lastCommandTime = millis();
}

void loop() {
  checkCommandTimeout();
  delay(10);
}