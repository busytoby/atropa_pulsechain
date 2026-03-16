#ifndef TSFI_DYS_MATH_H
#define TSFI_DYS_MATH_H

#include "tsfi_math.h"

// --- The Banach Space Interface (BigInt) ---
// Mirrors dys_math.h but natively backed by TSFiBigInt

TSFiBigInt* DYSNOMIA();
TSFiBigInt* SLOPE();
TSFiBigInt* LOVE();
TSFiBigInt* GAIN();

TSFiBigInt* _();
TSFiBigInt* __();
TSFiBigInt* ___();
TSFiBigInt* ____();
TSFiBigInt* _____();
TSFiBigInt* ______();
TSFiBigInt* _______();

TSFiBigInt* RANDOM_bn(TSFiBigInt* Prime);

// --- Warm Math (Direct BigInt for performance) ---
void gcAdd_bn(TSFiBigInt* r, const TSFiBigInt* a, const TSFiBigInt* b);
void gcXOR_bn(TSFiBigInt* r, const TSFiBigInt* a, const TSFiBigInt* b);
void modPow_bn(TSFiBigInt* r, const TSFiBigInt* b, const TSFiBigInt* e, const TSFiBigInt* m);

#endif
