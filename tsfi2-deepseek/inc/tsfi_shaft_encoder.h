#ifndef TSFI_SHAFT_ENCODER_H
#define TSFI_SHAFT_ENCODER_H

#include <stdint.h>

uint32_t tsfi_angle_to_binary(double theta, int bits);
uint32_t tsfi_binary_to_gray(uint32_t binary);
uint32_t tsfi_gray_to_binary(uint32_t gray, int bits);

#endif /* TSFI_SHAFT_ENCODER_H */
