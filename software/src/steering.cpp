#include "steering.h"

// Mechanical limits from docs/vehicle_params.md (4 deg safety margin from hard stops)
static const float MAX_WHEEL_DEG = 29.0f;
static const float MIN_SERVO_DEG = 38.0f;
static const float MAX_SERVO_DEG = 125.0f;

// Calibration lookup table: effective wheel angle (left < 0) -> servo angle (must stay sorted)
static const float WHEEL_TABLE[] = { -29.0f, -18.0f, -9.5f, -5.5f,  0.0f, 13.0f, 20.5f,  26.0f,  32.5f };
static const float SERVO_TABLE[] = {  38.0f,  50.0f, 62.0f, 74.0f, 84.0f, 94.0f, 104.0f, 114.0f, 125.0f };
static const int   TABLE_SIZE    = sizeof(WHEEL_TABLE) / sizeof(WHEEL_TABLE[0]);

Steering::Steering(ServoDriver& servoDriver)
    : _servo(servoDriver), _currentAngle(0.0f) {}

void Steering::begin() {
    _servo.begin();
    _servo.setLimits(MIN_SERVO_DEG, MAX_SERVO_DEG);
    center();
}

void Steering::setAngle(float wheelDeg) {
    wheelDeg = constrain(wheelDeg, -MAX_WHEEL_DEG, MAX_WHEEL_DEG);
    _currentAngle = wheelDeg;

    _servo.setAngle(calculateServoDeg(wheelDeg));
}

void Steering::center() {
    setAngle(0.0f);
}

float Steering::getAngle() const {
    return _currentAngle;
}

// Piecewise linear interpolation between calibrated wheel/servo points
float Steering::calculateServoDeg(float wheelDeg) const {
    for (int i = 0; i < TABLE_SIZE - 1; i++) {
        if (wheelDeg <= WHEEL_TABLE[i + 1]) {
            float t = (wheelDeg - WHEEL_TABLE[i]) / (WHEEL_TABLE[i + 1] - WHEEL_TABLE[i]);
            return SERVO_TABLE[i] + t * (SERVO_TABLE[i + 1] - SERVO_TABLE[i]);
        }
    }

    return SERVO_TABLE[TABLE_SIZE - 1];
}