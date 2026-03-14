#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);

#define IN1 5
#define IN2 18
#define IN3 19
#define IN4 21
#define ENA 23
#define ENB 22
#define BUZZER 32  // PIN BUZZER (bisa diganti sesuai kebutuhan)

// ========== TAMBAHAN UNTUK LAMPU LED ==========
#define LAMPU_PIN 12  // LED lampu mobil (GPIO12)
bool lampuMenyala = true; // state awal: menyala
// ========== AKHIR TAMBAHAN ==========

const int SPEED_NORMAL = 255; 
const int SPEED_BELOK  = 160; 

unsigned long lastCommandTime = 0;
const unsigned long TIMEOUT_MS = 500;

// ========== TAMBAHAN UNTUK OLED MATA ==========
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// State animasi mata
enum EyeState { EYE_IDLE_NORMAL, EYE_MOVING, EYE_SLEEPY };
EyeState currentEyeState = EYE_IDLE_NORMAL;
unsigned long idleStartTime = 0;
bool wasMoving = false;

// Variabel animasi idle
unsigned long lastIdleAnimTime = 0;
int idleAnimState = 0; // 0: normal, 1: blink, 2: look left, 3: look right
const unsigned long idleAnimInterval = 800; // ganti setiap 800ms

// Fungsi gambar mata static (normal kotak penuh)
void drawEyesStatic() {
  display.clearDisplay();
  // Mata kiri
  display.fillRect(30, 16, 25, 25, SSD1306_WHITE);
  // Mata kanan
  display.fillRect(73, 16, 25, 25, SSD1306_WHITE);
  display.display();
}

// Fungsi gambar mata marah (menyipit + alis)
void drawEyesMarah() {
  display.clearDisplay();
  // Mata menyipit (lebih pendek)
  display.fillRect(30, 20, 25, 18, SSD1306_WHITE); // y=20, tinggi 18
  display.fillRect(73, 20, 25, 18, SSD1306_WHITE);
  // Alis marah (garis miring)
  display.drawLine(25, 12, 50, 20, SSD1306_WHITE); // alis kiri
  display.drawLine(78, 20, 103, 12, SSD1306_WHITE); // alis kanan
  display.display();
}

// Fungsi gambar mata idle dengan animasi
void drawEyesIdle() {
  display.clearDisplay();
  switch(idleAnimState) {
    case 0: // normal
      display.fillRect(30, 16, 25, 25, SSD1306_WHITE);
      display.fillRect(73, 16, 25, 25, SSD1306_WHITE);
      break;
    case 1: // berkedip (mata jadi garis)
      display.fillRect(30, 28, 25, 5, SSD1306_WHITE);
      display.fillRect(73, 28, 25, 5, SSD1306_WHITE);
      break;
    case 2: // melihat kiri
      display.fillRect(20, 16, 25, 25, SSD1306_WHITE);
      display.fillRect(63, 16, 25, 25, SSD1306_WHITE);
      break;
    case 3: // melihat kanan
      display.fillRect(40, 16, 25, 25, SSD1306_WHITE);
      display.fillRect(83, 16, 25, 25, SSD1306_WHITE);
      break;
  }
  display.display();
}

// Update animasi idle (ubah state secara acak)
void updateIdleAnimation() {
  if (millis() - lastIdleAnimTime > idleAnimInterval) {
    lastIdleAnimTime = millis();
    idleAnimState = random(0, 4); // 0-3
  }
}

// Fungsi update state mata berdasarkan pergerakan robot dan obstacle
void updateEyeState() {
  // ===== TAMBAHAN PRIORITAS OBSTACLE =====
  extern bool obstacleDetected;  // variabel dari HC-SR04
  if (obstacleDetected) {
    if (currentEyeState != EYE_MOVING) {
      currentEyeState = EYE_MOVING;
      drawEyesMarah();
    }
    return;
  }
  // ===== AKHIR TAMBAHAN =====

  bool moving = (millis() - lastCommandTime <= TIMEOUT_MS);
  
  if (moving) {
    // Robot bergerak → tampilkan mata static (normal)
    if (currentEyeState != EYE_IDLE_NORMAL) {
      currentEyeState = EYE_IDLE_NORMAL;
      drawEyesStatic();
    }
    wasMoving = true;
    idleStartTime = 0; // reset idle timer
  } else {
    // Robot diam (idle)
    if (wasMoving) {
      // Baru saja berhenti
      wasMoving = false;
      idleStartTime = millis();
      currentEyeState = EYE_IDLE_NORMAL;
      drawEyesStatic();
    } else {
      // Sudah idle
      if (idleStartTime == 0) idleStartTime = millis();
      
      unsigned long idleDuration = millis() - idleStartTime;
      if (idleDuration > 3000) {  // setelah 3 detik idle, mulai animasi lucu
        updateIdleAnimation();
        drawEyesIdle();
        currentEyeState = EYE_SLEEPY; // gunakan state SLEEPY untuk animasi
      } else {
        // Idle kurang dari 3 detik → mata static
        if (currentEyeState != EYE_IDLE_NORMAL) {
          currentEyeState = EYE_IDLE_NORMAL;
          drawEyesStatic();
        }
      }
    }
  }
}
// ========== AKHIR TAMBAHAN ==========

// ========== TAMBAHAN UNTUK HC-SR04 ==========
#define TRIG_PIN 4
#define ECHO_PIN 16

bool obstacleDetected = false;
float jarakTerakhir = 999;
unsigned long lastSensorRead = 0;
const unsigned long sensorInterval = 100; // baca setiap 100ms

float bacaJarak() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30ms
  if (duration == 0) {
    return 999; // tidak terdeteksi
  }
  float jarak = duration * 0.034 / 2;
  return jarak;
}
// ========== AKHIR TAMBAHAN HC-SR04 ==========

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

// Fungsi maju dimodifikasi dengan pengecekan jarak
void maju() {
  if (jarakTerakhir < 40.0) {
    // Terlalu dekat, tidak boleh maju
    return;
  }
  lastCommandTime = millis();
  applySpeed(SPEED_NORMAL);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

void mundur() {
  lastCommandTime = millis();
  applySpeed(SPEED_NORMAL);
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
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
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

// Fungsi Buzzer
void buzzerOn() { 
  digitalWrite(BUZZER, HIGH); 
}

void buzzerOff() { 
  digitalWrite(BUZZER, LOW); 
}

void buzzerBeep() {
  digitalWrite(BUZZER, HIGH);
  delay(100);
  digitalWrite(BUZZER, LOW);
}

void buzzerAlert() {
  for(int i = 0; i < 3; i++) {
    digitalWrite(BUZZER, HIGH);
    delay(150);
    digitalWrite(BUZZER, LOW);
    delay(100);
  }
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>EXORA 1.0 PRO</title>
    <style>
        :root {
            --neon-green: #00ff88;
            --dark-bg: #0a0a0a;
            --glass-bg: rgba(255, 255, 255, 0.05);
            --buzzer-red: #ff4444;
            --buzzer-orange: #ff8800;
            --lampu-yellow: #ffaa00;
        }

        body {
            background-color: var(--dark-bg);
            color: white;
            font-family: 'Segoe UI', Tahoma, sans-serif;
            margin: 0;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 100vh;
            overflow: hidden;
            user-select: none;
            -webkit-tap-highlight-color: transparent;
        }

        /* --- Header Section --- */
        .header-section {
            text-align: center;
            z-index: 10;
            margin-bottom: 10px;
        }

        h2 {
            font-size: 1.5rem;
            letter-spacing: 3px;
            color: var(--neon-green);
            margin: 5px 0;
            text-shadow: 0 0 15px rgba(0, 255, 136, 0.4);
        }

        #status-box {
            background: #222;
            border: 1px solid #444;
            padding: 8px 15px;
            min-width: 140px;
            border-radius: 10px;
            font-size: 12px;
            font-weight: bold;
            color: #aaa;
            transition: 0.3s;
            display: inline-block;
        }

        .active-status {
            color: var(--neon-green) !important;
            border-color: var(--neon-green) !important;
            box-shadow: 0 0 15px rgba(0, 255, 136, 0.3);
        }

        /* --- Buzzer & Lampu Container --- */
        .button-panel {
            display: flex;
            gap: 20px;
            margin-top: 20px;
            margin-bottom: 10px;
            flex-wrap: wrap;
            justify-content: center;
        }
        
        .buzzer-btn, .lampu-btn {
            width: 140px;
            height: 60px;
            border: none;
            border-radius: 30px;
            background: linear-gradient(145deg, #2a2a2a, #1a1a1a);
            font-size: 24px;
            font-weight: bold;
            cursor: pointer;
            box-shadow: 0 5px #000;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 10px;
            transition: 0.1s;
            border: 1px solid #333;
        }

        .buzzer-btn {
            color: var(--buzzer-red);
        }

        .lampu-btn {
            color: var(--lampu-yellow);
        }

        .buzzer-btn:active {
            color: white;
            background: var(--buzzer-red);
            box-shadow: 0 0 20px var(--buzzer-red);
            transform: translateY(4px);
            border-color: var(--buzzer-red);
        }

        .lampu-btn:active {
            color: white;
            background: var(--lampu-yellow);
            box-shadow: 0 0 20px var(--lampu-yellow);
            transform: translateY(4px);
            border-color: var(--lampu-yellow);
        }

        .buzzer-active {
            animation: buzzerPulse 0.5s infinite;
        }

        @keyframes buzzerPulse {
            0% { box-shadow: 0 0 10px var(--buzzer-red); }
            50% { box-shadow: 0 0 30px var(--buzzer-red); color: white; }
            100% { box-shadow: 0 0 10px var(--buzzer-red); }
        }

        .lampu-on {
            background: var(--lampu-yellow) !important;
            color: white !important;
            box-shadow: 0 0 20px var(--lampu-yellow);
        }

        /* --- Controller Containers --- */
        .control-container {
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 20px;
            margin-top: 10px;
        }

        .btn {
            width: 80px;
            height: 80px;
            border: none;
            border-radius: 50%;
            background: linear-gradient(145deg, #1e1e1e, #111111);
            color: #777;
            font-size: 20px;
            cursor: pointer;
            box-shadow: 0 5px #000;
            display: flex;
            align-items: center;
            justify-content: center;
            transition: 0.1s;
            -webkit-tap-highlight-color: transparent;
        }

        .btn:active {
            color: var(--neon-green);
            background: #222;
            box-shadow: 0 0 15px var(--neon-green);
            transform: translateY(4px);
        }

        /* Grouping */
        .side-controls { 
            display: flex; 
            gap: 15px; 
        }
        .vertical { 
            flex-direction: column; 
        }
        .horizontal { 
            flex-direction: row; 
        }

        .footer {
            position: absolute;
            bottom: 10px;
            font-size: 9px;
            color: #ffffff;
        }

        /* ========================================= */
        /* LANDSCAPE OPTIMIZATION                    */
        /* ========================================= */
        @media (orientation: landscape) {
            .header-section {
                position: absolute;
                top: 5px;
                left: 0;
                right: 0;
                margin: 0 auto;
            }

            .button-panel {
                position: absolute;
                bottom: 20px;
                left: 0;
                right: 0;
                margin: 0 auto;
            }

            .control-container {
                width: 100%;
                justify-content: space-between;
                padding: 0 80px;
                box-sizing: border-box;
                margin-top: 30px;
            }

            .btn { 
                width: 90px; 
                height: 90px; 
                font-size: 24px;
            }
            
            .side-controls {
                gap: 25px;
            }
        }

        /* Portrait Adjustments */
        @media (orientation: portrait) {
            .control-container { 
                flex-direction: column; 
                gap: 15px;
            }
            .horizontal { 
                margin-top: 5px; 
            }
            .button-panel {
                margin-top: 25px;
            }
        }

        /* Small screen adjustments */
        @media (max-width: 480px) {
            .btn {
                width: 70px;
                height: 70px;
                font-size: 18px;
            }
            .buzzer-btn, .lampu-btn {
                width: 120px;
                height: 50px;
                font-size: 20px;
            }
        }
    </style>
</head>
<body>

    <div class="header-section">
        <h2>EXORA 1.0</h2>
        <div id="status-box">STATUS: STANDBY</div>
    </div>

    <!-- Panel Tombol Buzzer dan Lampu -->
    <div class="button-panel">
        <!-- BUZZER BUTTON -->
        <button class="buzzer-btn" id="buzzerBtn"
            onmousedown="buzzerStart()" onmouseup="buzzerStop()" onmouseleave="buzzerStop()"
            ontouchstart="buzzerStart()" ontouchend="buzzerStop()" ontouchcancel="buzzerStop()">
            🔔 KLAKSON
        </button>

        <!-- LAMPU BUTTON -->
        <button class="lampu-btn" id="lampuBtn" onclick="toggleLampu()">
            💡 LAMPU
        </button>
    </div>

    <div class="control-container">
        <!-- Vertical buttons (Maju & Mundur) -->
        <div class="side-controls vertical">
            <button class="btn" 
                onmousedown="holdStart('/maju', 'MAJU')" onmouseup="holdEnd()" onmouseleave="holdEnd()"
                ontouchstart="holdStart('/maju', 'MAJU')" ontouchend="holdEnd()" ontouchcancel="holdEnd()">▲</button>
            
            <button class="btn" 
                onmousedown="holdStart('/mundur', 'MUNDUR')" onmouseup="holdEnd()" onmouseleave="holdEnd()"
                ontouchstart="holdStart('/mundur', 'MUNDUR')" ontouchend="holdEnd()" ontouchcancel="holdEnd()">▼</button>
        </div>

        <!-- Horizontal buttons (Kiri & Kanan) -->
        <div class="side-controls horizontal">
            <button class="btn" 
                onmousedown="holdStart('/kiri', 'KIRI')" onmouseup="holdEnd()" onmouseleave="holdEnd()"
                ontouchstart="holdStart('/kiri', 'KIRI')" ontouchend="holdEnd()" ontouchcancel="holdEnd()">◀</button>
            
            <button class="btn" 
                onmousedown="holdStart('/kanan', 'KANAN')" onmouseup="holdEnd()" onmouseleave="holdEnd()"
                ontouchstart="holdStart('/kanan', 'KANAN')" ontouchend="holdEnd()" ontouchcancel="holdEnd()">▶</button>
        </div>
    </div>

    <div class="footer"> Design by ALFADIL ILHAMORA © 2026</div>

<script>
    const statusBox = document.getElementById('status-box');
    const buzzerBtn = document.getElementById('buzzerBtn');
    const lampuBtn = document.getElementById('lampuBtn');
    let cmdInterval;
    let buzzerInterval;
    let currentPath = '';
    let isPressed = false;
    let isBuzzerPressed = false;
    let lampuState = true; // true = menyala, false = mati

    // Inisialisasi tampilan lampu
    if (lampuState) {
        lampuBtn.classList.add('lampu-on');
    }

    // Fungsi untuk motor
    function holdStart(path, statusText) {
        if (isPressed) {
            holdEnd();
        }
        
        isPressed = true;
        currentPath = path;
        
        fetch(path).catch(e => handleError());
        
        statusBox.innerText = "STATUS: " + statusText;
        statusBox.classList.add('active-status');
        if (navigator.vibrate) navigator.vibrate(30);
        
        if (cmdInterval) {
            clearInterval(cmdInterval);
        }
        
        cmdInterval = setInterval(() => {
            if (isPressed) {
                fetch(currentPath).catch(e => handleError());
            }
        }, 100);
    }

    function holdEnd() {
        if (!isPressed) return;
        
        isPressed = false;
        
        if (cmdInterval) {
            clearInterval(cmdInterval);
            cmdInterval = null;
        }
        
        currentPath = '';
        fetch('/stop').catch(e => {});
        
        statusBox.innerText = "STATUS: STANDBY";
        statusBox.classList.remove('active-status');
    }

    // Fungsi untuk buzzer
    function buzzerStart() {
        if (isBuzzerPressed) {
            buzzerStop();
        }
        
        isBuzzerPressed = true;
        
        // Kirim perintah buzzer on
        fetch('/buzzer/on').catch(e => {});
        
        // Animasi button
        buzzerBtn.classList.add('buzzer-active');
        buzzerBtn.style.color = 'white';
        buzzerBtn.style.background = 'var(--buzzer-red)';
        
        if (navigator.vibrate) navigator.vibrate(20);
        
        // Interval untuk keep-alive buzzer
        buzzerInterval = setInterval(() => {
            if (isBuzzerPressed) {
                fetch('/buzzer/on').catch(e => {});
            }
        }, 200);
    }

    function buzzerStop() {
        if (!isBuzzerPressed) return;
        
        isBuzzerPressed = false;
        
        if (buzzerInterval) {
            clearInterval(buzzerInterval);
            buzzerInterval = null;
        }
        
        // Kirim perintah buzzer off
        fetch('/buzzer/off').catch(e => {});
        
        // Kembalikan animasi button
        buzzerBtn.classList.remove('buzzer-active');
        buzzerBtn.style.color = '';
        buzzerBtn.style.background = '';
    }

    // Fungsi untuk toggle lampu
    function toggleLampu() {
        lampuState = !lampuState;
        fetch('/lampu/toggle').catch(e => console.log('Gagal toggle lampu'));
        
        if (lampuState) {
            lampuBtn.classList.add('lampu-on');
        } else {
            lampuBtn.classList.remove('lampu-on');
        }
        
        if (navigator.vibrate) navigator.vibrate(20);
    }

    function handleError() {
        if (isPressed) {
            isPressed = false;
            
            if (cmdInterval) {
                clearInterval(cmdInterval);
                cmdInterval = null;
            }
            
            fetch('/stop').catch(e => {});
            
            statusBox.innerText = "STATUS: ERROR";
            statusBox.classList.remove('active-status');
            
            setTimeout(() => {
                statusBox.innerText = "STATUS: STANDBY";
            }, 1000);
        }
        
        // Juga matikan buzzer kalau error
        if (isBuzzerPressed) {
            buzzerStop();
        }
    }

    // Cleanup on page unload
    window.addEventListener('beforeunload', function() {
        if (cmdInterval) {
            clearInterval(cmdInterval);
        }
        if (buzzerInterval) {
            clearInterval(buzzerInterval);
        }
        fetch('/stop').catch(e => {});
        fetch('/buzzer/off').catch(e => {});
        // Lampu tidak perlu dimatikan, biarkan sesuai state terakhir
    });

    window.oncontextmenu = (e) => { e.preventDefault(); return false; };
</script>

</body>
</html>
  )rawliteral";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  
  // Pin Motor
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);
  
  // Pin Buzzer
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);  // Buzzer mati di awal

  // ===== TAMBAHAN SETUP HC-SR04 =====
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);
  // ===== AKHIR TAMBAHAN =====
  
  // ===== TAMBAHAN UNTUK LAMPU LED =====
  pinMode(LAMPU_PIN, OUTPUT);
  digitalWrite(LAMPU_PIN, HIGH);  // Lampu menyala default
  // ===== AKHIR TAMBAHAN =====

  stopMotor();

  // ===== TAMBAHAN INISIALISASI OLED =====
  Wire.begin(25, 26);  // SDA = 25, SCL = 26
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED tidak ditemukan, lanjut tanpa OLED");
  } else {
    randomSeed(millis()); // untuk animasi random
    drawEyesStatic();  // tampilan awal mata static
  }
  // ===== AKHIR TAMBAHAN =====

  WiFi.softAP(ssid, password);

  server.on("/", handleRoot);
  
  // Endpoint Motor
  server.on("/maju", [](){ maju(); server.send(200); });
  server.on("/mundur", [](){ mundur(); server.send(200); });
  server.on("/kiri", [](){ kiri(); server.send(200); });
  server.on("/kanan", [](){ kanan(); server.send(200); });
  server.on("/stop", [](){ stopMotor(); server.send(200); });
  
  // Endpoint Buzzer
  server.on("/buzzer/on", [](){ 
    digitalWrite(BUZZER, HIGH); 
    server.send(200); 
  });
  
  server.on("/buzzer/off", [](){ 
    digitalWrite(BUZZER, LOW); 
    server.send(200); 
  });
  
  // ===== TAMBAHAN ENDPOINT LAMPU =====
  server.on("/lampu/toggle", [](){ 
    lampuMenyala = !lampuMenyala; 
    digitalWrite(LAMPU_PIN, lampuMenyala ? HIGH : LOW); 
    server.send(200); 
  });
  // ===== AKHIR TAMBAHAN =====
  
  server.begin();
  
  Serial.println("Access Point dimulai");
  Serial.print("SSID: "); Serial.println(ssid);
  Serial.print("IP Address: "); Serial.println(WiFi.softAPIP());
  Serial.print("Buzzer PIN: "); Serial.println(BUZZER);
}

void loop() {
  server.handleClient();

  // ===== TAMBAHAN HC-SR04 LOOP =====
  if (millis() - lastSensorRead >= sensorInterval) {
    lastSensorRead = millis();
    jarakTerakhir = bacaJarak();
    obstacleDetected = (jarakTerakhir < 40.0); // deteksi jika kurang dari 40 cm
    
    // Jika jarak kurang dari 40 cm, hentikan motor (darurat)
    if (jarakTerakhir < 40.0) {
      stopMotor();
    }
  }
  // ===== AKHIR TAMBAHAN =====

  // ===== TAMBAHAN UPDATE MATA =====
  updateEyeState();
  // ===== AKHIR TAMBAHAN =====

  if (millis() - lastCommandTime > TIMEOUT_MS) {
    stopMotor();
  }
}
