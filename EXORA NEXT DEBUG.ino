#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);

#define IN1 5
#define IN2 18
#define IN3 19
#define IN4 21
#define ENA 23
#define ENB 22

const int SPEED_NORMAL = 255; 
const int SPEED_BELOK  = 140; 

// Variabel Failsafe
unsigned long lastCommandTime = 0;
const unsigned long TIMEOUT_MS = 500; // Berhenti otomatis jika 0.5 detik tanpa sinyal

const char* ssid = "exora1.0";
const char* password = "815297113";

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
  lastCommandTime = millis(); // Update waktu setiap ada perintah
  applySpeed(SPEED_NORMAL);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
}

void mundur() {
  lastCommandTime = millis();
  applySpeed(SPEED_NORMAL);
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void kiri() {
  lastCommandTime = millis();
  applySpeed(SPEED_BELOK);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void kanan() {
  lastCommandTime = millis();
  applySpeed(SPEED_BELOK);
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
}

void handleRoot() {
  String html = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
    <style>
      body { text-align:center; font-family:sans-serif; background-color:#121212; color:white; padding-top:20px; user-select:none; -webkit-tap-highlight-color:transparent; }
      h2 { color:#00ff88; }
      #status-box { margin: 15px auto; padding: 10px; width: 200px; border-radius: 10px; background: #222; border: 1px solid #444; font-weight: bold; }
      .active-status { color: #00ff88; border-color: #00ff88; box-shadow: 0 0 10px #00ff88; }
      .control-panel { display: grid; grid-template-columns: repeat(3, 1fr); gap: 15px; max-width: 300px; margin: auto; padding: 20px; }
      .btn { width:90px; height:90px; font-size:16px; font-weight:bold; cursor:pointer; border-radius:50%; border:none; background:#333; color:white; box-shadow: 0 6px #000; }
      .btn:active { background:#00ff88; color:#000; transform: translateY(4px); box-shadow: 0 0 15px #00ff88; }
      .btn-maju { grid-column: 2; }
      .btn-kiri { grid-column: 1; }
      .btn-kanan { grid-column: 3; }
      .btn-mundur { grid-column: 2; }
    </style>
  </head>
  <body>
    <h2>EXORA 1.0</h2>
    <div id="status-box">STATUS: STANDBY</div>
    <div class="control-panel">
      <button class="btn btn-maju" ontouchstart="startMove('/maju', 'MAJU')" ontouchend="stopMove()">UP</button>
      <button class="btn btn-kiri" ontouchstart="startMove('/kiri', 'KIRI')" ontouchend="stopMove()">LEFT</button>
      <button class="btn btn-kanan" ontouchstart="startMove('/kanan', 'KANAN')" ontouchend="stopMove()">RIGHT</button>
      <button class="btn btn-mundur" ontouchstart="startMove('/mundur', 'MUNDUR')" ontouchend="stopMove()">DOWN</button>
    </div>
    <script>
      let interval;
      function startMove(path, status) {
        move(path, status);
        // Kirim ulang perintah setiap 200ms agar failsafe tidak terpicu saat ditahan
        interval = setInterval(() => move(path, status), 200);
      }
      function stopMove() {
        clearInterval(interval);
        move('/stop', 'STANDBY');
      }
      function move(path, statusText) {
        fetch(path).catch(err => {});
        const sb = document.getElementById('status-box');
        sb.innerText = "STATUS: " + statusText;
        statusText !== 'STANDBY' ? sb.classList.add('active-status') : sb.classList.remove('active-status');
      }
      window.oncontextmenu = e => e.preventDefault();
    </script>
  </body>
  </html>
  )rawliteral";
  server.send(200, "text/html", html);
}

void setup() {
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

  // LOGIKA FAILSAFE: Jika tidak ada perintah masuk selama > 500ms, motor mati
  if (millis() - lastCommandTime > TIMEOUT_MS) {
    // Hanya panggil stop jika motor memang sedang tidak standby (opsional untuk efisiensi)
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
    analogWrite(ENA, 0); analogWrite(ENB, 0);
  }
}
