// Librerias necesarias (Arduino Library Manager): Adafruit NeoPixel, Adafruit VL53L1X
// (el servo se controla por LEDC directo, ya no hace falta ESP32Servo)

#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_VL53L1X.h>

#define LDR1_PIN D0
#define LDR2_PIN D1
#define NEOPIXEL_PIN D7
#define MOTOR_PIN1 D10
#define MOTOR_PIN2 D9
#define BUZZER_PIN D6
#define I2C_SDA_PIN D4
#define I2C_SCL_PIN D5
#define SERVO_PIN D8

#define NUM_PIXELS 1
#define MOTOR_STRENGTH 200

const char *AP_SSID = "CABLEBOTTT";
const char *AP_PASSWORD = "cablebottt";

WebServer server(80);
Adafruit_NeoPixel pixel(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_VL53L1X distanceSensor;

// PWM por canales LEDC EXPLICITOS para que motor y servo nunca compartan canal/timer.
// En el ESP32-C3 los canales se agrupan por timer de dos en dos (ch0-1 -> timer0,
// ch2-3 -> timer1, ch4-5 -> timer2). Ponemos los motores en el timer0 y el servo en el
// timer2, bien separados, y dejamos el timer1 libre para el tone() del zumbador.
#define MOTOR1_LEDC_CH 0
#define MOTOR2_LEDC_CH 1
#define SERVO_LEDC_CH 4
#define MOTOR_PWM_FREQ 20000  // 20 kHz: por encima del rango audible, sin pitido en el motor
#define MOTOR_PWM_RES 8       // duty 0-255

// Parametros del servo
#define SERVO_FREQ 50
#define SERVO_RES_BITS 14
#define SERVO_MIN_US 500
#define SERVO_MAX_US 2400
#define SERVO_PERIOD_US 20000

bool distanceSensorOK = false;
unsigned long lastDistanceAttempt = 0;
const unsigned long DISTANCE_RETRY_INTERVAL_MS = 5000;
int16_t lastDistanceMM = -1;

int motorDir = 0;  // -1 izquierda, 0 parado, 1 derecha
unsigned long lastMotorCommandTime = 0;
const unsigned long MOTOR_TIMEOUT_MS = 900;  // para el motor si no llega otro comando/latido a tiempo

bool distanceSensorEnabled = false;  // el sensor no viene conectado de serie: se activa desde la webapp

bool pixelOn = false;
uint8_t pixelR = 127, pixelG = 119, pixelB = 221;

int servoAngle = 90;

struct Note {
  int freq;
  int dur;
};
const Note marioTune[] = {
  { 660, 150 }, { 660, 150 }, { 0, 150 }, { 660, 150 }, { 0, 150 }, { 523, 150 }, { 660, 150 }, { 0, 150 }, { 784, 150 }, { 0, 450 }, { 392, 150 }
};
const int MARIO_TUNE_LENGTH = sizeof(marioTune) / sizeof(Note);
int marioIndex = -1;  // -1 = no esta sonando
unsigned long marioNoteStart = 0;

const char INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>cablebottt</title>
<style>
  body { font-family: -apple-system, sans-serif; background:#1c1c1e; color:#f2f2f2; margin:0; padding:16px; }
  h1 { font-size:18px; text-align:center; font-weight:500; margin:0 0 16px; }
  .card { background:#2c2c2e; border-radius:12px; padding:16px; margin-bottom:12px; }
  .label { font-size:13px; color:#9a9a9d; margin:0 0 10px; font-weight:500; }
  .row { display:grid; grid-template-columns:1fr 1fr; gap:10px; }
  button { font-size:14px; color:#f2f2f2; background:#3a3a3c; border:1px solid #48484a; border-radius:8px; padding:0; }
  .motor-btn { height:64px; touch-action:none; user-select:none; -webkit-user-select:none; -webkit-touch-callout:none; -webkit-tap-highlight-color:transparent; }
  .motor-btn.active { background:#0a84ff; }
  .full-btn { width:100%; height:44px; }
  .hint { font-size:11px; color:#77777a; text-align:center; margin:8px 0 0; }
  .pixel-header { display:flex; align-items:center; justify-content:space-between; margin-bottom:10px; }
  .pixel-header button { height:32px; padding:0 14px; }
  .color-row { display:flex; align-items:center; gap:10px; }
  input[type=color] { width:44px; height:36px; padding:2px; border:none; border-radius:8px; background:none; }
  .preview { flex:1; height:36px; border-radius:8px; border:1px solid #48484a; }
  .servo-header { display:flex; align-items:center; justify-content:space-between; margin-bottom:6px; }
  input[type=range] { width:100%; }
  .status-card { display:flex; align-items:center; justify-content:space-between; }
  .status-value { font-size:22px; font-weight:500; margin:4px 0 0; }
</style>
</head>
<body>
<h1>cablebottt</h1>

<div class="card">
  <p class="label">Motor</p>
  <div class="row">
    <button class="motor-btn" id="btnL">Izquierda</button>
    <button class="motor-btn" id="btnR">Derecha</button>
  </div>
  <p class="hint">Manten pulsado para mover, suelta para parar</p>
</div>

<div class="card">
  <div class="pixel-header">
    <p class="label" style="margin:0">Neopixel</p>
    <button id="pixelToggle">Encender</button>
  </div>
  <div class="color-row">
    <input type="color" id="pixelColor" value="#7f77dd">
    <div class="preview" id="pixelPreview" style="background:#7f77dd"></div>
  </div>
</div>

<div class="card">
  <p class="label">Zumbador</p>
  <button class="full-btn" id="btnBuzz">Reproducir melodia Mario</button>
</div>

<div class="card">
  <div class="servo-header">
    <p class="label" style="margin:0">Servo</p>
    <span id="servoOut">90 grados</span>
  </div>
  <input type="range" min="0" max="180" step="1" value="90" id="servoSlider">
</div>

<div class="card status-card">
  <div>
    <p class="label" style="margin:0">Distancia</p>
    <p class="status-value" id="distVal">--</p>
  </div>
  <button id="distToggle" data-on="0">Sensor: OFF</button>
</div>

<script>
let motorAbort = null;
function sendMotorCmd(dir) {
  if (motorAbort) motorAbort.abort();
  motorAbort = new AbortController();
  fetch('/motor?dir=' + dir, { signal: motorAbort.signal }).catch(() => {});
}
let motorInterval = null;
function startMotor(dir) {
  sendMotorCmd(dir);
  if (motorInterval) clearInterval(motorInterval);
  motorInterval = setInterval(() => sendMotorCmd(dir), 300);
}
function stopMotor() {
  if (motorInterval) { clearInterval(motorInterval); motorInterval = null; }
  sendMotorCmd('stop');
}

function bindMotorButton(el, dir) {
  let active = false;
  const start = (e) => {
    if (e.cancelable) e.preventDefault();  // evita scroll / click sintetico con retardo en movil
    if (active) return;
    active = true;
    el.classList.add('active');
    startMotor(dir);
  };
  const end = () => {
    if (!active) return;
    active = false;
    el.classList.remove('active');
    stopMotor();
  };
  // El "pulsar" se escucha en el boton; el "soltar" se escucha en toda la ventana,
  // asi se captura aunque el dedo se salga del boton antes de levantarlo.
  el.addEventListener('touchstart', start, { passive: false });
  el.addEventListener('mousedown', start);
  window.addEventListener('touchend', end);
  window.addEventListener('touchcancel', end);
  window.addEventListener('mouseup', end);
  window.addEventListener('blur', end);
}
bindMotorButton(document.getElementById('btnL'), 'left');
bindMotorButton(document.getElementById('btnR'), 'right');
document.addEventListener('visibilitychange', () => { if (document.hidden) stopMotor(); });

let pixelOn = false;
let lastPixelInteract = 0;
const pixelToggle = document.getElementById('pixelToggle');
const pixelColor = document.getElementById('pixelColor');
const pixelPreview = document.getElementById('pixelPreview');
function updatePixelLabel() {
  pixelToggle.textContent = pixelOn ? 'Apagar' : 'Encender';
}
function sendPixelState() {
  lastPixelInteract = Date.now();
  fetch('/pixel?on=' + (pixelOn ? '1' : '0') + '&color=' + encodeURIComponent(pixelColor.value.substring(1))).catch(() => {});
}
pixelToggle.addEventListener('click', () => {
  pixelOn = !pixelOn;
  updatePixelLabel();
  sendPixelState();
});
pixelColor.addEventListener('input', () => {
  pixelPreview.style.background = pixelColor.value;
  sendPixelState();
});

document.getElementById('btnBuzz').addEventListener('click', () => fetch('/buzzer'));

const servoSlider = document.getElementById('servoSlider');
const servoOut = document.getElementById('servoOut');
let servoSendTimer = null;
servoSlider.addEventListener('input', () => {
  servoOut.textContent = servoSlider.value + ' grados';
  if (servoSendTimer) clearTimeout(servoSendTimer);
  servoSendTimer = setTimeout(() => fetch('/servo?angle=' + servoSlider.value), 80);
});

const distVal = document.getElementById('distVal');
const distToggle = document.getElementById('distToggle');
distToggle.addEventListener('click', () => {
  const turningOn = distToggle.dataset.on !== '1';
  fetch('/distance/enabled?on=' + (turningOn ? '1' : '0'));
});
function pollStatus() {
  fetch('/status').then(r => r.json()).then(s => {
    // Reconciliar el boton del LED con el estado real del ESP32, salvo que el usuario
    // acabe de tocarlo (para no pisar su accion mientras la peticion esta en vuelo).
    if (Date.now() - lastPixelInteract > 1500 && pixelOn !== s.pixel_on) {
      pixelOn = s.pixel_on;
      updatePixelLabel();
    }
    distToggle.dataset.on = s.distance_enabled ? '1' : '0';
    distToggle.textContent = 'Sensor: ' + (s.distance_enabled ? 'ON' : 'OFF');
    if (!s.distance_enabled) {
      distVal.textContent = 'Desactivado';
    } else {
      distVal.textContent = s.distance_ok ? (s.distance_mm + ' mm') : 'Sensor no conectado';
    }
  }).catch(() => {});
}
setInterval(pollStatus, 1000);
pollStatus();
</script>
</body>
</html>
)rawliteral";

// Helpers de PWM con canal LEDC explicito (API distinta en core 2.x y 3.x).
// Asignar el canal a mano evita que el core reparta canales al azar y que motor y servo
// acaben compartiendo timer (era la causa de que mover el servo activara el motor).
void pwmAttach(uint8_t pin, uint32_t freq, uint8_t res, uint8_t ch) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttachChannel(pin, freq, res, ch);
#else
  ledcSetup(ch, freq, res);
  ledcAttachPin(pin, ch);
#endif
}

void pwmWrite(uint8_t pin, uint8_t ch, uint32_t duty) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(pin, duty);
#else
  ledcWrite(ch, duty);
#endif
}

void motorSetup() {
  pwmAttach(MOTOR_PIN1, MOTOR_PWM_FREQ, MOTOR_PWM_RES, MOTOR1_LEDC_CH);
  pwmAttach(MOTOR_PIN2, MOTOR_PWM_FREQ, MOTOR_PWM_RES, MOTOR2_LEDC_CH);
  pwmWrite(MOTOR_PIN1, MOTOR1_LEDC_CH, 0);
  pwmWrite(MOTOR_PIN2, MOTOR2_LEDC_CH, 0);
}

void servoSetup() {
  pwmAttach(SERVO_PIN, SERVO_FREQ, SERVO_RES_BITS, SERVO_LEDC_CH);
}

void servoWriteAngle(int angle) {
  angle = constrain(angle, 0, 180);
  uint32_t us = map(angle, 0, 180, SERVO_MIN_US, SERVO_MAX_US);
  uint32_t maxCount = (1UL << SERVO_RES_BITS);
  uint32_t duty = (uint32_t)(((uint64_t)us * maxCount) / SERVO_PERIOD_US);
  pwmWrite(SERVO_PIN, SERVO_LEDC_CH, duty);
}

void setMotor(int dir, int strength) {
  strength = constrain(strength, 0, 255);
  if (dir > 0) {
    pwmWrite(MOTOR_PIN2, MOTOR2_LEDC_CH, 0);
    pwmWrite(MOTOR_PIN1, MOTOR1_LEDC_CH, strength);
  } else if (dir < 0) {
    pwmWrite(MOTOR_PIN1, MOTOR1_LEDC_CH, 0);
    pwmWrite(MOTOR_PIN2, MOTOR2_LEDC_CH, strength);
  } else {
    pwmWrite(MOTOR_PIN1, MOTOR1_LEDC_CH, 0);
    pwmWrite(MOTOR_PIN2, MOTOR2_LEDC_CH, 0);
  }
}

void updateMotorSafety() {
  if (motorDir != 0 && millis() - lastMotorCommandTime > MOTOR_TIMEOUT_MS) {
    motorDir = 0;
    setMotor(0, 0);
  }
}

void pixelSelfTest() {
  const uint32_t testColors[] = { pixel.Color(255, 0, 0), pixel.Color(0, 255, 0), pixel.Color(0, 0, 255) };
  Serial.println("[LED] Autotest: rojo, verde, azul");
  for (int i = 0; i < 3; i++) {
    pixel.setPixelColor(0, testColors[i]);
    pixel.show();
    delay(300);
  }
  pixel.clear();
  pixel.show();
}

// Refresca el LED con el estado actual (sin logs). El WS2812 es "fire and forget": si un
// envio se corrompe por la actividad del WiFi, el LED se queda en un estado erroneo hasta el
// siguiente envio. Rerefrescar periodicamente hace que el color/on-off correcto se reimponga solo.
void refreshPixel() {
  if (pixelOn) {
    pixel.setPixelColor(0, pixel.Color(pixelR, pixelG, pixelB));
  } else {
    pixel.setPixelColor(0, 0);
  }
  pixel.show();
}

void applyPixel() {
  refreshPixel();
  Serial.print("[LED] on=");
  Serial.print(pixelOn);
  Serial.print(" r=");
  Serial.print(pixelR);
  Serial.print(" g=");
  Serial.print(pixelG);
  Serial.print(" b=");
  Serial.println(pixelB);
}

void startMarioNote(int idx) {
  marioIndex = idx;
  marioNoteStart = millis();
  int freq = marioTune[idx].freq;
  if (freq > 0) {
    tone(BUZZER_PIN, freq);
  } else {
    noTone(BUZZER_PIN);
  }
}

void updateMarioTune() {
  if (marioIndex < 0) return;
  if (millis() - marioNoteStart >= (unsigned long)marioTune[marioIndex].dur) {
    int next = marioIndex + 1;
    if (next >= MARIO_TUNE_LENGTH) {
      noTone(BUZZER_PIN);
      marioIndex = -1;
    } else {
      startMarioNote(next);
    }
  }
}

#define VL53L1X_I2C_ADDR 0x29

// El driver del VL53L1X tiene un bucle interno de espera sin limite dentro de begin() (espera a que
// el sensor marque "dato listo", y si no hay sensor eso no ocurre nunca). Por eso comprobamos primero
// con una transaccion I2C basica, que si respeta el timeout del bus, antes de llamar a begin().
bool isDistanceSensorPresent() {
  Wire.beginTransmission(VL53L1X_I2C_ADDR);
  return Wire.endTransmission() == 0;
}

void tryInitDistanceSensor() {
  Serial.println("[I2C] Comprobando presencia del sensor de distancia...");
  lastDistanceAttempt = millis();

  if (!isDistanceSensorPresent()) {
    distanceSensorOK = false;
    Serial.println("[I2C] Sensor no responde en el bus, se reintentara en 5s");
    return;
  }

  Serial.println("[I2C] Sensor presente, inicializando...");
  distanceSensorOK = distanceSensor.begin(VL53L1X_I2C_ADDR, &Wire);
  if (distanceSensorOK) {
    distanceSensor.startRanging();
    distanceSensor.setTimingBudget(50);
    Serial.println("[I2C] Sensor de distancia listo");
  } else {
    Serial.println("[I2C] El sensor respondio en el bus pero begin() fallo, se reintentara en 5s");
  }
}

void updateDistanceSensor() {
  if (!distanceSensorEnabled) return;

  unsigned long now = millis();
  if (!distanceSensorOK) {
    if (now - lastDistanceAttempt > DISTANCE_RETRY_INTERVAL_MS) {
      tryInitDistanceSensor();
    }
    return;
  }

  if (distanceSensor.dataReady()) {
    lastDistanceMM = distanceSensor.distance();
    distanceSensor.clearInterrupt();
  }
}

String pixelColorHex() {
  char buf[8];
  sprintf(buf, "#%02X%02X%02X", pixelR, pixelG, pixelB);
  return String(buf);
}

void handleRoot() {
  server.send(200, "text/html", INDEX_HTML);
}

void handleMotor() {
  if (!server.hasArg("dir")) {
    server.send(400, "text/plain", "falta dir");
    return;
  }
  String dir = server.arg("dir");
  lastMotorCommandTime = millis();
  if (dir == "left") {
    motorDir = -1;
  } else if (dir == "right") {
    motorDir = 1;
  } else {
    motorDir = 0;
  }
  setMotor(motorDir, MOTOR_STRENGTH);
  Serial.print("[MOTOR] dir=");
  Serial.println(dir);
  server.send(200, "text/plain", "ok");
}

void handleDistanceEnabled() {
  if (server.hasArg("on")) {
    distanceSensorEnabled = server.arg("on") == "1";
    Serial.print("[I2C] Sensor de distancia ");
    Serial.println(distanceSensorEnabled ? "activado" : "desactivado");
    if (!distanceSensorEnabled) {
      distanceSensorOK = false;
      lastDistanceMM = -1;
    } else {
      lastDistanceAttempt = 0;  // fuerza un intento de deteccion en el siguiente loop()
    }
  }
  server.send(200, "text/plain", "ok");
}

void handlePixel() {
  if (server.hasArg("on")) pixelOn = server.arg("on") == "1";
  if (server.hasArg("color")) {
    String hex = server.arg("color");
    if (hex.length() == 6) {
      long value = strtol(hex.c_str(), NULL, 16);
      pixelR = (value >> 16) & 0xFF;
      pixelG = (value >> 8) & 0xFF;
      pixelB = value & 0xFF;
    }
  }
  applyPixel();
  server.send(200, "text/plain", "ok");
}

void handleBuzzer() {
  startMarioNote(0);
  server.send(200, "text/plain", "ok");
}

void handleServo() {
  if (server.hasArg("angle")) {
    servoAngle = constrain(server.arg("angle").toInt(), 0, 180);
    servoWriteAngle(servoAngle);
    Serial.print("[SERVO] angulo=");
    Serial.println(servoAngle);
  }
  server.send(200, "text/plain", "ok");
}

void handleStatus() {
  String json = "{";
  json += "\"distance_ok\":" + String(distanceSensorOK ? "true" : "false") + ",";
  json += "\"distance_mm\":" + String(lastDistanceMM) + ",";
  json += "\"servo_angle\":" + String(servoAngle) + ",";
  json += "\"pixel_on\":" + String(pixelOn ? "true" : "false") + ",";
  json += "\"pixel_color\":\"" + pixelColorHex() + "\",";
  json += "\"distance_enabled\":" + String(distanceSensorEnabled ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

void setupRoutes() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/motor", HTTP_GET, handleMotor);
  server.on("/pixel", HTTP_GET, handlePixel);
  server.on("/buzzer", HTTP_GET, handleBuzzer);
  server.on("/servo", HTTP_GET, handleServo);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/distance/enabled", HTTP_GET, handleDistanceEnabled);
  server.onNotFound([]() {
    server.send(404, "text/plain", "No encontrado");
  });
}

void setup() {
  // LO PRIMERO de todo: forzar los pines del motor a LOW antes de cualquier otra cosa.
  // En el ESP32-C3, D9 (GPIO9) es pin de strapping con pull-up interno durante el arranque,
  // asi que el driver puede ver "activo" y girar el motor unos segundos hasta que lo silenciamos.
  // Ponerlos a LOW aqui, antes del delay del serie y del WiFi, reduce ese giro al minimo posible.
  pinMode(MOTOR_PIN1, OUTPUT);
  pinMode(MOTOR_PIN2, OUTPUT);
  digitalWrite(MOTOR_PIN1, LOW);
  digitalWrite(MOTOR_PIN2, LOW);

  Serial.begin(115200);
  delay(600);  // margen para que enumere el USB-CDC del ESP32-C3 (motor ya esta en LOW)
  Serial.println();
  Serial.println("[BOOT] Iniciando cablebottt...");

  motorSetup();  // pasa los pines del motor a PWM (canales LEDC fijos) con duty 0
  Serial.println("[BOOT] Motor listo (D9/D10)");

  pixel.begin();
  pixelSelfTest();  // parpadeo rojo/verde/azul: si no se ve, el problema es libreria o hardware, no la webapp
  applyPixel();
  Serial.println("[BOOT] Neopixel listo (D7)");

  servoSetup();
  servoWriteAngle(servoAngle);
  Serial.println("[BOOT] Servo listo (D8)");

  // WiFi y servidor web se levantan ANTES de tocar el sensor I2C, para que el movil
  // pueda conectarse aunque el sensor de distancia tarde en detectarse o no este conectado.
  Serial.println("[BOOT] Iniciando WiFi AP...");
  WiFi.mode(WIFI_AP);                       // fija explicitamente el modo AP antes de arrancar
  delay(100);
  // canal 1, oculto=0, max 4 clientes. Se comprueba el valor de retorno.
  bool apOk = WiFi.softAP(AP_SSID, AP_PASSWORD, 1, 0, 4);
  delay(100);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);       // baja la potencia TX: reduce picos de corriente que tumban la radio
  Serial.print("[BOOT] softAP() devolvio: ");
  Serial.println(apOk ? "OK" : "FALLO");
  Serial.print("[BOOT] SSID: ");
  Serial.print(AP_SSID);
  Serial.print("  IP: ");
  Serial.print(WiFi.softAPIP());
  Serial.print("  MAC: ");
  Serial.print(WiFi.softAPmacAddress());
  Serial.print("  canal: ");
  Serial.println(WiFi.channel());

  setupRoutes();
  server.begin();
  Serial.println("[BOOT] Servidor web iniciado");

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setTimeOut(50);
  if (distanceSensorEnabled) {
    tryInitDistanceSensor();
  } else {
    Serial.println("[I2C] Sensor de distancia desactivado por defecto, activalo desde la webapp");
  }

  Serial.println("[BOOT] Setup completo, entrando en loop()");
}

unsigned long lastStaReport = 0;
unsigned long lastPixelRefresh = 0;
void loop() {
  server.handleClient();
  updateMotorSafety();
  updateMarioTune();
  updateDistanceSensor();

  // Reimpone el estado del LED cada 500 ms para corregir envios corrompidos por el WiFi.
  if (millis() - lastPixelRefresh > 500) {
    lastPixelRefresh = millis();
    refreshPixel();
  }

  if (millis() - lastStaReport > 10000) {
    lastStaReport = millis();
    Serial.print("[WIFI] clientes conectados: ");
    Serial.println(WiFi.softAPgetStationNum());
  }
}
