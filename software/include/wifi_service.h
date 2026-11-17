#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include "protocol.h"
#include "imu_driver.h"

// ==============================================================================
// VDT Platform - Wi-Fi AP & WebSocket Servis Sinifi (Hafta 3)
// Gorevi: ESP32'yi bagimsiz bir Wi-Fi erisim noktasi (Access Point) yapmak,
// web sitesini telefona sunmak ve WebSocket uzerinden surus komutlarini almak.
// ==============================================================================

class WifiService {
public:
    // Kurucu fonksiyon
    WifiService();

    // Wi-Fi AP, Web Server ve WebSocket sunucusunu baslatir
    void begin(QueueHandle_t commandQueue, QueueHandle_t imuQueue = nullptr,
               QueueHandle_t tcsStatusQueue = nullptr,
               const char* ssid = "VDT-Car", const char* password = nullptr);

    // Web ve WebSocket olaylarini isleyen dongu (Core 0 gorevinde cagrilir)
    void loop();

    // Baglanti durumunu dondurur
    bool isConnected() const;

    // Heartbeat zaman asimina ugradi mi kontrol eder (Fail-Safe)
    bool isHeartbeatTimedOut() const;

    // Son canlilik zamanini gunceller
    void refreshHeartbeat();

private:
    QueueHandle_t     _commandQueue;     // Komutlarin iletildigi kargo bandi
    QueueHandle_t     _imuQueue;         // IMU verisinin okundugu kargo bandi
    QueueHandle_t     _tcsStatusQueue;   // TCS durum bilgisinin okundugu kargo bandi
    WebServer         _server;           // Port 80 HTTP Web Sunucusu
    WebSocketsServer  _webSocket;        // Port 81 WebSocket Sunucusu
    bool              _connected;        // En az 1 istemci bagli mi?
    uint32_t          _lastHeartbeatMs;  // Son gecerli paket zamani (ms)
    uint32_t          _lastTelemetryMs;  // Son telemetri yayin zamani (ms)
    DriveCommand      _activeCommand;    // Son gecerli gaz ve direksiyon durumu

    // Telemetri verisini 20 Hz ile telefona yayinlar (Core 0)
    void publishTelemetry();

    // WebSocket olay dinleyicisi
    void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);

    // HTTP sayfa yonlendiricileri
    void handleRoot();
    void handleNotFound();

    static void webSocketEventWrapper(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
    static WifiService* _instance;
};
