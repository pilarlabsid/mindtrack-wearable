#include "GSR.h"

static int lastValidGSR = -1; // -1 = belum ada pembacaan valid sama sekali

void initGSR() {
    analogReadResolution(12); // 12-bit ADC (0–4095)
    pinMode(PIN_GSR, INPUT);
}

// Mengembalikan nilai ADC 0–4095
// GSR = 0 bisa valid (kulit sangat kering) ATAU elektroda tidak terpasang
// Kita tidak bisa membedakan keduanya secara pasti, jadi nilai 0 tetap diterima
// Tapi jika SEMUA 5 sampel = 0 secara konsisten → kemungkinan elektroda lepas
int readGSR() {
    long sum    = 0;
    int  zeros  = 0;
    const int samples = 5;

    for (int i = 0; i < samples; i++) {
        int val = analogRead(PIN_GSR);
        sum += val;
        if (val == 0) zeros++;
    }

    int avg = (int)(sum / samples);

    // Jika semua sampel = 0 → kemungkinan elektroda lepas
    // Gunakan last-known-good jika ada, jika tidak, kembalikan 0
    if (zeros == samples) {
        return (lastValidGSR >= 0) ? lastValidGSR : 0;
    }

    lastValidGSR = avg;
    return avg;
}
