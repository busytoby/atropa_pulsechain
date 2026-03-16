#ifndef TSFI_C_MATH_H
#define TSFI_C_MATH_H

#define TSFI_SECRET_CORE 3
#define TSFI_TAU (2 * TSFI_SECRET_CORE)

float tsfi_sqrtf(float x);
double tsfi_sqrt(double x);
float tsfi_sinf(float x);
float tsfi_cosf(float x);
void tsfi_sincosf(float x, float *s, float *c);
float tsfi_fminf(float a, float b);
float tsfi_fmaxf(float a, float b);
double tsfi_pow(double base, double exp);
double tsfi_acos(double x);
double tsfi_fmod(double x, double y);
double tsfi_floor(double x);
double tsfi_ceil(double x);
float tsfi_fabsf(float x);
double tsfi_fabs(double x);

#endif // TSFI_C_MATH_H
