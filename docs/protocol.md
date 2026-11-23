# 📡 RC-Car-ESP32-IMU — Wi-Fi WebSocket Communication Protocol

## 1. Overview
The vehicle uses an **independent ESP32 Wi-Fi Access Point (SoftAP)** paired with an embedded HTTP server and a **full-duplex WebSocket** channel for low-latency Drive-by-Wire control and live IMU telemetry.

* **Wi-Fi SSID:** `VDT-Car` (Open / No password)
* **ESP32 Gateway IP:** `192.168.4.1`
* **HTTP Web Server Port:** `80` (`http://192.168.4.1`)
* **WebSocket Port:** `81` (`ws://192.168.4.1:81/`)

---

## 2. Network Architecture

```
[ Phone / Browser (Safari / Chrome) ]
   │
   ├─► HTTP GET http://192.168.4.1:80  ──► Mobile Touch Cockpit (HTML5/CSS3/JS in PROGMEM)
   │
   └─► WS ws://192.168.4.1:81          ──► Full-Duplex Commands (25 Hz) & Telemetry (20 Hz)
```

---

## 3. Inbound Commands (Client ➔ ESP32)

The web dashboard streams JSON control packets at 25 Hz (every 40 ms) and immediately upon touch-end or button press:

### 3.1 Drive & TCS Control
```json
{
  "throttle": 45,
  "steering": 12.5,
  "tcs": true
}
```
* `throttle`: `int8_t` (`-100` full reverse to `+100` full forward).
* `steering`: `float` (`-29.0` full left to `+29.0` full right degrees).
* `tcs`: `bool` (enables/disables IMU-based Traction Control).

### 3.2 Emergency Stop
```json
{
  "emergency": true
}
```
* Immediately stops both motors (`PWM 0`), centers steering (`0°`), and resets the TCS ramp.

---

## 4. Outbound Telemetry (ESP32 ➔ Client)

Core 0 peeks `imuQueue` and `tcsStatusQueue` every 50 ms (**20 Hz**) and broadcasts:
```json
{
  "type": "telemetry",
  "ax": 0.12,
  "ay": -0.34,
  "az": 0.98,
  "gz": 45.2,
  "temp": 26.8,
  "thr": 50,
  "tcs": true,
  "act": true
}
```
* `ax`, `ay`, `az`: Calibrated acceleration in `g` (`+ax` forward, `+ay` left, `+az` up).
* `gz`: Yaw angular velocity in `deg/s` (`+` left turn, `-` right turn).
* `temp`: MPU6050 die temperature in `°C`.
* `thr`: Actual post-TCS throttle sent to motor driver (`%`).
* `tcs`: Current TCS enabled state.
* `act`: `true` when TCS is actively limiting throttle to prevent wheelspin or oversteer.

---

## 5. Safety & Fail-Safe Mechanisms

1. **WebSocket Disconnect (`WStype_DISCONNECTED`):** Motors stop immediately if the browser closes or drops off Wi-Fi.
2. **Heartbeat Watchdog (`PROTOCOL_HEARTBEAT_TIMEOUT_MS = 500 ms`):** Core 1's `vehicleTask` monitors packet freshness and triggers `emergencyStop()` if no message arrives for 500 ms.
3. **Input Clamping (`DriveCommand::validate()`):** Enforces physical bounds (`[-100, 100]` throttle, `[-29, 29]` steering) regardless of payload content.
