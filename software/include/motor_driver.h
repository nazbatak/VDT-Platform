#pragma once
#include <Arduino.h>

enum MotorId {
    MOTOR_LEFT,
    MOTOR_RIGHT
};

enum MotorDirection {
    DIR_FORWARD,
    DIR_BACKWARD,
    DIR_STOP
};

struct MotorPins {
    uint8_t pwm;
    uint8_t in1;
    uint8_t in2;
};

// Low-level TB6612FNG dual DC motor driver
class MotorDriver {
public:
    MotorDriver(MotorPins leftPins, MotorPins rightPins, uint8_t stbyPin);

    // Configure GPIO pins and attach hardware PWM channels
    void begin();

    // Set direction and PWM speed (0-255) for a motor
    void setMotor(MotorId id, MotorDirection dir, uint8_t speed);

    // Stop both motors immediately
    void stopAll();

    // Software polarity inversion to fix reversed motor wiring
    void invertMotor(MotorId id, bool invert);

private:
    MotorPins _left;
    MotorPins _right;
    uint8_t _stby;
    bool _leftInverted;
    bool _rightInverted;

    void writeMotorHardware(const MotorPins& pins, uint8_t pwmChannel, MotorDirection dir, uint8_t speed, bool isInverted);
};
