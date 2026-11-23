# 🗺️ RC-Car-ESP32-IMU Roadmap

## ✅ Version 1.0 — Completed Scope

V1.0 focused on building a complete, modular, wireless Drive-by-Wire testbed from scratch:

- [x] **Power & Propulsion:** 2S Li-Ion + BMS + LM2596 5 V regulation, TB6612FNG dual DC motor driver (`20 kHz` PWM), software polarity inversion.
- [x] **Ackermann Steering:** 3D-printed front axle/tie-rod assembly, MG995 servo driver (`50 Hz` 16-bit PWM), piecewise linear calibration table (`±29°` symmetric wheel travel).
- [x] **Standalone Wi-Fi Cockpit:** ESP32 SoftAP (`VDT-Car`), embedded `PROGMEM` HTML5/CSS3/JS touch dashboard, full-duplex WebSocket streaming (`25 Hz` commands / `20 Hz` telemetry).
- [x] **Dual-Core FreeRTOS & Fail-Safe:** Core 0 networking isolated from Core 1 `vehicleTask` and `imuTask`; 500 ms heartbeat watchdog + disconnect cutoff.
- [x] **6-DOF IMU & Traction Control:** Custom MPU6050 I2C driver (`±4g`, `±500°/s`, 42 Hz DLPF, startup gyro calibration) feeding a 3-layer IMU-based Traction Control System (`TractionController`) with live mobile `TCS: ON/OFF` A/B testing.

---

## 🔮 Future Direction (V2.0 Lessons & Next-Gen Platform)

Key architectural lessons learned from building V1.0 that will shape our next-generation platform:

1. **Closed-Loop Wheel Speed (Encoders):** Adding quadrature wheel encoders enables true ground-speed estimation (`km/h`), PID cruise control, and genuine closed-loop Torque Vectoring / E-Diff.
2. **Brushless Drivetrain & Custom ESC / STM32 ECU:** Moving from dual brushed DC motors + TB6612FNG to higher-power brushless propulsion controlled by an STM32 MCU over **CAN-Bus**.
3. **Battery Telemetry & Custom PCB:** Designing a dedicated 2-layer KiCad mainboard with voltage/current sensing, reverse-polarity protection, and clean power distribution instead of breadboard/jumper wiring.
4. **Autonomous Perception:** Adding a ToF/LiDAR sensor or companion computer (Raspberry Pi / Jetson + Camera) on top of the low-level ECU for Autonomous Emergency Braking (AEB) and vision-based navigation.
