# 📐 RC-Car-ESP32-IMU — Vehicle & Steering Parameters

> Measured calibration data for Ackermann front steering geometry and servo limits (`software/src/steering.cpp`).

---

## 1. Servo Calibration

| Parameter | Servo Angle (°) | Notes |
|---|---|---|
| **Center (straight)** | **84** | Wheels aligned straight ahead |
| **Hard left stop** | **34** | Chassis mechanical stop |
| **Hard right stop** | **129** | Chassis mechanical stop |
| **Left travel** | 50 | `84 − 34` |
| **Right travel** | 45 | `129 − 84` |

### Software Safety Limits (4° Margin)
To prevent servo stall current spikes against mechanical hard stops:
* **Left limit:** `38°`
* **Right limit:** `125°`
* **Center trim:** `84°`

---

## 2. Servo-to-Wheel Calibration Lookup Table

| Servo (°) | Servo − Center (°) | Left Wheel (°) | Right Wheel (°) | Effective Avg Angle (°) | State |
|---|---|---|---|---|---|
| 38 | −46 | −45 | −13 | **−29.0** | Max Left (Inner 45°, Outer 13°) |
| 50 | −34 | −26 | −10 | **−18.0** | Left Turn |
| 62 | −22 | −14 | −5 | **−9.5** | Left Turn |
| 74 | −10 | −9 | −2 | **−5.5** | Slight Left |
| 84 | 0 | 0 | 0 | **0.0** | Straight Ahead |
| 94 | +10 | +8 | +18 | **+13.0** | Slight Right |
| 104 | +20 | +12 | +29 | **+20.5** | Right Turn |
| 114 | +30 | +17 | +35 | **+26.0** | Right Turn |
| 125 | +41 | +20 | +45 | **+32.5** | Max Right (Inner 45°, Outer 20°) |

Firmware clamps effective steering to **`±29.0°`** for symmetric turning response.

---

## 3. Chassis & Ackermann Geometry

| Parameter | Value (mm) |
|---|---|
| **Track Width ($T$)** — Front wheel center-to-center | **165 mm** |
| **Wheelbase ($L$)** — Front-to-rear axle distance | **253 mm** |
| **$T / L$ Ratio** | **0.652** |

**Ideal Ackermann Condition:**
$$\cot(\delta_{\text{outer}}) - \cot(\delta_{\text{inner}}) = \frac{T}{L} = 0.652$$

Both left and right turns exhibit clear Pro-Ackermann geometry (inner wheel turns 45° while outer turns 13°–20°), reducing tire scrub in tight turns.
