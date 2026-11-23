#pragma once
#include <Arduino.h>
#include "motor_driver.h"
#include "steering.h"

// Vehicle Control Unit (VCU): unified interface for propulsion and steering
class VehicleController {
public:
    VehicleController(MotorDriver& motors, Steering& steering);

    // Initialize motor and steering subsystems in a safe stopped state
    void begin();

    // Drive vehicle with throttle (-100 to +100 %) and steering (-29 to +29 deg)
    void drive(int throttlePct, float steeringDeg);

    // Cut motor power and center steering immediately
    void emergencyStop();

    int getThrottle() const;
    float getSteering() const;

private:
    MotorDriver& _motors;
    Steering&    _steering;
    int          _currentThrottle;
    float        _currentSteering;

    uint8_t throttleToPwm(int throttlePct) const;
};
