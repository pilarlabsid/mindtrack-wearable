#include "FusionIndex.h"
#include <Arduino.h>
#include <math.h>

// ======================== NILAI NETRAL TIAP SENSOR ========================
// Digunakan saat sensor mengembalikan data invalid/belum siap.
// Nilai netral dipilih agar tidak bias ke stres tinggi MAUPUN rendah.
#define NEUTRAL_HR       80.0f   // BPM — tengah range normal (60–140)
#define NEUTRAL_HRV      45.0f   // ms RMSSD — nilai "biasa" saat duduk santai
#define NEUTRAL_GSR    1500.0f   // ADC — tengah range 12-bit, konduktansi sedang
#define NEUTRAL_TEMP     36.5f   // °C — suhu kulit normal
#define NEUTRAL_MOTION    0.0f   // m/s² — diam

FusionIndex::FusionIndex() {
    index = 0;
    kalman_estimate = 0;
    kalman_error    = 1;
    smooth_prev     = 0;
    for (int i = 0; i < 5; i++) {
        hr_buffer[i]     = NEUTRAL_HR;
        hrv_buffer[i]    = NEUTRAL_HRV;
        gsr_buffer[i]    = NEUTRAL_GSR;
        temp_buffer[i]   = NEUTRAL_TEMP;
        motion_buffer[i] = NEUTRAL_MOTION;
    }
}

float FusionIndex::movingAverage(float value, float *buffer) {
    buffer[index] = value;
    float sum = 0;
    for (int i = 0; i < 5; i++) sum += buffer[i];
    return sum / 5.0f;
}

float FusionIndex::normalize(float value, float minVal, float maxVal) {
    if (maxVal <= minVal) return 0;
    float norm = (value - minVal) / (maxVal - minVal) * 100.0f;
    if (norm < 0)   norm = 0;
    if (norm > 100) norm = 100;
    return norm;
}

float FusionIndex::motionNet(float ax, float ay, float az) {
    float magnitude = sqrt(ax * ax + ay * ay + az * az);
    float net = magnitude - 9.81f;
    return (net < 0) ? 0 : net;
}

void FusionIndex::adaptiveWeights(
    float motion_n,
    float &w_hr, float &w_hrv, float &w_gsr, float &w_motion, float &w_temp
) {
    if (motion_n > 60) {
        // Mode aktif bergerak — HR tidak reliable, andalkan GSR & motion
        w_hr     = 0.10f;
        w_hrv    = 0.20f;
        w_gsr    = 0.30f;
        w_motion = 0.30f;
        w_temp   = 0.10f;
    } else {
        // Mode istirahat — HR & HRV paling informatif
        w_hr     = 0.25f;
        w_hrv    = 0.30f;
        w_gsr    = 0.25f;
        w_motion = 0.10f;
        w_temp   = 0.10f;
    }
}

float FusionIndex::fuse(
    float hr, float hrv, float gsr, float motion, float temp,
    float w_hr, float w_hrv, float w_gsr, float w_motion, float w_temp
) {
    return w_hr * hr + w_hrv * hrv + w_gsr * gsr + w_motion * motion + w_temp * temp;
}

float FusionIndex::kalman(float measurement) {
    const float q              = 0.01f;
    const float error_measure  = 2.0f;
    float gain         = kalman_error / (kalman_error + error_measure);
    kalman_estimate    = kalman_estimate + gain * (measurement - kalman_estimate);
    kalman_error       = (1.0f - gain) * kalman_error + fabsf(kalman_estimate - measurement) * q;
    return kalman_estimate;
}

float FusionIndex::smooth(float current) {
    const float alpha = 0.3f;
    smooth_prev = alpha * current + (1.0f - alpha) * smooth_prev;
    return smooth_prev;
}

// ==========================================================================
// FUNGSI UTAMA — Menerima data mentah, mengembalikan Stress Index 0–100
//
// Penanganan nilai 0 / invalid per sensor:
//
//  HR    = 0  → Sensor tidak dipakai, compute() tidak boleh dipanggil dari main.
//               Tapi jika dipanggil, gunakan NEUTRAL_HR agar tidak bias.
//
//  HRV   = -1 → Sinyal "data belum cukup" dari calculateHRV().
//               Gunakan NEUTRAL_HRV (45ms) agar stres tidak tampak tinggi/rendah.
//
//  GSR   = 0  → Elektroda lepas (setelah last-known-good di GSR.cpp habis).
//               Gunakan NEUTRAL_GSR. GSR normal rendah (kulit kering) < 200 ADC,
//               bukan 0 terus-menerus.
//
//  Temp  = 0  → Tidak akan terjadi (MLX90614.cpp sudah pakai last-known-good).
//               Tapi jika sampai 0: gunakan NEUTRAL_TEMP.
//
//  Accel = 0,0,0 → Hardware fault MPU6050. motionNet() = |0| - 9.81 → -9.81 → 0.
//               Dianggap "diam" → motion_n=0. Aman karena tidak bias stres.
// ==========================================================================
float FusionIndex::compute(
    float hr,
    float hrv,
    float gsr,
    float temperature,
    float ax, float ay, float az
) {
    // ---------- Sanitasi input ----------
    // HR
    float hr_in = (hr <= 0) ? NEUTRAL_HR : hr;

    // HRV: -1 = data belum cukup → gunakan netral agar hrv_n = 50% (tidak bias)
    float hrv_in = (hrv < 0) ? NEUTRAL_HRV : hrv;

    // GSR
    float gsr_in = (gsr < 0) ? NEUTRAL_GSR : gsr;

    // Suhu
    float temp_in = (temperature <= 0 || isnan(temperature)) ? NEUTRAL_TEMP : temperature;

    // Motion: motionNet() sudah handle negatif → floor ke 0
    float motion_in = motionNet(ax, ay, az);

    // ---------- Moving Average (noise reduction) ----------
    float hr_f     = movingAverage(hr_in,     hr_buffer);
    float hrv_f    = movingAverage(hrv_in,    hrv_buffer);
    float gsr_f    = movingAverage(gsr_in,    gsr_buffer);
    float temp_f   = movingAverage(temp_in,   temp_buffer);
    float motion_f = movingAverage(motion_in, motion_buffer);

    index++;
    if (index >= 5) index = 0;

    // ---------- Normalisasi ke 0–100% ----------
    float hr_n     = normalize(hr_f,     60.0f,  140.0f);

    // HRV diinversi: HRV tinggi = rileks = stres rendah
    // RMSSD normal: ~10ms (stres berat) – ~80ms (sangat rileks)
    // Dengan NEUTRAL_HRV=45ms → hrv_n = 100 - normalize(45, 10, 80) = 100 - 50 = 50% ✓
    float hrv_n    = 100.0f - normalize(hrv_f, 10.0f, 80.0f);

    // GSR raw ADC 12-bit
    float gsr_n    = normalize(gsr_f,    0.0f,   4095.0f);

    // Suhu kulit: 36°C normal (0%) → 39°C sangat tinggi/stres (100%)
    float temp_n   = normalize(temp_f,   36.0f,  39.0f);

    // Net motion: 0 = diam, ≥5 m/s² = sangat aktif
    float motion_n = normalize(motion_f, 0.0f,   5.0f);

    // ---------- Adaptive Weighting ----------
    float w_hr, w_hrv, w_gsr, w_motion, w_temp;
    adaptiveWeights(motion_n, w_hr, w_hrv, w_gsr, w_motion, w_temp);

    // ---------- Weighted Fusion ----------
    float fusedVal = fuse(hr_n, hrv_n, gsr_n, motion_n, temp_n,
                          w_hr, w_hrv, w_gsr, w_motion, w_temp);

    // ---------- Kalman → EMA ----------
    return smooth(kalman(fusedVal));
}