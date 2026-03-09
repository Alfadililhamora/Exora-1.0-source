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

// Definisi Kecepatan Tetap
const int SPEED_NORMAL = 255; // Kecepatan penuh untuk Maju/Mundur
const int SPEED_BELOK  = 130; // Kecepatan pelan khusus saat Belok

const char* ssid = "exora1.0";
const char* password = "815297113";

// --- FUNGSI MOTOR ---

void applySpeed(int spd) {
  analogWrite(ENA, spd);
  analogWrite(ENB, spd);
}

void stopMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  applySpeed(0);
}

void maju() {
  applySpeed(SPEED_NORMAL); // Pakai speed cepat
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void mundur() {
  applySpeed(SPEED_NORMAL); // Pakai speed cepat
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void kiri() {
  applySpeed(SPEED_BELOK); // Pakai speed pelan
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void kanan() {
  applySpeed(SPEED_BELOK); // Pakai speed pelan
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

// --- WEB SERVER ---

void handleRoot() {
  String html = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>EXORA 1.0</title>
    <style>
      body { text-align:center; font-family:Arial; background-color:#f4f4f4; padding-top:50px; }
      .btn { width:100px; height:70px; margin:10px; font-size:18px; cursor:pointer; border-radius:15px; border:none; background:#333; color:white; }
      .btn-stop { background:#d9534f; width:100px; }
      h2 { color:#333; margin-bottom:30px; }
      .footer { margin-top:30px; font-size:12px; color:gray; }
    </style>
  </head>
  <body>
    <h2>EXORA 1.0</h2>
    <a href="/maju"><button class="btn">MAJU</button></a><br>
    <a href="/kanan"><button class="btn">KIRI</button></a>
    <a href="/stop"><button class="btn btn-stop">STOP</button></a>
    <a href="/kiri"><button class="btn">KANAN</button></a><br>
    <a href="/mundur"><button class="btn">MUNDUR</button></a>
    <div class="footer">Auto-Speed: Belok (Slow) | Lurus (Fast)</div>
  </body>
  </html>
  )rawliteral";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  stopMotor();

  WiFi.softAP(ssid, password);
  server.on("/", handleRoot);
  server.on("/maju", [](){ maju(); handleRoot(); });
  server.on("/mundur", [](){ mundur(); handleRoot(); });
  server.on("/kiri", [](){ kiri(); handleRoot(); });
  server.on("/kanan", [](){ kanan(); handleRoot(); });
  server.on("/stop", [](){ stopMotor(); handleRoot(); });

  server.begin();
  Serial.println("Server Started");
}

void loop() {
  server.handleClient();
}
