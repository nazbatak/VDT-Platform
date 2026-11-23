#pragma once
#include <Arduino.h>
#include "imu_driver.h"
#include "protocol.h"

// IMU-based Traction Control System (TCS): limits launch wheelspin and cornering oversteer
class TractionController {
public:
    TractionController();

    void setEnabled(bool enabled);
    bool isEnabled() const;

    // Filter requested throttle using steering angle and live IMU readings
    int8_t process(int8_t requestedThrottle, float steeringDeg, const ImuData& imu, bool imuValid);

    bool isIntervening() const;
    void reset();

private:
    bool    _enabled;
    bool    _intervening;
    int8_t  _currentThrottle;
    int8_t  _slipCap;
    uint8_t _highThrottleTicks;

    int8_t computeThrottleCap(int8_t requestedThrottle, float steeringDeg, const ImuData& imu, bool imuValid);
};
