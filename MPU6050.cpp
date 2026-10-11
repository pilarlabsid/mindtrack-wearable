#include "MPU6050.h"

static Adafruit_MPU6050 mpu;
static float motionEMA = 0;
static const float motionAlpha = 0.2;

void initMPU() {
    if (!mpu.begin()) {
        Serial.println("[ERROR] MPU6050 tidak terdeteksi! Cek kabel I2C.");
        while (1) delay(100);
    }
    mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    Serial.println("[OK] MPU6050 siap.");
}

float readMotion(sensors_event_t &accel) {
    sensors_event_t gyro, temp;
    mpu.getEvent(&accel, &gyro, &temp);
    float magnitude = sqrt(
        accel.acceleration.x * accel.acceleration.x +
        accel.acceleration.y * accel.acceleration.y +
        accel.acceleration.z * accel.acceleration.z
    );
    float motion = magnitude - 9.81;
    if (motion < 0) motion = 0;
    motionEMA = (motionAlpha * motion) + ((1 - motionAlpha) * motionEMA);
    return constrain(motionEMA * 10, 0, 100);
}
