# ESP32 DevKit V1 Pinout

> Firmware definitions: `software/include/pins.h`. Keep both files synchronized.

## Active Pins

| Module | Signal | ESP32 GPIO | Type | Function |
|---|---|---|---|---|
| TB6612FNG | PWMA | 25 | PWM | Right Motor Speed (PWM) |
| TB6612FNG | AIN1 | 26 | GPIO | Right Motor Direction 1 |
| TB6612FNG | AIN2 | 27 | GPIO | Right Motor Direction 2 |
| TB6612FNG | PWMB | 14 | PWM | Left Motor Speed (PWM) |
| TB6612FNG | BIN1 | 32 | GPIO | Left Motor Direction 1 (avoids GPIO 12 strapping) |
| TB6612FNG | BIN2 | 13 | GPIO | Left Motor Direction 2 |
| TB6612FNG | STBY | 33 | GPIO | Motor Driver Standby / Enable |
| MG995 Servo | Signal | 4 | PWM | Front Steering Angle (50 Hz PWM) |
| MPU6050 | SDA | 21 | I2C | I2C Serial Data |
| MPU6050 | SCL | 22 | I2C | I2C Serial Clock |

## Pins to Avoid on ESP32

- **Strapping pins:** 0, 2, 5, 12, 15
- **SPI Flash pins:** 6–11
- **Input-only pins:** 34, 35, 36, 39
- **USB Serial (UART0):** 1, 3
