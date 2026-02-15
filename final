#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);

// Pin Driver Motor L298N
#define IN1 5
#define IN2 18
#define IN3 19
#define IN4 21
#define ENA 23
#define ENB 22

// Definisi Kecepatan
const int SPEED_NORMAL = 255; 
const int SPEED_BELOK  = 140; 

const char* ssid = "exora1.0";
const char* password = "815297113";

// --- FUNGSI MOTOR ---

void applySpeed(int spd) {
  analogWrite(ENA, spd);
  analogWrite(ENB, spd);
}

void stopMotor() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  applySpeed(0);
}

void maju() {
  applySpeed(SPEED_NORMAL);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
}

void mundur() {
  applySpeed(SPEED_NORMAL);
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

// FUNGSI KIRI (Sudah Ditukar Logikanya)
void kiri() {
  applySpeed(SPEED_BELOK);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

// FUNGSI KANAN (Sudah Ditukar Logikanya)
void kanan() {
  applySpeed(SPEED_BELOK);
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
}

// --- WEB SERVER ---

void handleRoot() {
  String html = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
    <title>EXORA 1.0 PRO</title>
    <style>
      body { text-align:center; font-family:'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background-color:#121212; color:white; padding-top:20px; user-select:none; -webkit-tap-highlight-color:transparent; }
      h2 { color:#00ff88; text-shadow: 0 0 10px #00ff88; margin-bottom:10px; }
      #status-box { margin: 15px auto; padding: 10px; width: 200px; border-radius: 10px; background: #222; border: 1px solid #444; font-weight: bold; color: #aaa; }
      .active-status { color: #00ff88 !important; border-color: #00ff88 !important; box-shadow: 0 0 10px #00ff88; }
      .control-panel { display: grid; grid-template-columns: repeat(3, 1fr); gap: 15px; max-width: 300px; margin: auto; padding: 20px; }
      .btn { width:90px; height:90px; font-size:16px; font-weight:bold; cursor:pointer; border-radius:50%; border:none; background:#333; color:white; box-shadow: 0 6px #000; transition: 0.1s; }
      .btn:active { background:#00ff88; color:#000; transform: translateY(4px); box-shadow: 0 0 15px #00ff88; }
      .btn-maju { grid-column: 2; }
      .btn-kiri { grid-column: 1; }
      .btn-kanan { grid-column: 3; }
      .btn-mundur { grid-column: 2; }
      .footer { margin-top:40px; font-size:12px; color:#555; }
    </style>
  </head>
  <body>
    <h2>EXORA 1.0</h2>
    <div id="status-box">STATUS: STANDBY</div>
    <div class="control-panel">
      <button class="btn btn-maju" onmousedown="move('/maju', 'MAJU')" onmouseup="move('/stop', 'STANDBY')" ontouchstart="move('/maju', 'MAJU')" ontouchend="move('/stop', 'STANDBY')">UP</button>
      <button class="btn btn-kiri" onmousedown="move('/kiri', 'KIRI')" onmouseup="move('/stop', 'STANDBY')" ontouchstart="move('/kiri', 'KIRI')" ontouchend="move('/stop', 'STANDBY')">LEFT</button>
      <button class="btn btn-kanan" onmousedown="move('/kanan', 'KANAN')" onmouseup="move('/stop', 'STANDBY')" ontouchstart="move('/kanan', 'KANAN')" ontouchend="move('/stop', 'STANDBY')">RIGHT</button>
      <button class="btn btn-mundur" onmousedown="move('/mundur', 'MUNDUR')" onmouseup="move('/stop', 'STANDBY')" ontouchstart="move('/mundur', 'MUNDUR')" ontouchend="move('/stop', 'STANDBY')">DOWN</button>
    </div>
    <div class="footer">Credit by Alfadil ilhamora</div>
    <script>
      const statusBox = document.getElementById('status-box');
      function move(path, statusText) {
        fetch(path).catch(err => console.log("Err: " + err));
        statusBox.innerText = "STATUS: " + statusText;
        if(statusText !== 'STANDBY') { statusBox.classList.add('active-status'); } 
        else { statusBox.classList.remove('active-status'); }
      }
      window.oncontextmenu = function(event) { event.preventDefault(); return false; };
    </script>
  </body>
  </html>
  )rawliteral";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);
  stopMotor();
  WiFi.softAP(ssid, password);
  server.on("/", handleRoot);
  server.on("/maju", [](){ maju(); server.send(200, "text/plain", "OK"); });
  server.on("/mundur", [](){ mundur(); server.send(200, "text/plain", "OK"); });
  server.on("/kiri", [](){ kiri(); server.send(200, "text/plain", "OK"); });
  server.on("/kanan", [](){ kanan(); server.send(200, "text/plain", "OK"); });
  server.on("/stop", [](){ stopMotor(); server.send(200, "text/plain", "OK"); });
  server.begin();
}

void loop() {
  server.handleClient();
}
