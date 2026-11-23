#pragma once
#include <Arduino.h>
#include <Wire.h>

// Standard SI IMU telemetry packet
struct ImuData {
    float ax;              // Longitudinal acceleration (g)
    float ay;              // Lateral acceleration (g)
    float az;              // Vertical acceleration (g)
    float gx;              // Roll rate (deg/s)
    float gy;              // Pitch rate (deg/s)
    float gz;              // Yaw rate (deg/s)
    float temp;            // Die temperature (C)
    uint32_t timestampMs;
};

// MPU6050 6-DOF IMU driver over I2C (initialization, calibration, burst read)
class ImuDriver {
public:
    ImuDriver(uint8_t sdaPin, uint8_t sclPin, uint8_t address = 0x68);

    // Initialize I2C, wake MPU6050, configure +-4g / +-500 deg/s, and calibrate gyro
    bool begin(uint32_t i2cClock = 400000);

    // Burst-read 14 bytes and convert to SI units
    bool read();

    // Measure stationary gyro bias offset
    void calibrateGyro(uint16_t samples = 200);

    const ImuData& getData() const;
    bool isConnected() const;
    uint32_t getErrorCount() const;

private:
    uint8_t   _sdaPin;
    uint8_t   _sclPin;
    uint8_t   _address;
    bool      _connected;
    uint32_t  _errorCount;
    float     _gyroBiasX;
    float     _gyroBiasY;
    float     _gyroBiasZ;
    ImuData   _data;

    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegisters(uint8_t reg, uint8_t* buffer, size_t length);
};
