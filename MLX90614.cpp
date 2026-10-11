#include "MLX90614.h"

static Adafruit_MLX90614 mlx = Adafruit_MLX90614();
static float lastValidTemp = 36.5f; // Suhu default fisiologis normal

void initMLX() {
    if (!mlx.begin()) {
        Serial.println("[ERROR] MLX90614 tidak terdeteksi! Cek kabel I2C.");
        while (1) delay(100);
    }
    Serial.println("[OK] MLX90614 siap.");
}

float readTemperature() {
    float temp = mlx.readObjectTempC();

    // Validasi: NaN, 0, atau di luar rentang fisiologis kulit manusia (30–42°C)
    if (isnan(temp) || temp == 0.0f || temp < 30.0f || temp > 42.0f) {
        // Kembalikan nilai terakhir yang valid (bukan 0 atau NaN)
        return lastValidTemp;
    }

    lastValidTemp = temp; // Simpan sebagai last-known-good
    return temp;
}
