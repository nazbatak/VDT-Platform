#include <Arduino.h>
#include "pins.h"
#include "motor_driver.h"
#include "servo_driver.h"
#include "steering.h"
#include "vehicle_controller.h"
#include "protocol.h"
#include "wifi_service.h"
#include "imu_driver.h"

// Donanim nesneleri
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

// Direksiyon servosu ve katmani
ServoDriver servo(PIN_SERVO_STEERING);
Steering steering(servo);

// Arac Kontrol Unitesi (VCU)
VehicleController vehicle(motors, steering);

// MPU6050 IMU Sensöru (Hafta 4)
ImuDriver imu(PIN_IMU_SDA, PIN_IMU_SCL);

// --- FreeRTOS ve Wi-Fi Katmani (Hafta 3) ---
QueueHandle_t commandQueue = nullptr;
WifiService   wifiService;

// Bagimsiz IMU gorevi (Core 1, Oncelik 1 - Motor gorevinden daha dusuk oncelik)
// 100 Hz (10 ms) ile sensoru okur; her 100 ms'de bir Serial Monitor'e telemetri basar
void imuTask(void* pvParameters) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(10); // 100 Hz (10 ms)
    uint32_t lastPrintMs = 0;

    while (true) {
        if (imu.isConnected()) {
            imu.read();

            // Serial Monitor'e 10 Hz (100 ms) periyotla telemetri yazdir
            if (millis() - lastPrintMs >= 100) {
                lastPrintMs = millis();
                const ImuData& d = imu.getData();
                Serial.printf("📊 [IMU] Acc(g): [%+0.2f, %+0.2f, %+0.2f] | Gyro(°/s): [%+05.1f, %+05.1f, %+05.1f] | Temp: %.1f°C\n",
                              d.ax, d.ay, d.az, d.gx, d.gy, d.gz, d.temp);
            }
        }

        vTaskDelayUntil(&lastWakeTime, period);
    }
}


// Tek motor gorevi: Motor ve direksiyon donanimini yalnizca bu gorev surer (SOLID & Eşzamanlilik Guvenligi)
void vehicleTask(void* pvParameters) {
    DriveCommand cmd = {0, 0.0f};
    bool wasTimedOut = false;

    while (true) {
        // 1. Fail-Safe / Watchdog Kontrolu
        // Eger istemci bagliysa ve 500ms boyunca heartbeat gelmediyse acil durdur
        if (wifiService.isHeartbeatTimedOut()) {
            if (!wasTimedOut) {
                vehicle.emergencyStop();
                wasTimedOut = true;
                Serial.println("\n🚨 [WATCHDOG / FAIL-SAFE] Baglanti zaman asimi! Motorlar durduruldu.");
            }
        } else {
            wasTimedOut = false;
        }

        // 2. Kuyruktan yeni surus komutunu bekle (20ms periyot)
        if (xQueueReceive(commandQueue, &cmd, pdMS_TO_TICKS(20)) == pdPASS) {
            vehicle.drive(cmd.throttle, cmd.steering);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Baslatma bilgilerini yazdir
void printStartupBanner() {
    Serial.println("\n==================================================");
    Serial.println("🚗 VDT-Platform: Wi-Fi Mobil Kokpit Yayinda");
    Serial.println("==================================================");
    Serial.println("📶 Wi-Fi Erisim Noktasi: 'VDT-Car' (Sifresiz)");
    Serial.println("🌐 Tarayici Adresi    : http://192.168.4.1");
    Serial.println("📱 Telefonunuzdan baglanip surus yapabilirsiniz.");
    Serial.println("==================================================\n");
}

void setup() {
    Serial.begin(115200);
    delay(1500);

    // B kanalina bagli sol motor mekanik yerlesimden dolayi ters dondugu icin duzeltildi
    motors.invertMotor(MOTOR_LEFT, true);

    // Araci baslat (motorlari kurar, servoyu baslatir ve guvenli durusa alir)
    vehicle.begin();

    // 1 elemanlik komut kuyrugu olustur (En yeni komut her zaman eskini ezer)
    commandQueue = xQueueCreate(1, sizeof(DriveCommand));

    // Wi-Fi Access Point ve Web/WebSocket sunucusunu baslat
    wifiService.begin(commandQueue, "VDT-Car");

    // MPU6050 IMU sensorunu baslat (I2C 400 kHz)
    imu.begin();

    // Motor gorevini Core 1 uzerinde calistir (Katı zamanlama ve guvenlik, Oncelik 2)
    xTaskCreatePinnedToCore(
        vehicleTask,
        "VehicleTask",
        4096,
        nullptr,
        2,              // Yuksek oncelik
        nullptr,
        1               // Core 1
    );

    // IMU okuma gorevini Core 1 uzerinde calistir (Oncelik 1 - Motor gorevinden bagimsiz)
    xTaskCreatePinnedToCore(
        imuTask,
        "ImuTask",
        4096,
        nullptr,
        1,              // Dusuk oncelik
        nullptr,
        1               // Core 1
    );

    printStartupBanner();
}

void loop() {
    // Wi-Fi ve WebSocket ag olaylarini isle (Core 0 / Standart Dongu)
    wifiService.loop();
}