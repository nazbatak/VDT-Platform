#pragma once

// Single source of truth for ESP32 GPIO pin assignments (keep docs/pinout.md in sync).

// TB6612FNG Motor Driver (Channel A -> Right Motor, Channel B -> Left Motor)
constexpr int PIN_MOTOR_RIGHT_PWM = 25;
constexpr int PIN_MOTOR_RIGHT_IN1 = 26;
constexpr int PIN_MOTOR_RIGHT_IN2 = 27;

constexpr int PIN_MOTOR_LEFT_PWM  = 14;
constexpr int PIN_MOTOR_LEFT_IN1  = 32;  // Avoid GPIO 12 (strapping) and 16 (missing on DevKit V1)
constexpr int PIN_MOTOR_LEFT_IN2  = 13;

constexpr int PIN_MOTOR_STBY      = 33;

// Schematic pin aliases
constexpr int PIN_MOTOR_PWMA = PIN_MOTOR_RIGHT_PWM;
constexpr int PIN_MOTOR_AIN1 = PIN_MOTOR_RIGHT_IN1;
constexpr int PIN_MOTOR_AIN2 = PIN_MOTOR_RIGHT_IN2;

constexpr int PIN_MOTOR_PWMB = PIN_MOTOR_LEFT_PWM;
constexpr int PIN_MOTOR_BIN1 = PIN_MOTOR_LEFT_IN1;
constexpr int PIN_MOTOR_BIN2 = PIN_MOTOR_LEFT_IN2;

// MG995 Steering Servo
constexpr int PIN_SERVO_STEERING = 4;

// MPU6050 6-DOF IMU (I2C)
constexpr int PIN_IMU_SDA = 21;
constexpr int PIN_IMU_SCL = 22;

