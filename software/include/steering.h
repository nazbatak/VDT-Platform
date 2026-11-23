#pragma once
#include <Arduino.h>
#include "servo_driver.h"

// Steering abstraction layer: maps target wheel angle (deg) to calibrated servo angle
class Steering {
public:
    explicit Steering(ServoDriver& servoDriver);

    // Initialize servo, apply mechanical limits, and center steering
    void begin();

    // Set target effective wheel angle (-29.0 to +29.0 deg)
    void setAngle(float wheelDeg);

    // Reset steering to straight ahead (0 deg)
    void center();

    float getAngle() const;

private:
    float calculateServoDeg(float wheelDeg) const;

    ServoDriver& _servo;
    float _currentAngle;
};

