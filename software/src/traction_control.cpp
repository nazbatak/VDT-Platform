#include "traction_control.h"
#include <math.h>

// Layer 1: Launch slew-rate limit (+2% per ~25ms tick -> 0 to 100% in ~1.25s)
static constexpr int8_t  MAX_THROTTLE_STEP_UP   = 2;

// Layer 2: Steering-based throttle cap and spin thresholds
static constexpr float   STEERING_DEADBAND_DEG  = 8.0f;
static constexpr int8_t  CORNER_MIN_CAP_PCT     = 50;
static constexpr float   SPIN_YAW_THRESHOLD_DPS = 100.0f; // deg/s
static constexpr float   SPIN_LAT_G_THRESHOLD   = 0.30f;  // g
static constexpr int8_t  SPIN_THROTTLE_CAP_PCT  = 40;

// Layer 3: Longitudinal wheelspin detection and progressive recovery
static constexpr int8_t  SLIP_CHECK_MIN_THR     = 55;
static constexpr float   MIN_EXPECTED_ACCEL_G   = 0.08f;
static constexpr uint8_t SLIP_DETECT_TICKS      = 5;
static constexpr int8_t  SLIP_THROTTLE_CAP_PCT  = 45;
static constexpr int8_t  SLIP_RECOVERY_STEP     = 1;

TractionController::TractionController()
    : _enabled(true),
      _intervening(false),
      _currentThrottle(0),
      _slipCap(100),
      _highThrottleTicks(0) {
}

void TractionController::setEnabled(bool enabled) {
    _enabled = enabled;
    if (!enabled) {
        _intervening = false;
        _slipCap = 100;
        _highThrottleTicks = 0;
    }
}

bool TractionController::isEnabled() const {
    return _enabled;
}

bool TractionController::isIntervening() const {
    return _intervening;
}

void TractionController::reset() {
    _currentThrottle = 0;
    _slipCap = 100;
    _highThrottleTicks = 0;
    _intervening = false;
}

int8_t TractionController::computeThrottleCap(int8_t requestedThrottle, float steeringDeg, const ImuData& imu, bool imuValid) {
    int8_t cap = 100;

    // Scale down max throttle as steering angle increases
    float absSteer = fabsf(steeringDeg);
    if (absSteer > STEERING_DEADBAND_DEG) {
        float ratio = (absSteer - STEERING_DEADBAND_DEG) / (PROTOCOL_STEERING_MAX - STEERING_DEADBAND_DEG);
        if (ratio > 1.0f) ratio = 1.0f;
        cap = (int8_t)(100.0f - ratio * (100.0f - CORNER_MIN_CAP_PCT));
    }

    if (!imuValid) {
        return cap;
    }

    // Cut throttle on cornering oversteer / excessive lateral G
    if (absSteer > STEERING_DEADBAND_DEG &&
        (fabsf(imu.gz) > SPIN_YAW_THRESHOLD_DPS || fabsf(imu.ay) > SPIN_LAT_G_THRESHOLD)) {
        if (cap > SPIN_THROTTLE_CAP_PCT) {
            cap = SPIN_THROTTLE_CAP_PCT;
        }
    }

    // Detect straight-line wheelspin (high throttle without forward acceleration) and recover gradually
    if (absSteer <= STEERING_DEADBAND_DEG && requestedThrottle >= SLIP_CHECK_MIN_THR) {
        if (imu.ax < MIN_EXPECTED_ACCEL_G) {
            if (_highThrottleTicks < 255) _highThrottleTicks++;
        } else {
            _highThrottleTicks = 0;
            if (_slipCap < 100) {
                _slipCap += SLIP_RECOVERY_STEP;
            }
        }

        if (_highThrottleTicks >= SLIP_DETECT_TICKS) {
            _slipCap = SLIP_THROTTLE_CAP_PCT;
        }
    } else {
        _highThrottleTicks = 0;
        if (_slipCap < 100) {
            _slipCap += SLIP_RECOVERY_STEP;
        }
    }

    if (cap > _slipCap) {
        cap = _slipCap;
    }

    return cap;
}

int8_t TractionController::process(int8_t requestedThrottle, float steeringDeg, const ImuData& imu, bool imuValid) {
    // Safety rule: never delay braking, coasting, or reverse
    if (!_enabled || requestedThrottle <= 0) {
        _currentThrottle = requestedThrottle;
        _slipCap = 100;
        _highThrottleTicks = 0;
        _intervening = false;
        return requestedThrottle;
    }

    int8_t cap = computeThrottleCap(requestedThrottle, steeringDeg, imu, imuValid);
    int8_t target = (requestedThrottle > cap) ? cap : requestedThrottle;

    // Limit only upward throttle steps; allow instant throttle reduction
    int8_t nextThrottle = target;
    if (target > _currentThrottle + MAX_THROTTLE_STEP_UP) {
        nextThrottle = _currentThrottle + MAX_THROTTLE_STEP_UP;
    }

    _currentThrottle = nextThrottle;
    _intervening = (nextThrottle < requestedThrottle);
    return nextThrottle;
}
