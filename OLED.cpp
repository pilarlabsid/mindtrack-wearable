#include "OLED.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Objek display SSD1306 mengacu pada I2C default (Wire) yang sudah di-begin
static Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

// Flag untuk mendeteksi apakah hardware OLED berhasil diinisialisasi
static bool oledReady = false;

// ── Karakter Khusus / Icon 8x8 pixel ──────────────────────────
// Icon Jantung Kecil (untuk penanda detak jantung / BPM)
static const uint8_t PROGMEM heartIcon[] = {
    0b00000000,
    0b01101100,
    0b11111110,
    0b11111110,
    0b01111100,
    0b00111000,
    0b00010000,
    0b00000000
};

// ── Helper Internal: Status Bar Atas ──────────────────────────
static void drawStatusBar(bool bleConnected) {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("MindTrack");

    // Indikator status BLE di sudut kanan atas
    if (bleConnected) {
        display.setCursor(85, 0);
        display.print("BLE:ON");
        display.drawCircle(124, 3, 2, SSD1306_WHITE);
        display.fillCircle(124, 3, 1, SSD1306_WHITE);
    } else {
        display.setCursor(78, 0);
        display.print("BLE:WAIT");
    }

    // Garis pemisah horizontal di bawah status bar (y=9)
    display.drawFastHLine(0, 9, OLED_WIDTH, SSD1306_WHITE);
}

// ── Helper Internal: Progress Bar Level Stres ─────────────────
static void drawStressBar(int stress) {
    display.setTextSize(1);
    display.setCursor(0, 52);
    display.print("Stress:");

    // Outline kotak bar stres (lebar 70 pixel)
    display.drawRect(45, 52, 56, 8, SSD1306_WHITE);

    // Isi bar proporsional terhadap nilai stres (0 - 100)
    int fillW = (int)((constrain(stress, 0, 100) / 100.0f) * 52);
    if (fillW > 0) {
        display.fillRect(47, 54, fillW, 4, SSD1306_WHITE);
    }

    // Nilai numerik stres di samping kanan bar
    display.setCursor(105, 52);
    display.printf("%3d", stress);
}

// =============================================================
//  FUNGSI PUBLIK
// =============================================================

// Inisialisasi layar OLED SSD1306
void initOLED() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println("[ERROR] OLED SSD1306 tidak terdeteksi pada 0x3C! Sistem tetap lanjut.");
        oledReady = false;
        return;
    }
    oledReady = true;
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    // Tampilkan Splash Screen saat boot
    display.setTextSize(2);
    display.setCursor(12, 8);
    display.print("MindTrack");
    display.setTextSize(1);
    display.setCursor(24, 28);
    display.print("v2.1 ESP32-C3");
    display.setCursor(16, 44);
    display.print("Inisialisasi...");
    display.display();
    delay(1200);

    Serial.println("[OK] OLED SSD1306 siap digunakan.");
}

// Tampilkan pesan boot / pengujian modul
void oledShowBoot(const char* message) {
    if (!oledReady) return;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("MindTrack Setup");
    display.drawFastHLine(0, 9, OLED_WIDTH, SSD1306_WHITE);

    display.setCursor(4, 24);
    display.print("Memuat Mode:");
    display.setCursor(4, 38);
    display.setTextSize(1);
    display.print(message);
    display.display();
}

// Tampilkan layar jika sensor MAX30102 belum mendeteksi jari/kulit
void oledShowUnworn() {
    if (!oledReady) return;
    display.clearDisplay();
    drawStatusBar(false);

    display.setTextSize(1);
    display.setCursor(14, 22);
    display.print("Sensor Dilepas!");
    display.setCursor(6, 36);
    display.print("Pasang ke jari/kulit");

    // Animasi titik berkedip halus
    static uint8_t dotCounter = 0;
    dotCounter = (dotCounter + 1) % 4;
    display.setCursor(54, 50);
    for (int i = 0; i < dotCounter; i++) display.print(".");

    display.display();
}

// Tampilkan layar peringatan sensor bermasalah
void oledShowError(const char* sensorName) {
    if (!oledReady) return;
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("! PERINGATAN SENSOR !");
    display.drawFastHLine(0, 9, OLED_WIDTH, SSD1306_WHITE);

    display.setCursor(0, 20);
    display.print("Sensor Gagal:");
    display.setTextSize(2);
    display.setCursor(0, 32);
    display.print(sensorName);

    display.setTextSize(1);
    display.setCursor(0, 52);
    display.print("Cek Kabel I2C/VCC");
    display.display();
}

// Tampilan Data Lengkap (Mode Normal)
void oledShowData(
    bool    worn,
    float   hr,
    float   hrv,
    float   temp,
    int     gsrPct,
    int     stress,
    bool    bleConnected
) {
    if (!oledReady) return;
    display.clearDisplay();
    drawStatusBar(bleConnected);

    display.setTextSize(1);

    // ── Baris 1: Heart Rate & HRV ──
    display.drawBitmap(0, 13, heartIcon, 8, 8, SSD1306_WHITE);
    display.setCursor(11, 13);
    if (hr > 0) {
        display.printf("%3d BPM", (int)round(hr));
    } else {
        display.print("--- BPM");
    }

    display.setCursor(72, 13);
    display.print("HRV:");
    if (hrv >= 0) {
        display.printf("%2dms", (int)round(hrv));
    } else {
        display.print("--ms");
    }

    // ── Baris 2: Suhu Tubuh & GSR % ──
    display.setCursor(0, 26);
    display.printf("T:%4.1f\xf8""C", temp); // \xf8 = simbol derajat
    display.setCursor(68, 26);
    display.printf("GSR:%3d%%", gsrPct);

    // ── Baris 3: Status Worn & Kategori Stres ──
    display.setCursor(0, 38);
    display.print(worn ? "Worn OK" : "Lepas");

    display.setCursor(68, 38);
    if (stress < 30)       display.print("[Tenang]");
    else if (stress < 60)  display.print("[Waspada]");
    else                   display.print("[STRES!]");

    // Garis pemisah sebelum progress bar
    display.drawFastHLine(0, 48, OLED_WIDTH, SSD1306_WHITE);

    // ── Baris 4: Stress Bar ──
    drawStressBar(stress);

    display.display();
}

// ======================== LAYAR KHUSUS MODE TEST ========================

// 1. Tampilan Khusus TEST_MAX30102
void oledShowTestMAX30102(bool worn, uint32_t ir, float hr, unsigned long ibi, float hrv) {
    if (!oledReady) return;
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("TEST: MAX30102");
    display.drawFastHLine(0, 9, OLED_WIDTH, SSD1306_WHITE);

    display.setCursor(0, 13);
    display.printf("Status : %s", worn ? "DIPAKAI (OK)" : "LEPAS / JAUH");

    display.setCursor(0, 24);
    display.printf("IR Val : %lu", (unsigned long)ir);

    display.setCursor(0, 35);
    if (worn) {
        display.printf("HR : %3.0f BPM", hr);
    } else {
        display.print("HR : Tempel jari...");
    }

    display.setCursor(0, 46);
    display.printf("IBI: %lu ms", ibi);

    display.setCursor(0, 56);
    if (hrv >= 0) {
        display.printf("HRV(RMSSD): %.1f ms", hrv);
    } else {
        display.print("HRV: Kalibrasi beat...");
    }

    display.display();
}

// 2. Tampilan Khusus TEST_MLX90614
void oledShowTestMLX(float temp) {
    if (!oledReady) return;
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("TEST: MLX90614 (Suhu)");
    display.drawFastHLine(0, 9, OLED_WIDTH, SSD1306_WHITE);

    display.setCursor(0, 16);
    display.print("Suhu Objek Kulit:");

    display.setTextSize(2);
    display.setCursor(14, 30);
    display.printf("%.2f \xf8""C", temp);

    display.setTextSize(1);
    display.setCursor(0, 52);
    if (temp < 30.0f) {
        display.print("Status: Terlalu Dingin");
    } else if (temp > 40.0f) {
        display.print("Status: Terlalu Panas!");
    } else {
        display.print("Status: Normal (Fisiologis)");
    }

    display.display();
}

// 3. Tampilan Khusus TEST_MPU6050
void oledShowTestMPU(float ax, float ay, float az, float magnitude, float motion) {
    if (!oledReady) return;
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("TEST: MPU6050 (Gerak)");
    display.drawFastHLine(0, 9, OLED_WIDTH, SSD1306_WHITE);

    display.setCursor(0, 13);
    display.printf("X:%+5.1f Y:%+5.1f", ax, ay);

    display.setCursor(0, 24);
    display.printf("Z:%+5.1f  Mag:%.1f", az, magnitude);

    display.setCursor(0, 36);
    display.printf("Net Motion: %.2f", motion);

    // Visual Bar intensitas gerakan
    display.drawRect(0, 47, 85, 7, SSD1306_WHITE);
    int barW = (int)((constrain(motion, 0.0f, 50.0f) / 50.0f) * 81);
    if (barW > 0) display.fillRect(2, 49, barW, 3, SSD1306_WHITE);

    display.setCursor(90, 47);
    if (motion < 5)       display.print("Diam");
    else if (motion < 25) display.print("Aktif");
    else                  display.print("Lari");

    display.display();
}

// 4. Tampilan Khusus TEST_GSR
void oledShowTestGSR(int raw, int pct) {
    if (!oledReady) return;
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("TEST: GSR (Kulit)");
    display.drawFastHLine(0, 9, OLED_WIDTH, SSD1306_WHITE);

    display.setCursor(0, 14);
    display.printf("Raw ADC  : %4d/4095", raw);

    display.setCursor(0, 26);
    display.printf("Kondukt. : %3d %%", pct);

    // Visual Bar konduktansi
    display.drawRect(0, 38, 127, 8, SSD1306_WHITE);
    int barW = (int)((constrain(pct, 0, 100) / 100.0f) * 123);
    if (barW > 0) display.fillRect(2, 40, barW, 4, SSD1306_WHITE);

    display.setCursor(0, 52);
    if (raw == 0) {
        display.print("Elektroda Lepas!");
    } else if (pct < 20) {
        display.print("Kulit Kering (Santai)");
    } else if (pct < 55) {
        display.print("Normal / Sedang");
    } else {
        display.print("Berkeringat / Responsif");
    }

    display.display();
}

// 5. Tampilan Khusus TEST_BLE
void oledShowTestBLE(bool connected, int count, uint32_t uptime) {
    if (!oledReady) return;
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("TEST: BLE Wireless");
    display.drawFastHLine(0, 9, OLED_WIDTH, SSD1306_WHITE);

    display.setCursor(0, 14);
    display.print("Device: MindTrack_BLE");

    display.setCursor(0, 26);
    display.printf("Status: %s", connected ? "TERHUBUNG (OK)" : "Menunggu HP...");

    display.setCursor(0, 38);
    display.printf("Paket : %d tx", count);

    display.setCursor(0, 50);
    display.printf("Uptime: %lu detik", (unsigned long)uptime);

    display.display();
}
