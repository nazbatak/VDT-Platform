#pragma once
#include <Arduino.h>

// MG995 RC servo driver (angle to 50 Hz PWM conversion)
class ServoDriver {
public:
    explicit ServoDriver(uint8_t pin);

    // Attach 50 Hz 16-bit PWM channel and move servo to center
    void begin();

    // Move servo within software safety bounds (0.0 to 180.0 deg)
    void setAngle(float angleDeg);

    // Move servo to neutral 90 deg position
    void center();

    // Set software min/max angle limits to prevent mechanical binding
    void setLimits(float minDeg, float maxDeg);

    float getAngle() const;

private:
    uint8_t _pin;
    float   _minDeg;
    float   _maxDeg;
    float   _currentAngle;

    uint32_t angleToDuty(float angleDeg) const;
};
