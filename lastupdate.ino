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

const int SPEED_NORMAL = 255; 
const int SPEED_BELOK  = 160; 

unsigned long lastCommandTime = 0;
const unsigned long TIMEOUT_MS = 500;

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

void maju() { lastCommandTime = millis(); applySpeed(SPEED_NORMAL); digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH); }
void mundur() { lastCommandTime = millis(); applySpeed(SPEED_NORMAL); digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH); digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); }
void kiri() { lastCommandTime = millis(); applySpeed(SPEED_BELOK); digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); }
void kanan() { lastCommandTime = millis(); applySpeed(SPEED_BELOK); digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH); digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH); }

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

        /* --- Buzzer Button --- */
        .buzzer-container {
            margin-top: 20px;
            margin-bottom: 10px;
            text-align: center;
        }
        
        .buzzer-btn {
            width: 140px;
            height: 60px;
            border: none;
            border-radius: 30px;
            background: linear-gradient(145deg, #2a2a2a, #1a1a1a);
            color: var(--buzzer-red);
            font-size: 24px;
            font-weight: bold;
            cursor: pointer;
            box-shadow: 0 5px #000;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 10px;
            transition: 0.1s;
            margin: 0 auto;
            border: 1px solid #333;
        }

        .buzzer-btn:active {
            color: white;
            background: var(--buzzer-red);
            box-shadow: 0 0 20px var(--buzzer-red);
            transform: translateY(4px);
            border-color: var(--buzzer-red);
        }

        .buzzer-active {
            animation: buzzerPulse 0.5s infinite;
        }

        @keyframes buzzerPulse {
            0% { box-shadow: 0 0 10px var(--buzzer-red); }
            50% { box-shadow: 0 0 30px var(--buzzer-red); color: white; }
            100% { box-shadow: 0 0 10px var(--buzzer-red); }
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

            .buzzer-container {
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
            .buzzer-container {
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
            .buzzer-btn {
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

    <!-- BUZZER BUTTON -->
    <div class="buzzer-container">
        <button class="buzzer-btn" id="buzzerBtn"
            onmousedown="buzzerStart()" onmouseup="buzzerStop()" onmouseleave="buzzerStop()"
            ontouchstart="buzzerStart()" ontouchend="buzzerStop()" ontouchcancel="buzzerStop()">
            🔔 KLAKSON
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
    let cmdInterval;
    let buzzerInterval;
    let currentPath = '';
    let isPressed = false;
    let isBuzzerPressed = false;

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
  
  stopMotor();
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
  
  server.begin();
  
  Serial.println("Access Point dimulai");
  Serial.print("SSID: "); Serial.println(ssid);
  Serial.print("IP Address: "); Serial.println(WiFi.softAPIP());
  Serial.print("Buzzer PIN: "); Serial.println(BUZZER);
}

void loop() {
  server.handleClient();
  if (millis() - lastCommandTime > TIMEOUT_MS) {
    stopMotor();
  }
}
