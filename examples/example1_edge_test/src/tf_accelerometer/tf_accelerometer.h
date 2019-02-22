
#ifndef TF_ACCELEROMETER_H
#define TF_ACCELEROMETER_H

#include "lis2dh12_reg.h"

#include <string.h> // Temporary, just for memset

int initAccelerometer(void);

extern lis2dh12_ctx_t dev_ctx;

#endif /* TF_ACCELEROMETER_H */