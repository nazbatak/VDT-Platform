#include "servo_driver.h"

static const uint32_t SERVO_FREQ    = 50;     // 50 Hz (20 ms period)
static const uint8_t  SERVO_RES     = 16;     // 16-bit resolution
static const uint8_t  PWM_CH_SERVO  = 2;      // Uses Timer 1 (motors use channels 0-1 on Timer 0)
static const uint32_t PERIOD_US     = 20000;
static const uint16_t MIN_PULSE_US  = 500;    // 0 deg pulse width (us)
static const uint16_t MAX_PULSE_US  = 2500;   // 180 deg pulse width (us)

ServoDriver::ServoDriver(uint8_t pin)
    : _pin(pin),
      _minDeg(0.0f),
      _maxDeg(180.0f),
      _currentAngle(90.0f) {}

void ServoDriver::begin() {
    ledcSetup(PWM_CH_SERVO, SERVO_FREQ, SERVO_RES);
    ledcAttachPin(_pin, PWM_CH_SERVO);
    center();
}

void ServoDriver::setLimits(float minDeg, float maxDeg) {
    _minDeg = minDeg;
    _maxDeg = maxDeg;
}

void ServoDriver::setAngle(float angleDeg) {
    angleDeg = constrain(angleDeg, _minDeg, _maxDeg);
    _currentAngle = angleDeg;

    ledcWrite(PWM_CH_SERVO, angleToDuty(angleDeg));
}

void ServoDriver::center() {
    setAngle(90.0f);
}

float ServoDriver::getAngle() const {
    return _currentAngle;
}

uint32_t ServoDriver::angleToDuty(float angleDeg) const {
    float pulseUs = MIN_PULSE_US + (angleDeg / 180.0f) * (MAX_PULSE_US - MIN_PULSE_US);
    return (uint32_t)((uint64_t)pulseUs * ((1UL << SERVO_RES) - 1) / PERIOD_US);
}
