#include "vehicle_controller.h"

VehicleController::VehicleController(MotorDriver& motors, Steering& steering)
    : _motors(motors),
      _steering(steering),
      _currentThrottle(0),
      _currentSteering(0.0f) {}

void VehicleController::begin() {
    _motors.begin();
    _steering.begin();
    emergencyStop();
}

void VehicleController::drive(int throttlePct, float steeringDeg) {
    throttlePct = constrain(throttlePct, -100, 100);
    _currentThrottle = throttlePct;

    _steering.setAngle(steeringDeg);
    _currentSteering = _steering.getAngle();

    if (throttlePct > 0) {
        uint8_t pwm = throttleToPwm(throttlePct);
        _motors.setMotor(MOTOR_LEFT, DIR_FORWARD, pwm);
        _motors.setMotor(MOTOR_RIGHT, DIR_FORWARD, pwm);
    } else if (throttlePct < 0) {
        uint8_t pwm = throttleToPwm(throttlePct);
        _motors.setMotor(MOTOR_LEFT, DIR_BACKWARD, pwm);
        _motors.setMotor(MOTOR_RIGHT, DIR_BACKWARD, pwm);
    } else {
        _motors.stopAll();
    }
}

void VehicleController::emergencyStop() {
    _motors.stopAll();
    _steering.center();
    _currentThrottle = 0;
    _currentSteering = 0.0f;
}

int VehicleController::getThrottle() const {
    return _currentThrottle;
}

float VehicleController::getSteering() const {
    return _currentSteering;
}

uint8_t VehicleController::throttleToPwm(int throttlePct) const {
    return (uint8_t)((abs(throttlePct) * 255UL) / 100UL);
}
