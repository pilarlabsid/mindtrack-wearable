#ifndef MPU6050_H
#define MPU6050_H

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

void initMPU();
float readMotion(sensors_event_t &accel);

#endif
