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
  int left = constrain(throttle + rotation, -100, 100);
  int right = constrain(throttle - rotation, -100, 100);
  
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
body { 
  font-family: sans-serif; 
  text-align: center; 
  background: #111; 
  color: #fff; 
  margin: 0;
  padding: 20px;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  min-height: 100vh;
}
.controls {
  display: flex;
  gap: 40px;
  align-items: center;
  flex-wrap: wrap;
  justify-content: center;
}
.joy { 
  width: 200px; 
  height: 200px; 
  background: #333; 
  border-radius: 50%; 
  display: inline-block; 
  position: relative; 
  touch-action: none; 
}
.knob { 
  width: 60px; 
  height: 60px; 
  background: #888; 
  border-radius: 50%; 
  position: absolute; 
  left: 70px; 
  top: 70px; 
}
.label {
  margin-top: 10px;
  font-size: 14px;
  color: #aaa;
}
.keyboard-hint {
  margin-top: 20px;
  padding: 10px 20px;
  background: #222;
  border-radius: 8px;
  font-size: 12px;
  color: #888;
}
.led-legend {
  margin-top: 20px;
  padding: 15px;
  background: #222;
  border-radius: 8px;
  font-size: 11px;
  color: #aaa;
  max-width: 500px;
}
.led-legend h3 {
  margin: 0 0 10px 0;
  font-size: 13px;
  color: #fff;
}
.color-item {
  display: inline-block;
  margin: 3px 8px;
}
.color-box {
  display: inline-block;
  width: 12px;
  height: 12px;
  border-radius: 2px;
  margin-right: 4px;
  vertical-align: middle;
}
</style>
</head>
<body>

<h2>Robot Control</h2>

<div class="controls">
  <div>
    <div class="joy" id="throttle">
      <div class="knob"></div>
    </div>
    <div class="label">Throttle</div>
  </div>

  <div>
    <div class="joy" id="rotate">
      <div class="knob"></div>
    </div>
    <div class="label">Rotation</div>
  </div>
</div>

<div class="keyboard-hint">
  Desktop: Use W/S for throttle, A/D for rotation
</div>

<div class="led-legend">
  <h3>LED Status Colors</h3>
  <div class="color-item"><span class="color-box" style="background: white;"></span>Forward</div>
  <div class="color-item"><span class="color-box" style="background: red;"></span>Backward</div>
  <div class="color-item"><span class="color-box" style="background: lime;"></span>Left</div>
  <div class="color-item"><span class="color-box" style="background: blue;"></span>Right</div>
  <br>
  <div class="color-item"><span class="color-box" style="background: cyan;"></span>Forward+Left</div>
  <div class="color-item"><span class="color-box" style="background: lightblue;"></span>Forward+Right</div>
  <div class="color-item"><span class="color-box" style="background: yellow;"></span>Back+Left</div>
  <div class="color-item"><span class="color-box" style="background: magenta;"></span>Back+Right</div>
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