#pragma once
#include <Arduino.h>
#include "imu_driver.h"
#include "protocol.h"

// ==============================================================================
// VDT Platform - IMU Tabanli Traction Control (TCS) Sinifi (Hafta 5)
// Tek Sorumluluk (SRP): Surucunun istedigi gaz ve direksiyon komutunu,
// IMU sensor verileriyle karsilastirarak kalkis patinajini ve viraj spinini onler.
// ==============================================================================

class TractionController {
public:
    // Kurucu fonksiyon: TCS varsayilan olarak acik baslar
    TractionController();

    // TCS sistemini acar veya kapatir (A/B testi icin)
    void setEnabled(bool enabled);

    // TCS aktif mi?
    bool isEnabled() const;

    // Surus komutunu ve IMU verisini isleyerek guvenli gaz yuzdesini hesaplar
    int8_t process(int8_t requestedThrottle, float steeringDeg, const ImuData& imu, bool imuValid);

    // TCS su an gaza mudahale edip kisiyor mu?
    bool isIntervening() const;

    // Acil durdurma veya baglanti kopmasinda ic rampayi sifirlar
    void reset();

private:
    bool    _enabled;            // Kullanici TCS'i acti mi?
    bool    _intervening;        // Su an aktif mudahale var mi?
    int8_t  _currentThrottle;    // Rampa/sinirlama sonrasi uygulanan son gaz (%)
    int8_t  _slipCap;            // Katman 3 patinaj sonrasi yavasca toparlanan gaz tavani (%)
    uint8_t _highThrottleTicks;  // Yuksek gazda ivmelenememe sayaci (patinaj tespiti)

    // Viraj acisina ve IMU verisine gore izin verilen maksimum gaz tavanini hesaplar
    int8_t computeThrottleCap(int8_t requestedThrottle, float steeringDeg, const ImuData& imu, bool imuValid);
};
