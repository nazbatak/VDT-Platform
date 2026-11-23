#include "../include/wifi_service.h"

WifiService* WifiService::_instance = nullptr;

// Embedded touch cockpit HTML/CSS/JS stored in flash (PROGMEM) to preserve RAM
static const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no, viewport-fit=cover">
  <title>RC-Car-ESP32-IMU — Mobile Cockpit</title>
  <style>
    :root {
      --bg: #0f1117;
      --card: #181b24;
      --accent: #00d26a;
      --danger: #f8312f;
      --text: #e6edf3;
      --text-dim: #8b949e;
      --border: #30363d;
    }
    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
      user-select: none;
      -webkit-user-select: none;
      touch-action: none;
    }
    body {
      background: var(--bg);
      color: var(--text);
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, sans-serif;
      height: 100vh;
      display: flex;
      flex-direction: column;
      overflow: hidden;
      padding: 12px;
    }
    header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding-bottom: 10px;
      border-bottom: 1px solid var(--border);
    }
    h1 {
      font-size: 1.1rem;
      font-weight: 700;
      letter-spacing: 0.5px;
    }
    .badge {
      font-size: 0.75rem;
      padding: 4px 10px;
      border-radius: 12px;
      font-weight: 600;
      background: #331515;
      color: var(--danger);
      transition: all 0.3s;
    }
    .badge.connected {
      background: #0d2818;
      color: var(--accent);
    }
    .metrics {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 10px;
      margin: 10px 0;
    }
    .metric-card {
      background: var(--card);
      border: 1px solid var(--border);
      border-radius: 10px;
      padding: 8px 12px;
      text-align: center;
    }
    .metric-title {
      font-size: 0.7rem;
      color: var(--text-dim);
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }
    .metric-val {
      font-size: 1.4rem;
      font-weight: 800;
      margin-top: 2px;
    }
    .controls {
      flex: 1;
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 12px;
      min-height: 0;
    }
    .stick-zone {
      background: var(--card);
      border: 1px solid var(--border);
      border-radius: 14px;
      position: relative;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      overflow: hidden;
    }
    .stick-label {
      position: absolute;
      top: 10px;
      font-size: 0.75rem;
      font-weight: 600;
      color: var(--text-dim);
      text-transform: uppercase;
    }
    .stick-sub {
      position: absolute;
      bottom: 10px;
      font-size: 0.65rem;
      color: var(--text-dim);
    }
    .puck {
      width: 80px;
      height: 80px;
      border-radius: 50%;
      background: radial-gradient(circle, #2d3342 0%, #1e222d 100%);
      border: 2px solid var(--border);
      box-shadow: 0 4px 15px rgba(0,0,0,0.5);
      position: absolute;
      transition: transform 0.05s ease-out;
      pointer-events: none;
    }
    .puck::after {
      content: '';
      position: absolute;
      width: 14px;
      height: 14px;
      background: var(--accent);
      border-radius: 50%;
      top: 50%;
      left: 50%;
      transform: translate(-50%, -50%);
      box-shadow: 0 0 10px var(--accent);
    }
    .btn-emergency {
      background: var(--danger);
      color: #fff;
      font-weight: 800;
      font-size: 1rem;
      padding: 14px;
      border: none;
      border-radius: 12px;
      margin-top: 10px;
      text-transform: uppercase;
      letter-spacing: 1px;
      box-shadow: 0 4px 15px rgba(248, 49, 47, 0.4);
      cursor: pointer;
    }
    .btn-emergency:active {
      transform: scale(0.98);
      filter: brightness(0.9);
    }
    .imu-bar {
      display: grid;
      grid-template-columns: repeat(4, 1fr);
      gap: 6px;
      margin-bottom: 10px;
    }
    .imu-item {
      background: var(--card);
      border: 1px solid var(--border);
      border-radius: 10px;
      padding: 6px 4px;
      text-align: center;
    }
    .imu-lbl {
      font-size: 0.6rem;
      color: var(--text-dim);
      text-transform: uppercase;
    }
    .imu-val {
      font-size: 0.85rem;
      font-weight: 700;
      font-variant-numeric: tabular-nums;
      color: #58a6ff;
      margin-top: 2px;
    }
    .tcs-row {
      display: flex;
      gap: 10px;
      margin-top: 10px;
    }
    .btn-tcs {
      flex: 1;
      padding: 12px;
      border-radius: 12px;
      border: 1px solid var(--accent);
      background: rgba(0, 210, 106, 0.15);
      color: var(--accent);
      font-weight: 800;
      font-size: 0.85rem;
      cursor: pointer;
    }
    .btn-tcs.off {
      border-color: var(--border);
      background: var(--card);
      color: var(--text-dim);
    }
    .tcs-lamp {
      flex: 1;
      display: flex;
      align-items: center;
      justify-content: center;
      border-radius: 12px;
      border: 1px solid var(--border);
      background: var(--card);
      color: var(--text-dim);
      font-weight: 700;
      font-size: 0.8rem;
      transition: all 0.1s;
    }
    .tcs-lamp.active {
      border-color: #f59e0b;
      background: rgba(245, 158, 11, 0.25);
      color: #fbbf24;
      box-shadow: 0 0 12px rgba(245, 158, 11, 0.45);
    }
  </style>
</head>
<body>
  <header>
    <h1>🚗 RC-Car Cockpit</h1>
    <div id="statusBadge" class="badge">DISCONNECTED</div>
  </header>

  <div class="metrics">
    <div class="metric-card">
      <div class="metric-title">Throttle</div>
      <div id="throttleDisplay" class="metric-val">0%</div>
    </div>
    <div class="metric-card">
      <div class="metric-title">Steering</div>
      <div id="steeringDisplay" class="metric-val">0°</div>
    </div>
  </div>

  <div class="imu-bar">
    <div class="imu-item">
      <div class="imu-lbl">Accel X</div>
      <div id="imuAx" class="imu-val">0.00g</div>
    </div>
    <div class="imu-item">
      <div class="imu-lbl">Lateral Y</div>
      <div id="imuAy" class="imu-val">0.00g</div>
    </div>
    <div class="imu-item">
      <div class="imu-lbl">Yaw (gz)</div>
      <div id="imuGz" class="imu-val">0.0°/s</div>
    </div>
    <div class="imu-item">
      <div class="imu-lbl">Temp</div>
      <div id="imuTemp" class="imu-val">--°C</div>
    </div>
  </div>

  <div class="controls">
    <!-- Left: Vertical throttle joystick -->
    <div id="throttleZone" class="stick-zone">
      <div class="stick-label">Drive / Reverse</div>
      <div id="throttlePuck" class="puck"></div>
      <div class="stick-sub">Forward (+100) / Reverse (-100)</div>
    </div>

    <!-- Right: Horizontal steering joystick -->
    <div id="steeringZone" class="stick-zone">
      <div class="stick-label">Steering</div>
      <div id="steeringPuck" class="puck"></div>
      <div class="stick-sub">Left (-29°) / Right (+29°)</div>
    </div>
  </div>

  <div class="tcs-row">
    <button id="btnTcs" class="btn-tcs">TCS: ON</button>
    <div id="tcsLamp" class="tcs-lamp">TCS STANDBY</div>
  </div>

  <button id="btnEmergency" class="btn-emergency">🚨 EMERGENCY STOP</button>

  <script>
    let ws = null;
    let throttleVal = 0;
    let steeringVal = 0.0;
    let tcsEnabled = true;
    const badge = document.getElementById('statusBadge');
    const throttleDisp = document.getElementById('throttleDisplay');
    const steeringDisp = document.getElementById('steeringDisplay');
    const btnTcs = document.getElementById('btnTcs');
    const tcsLamp = document.getElementById('tcsLamp');
    const throttlePuck = document.getElementById('throttlePuck');
    const steeringPuck = document.getElementById('steeringPuck');
    const throttleZone = document.getElementById('throttleZone');
    const steeringZone = document.getElementById('steeringZone');

    function connectWS() {
      const wsUrl = 'ws://' + window.location.hostname + ':81/';
      ws = new WebSocket(wsUrl);

      ws.onopen = () => {
        badge.textContent = 'CONNECTED';
        badge.className = 'badge connected';
      };

      ws.onclose = () => {
        badge.textContent = 'DISCONNECTED';
        badge.className = 'badge';
        setTimeout(connectWS, 1000);
      };

      ws.onerror = (err) => {
        ws.close();
      };

      ws.onmessage = (event) => {
        try {
          const data = JSON.parse(event.data);
          if (data.type === 'telemetry') {
            document.getElementById('imuAx').textContent = (data.ax >= 0 ? '+' : '') + data.ax.toFixed(2) + 'g';
            document.getElementById('imuAy').textContent = (data.ay >= 0 ? '+' : '') + data.ay.toFixed(2) + 'g';
            document.getElementById('imuGz').textContent = (data.gz >= 0 ? '+' : '') + data.gz.toFixed(1) + '°/s';
            document.getElementById('imuTemp').textContent = data.temp.toFixed(1) + '°C';
            if (data.act) {
              tcsLamp.textContent = '⚡ TCS ACTIVE (' + data.thr + '%)';
              tcsLamp.className = 'tcs-lamp active';
            } else {
              tcsLamp.textContent = data.tcs ? 'TCS READY' : 'TCS OFF';
              tcsLamp.className = 'tcs-lamp';
            }
          }
        } catch (e) {}
      };
    }

    function sendCommand() {
      if (ws && ws.readyState === WebSocket.OPEN) {
        const payload = JSON.stringify({
          throttle: Math.round(throttleVal),
          steering: parseFloat(steeringVal.toFixed(1)),
          tcs: tcsEnabled
        });
        ws.send(payload);
      }
    }

    // Periodic heartbeat & command stream (25 Hz / 40 ms)
    setInterval(sendCommand, 40);

    // Throttle joystick (vertical)
    let throttleActive = false;
    let throttleTouchId = null;

    function handleThrottleTouch(e) {
      const rect = throttleZone.getBoundingClientRect();
      const centerY = rect.top + rect.height / 2;
      const touch = Array.from(e.touches).find(t => t.identifier === throttleTouchId) || e.touches[0];
      if (!touch) return;

      const dy = touch.clientY - centerY;
      const maxDy = rect.height / 2 - 40;
      const clampedDy = Math.max(-maxDy, Math.min(maxDy, dy));

      // Invert Y-axis because screen coordinates grow downward
      throttleVal = -Math.round((clampedDy / maxDy) * 100);
      throttlePuck.style.transform = `translateY(${clampedDy}px)`;
      throttleDisp.textContent = (throttleVal > 0 ? '+' : '') + throttleVal + '%';
    }

    throttleZone.addEventListener('touchstart', (e) => {
      throttleActive = true;
      throttleTouchId = e.changedTouches[0].identifier;
      handleThrottleTouch(e);
    });

    throttleZone.addEventListener('touchmove', (e) => {
      if (throttleActive) handleThrottleTouch(e);
    });

    function resetThrottle() {
      throttleActive = false;
      throttleTouchId = null;
      throttleVal = 0;
      throttlePuck.style.transform = `translateY(0px)`;
      throttleDisp.textContent = '0%';
      sendCommand();
    }
    throttleZone.addEventListener('touchend', resetThrottle);
    throttleZone.addEventListener('touchcancel', resetThrottle);

    // Steering joystick (horizontal)
    let steeringActive = false;
    let steeringTouchId = null;

    function handleSteeringTouch(e) {
      const rect = steeringZone.getBoundingClientRect();
      const centerX = rect.left + rect.width / 2;
      const touch = Array.from(e.touches).find(t => t.identifier === steeringTouchId) || e.touches[0];
      if (!touch) return;

      const dx = touch.clientX - centerX;
      const maxDx = rect.width / 2 - 40;
      const clampedDx = Math.max(-maxDx, Math.min(maxDx, dx));

      steeringVal = parseFloat(((clampedDx / maxDx) * 29.0).toFixed(1));
      steeringPuck.style.transform = `translateX(${clampedDx}px)`;
      steeringDisp.textContent = (steeringVal > 0 ? '+' : '') + steeringVal + '°';
    }

    steeringZone.addEventListener('touchstart', (e) => {
      steeringActive = true;
      steeringTouchId = e.changedTouches[0].identifier;
      handleSteeringTouch(e);
    });

    steeringZone.addEventListener('touchmove', (e) => {
      if (steeringActive) handleSteeringTouch(e);
    });

    function resetSteering() {
      steeringActive = false;
      steeringTouchId = null;
      steeringVal = 0.0;
      steeringPuck.style.transform = `translateX(0px)`;
      steeringDisp.textContent = '0°';
      sendCommand();
    }
    steeringZone.addEventListener('touchend', resetSteering);
    steeringZone.addEventListener('touchcancel', resetSteering);

    // TCS toggle
    btnTcs.addEventListener('click', () => {
      tcsEnabled = !tcsEnabled;
      btnTcs.textContent = tcsEnabled ? 'TCS: ON' : 'TCS: OFF';
      btnTcs.className = tcsEnabled ? 'btn-tcs' : 'btn-tcs off';
      sendCommand();
    });

    // Emergency stop
    document.getElementById('btnEmergency').addEventListener('click', () => {
      resetThrottle();
      resetSteering();
      if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify({ emergency: true }));
      }
    });

    connectWS();
  </script>
</body>
</html>
)rawliteral";

WifiService::WifiService()
    : _commandQueue(nullptr),
      _imuQueue(nullptr),
      _tcsStatusQueue(nullptr),
      _server(80),
      _webSocket(81),
      _connected(false),
      _lastHeartbeatMs(0),
      _lastTelemetryMs(0),
      _activeCommand{0, 0.0f, true} {
    _instance = this;
}

void WifiService::webSocketEventWrapper(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    if (_instance) {
        _instance->handleWebSocketEvent(num, type, payload, length);
    }
}

void WifiService::begin(QueueHandle_t commandQueue, QueueHandle_t imuQueue,
                        QueueHandle_t tcsStatusQueue, const char* ssid, const char* password) {
    _commandQueue = commandQueue;
    _imuQueue = imuQueue;
    _tcsStatusQueue = tcsStatusQueue;
    Serial.printf("\n[Wi-Fi] Access Point olusturuluyor: '%s'...\n", ssid);

    // 1. Start Wi-Fi SoftAP
    WiFi.mode(WIFI_AP);
    if (password && strlen(password) >= 8) {
        WiFi.softAP(ssid, password);
    } else {
        WiFi.softAP(ssid);
    }

    IPAddress IP = WiFi.softAPIP();
    Serial.printf("[Wi-Fi] AP Ready! IP: %s\n", IP.toString().c_str());

    // 2. Start HTTP web server (:80)
    _server.on("/", HTTP_GET, [this]() { handleRoot(); });
    _server.onNotFound([this]() { handleNotFound(); });
    _server.begin();

    // 3. Start WebSocket server (:81)
    _webSocket.begin();
    _webSocket.onEvent(webSocketEventWrapper);
    Serial.println("[WebSocket] Server started on port 81.");
}

void WifiService::loop() {
    _server.handleClient();
    _webSocket.loop();
    publishTelemetry();
}

void WifiService::publishTelemetry() {
    if (!_connected || _imuQueue == nullptr) return;

    uint32_t now = millis();
    if ((now - _lastTelemetryMs) < 50) return; // 20 Hz broadcast limit
    _lastTelemetryMs = now;

    ImuData data;
    if (xQueuePeek(_imuQueue, &data, 0) != pdPASS) return;

    TcsStatus tcs = {0, true, false};
    if (_tcsStatusQueue != nullptr) {
        xQueuePeek(_tcsStatusQueue, &tcs, 0);
    }

    char jsonBuf[160];
    snprintf(jsonBuf, sizeof(jsonBuf),
             "{\"type\":\"telemetry\",\"ax\":%.2f,\"ay\":%.2f,\"az\":%.2f,\"gz\":%.1f,\"temp\":%.1f,\"thr\":%d,\"tcs\":%s,\"act\":%s}",
             data.ax, data.ay, data.az, data.gz, data.temp,
             tcs.appliedThrottle,
             tcs.enabled ? "true" : "false",
             tcs.intervening ? "true" : "false");

    _webSocket.broadcastTXT(jsonBuf);
}

bool WifiService::isConnected() const {
    return _connected;
}

bool WifiService::isHeartbeatTimedOut() const {
    if (!_connected) return false;
    return (millis() - _lastHeartbeatMs) > PROTOCOL_HEARTBEAT_TIMEOUT_MS;
}

void WifiService::refreshHeartbeat() {
    _lastHeartbeatMs = millis();
}

void WifiService::handleRoot() {
    _server.send_P(200, "text/html", DASHBOARD_HTML);
}

void WifiService::handleNotFound() {
    _server.send(404, "text/plain", "404: Not Found");
}

void WifiService::handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_CONNECTED: {
            _connected = true;
            _lastHeartbeatMs = millis();
            IPAddress ip = _webSocket.remoteIP(num);
            Serial.printf("[WebSocket] [#%u] Istemci baglandi: %s\n", num, ip.toString().c_str());
            break;
        }

        case WStype_DISCONNECTED: {
            _connected = false;
            Serial.printf("[WebSocket] [#%u] Client disconnected. Stopping motors.\n", num);

            _activeCommand.throttle = 0;
            _activeCommand.steering = 0.0f;
            if (_commandQueue != nullptr) {
                xQueueOverwrite(_commandQueue, &_activeCommand);
            }
            break;
        }

        case WStype_TEXT: {
            refreshHeartbeat();

            StaticJsonDocument<128> doc;
            DeserializationError error = deserializeJson(doc, payload, length);
            if (error) {
                return;
            }

            if (doc["emergency"] == true) {
                _activeCommand.throttle = 0;
                _activeCommand.steering = 0.0f;
                if (_commandQueue != nullptr) {
                    xQueueOverwrite(_commandQueue, &_activeCommand);
                }
                Serial.println("[WebSocket] Emergency stop triggered.");
                return;
            }

            if (doc.containsKey("throttle")) {
                _activeCommand.throttle = doc["throttle"].as<int8_t>();
            }
            if (doc.containsKey("steering")) {
                _activeCommand.steering = doc["steering"].as<float>();
            }
            if (doc.containsKey("tcs")) {
                _activeCommand.tcsEnabled = doc["tcs"].as<bool>();
            }

            _activeCommand.validate();

            if (_commandQueue != nullptr) {
                xQueueOverwrite(_commandQueue, &_activeCommand);
            }
            break;
        }

        default:
            break;
    }
}
