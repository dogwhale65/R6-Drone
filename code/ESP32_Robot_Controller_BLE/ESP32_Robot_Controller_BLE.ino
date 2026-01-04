#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Adafruit_NeoPixel.h>

// BLE UUIDs - These must match in the Android app
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define THROTTLE_CHAR_UUID  "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define ROTATION_CHAR_UUID  "1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e"

// Motor pins
const int ENA = 5;
const int IN1 = 6;
const int IN2 = 7;
const int ENB = 8;
const int IN3 = 9;
const int IN4 = 10;

// RGB LED
#define RGB_PIN 48
#define NUM_PIXELS 1
Adafruit_NeoPixel pixels(NUM_PIXELS, RGB_PIN, NEO_GRB + NEO_KHZ800);

// PWM settings
const int pwmFreq = 20000;
const int pwmResolution = 8;

// BLE objects
BLEServer *pServer = NULL;
BLECharacteristic *throttleCharacteristic = NULL;
BLECharacteristic *rotationCharacteristic = NULL;
bool deviceConnected = false;

// Control values
int throttle = 0;
int rotation = 0;

// Watchdog timer for lost commands
unsigned long lastCommandTime = 0;
const unsigned long COMMAND_TIMEOUT = 50; // 500ms timeout

void setLED(int r, int g, int b) {
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
}

void updateLED() {
  int r = 0, g = 0, b = 0;
  
  // Forward/backward status
  if (throttle > 10) { 
    r = 255; g = 255; b = 255; // White - forward
  } else if (throttle < -10) { 
    r = 255; g = 0; b = 0; // Red - backward
  }
  
  // Rotation status overlays
  if (rotation < -10) { // Left turn
    if (throttle > 10) { 
      r = 0; g = 255; b = 255; // Cyan - forward + left
    } else if (throttle < -10) { 
      r = 255; g = 255; b = 0; // Yellow - backward + left
    } else { 
      r = 0; g = 255; b = 0; // Green - left only
    }
  } else if (rotation > 10) { // Right turn
    if (throttle > 10) { 
      r = 128; g = 128; b = 255; // Light blue - forward + right
    } else if (throttle < -10) { 
      r = 255; g = 0; b = 255; // Magenta - backward + right
    } else { 
      r = 0; g = 0; b = 255; // Blue - right only
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

void updateMotors() {
  // Tank drive: left = throttle - rotation, right = throttle + rotation
  int left = constrain(throttle - rotation, -100, 100);
  int right = constrain(throttle + rotation, -100, 100);
  
  setMotor(ENA, IN1, IN2, left);
  setMotor(ENB, IN3, IN4, right);
  updateLED();
}

void checkCommandTimeout() {
  // If no command received in COMMAND_TIMEOUT ms, stop the robot
  if (deviceConnected && (millis() - lastCommandTime > COMMAND_TIMEOUT)) {
    if (throttle != 0 || rotation != 0) {
      throttle = 0;
      rotation = 0;
      updateMotors();
      Serial.println("Command timeout - stopping motors");
    }
  }
}

// Server callbacks for connection status
class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Device connected");
      setLED(0, 255, 0); // Green when connected
      delay(500);
      updateLED(); // Return to status LED
    }

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Device disconnected");
      throttle = 0;
      rotation = 0;
      updateMotors();
      setLED(255, 0, 0); // Red when disconnected
      
      // Restart advertising
      BLEDevice::startAdvertising();
      Serial.println("Waiting for connection...");
    }
};

// Characteristic callbacks for receiving control values
class ControlCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        String value = pCharacteristic->getValue().c_str();
        
        if (value.length() > 0) {
            int val = value.toInt();
            val = constrain(val, -100, 100); // Safety constraint
            
            if (pCharacteristic->getUUID().toString() == THROTTLE_CHAR_UUID) {
                throttle = val;
                Serial.print("Throttle: ");
                Serial.println(throttle);
            } else if (pCharacteristic->getUUID().toString() == ROTATION_CHAR_UUID) {
                rotation = val;
                Serial.print("Rotation: ");
                Serial.println(rotation);
            }
            
            lastCommandTime = millis(); // Update watchdog timer
            updateMotors();
        }
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE Robot Control...");
  
  // Configure motor pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  ledcAttach(ENA, pwmFreq, pwmResolution);
  ledcAttach(ENB, pwmFreq, pwmResolution);

  // Initialize RGB LED
  pixels.begin();
  pixels.setBrightness(50);
  setLED(255, 0, 0); // Red on startup

  // Initialize BLE
  BLEDevice::init("Robot-ESP32");
  
  // Create BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  // Create BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create Throttle Characteristic
  throttleCharacteristic = pService->createCharacteristic(
                              THROTTLE_CHAR_UUID,
                              BLECharacteristic::PROPERTY_READ |
                              BLECharacteristic::PROPERTY_WRITE
                            );
  throttleCharacteristic->setCallbacks(new ControlCallbacks());
  throttleCharacteristic->setValue("0");

  // Create Rotation Characteristic
  rotationCharacteristic = pService->createCharacteristic(
                              ROTATION_CHAR_UUID,
                              BLECharacteristic::PROPERTY_READ |
                              BLECharacteristic::PROPERTY_WRITE
                            );
  rotationCharacteristic->setCallbacks(new ControlCallbacks());
  rotationCharacteristic->setValue("0");

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.println("BLE Robot ready!");
  Serial.println("Waiting for connection...");
  setLED(0, 0, 255); // Blue when waiting
  
  lastCommandTime = millis(); // Initialize watchdog
}

void loop() {
  checkCommandTimeout(); // Safety feature
  delay(10);
}