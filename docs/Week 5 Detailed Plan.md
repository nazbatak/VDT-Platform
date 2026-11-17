# 📋 VDT Platform — Hafta 5 Planı (IMU Tabanlı Traction Control & V1.0 Kapanış)

> **Mevcut Durum:** Hafta 1–4 tamamlandı. Araç kendi Wi-Fi ağı (`VDT-Car`) üzerinden telefonla sürülüyor; FreeRTOS çift çekirdek mimarisi, 500 ms fail-safe ve `ImuDriver` (`MPU6050`, 100 Hz `imuTask`) çalışıyor.  
> **Stratejik Karar (V1.0 Kapsam Güncellemesi):** ROADMAP'teki teorik/donanım gerektiren Hafta 5–7 maddeleri (Kalman, E-Diff, Rejeneratif Frenleme) yerine; mevcut donanımımızla **sahada doğrudan hissedilen, somut ve eğlenceli** bir final özelliği olan **IMU Tabanlı Traction Control (Patinaj Önleme Sistemi)** + **Canlı Telemetri Akışı** tamamlanarak proje **V1.0** olarak gururla sonlandırılacaktır.  
> **Hedef:** Ani kalkışta ve viraj içinde tekerleklerin patinaj/spin atmasını IMU (`ax`, `ay`, `gz`) verileriyle algılayıp gazı otomatik yumuşatan bir **Traction Control (TCS)** modülü yazmak, telefondan **TCS Aç/Kapat** yapabilmek ve canlı telemetriyi kokpitte görmek.

> Bu plan zaman değil **konu/iş** odaklıdır. Yazılım kısımları **gereksinim** olarak tanımlanmıştır.

---

## 🧭 1. Hedef Mimari (Mevcut Kodla Birebir Uyumlu)

Mevcut kodumuzdaki `ImuDriver`, `VehicleController` ve `WifiService` sınıflarına dokunmadan araya **tek sorumluluk (SRP)** ilkesine uygun iki temiz köprü ekliyoruz:

```
┌──────────────────────────────────────────────────────────────────────┐
│ Telefon Tarayıcısı (Safari / Chrome)                                 │
│ • Sürüş Komutu + TCS Toggle: {"throttle": 90, "steering": 20, "tcs": true}
│ • Canlı Gösterim: ax/ay/gz, Uygulanan Gaz %, [TCS ACTIVE] Uyarı Işığı│
└──────────────────────────┬─────────────────────────▲─────────────────┘
                           │ WebSocket (Port 81)     │ 20 Hz JSON Telemetri
┌──────────────────────────▼─────────────────────────┴─────────────────┐
│ ESP32 — Core 0 (WifiService)                                         │
│   commandQueue (1 eleman) ──▶               ◀── imuQueue (1 eleman)   │
└──────────────┬────────────────────────────────────▲──────────────────┘
               │                                    │
┌──────────────▼────────────────────────────────────┴──────────────────┐
│ ESP32 — Core 1 (FreeRTOS Görevleri)                                  │
│                                                                      │
│  ┌─────────────────────┐          ┌───────────────────────────────┐  │
│  │ imuTask (Öncelik 1) │─imuQueue─▶│ vehicleTask (Öncelik 2)       │  │
│  │ 100 Hz ImuDriver    │          │  1. Watchdog (500 ms)         │  │
│  └─────────────────────┘          │  2. TractionController::filter│  │
│                                   │  3. VehicleController::drive  │  │
│                                   └───────────────────────────────┘  │
└──────────────────────────────────────────────────────────────────────┘
```

---

## 🧠 2. Encoder Olmadan Traction Control (TCS) Nasıl Çalışır?

Gerçek otomobillerde TCS, tekerlek hız sensörleri (Encoder) ile aracın gerçek hızını karşılaştırır. Bizim V1.0 şasemize henüz encoder takmadığımız için elimizdeki 3 gerçek veriyi (**İstenen Gaz**, **Direksiyon Açısı** ve **MPU6050 IMU**) kullanarak 3 katmanlı akıllı bir **IMU Tabanlı TCS** kurguluyoruz:

### Katman 1: Kalkış Yumuşatma (Launch / Slew-Rate Control)
* **Problem:** Joystick'i aniden `%0`'dan `%100`'e çektiğinizde DC motorlar anlık yüksek tork üretir ve kaygan zeminde (parke, fayans, tozlu asfalt) lastikler boşa döner.
* **Çözüm:** Gaz artış hızına (`+Δthrottle / döngü`) sınır konur (ör. her 20 ms'de en fazla `+%5–8` artış). Gaz azaltma/frenlemede sınır yoktur; kullanıcı gazı bıraktığı an motor **anında** kesilir (güvenlik kuralı).

### Katman 2: Boyuna Patinaj Algılama (Longitudinal Slip Control)
* **Problem:** Araç sabit dururken veya yavaşken kullanıcı tam gaz (`> %70`) verdi ama lastikler tutunamadığı için yerinde patinaj çekiyor.
* **Fiziksel İpucu:** Lastikler yolu kavrarsa araç öne fırlar ve **`ax` (boyuna ivme) belirgin şekilde artar**. Eğer yüksek gaz komutuna rağmen **`ax` çok düşük kalıyorsa** (araç hızlanamıyorsa) veya ani sarsıntı/titreşim görülüyorsa patinaj vardır.
* **Müdahale:** TCS geçici olarak maksimum gazı düşürür (ör. `%65` tavanına çeker) ki lastikler tekrar tutunabilsin (statik sürtünme katsayısı > kinetik sürtünme katsayısı!).

### Katman 3: Viraj İçi Spin / Oversteer Kontrolü (Cornering TCS)
* **Problem:** Direksiyon tam kırıkken (`|steering| > 12°`) yüksek gaz verilirse arka/ön tekerlekler yana kayar ve araç kendi etrafında spin atar.
* **Fiziksel İpucu:** Direksiyon açısı yüksekken MPU6050'den okunan **Yaw Rate (`|gz|`)** veya **Yanal İvme (`|ay|`)** belirlenen eşiği aşarsa araç kaymaya başlamıştır.
* **Müdahale:**
  1. Direksiyon açısı arttıkça izin verilen maksimum gaz dinamik olarak daraltılır (ör. düz giderken `%100`, tam sağ/solda en fazla `%65–70`).
  2. Eğer `|gz|` (dönüş hızı) eşiği aşarsa **TCS aktif olur**, gazı anlık olarak kısar ve araç çizgisini koruyana kadar kontrolü elinde tutar.

---

## 🧱 3. İş Paketleri (Konu Haritası)

```
[A] Çekirdekler Arası Telemetri (imuQueue) ──▶ [B] TractionController Modülü ──▶ [C] vehicleTask Entegrasyonu
                                                                                            │
[E] Saha Testi (TCS ON vs OFF) & V1.0 Kapanış ◀── [D] WebSocket Telemetri & Mobil Kokpit ◀──┘
```

### A. Çekirdekler Arası Telemetri Köprüsü (`imuQueue`)
**Amaç:** `imuTask` (Core 1) tarafından okunan veriyi, `WifiService` (Core 0) ile thread-safe paylaşmak.
- `main.cpp` içinde 1 elemanlık `imuQueue = xQueueCreate(1, sizeof(ImuData))` oluşturulur.
- `imuTask` her okuma sonrası `xQueueOverwrite(imuQueue, &d)` ile en taze veriyi bırakır.
- `WifiService::begin(...)` fonksiyonu opsiyonel olarak `imuQueue` alır ve `loop()` içinde her **50 ms'de bir (20 Hz)** `xQueuePeek` ile okuyup telefona yollar.
- **Kural:** Kuyruk okuması başarısızsa (sensör yoksa) telemetri paketi `"imu": false` gider, sürüş asla etkilenmez.

### B. `TractionController` Sınıfı (`traction_control.h` / `.cpp`)
**Amaç:** SOLID (Tek Sorumluluk) ilkesine uygun, motor sürücüden bağımsız saf kontrol algoritması.
- **Girdi:** `DriveCommand` (`throttle`, `steering`), `ImuData` (`ax`, `ay`, `gz`), `dt` (saniye).
- **Çıktı:** `int8_t appliedThrottle` (güvenli gaz) ve `bool isIntervening` (TCS şu an müdahale ediyor mu?).
- **Özellikler:**
  - `setEnabled(bool enabled)` / `isEnabled()` — Telefondan açılıp kapatılabilir.
  - `reset()` — Acil durdurma veya bağlantı kopmasında iç rampayı sıfırlar.
  - **Önemli Güvenlik Kuralı:** `throttle <= 0` (gaz kesme veya geri vites) durumlarında TCS müdahale etmez, doğrudan kullanıcı komutunu geçirir.

### C. `vehicleTask` ve Protokol Güncellemesi (`main.cpp` & `protocol.h`)
**Amaç:** TCS'i sürüş döngüsüne bağlamak.
- `protocol.h` içine `tcsEnabled` komutu (`DriveCommand` veya ayrı flag) ve telemetri özeti eklenir.
- `vehicleTask` içinde akış şu sıraya oturur:
  1. Watchdog (500 ms) kontrolü $\rightarrow$ zaman aşımı varsa `emergencyStop()` + `tcs.reset()`.
  2. `commandQueue`'dan güncel sürüş komutunu al.
  3. `imuQueue`'dan `xQueuePeek` ile güncel IMU verisini al.
  4. `tcs.process(cmd, imuData)` ile güvenli gazı hesapla.
  5. `vehicle.drive(safeThrottle, cmd.steering)` ile motorları sür.

### D. Mobil Kokpit & Canlı WebSocket Telemetri (`wifi_service.cpp`)
**Amaç:** Sahada telefonu elinize aldığınızda hem TCS farkını görüp yönetmek hem de canlı IMU verisini izlemek.
- **Telemetri JSON (ESP32 $\rightarrow$ Telefon, 20 Hz):**
  ```json
  {"type":"telemetry","ax":0.12,"ay":-0.34,"gz":45.2,"thr":62,"tcs":true,"act":true}
  ```
  - `thr`: TCS sonrası tekerleklere gerçekten uygulanan gaz (%)
  - `tcs`: TCS açık mı?
  - `act`: TCS şu an patinaj algılayıp müdahale ediyor mu?
- **Dashboard Arayüzü (Mevcut 2 joystick arasına sade ekleme):**
  - **`TCS: AÇIK / KAPALI`** butonu (Tek dokunuşla sahada A/B testi yapabilmek için).
  - **`TCS MÜDAHALE` LED Göstergesi:** Müdahale anında turuncu/kehribar renkte yanıp söner.
  - **Canlı Telemetri Kutusu:** İleri İvme (`ax`), Yanal G (`ay`), Dönüş Hızı (`gz` °/s) ve İstenen vs. Uygulanan Gaz (`%`).

### E. Saha Testi (TCS ON vs OFF) & V1.0 Kapanış
**Amaç:** Sistemin gerçek dünyada çalıştığını doğrulamak ve projeyi temiz bir **V1.0** sürümü olarak kapatmak.
- **Test 1 (Kalkış Testi — Parke / Kaygan Zemin):**
  - `TCS KAPALI`: Tam gaz ver $\rightarrow$ tekerleklerin anlık patinajını/savrulmasını gözle.
  - `TCS AÇIK`: Tam gaz ver $\rightarrow$ yumuşak kalkışı ve `TCS MÜDAHALE` ışığının yanmasını gözle.
- **Test 2 (Viraj Testi):**
  - Direksiyonu tam kırıp yüksek gazla dön $\rightarrow$ `TCS AÇIK` modda aracın spin atmadan virajı daha kararlı dönmesini doğrula.
- **Test 3 (Güvenlik Regresyonu):**
  - TCS ve telemetri aktifken tarayıcıyı kapat $\rightarrow$ araç 500 ms içinde durmalı.
- **V1.0 Kapanış:**
  - `docs/protocol.md` ve kök `README.md` güncellenir (mimari diyagram, kurulum, sürüş videosu/fotoğrafı).
  - İstenirse KiCad şeması/PCB taslağı eklenir.
  - `git tag v1.0` ile proje milestone'u mühürlenir.

---

## ⚡ 4. Dikkat Edilecek Mühendislik Kuralları

1. **Önce Eksen Yönü Doğrulaması:** TCS yazılmadan önce aracın gerçek ileri yönünün `+ax` mi `-ax` mi, sağa dönüşün `+gz` mi `-gz` mi olduğu `Serial Monitor`'den teyit edilmelidir! (Sensör ters takıldıysa işaret düzeltmesi `imu_driver` içinde yapılır).
2. **Eşikler Sabit ve Ayarlanabilir Olmalı:** Slew-rate, yanal G ve Yaw Rate eşikleri (`traction_control.h/.cpp` içinde `constexpr` olarak) tek yerde toplanmalı ki saha testinde kolayca kalibre edilebilsin.
3. **Sürüş Güvenliği Her Zaman Öncelikli:** `tcs.process()` asla döngüyü bekletmemeli, gaz kesme ve acil durdurma komutları gecikmesiz doğrudan motorlara ulaşmalıdır.

---

## 📁 5. Hedeflenen Proje Yapısı (V1.0 Final)

```
VDT-Platform/
├── docs/
│   ├── ROADMAP.md
│   ├── pinout.md
│   ├── protocol.md             # (+telemetri & tcs komutu)
│   ├── vehicle_params.md       # Ackermann + TCS kalibrasyon eşikleri
│   └── Week 1..5 Detailed Plan.md
└── software/
    ├── include/
    │   ├── pins.h
    │   ├── protocol.h          # (+tcs flag & TelemetryPacket)
    │   ├── motor_driver.h
    │   ├── servo_driver.h
    │   ├── steering.h
    │   ├── vehicle_controller.h
    │   ├── imu_driver.h
    │   ├── traction_control.h  # (YENİ - Hafta 5)
    │   └── wifi_service.h
    └── src/
        ├── main.cpp
        ├── motor_driver.cpp
        ├── servo_driver.cpp
        ├── steering.cpp
        ├── vehicle_controller.cpp
        ├── imu_driver.cpp
        ├── traction_control.cpp # (YENİ - Hafta 5)
        └── wifi_service.cpp
```

---

## 🎯 6. V1.0 Başarı Kriterleri (Definition of Done)

- [ ] IMU eksen yönleri (`+ax` ileri, `+ay` yanal, `gz` dönüş) fiziksel olarak doğrulandı.
- [ ] `imuQueue` ile Core 1 $\rightarrow$ Core 0 telemetri köprüsü sıfır çakışmayla çalışıyor.
- [ ] `TractionController` modülü bağımsız, temiz ve gaz kesmede sıfır gecikmeli çalışıyor.
- [ ] Telefon kokpitinde canlı `ax`, `ay`, `gz`, uygulanan gaz `%`, `TCS AÇIK/KAPALI` butonu ve `TCS MÜDAHALE` göstergesi sorunsuz çalışıyor.
- [ ] Kaygan zeminde `TCS KAPALI` vs `TCS AÇIK` testlerinde kalkış ve viraj davranış farkı gözlemlendi.
- [ ] Bağlantı koptuğunda 500 ms fail-safe hâlâ kusursuz çalışıyor.
- [ ] `README.md` ve `protocol.md` güncellendi, proje `v1.0` olarak commit edildi.
