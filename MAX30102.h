#ifndef MAX30102_H
#define MAX30102_H

#include <Arduino.h>
#include "MAX30105.h"
#include "heartRate.h"

void     initMAX30102();
float    readHeartRate(unsigned long &ibi);
float    calculateHRV();          // -1 jika data belum cukup (< 2 IBI sampel)
uint32_t getIRValue();

#endif
