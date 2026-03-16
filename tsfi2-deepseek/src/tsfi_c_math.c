#include "tsfi_c_math.h"
#include <stdint.h>

double tsfi_fabs(double x) {
    union { double d; uint64_t i; } u = {x};
    u.i &= 0x7FFFFFFFFFFFFFFFULL;
    return u.d;
}

double tsfi_sqrt(double x) {
    if (x <= 0) return 0;
    double res = x;
    if (x > 1.0) res = x / 2.0;
    else res = 0.5;
    for (int i = 0; i < 10; i++) {
        res = 0.5 * (res + x / res);
    }
    return res;
}

double tsfi_floor(double x) {
    int64_t i = (int64_t)x;
    if (x < 0 && (double)i != x) i--;
    return (double)i;
}

double tsfi_ceil(double x) {
    int64_t i = (int64_t)x;
    if (x > 0 && (double)i != x) i++;
    return (double)i;
}

double tsfi_fmod(double x, double y) {
    if (y == 0) return 0;
    return x - y * tsfi_floor(x / y);
}

// Log/Exp for pow
static double tsfi_log(double x) {
    if (x <= 0) return -1e30; // Close to -inf
    double res = 0;
    double y = (x - 1.0) / (x + 1.0);
    double y2 = y * y;
    double term = y;
    for (int i = 0; i < 20; i++) {
        res += term / (2.0 * i + 1.0);
        term *= y2;
    }
    return 2.0 * res;
}

static double tsfi_exp(double x) {
    double res = 1.0;
    double term = 1.0;
    for (int i = 1; i < 20; i++) {
        term *= x / i;
        res += term;
    }
    return res;
}

double tsfi_pow(double base, double exp) {
    if (base == 0) return 0;
    if (exp == 0) return 1.0;
    if (base < 0) {
        // Only integer exponents supported for negative base
        if (exp == (double)(int64_t)exp) {
            double res = tsfi_exp(exp * tsfi_log(-base));
            if ((int64_t)exp % 2 != 0) return -res;
            return res;
        }
        return 0; // Error
    }
    return tsfi_exp(exp * tsfi_log(base));
}

double tsfi_acos(double x) {
    if (x > 1.0) x = 1.0;
    if (x < -1.0) x = -1.0;
    // Approximation: acos(x) = PI/2 - asin(x)
    // asin(x) = x + 1/2 * x^3/3 + 3/8 * x^5/5 + 5/16 * x^7/7
    double x2 = x * x;
    double res = x;
    double term = x;
    term *= 0.5 * x2; res += term / 3.0;
    term *= 0.75 * x2; res += term / 5.0;
    term *= 0.8333 * x2; res += term / 7.0;
    return 1.57079632679 - res;
}
