#include <Arduino.h>
#include "pins.h"
#include "motor_driver.h"
#include "servo_driver.h"
#include "steering.h"
#include "vehicle_controller.h"
#include "protocol.h"
#include "wifi_service.h"
#include "imu_driver.h"
#include "traction_control.h"

// Hardware abstraction instances
MotorPins leftPins = {
    .pwm = PIN_MOTOR_LEFT_PWM,
    .in1 = PIN_MOTOR_LEFT_IN1,
    .in2 = PIN_MOTOR_LEFT_IN2
};
MotorPins rightPins = {
    .pwm = PIN_MOTOR_RIGHT_PWM,
    .in1 = PIN_MOTOR_RIGHT_IN1,
    .in2 = PIN_MOTOR_RIGHT_IN2
};
MotorDriver motors(leftPins, rightPins, PIN_MOTOR_STBY);

ServoDriver       servo(PIN_SERVO_STEERING);
Steering          steering(servo);
VehicleController vehicle(motors, steering);
ImuDriver         imu(PIN_IMU_SDA, PIN_IMU_SCL);
TractionController tcs;

// FreeRTOS inter-core queues & Wi-Fi service
QueueHandle_t commandQueue   = nullptr;
QueueHandle_t imuQueue       = nullptr;
QueueHandle_t tcsStatusQueue = nullptr;
WifiService   wifiService;

// Core 1 IMU task (Priority 1): samples MPU6050 at 100 Hz without blocking vehicleTask
void imuTask(void* pvParameters) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(10);
    uint32_t lastPrintMs = 0;

    while (true) {
        if (imu.isConnected()) {
            if (imu.read() && imuQueue != nullptr) {
                xQueueOverwrite(imuQueue, &imu.getData());
            }

            if (millis() - lastPrintMs >= 100) {
                lastPrintMs = millis();
                const ImuData& d = imu.getData();
                Serial.printf("[IMU] Acc(g): [%+0.2f, %+0.2f, %+0.2f] | Gyro(deg/s): [%+05.1f, %+05.1f, %+05.1f] | Temp: %.1fC\n",
                              d.ax, d.ay, d.az, d.gx, d.gy, d.gz, d.temp);
            }
        }

        vTaskDelayUntil(&lastWakeTime, period);
    }
}

// Core 1 vehicle control task (Priority 2): single owner of motor/servo hardware and fail-safe watchdog
void vehicleTask(void* pvParameters) {
    DriveCommand activeCmd = {0, 0.0f, true};
    DriveCommand incomingCmd = {0, 0.0f, true};
    ImuData latestImu = {0};
    bool wasTimedOut = false;

    while (true) {
        if (wifiService.isHeartbeatTimedOut()) {
            if (!wasTimedOut) {
                vehicle.emergencyStop();
                tcs.reset();
                activeCmd.throttle = 0;
                activeCmd.steering = 0.0f;
                wasTimedOut = true;
                Serial.println("\n[WATCHDOG] Heartbeat timeout! Motors stopped.");
            }
        } else {
            wasTimedOut = false;
        }

        if (xQueueReceive(commandQueue, &incomingCmd, pdMS_TO_TICKS(20)) == pdPASS) {
            activeCmd = incomingCmd;
        }

        if (!wasTimedOut) {
            tcs.setEnabled(activeCmd.tcsEnabled);

            bool imuValid = (imuQueue != nullptr) && (xQueuePeek(imuQueue, &latestImu, 0) == pdPASS) && imu.isConnected();
            int8_t safeThrottle = tcs.process(activeCmd.throttle, activeCmd.steering, latestImu, imuValid);

            vehicle.drive(safeThrottle, activeCmd.steering);

            if (tcsStatusQueue != nullptr) {
                TcsStatus status = { safeThrottle, tcs.isEnabled(), tcs.isIntervening() };
                xQueueOverwrite(tcsStatusQueue, &status);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void printStartupBanner() {
    Serial.println("\n==================================================");
    Serial.println("RC-Car-ESP32-IMU: Wi-Fi Mobile Cockpit Ready");
    Serial.println("Wi-Fi SSID : 'VDT-Car' (Open)");
    Serial.println("Dashboard  : http://192.168.4.1");
    Serial.println("==================================================\n");
}

void setup() {
    Serial.begin(115200);
    delay(1500);

    // Left motor is mechanically mirrored on the chassis
    motors.invertMotor(MOTOR_LEFT, true);
    vehicle.begin();

    commandQueue   = xQueueCreate(1, sizeof(DriveCommand));
    imuQueue       = xQueueCreate(1, sizeof(ImuData));
    tcsStatusQueue = xQueueCreate(1, sizeof(TcsStatus));

    wifiService.begin(commandQueue, imuQueue, tcsStatusQueue, "VDT-Car");
    imu.begin();

    xTaskCreatePinnedToCore(vehicleTask, "VehicleTask", 4096, nullptr, 2, nullptr, 1);
    xTaskCreatePinnedToCore(imuTask, "ImuTask", 4096, nullptr, 1, nullptr, 1);

    printStartupBanner();
}

void loop() {
    wifiService.loop();
}