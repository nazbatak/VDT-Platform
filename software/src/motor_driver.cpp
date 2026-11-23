#include "motor_driver.h"
#include <esp_arduino_version.h>

// 20 kHz PWM avoids audible motor whine; 8-bit resolution (0-255)
static const uint32_t PWM_FREQ     = 20000;
static const uint8_t  PWM_RES      = 8;
static const uint8_t  PWM_CH_LEFT  = 0;
static const uint8_t  PWM_CH_RIGHT = 1;

MotorDriver::MotorDriver(MotorPins leftPins, MotorPins rightPins, uint8_t stbyPin)
    : _left(leftPins), _right(rightPins), _stby(stbyPin),
      _leftInverted(false), _rightInverted(false) {}

void MotorDriver::begin() {
    // Keep driver in standby during setup to prevent startup twitch
    pinMode(_stby, OUTPUT);
    digitalWrite(_stby, LOW);

    pinMode(_left.in1, OUTPUT);
    pinMode(_left.in2, OUTPUT);
    pinMode(_right.in1, OUTPUT);
    pinMode(_right.in2, OUTPUT);

    ledcSetup(PWM_CH_LEFT, PWM_FREQ, PWM_RES);
    ledcAttachPin(_left.pwm, PWM_CH_LEFT);
    ledcSetup(PWM_CH_RIGHT, PWM_FREQ, PWM_RES);
    ledcAttachPin(_right.pwm, PWM_CH_RIGHT);

    stopAll();
    digitalWrite(_stby, HIGH);
}

void MotorDriver::invertMotor(MotorId id, bool invert) {
    if (id == MOTOR_LEFT) {
        _leftInverted = invert;
    } else {
        _rightInverted = invert;
    }
}

void MotorDriver::setMotor(MotorId id, MotorDirection dir, uint8_t speed) {
    if (id == MOTOR_LEFT) {
        writeMotorHardware(_left, PWM_CH_LEFT, dir, speed, _leftInverted);
    } else {
        writeMotorHardware(_right, PWM_CH_RIGHT, dir, speed, _rightInverted);
    }
}

void MotorDriver::stopAll() {
    setMotor(MOTOR_LEFT, DIR_STOP, 0);
    setMotor(MOTOR_RIGHT, DIR_STOP, 0);
}

void MotorDriver::writeMotorHardware(const MotorPins& pins, uint8_t pwmChannel, MotorDirection dir, uint8_t speed, bool isInverted) {
    if (isInverted && dir != DIR_STOP) {
        dir = (dir == DIR_FORWARD) ? DIR_BACKWARD : DIR_FORWARD;
    }

    switch (dir) {
        case DIR_FORWARD:
            digitalWrite(pins.in1, HIGH);
            digitalWrite(pins.in2, LOW);
            break;
        case DIR_BACKWARD:
            digitalWrite(pins.in1, LOW);
            digitalWrite(pins.in2, HIGH);
            break;
        case DIR_STOP:
        default:
            digitalWrite(pins.in1, LOW);
            digitalWrite(pins.in2, LOW);
            speed = 0;
            break;
    }

#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
    ledcWrite(pins.pwm, speed);
#else
    ledcWrite(pwmChannel, speed);
#endif
}
