#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include "protocol.h"
#include "imu_driver.h"

// Hosts the ESP32 SoftAP, HTTP cockpit dashboard (:80), and WebSocket server (:81)
class WifiService {
public:
    WifiService();

    // Start Wi-Fi AP, HTTP server, and WebSocket server
    void begin(QueueHandle_t commandQueue, QueueHandle_t imuQueue = nullptr,
               QueueHandle_t tcsStatusQueue = nullptr,
               const char* ssid = "VDT-Car", const char* password = nullptr);

    // Process HTTP/WebSocket events and publish 20 Hz telemetry (run on Core 0)
    void loop();

    bool isConnected() const;
    bool isHeartbeatTimedOut() const;
    void refreshHeartbeat();

private:
    QueueHandle_t     _commandQueue;
    QueueHandle_t     _imuQueue;
    QueueHandle_t     _tcsStatusQueue;
    WebServer         _server;
    WebSocketsServer  _webSocket;
    bool              _connected;
    uint32_t          _lastHeartbeatMs;
    uint32_t          _lastTelemetryMs;
    DriveCommand      _activeCommand;

    void publishTelemetry();
    void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
    void handleRoot();
    void handleNotFound();

    // Static C-callback trampoline for WebSocketsServer
    static void webSocketEventWrapper(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
    static WifiService* _instance;
};
