#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

const char* ssid = "Robot-ESP32";
const char* password = "password123";

const int ENA = 5;
const int IN1 = 6;
const int IN2 = 7;
const int ENB = 8;
const int IN3 = 9;
const int IN4 = 10;

#define RGB_PIN 48
#define NUM_PIXELS 1
Adafruit_NeoPixel pixels(NUM_PIXELS, RGB_PIN, NEO_GRB + NEO_KHZ800);

const int pwmFreq = 20000;
const int pwmResolution = 8;

WebServer server(80);

int throttle = 0;
int rotation = 0;

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

void updateMotors() {
  int left = constrain(throttle - rotation, -100, 100);
  int right = constrain(throttle + rotation, -100, 100);
  
  setMotor(ENA, IN1, IN2, left);
  setMotor(ENB, IN3, IN4, right);
  updateLED();
}

void handleControl() {
  if (server.hasArg("t")) throttle = server.arg("t").toInt();
  if (server.hasArg("r")) rotation = server.arg("r").toInt();
  
  updateMotors();
  server.send(200, "text/plain", "OK");
}

void handleRoot() {
  server.send(200, "text/html", R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
* {
  box-sizing: border-box;
}

body { 
  font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
  background: #0a0e27;
  color: #e0e6ed;
  margin: 0;
  padding: 40px 20px;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  min-height: 100vh;
}

.container {
  max-width: 800px;
  width: 100%;
}

h1 {
  font-size: 28px;
  font-weight: 600;
  text-align: center;
  margin: 0 0 50px 0;
  color: #e0e6ed;
  letter-spacing: 0.5px;
}

.controls {
  display: flex;
  gap: 80px;
  justify-content: center;
  flex-wrap: wrap;
  margin-bottom: 60px;
}

.control-group {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 20px;
}

.joystick-label {
  font-size: 12px;
  font-weight: 600;
  color: #8892b0;
  text-transform: uppercase;
  letter-spacing: 1.5px;
}

.joy { 
  width: 200px; 
  height: 200px; 
  background: #111d3c;
  border: 2px solid #293458;
  border-radius: 50%; 
  position: relative; 
  touch-action: none;
  cursor: grab;
}

.joy:active {
  cursor: grabbing;
}

.knob { 
  width: 60px; 
  height: 60px; 
  background: #4a7ba7;
  border-radius: 50%; 
  position: absolute; 
  left: 70px; 
  top: 70px;
  cursor: grab;
  transition: background 0.1s;
}

.knob:active {
  background: #5a8bc7;
}

.info-section {
  margin-top: 40px;
  padding: 24px;
  background: #111d3c;
  border: 2px solid #293458;
  border-radius: 12px;
}

.info-title {
  font-size: 12px;
  font-weight: 600;
  color: #8892b0;
  text-transform: uppercase;
  letter-spacing: 1.5px;
  margin: 0 0 16px 0;
}

.keyboard-controls {
  display: flex;
  gap: 24px;
  flex-wrap: wrap;
  margin-bottom: 20px;
}

.control-item {
  display: flex;
  align-items: center;
  gap: 8px;
}

.key {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  min-width: 32px;
  height: 32px;
  padding: 0 8px;
  background: #1a2847;
  border: 1px solid #293458;
  border-radius: 6px;
  font-weight: 600;
  font-size: 13px;
  color: #e0e6ed;
}

.control-desc {
  font-size: 13px;
  color: #a0aab8;
}

.led-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(140px, 1fr));
  gap: 12px;
  margin-top: 16px;
}

.led-item {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 8px;
  border-radius: 6px;
  background: rgba(0, 0, 0, 0.2);
}

.led-box {
  width: 16px;
  height: 16px;
  border-radius: 4px;
  flex-shrink: 0;
}

.led-label {
  font-size: 12px;
  color: #a0aab8;
}

@media (max-width: 600px) {
  body {
    padding: 20px 20px;
  }
  
  h1 {
    font-size: 24px;
    margin-bottom: 30px;
  }
  
  .controls {
    gap: 40px;
  }
  
  .joy {
    width: 160px;
    height: 160px;
  }
  
  .knob {
    width: 48px;
    height: 48px;
    left: 56px;
    top: 56px;
  }
}
</style>
</head>
<body>

<div class="container">
  <h1>Robot Control</h1>

  <div class="controls">
    <div class="control-group">
      <div class="joystick-label">Throttle</div>
      <div class="joy" id="throttle">
        <div class="knob"></div>
      </div>
    </div>

    <div class="control-group">
      <div class="joystick-label">Rotation</div>
      <div class="joy" id="rotate">
        <div class="knob"></div>
      </div>
    </div>
  </div>

  <div class="info-section">
    <div class="info-title">Keyboard Controls</div>
    <div class="keyboard-controls">
      <div class="control-item">
        <span class="key">W</span>
        <span class="key">S</span>
        <span class="control-desc">Throttle</span>
      </div>
      <div class="control-item">
        <span class="key">A</span>
        <span class="key">D</span>
        <span class="control-desc">Rotate</span>
      </div>
    </div>

    <div class="info-title" style="margin-top: 24px;">LED Status</div>
    <div class="led-grid">
      <div class="led-item">
        <div class="led-box" style="background: white;"></div>
        <span class="led-label">Forward</span>
      </div>
      <div class="led-item">
        <div class="led-box" style="background: #ff4444;"></div>
        <span class="led-label">Backward</span>
      </div>
      <div class="led-item">
        <div class="led-box" style="background: #44ff44;"></div>
        <span class="led-label">Left Turn</span>
      </div>
      <div class="led-item">
        <div class="led-box" style="background: #4444ff;"></div>
        <span class="led-label">Right Turn</span>
      </div>
      <div class="led-item">
        <div class="led-box" style="background: #00ffff;"></div>
        <span class="led-label">Forward+Left</span>
      </div>
      <div class="led-item">
        <div class="led-box" style="background: #88ddff;"></div>
        <span class="led-label">Forward+Right</span>
      </div>
      <div class="led-item">
        <div class="led-box" style="background: #ffff00;"></div>
        <span class="led-label">Back+Left</span>
      </div>
      <div class="led-item">
        <div class="led-box" style="background: #ff00ff;"></div>
        <span class="led-label">Back+Right</span>
      </div>
    </div>
  </div>
</div>

<script>
function joystick(el, callback) {
  const knob = el.querySelector('.knob');
  const rect = el.getBoundingClientRect();
  const center = rect.width / 2;

  function move(x, y) {
    const dx = x - center;
    const dy = y - center;
    const dist = Math.min(center - 30, Math.hypot(dx, dy));
    const angle = Math.atan2(dy, dx);

    const nx = Math.cos(angle) * dist;
    const ny = Math.sin(angle) * dist;

    knob.style.left = center + nx - 30 + "px";
    knob.style.top = center + ny - 30 + "px";

    callback(nx / (center - 30), -ny / (center - 30));
  }

  el.addEventListener("touchmove", e => {
    const t = e.touches[0];
    const rect2 = el.getBoundingClientRect();
    move(t.clientX - rect2.left, t.clientY - rect2.top);
    e.preventDefault();
  });

  el.addEventListener("touchend", () => {
    knob.style.left = "70px";
    knob.style.top = "70px";
    callback(0, 0);
  });
}

let throttle = 0;
let rotation = 0;
let keysPressed = {};

function send() {
  fetch(`/control?t=${Math.round(throttle*100)}&r=${Math.round(rotation*100)}`);
}

function updateFromKeys() {
  let newThrottle = 0;
  let newRotation = 0;

  if (keysPressed['w'] || keysPressed['W']) newThrottle += 1;
  if (keysPressed['s'] || keysPressed['S']) newThrottle -= 1;
  if (keysPressed['a'] || keysPressed['A']) newRotation -= 1;
  if (keysPressed['d'] || keysPressed['D']) newRotation += 1;

  if (newThrottle !== throttle || newRotation !== rotation) {
    throttle = newThrottle;
    rotation = newRotation;
    send();
  }
}

document.addEventListener('keydown', e => {
  if (['w', 's', 'a', 'd', 'W', 'S', 'A', 'D'].includes(e.key)) {
    e.preventDefault();
    keysPressed[e.key] = true;
    updateFromKeys();
  }
});

document.addEventListener('keyup', e => {
  if (['w', 's', 'a', 'd', 'W', 'S', 'A', 'D'].includes(e.key)) {
    e.preventDefault();
    delete keysPressed[e.key];
    updateFromKeys();
  }
});

joystick(document.getElementById("throttle"), (x, y) => {
  if (Object.keys(keysPressed).length === 0) {
    throttle = y;
    send();
  }
});

joystick(document.getElementById("rotate"), (x, y) => {
  if (Object.keys(keysPressed).length === 0) {
    rotation = x;
    send();
  }
});
</script>

</body>
</html>
)rawliteral");
}

void setup() {
  Serial.begin(115200);
  
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  ledcAttach(ENA, pwmFreq, pwmResolution);
  ledcAttach(ENB, pwmFreq, pwmResolution);

  pixels.begin();
  pixels.setBrightness(50);
  setLED(0, 0, 0);

  WiFi.softAP(ssid, password);
  
  Serial.println("WiFi AP Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/control", handleControl);
  server.begin();
  
  Serial.println("Server started");
}

void loop() {
  server.handleClient();
}