#pragma once
#include <Arduino.h>

// Drive-by-Wire communication protocol limits and FreeRTOS queue payloads.

constexpr int8_t   PROTOCOL_THROTTLE_MIN = -100;   // Max reverse (%)
constexpr int8_t   PROTOCOL_THROTTLE_MAX =  100;   // Max forward (%)
constexpr float    PROTOCOL_STEERING_MIN = -29.0f; // Full left (deg)
constexpr float    PROTOCOL_STEERING_MAX =  29.0f; // Full right (deg)
constexpr uint32_t PROTOCOL_HEARTBEAT_TIMEOUT_MS = 500;

// Incoming drive command from WebSocket client
struct DriveCommand {
    int8_t throttle;    // -100 to +100 (%)
    float  steering;    // -29.0 to +29.0 (deg)
    bool   tcsEnabled;  // Traction Control enabled flag

    // Clamp values to physical safety limits
    void validate() {
        if (throttle < PROTOCOL_THROTTLE_MIN) throttle = PROTOCOL_THROTTLE_MIN;
        if (throttle > PROTOCOL_THROTTLE_MAX) throttle = PROTOCOL_THROTTLE_MAX;
        if (steering < PROTOCOL_STEERING_MIN) steering = PROTOCOL_STEERING_MIN;
        if (steering > PROTOCOL_STEERING_MAX) steering = PROTOCOL_STEERING_MAX;
    }
};

// Outbound Traction Control telemetry snapshot
struct TcsStatus {
    int8_t appliedThrottle;  // Final throttle sent to motors (%)
    bool   enabled;
    bool   intervening;      // True when TCS actively limits throttle
};
