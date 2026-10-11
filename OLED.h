#ifndef OLED_H
#define OLED_H

#include <Arduino.h>
#include "pins_config.h"

// Inisialisasi OLED — panggil di setup() setelah Wire.begin()
void initOLED();

// Tampilkan layar data utama (Mode Normal)
void oledShowData(
    bool    worn,
    float   hr,
    float   hrv,
    float   temp,
    int     gsrPct,
    int     stress,
    bool    bleConnected
);

// Tampilkan layar status boot / inisialisasi modul
void oledShowBoot(const char* message);

// Tampilkan layar "Sensor Belum Dipakai"
void oledShowUnworn();

// Tampilkan layar error jika ada sensor gagal terdeteksi
void oledShowError(const char* sensorName);

// ======================== LAYAR KHUSUS MODE DEBUG ========================
// Tampilan terdedikasi untuk pengujian MAX30102 (HR, IBI, HRV, IR)
void oledShowTestMAX30102(bool worn, uint32_t ir, float hr, unsigned long ibi, float hrv);

// Tampilan terdedikasi untuk pengujian MLX90614 (Suhu Objek, Status Suhu)
void oledShowTestMLX(float temp);

// Tampilan terdedikasi untuk pengujian MPU6050 (Akselerasi X/Y/Z, Magnitude, Net Motion)
void oledShowTestMPU(float ax, float ay, float az, float magnitude, float motion);

// Tampilan terdedikasi untuk pengujian GSR (ADC Raw, Persentase %, Bar Grafis, Status)
void oledShowTestGSR(int raw, int pct);

// Tampilan terdedikasi untuk pengujian BLE (Status Koneksi, Paket Count, Uptime)
void oledShowTestBLE(bool connected, int count, uint32_t uptime);

#endif
