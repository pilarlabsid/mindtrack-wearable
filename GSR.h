#ifndef GSR_H
#define GSR_H

#include <Arduino.h>
#include "pins_config.h"   // Sumber pin terpusat — PIN_GSR didefinisikan di sini

void initGSR();
int  readGSR();

#endif
