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
<html lang="en">
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
:root {
  --bg: #0e0f12;
  --panel: #151821;
  --border: #2a2f3a;
  --text: #e6e6e6;
  --muted: #9aa0aa;
  --accent: #4da3ff;
}

* {
  box-sizing: border-box;
}

body {
  margin: 0;
  font-family: system-ui, -apple-system, Segoe UI, Roboto, sans-serif;
  background: var(--bg);
  color: var(--text);
  min-height: 100vh;
  display: flex;
  align-items: center;
  justify-content: center;
}

.container {
  width: 100%;
  max-width: 900px;
  padding: 24px;
}

header {
  margin-bottom: 32px;
  text-align: center;
}

header h1 {
  margin: 0;
  font-size: 26px;
  font-weight: 500;
}

header p {
  margin-top: 6px;
  font-size: 14px;
  color: var(--muted);
}

.controls {
  display: flex;
  gap: 48px;
  justify-content: center;
  flex-wrap: wrap;
}

.control-group {
  background: var(--panel);
  border: 1px solid var(--border);
  border-radius: 12px;
  padding: 20px 24px 24px;
  text-align: center;
}

.control-group h2 {
  margin: 0 0 14px;
  font-size: 14px;
  font-weight: 500;
  color: var(--muted);
  letter-spacing: 0.5px;
}

.joy {
  width: 200px;
  height: 200px;
  border-radius: 50%;
  background: #0b0d13;
  border: 2px solid var(--border);
  position: relative;
  touch-action: none;
}

.knob {
  width: 64px;
  height: 64px;
  border-radius: 50%;
  background: var(--accent);
  position: absolute;
  left: 68px;
  top: 68px;
  box-shadow: 0 6px 16px rgba(0,0,0,0.5);
}

.info {
  margin-top: 32px;
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 20px;
}

.card {
  background: var(--panel);
  border: 1px solid var(--border);
  border-radius: 12px;
  padding: 16px 18px;
  font-size: 14px;
}

.card h3 {
  margin: 0 0 10px;
  font-size: 14px;
  font-weight: 500;
  color: var(--muted);
}

.key-list span {
  display: inline-block;
  padding: 2px 6px;
  margin: 0 4px;
  border: 1px solid var(--border);
  border-radius: 4px;
  font-size: 12px;
  background: #0b0d13;
}

.color-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(140px, 1fr));
  gap: 8px;
}

.color-item {
  display: flex;
  align-items: center;
  gap: 8px;
}

.color-box {
  width: 12px;
  height: 12px;
  border-radius: 3px;
}

@media (max-width: 600px) {
  .controls {
    gap: 24px;
  }
  .joy {
    width: 170px;
    height: 170px;
  }
  .knob {
    width: 54px;
    height: 54px;
    left: 58px;
    top: 58px;
  }
  .info {
    grid-template-columns: 1fr;
  }
}
</style>
</head>
<body>

<div class="container">

  <header>
    <h1>Robot Control</h1>
    <p>Web-based manual drive interface</p>
  </header>

  <div class="controls">
    <div class="control-group">
      <h2>Throttle</h2>
      <div class="joy" id="throttle">
        <div class="knob"></div>
      </div>
    </div>

    <div class="control-group">
      <h2>Rotation</h2>
      <div class="joy" id="rotate">
        <div class="knob"></div>
      </div>
    </div>
  </div>

  <div class="info">
    <div class="card">
      <h3>Keyboard control</h3>
      <div class="key-list">
        <span>W</span><span>S</span> throttle
        &nbsp;&nbsp;
        <span>A</span><span>D</span> rotation
      </div>
    </div>

    <div class="card">
      <h3>LED status</h3>
      <div class="color-grid">
        <div class="color-item"><span class="color-box" style="background:white"></span>Forward</div>
        <div class="color-item"><span class="color-box" style="background:red"></span>Backward</div>
        <div class="color-item"><span class="color-box" style="background:lime"></span>Left</div>
        <div class="color-item"><span class="color-box" style="background:blue"></span>Right</div>
        <div class="color-item"><span class="color-box" style="background:cyan"></span>Forward + Left</div>
        <div class="color-item"><span class="color-box" style="background:lightblue"></span>Forward + Right</div>
        <div class="color-item"><span class="color-box" style="background:yellow"></span>Back + Left</div>
        <div class="color-item"><span class="color-box" style="background:magenta"></span>Back + Right</div>
      </div>
    </div>
  </div>

</div>

<script>
function joystick(el, callback) {
  const knob = el.querySelector('.knob');
  const center = el.offsetWidth / 2;
  const limit = center - knob.offsetWidth / 2;

  function move(x, y) {
    const dx = x - center;
    const dy = y - center;
    const dist = Math.min(limit, Math.hypot(dx, dy));
    const angle = Math.atan2(dy, dx);
    const nx = Math.cos(angle) * dist;
    const ny = Math.sin(angle) * dist;
    knob.style.left = center + nx - knob.offsetWidth / 2 + "px";
    knob.style.top = center + ny - knob.offsetHeight / 2 + "px";
    callback(nx / limit, -ny / limit);
  }

  el.addEventListener("touchmove", e => {
    const t = e.touches[0];
    const r = el.getBoundingClientRect();
    move(t.clientX - r.left, t.clientY - r.top);
    e.preventDefault();
  });

  el.addEventListener("touchend", () => {
    knob.style.left = (center - knob.offsetWidth / 2) + "px";
    knob.style.top = (center - knob.offsetHeight / 2) + "px";
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
  let t = 0, r = 0;
  if (keysPressed.w) t++;
  if (keysPressed.s) t--;
  if (keysPressed.a) r--;
  if (keysPressed.d) r++;
  throttle = t;
  rotation = r;
  send();
}

document.addEventListener('keydown', e => {
  const k = e.key.toLowerCase();
  if ("wasd".includes(k)) {
    e.preventDefault();
    keysPressed[k] = true;
    updateFromKeys();
  }
});

document.addEventListener('keyup', e => {
  const k = e.key.toLowerCase();
  if ("wasd".includes(k)) {
    e.preventDefault();
    delete keysPressed[k];
    updateFromKeys();
  }
});

joystick(document.getElementById("throttle"), (x, y) => {
  if (!Object.keys(keysPressed).length) {
    throttle = y;
    send();
  }
});

joystick(document.getElementById("rotate"), (x) => {
  if (!Object.keys(keysPressed).length) {
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