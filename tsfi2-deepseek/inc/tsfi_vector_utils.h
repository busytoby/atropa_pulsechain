#ifndef TSFI_VECTOR_UTILS_H
#define TSFI_VECTOR_UTILS_H

#include <math.h>
#include "tsfi_pbr.h"

static inline Vector3 v_sub(Vector3 a, Vector3 b) { return (Vector3){a.x - b.x, a.y - b.y, a.z - b.z}; }
static inline float v_dot(Vector3 a, Vector3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static inline Vector3 v_cross(Vector3 a, Vector3 b) { return (Vector3){a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x}; }
static inline Vector3 v_normalize(Vector3 v) {
    float mag = sqrtf(v_dot(v, v)) + 1e-6f;
    return (Vector3){v.x / mag, v.y / mag, v.z / mag};
}

#endif
