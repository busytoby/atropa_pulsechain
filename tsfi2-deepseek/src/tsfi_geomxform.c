#include "tsfi_geomxform.h"
#include <string.h>

void tsfi_geomxform_init(TSFiGeomXform *xform, const char *path) {
    if (!xform) return;
    
    if (path) {
        strncpy(xform->xform_path, path, sizeof(xform->xform_path) - 1);
    } else {
        strcpy(xform->xform_path, "/auncient/xform/main");
    }
    
    // Set identity matrix
    memset(xform->matrix, 0, sizeof(xform->matrix));
    xform->matrix[0][0] = 1.0f;
    xform->matrix[1][1] = 1.0f;
    xform->matrix[2][2] = 1.0f;
    xform->matrix[3][3] = 1.0f;
}

void tsfi_geomxform_translate(TSFiGeomXform *xform, float tx, float ty, float tz) {
    if (!xform) return;
    
    // Post-multiply by translation matrix:
    // M = M * T
    xform->matrix[0][3] += xform->matrix[0][0] * tx + xform->matrix[0][1] * ty + xform->matrix[0][2] * tz;
    xform->matrix[1][3] += xform->matrix[1][0] * tx + xform->matrix[1][1] * ty + xform->matrix[1][2] * tz;
    xform->matrix[2][3] += xform->matrix[2][0] * tx + xform->matrix[2][1] * ty + xform->matrix[2][2] * tz;
    xform->matrix[3][3] += xform->matrix[3][0] * tx + xform->matrix[3][1] * ty + xform->matrix[3][2] * tz;
}

void tsfi_geomxform_scale(TSFiGeomXform *xform, float sx, float sy, float sz) {
    if (!xform) return;
    
    // Post-multiply by scaling matrix:
    // M = M * S
    for (int i = 0; i < 4; i++) {
        xform->matrix[i][0] *= sx;
        xform->matrix[i][1] *= sy;
        xform->matrix[i][2] *= sz;
    }
}

void tsfi_geomxform_transform_vector(const TSFiGeomXform *xform, const float input[3], float output[3]) {
    if (!xform || !input || !output) return;
    
    float x = xform->matrix[0][0] * input[0] + xform->matrix[0][1] * input[1] + xform->matrix[0][2] * input[2] + xform->matrix[0][3];
    float y = xform->matrix[1][0] * input[0] + xform->matrix[1][1] * input[1] + xform->matrix[1][2] * input[2] + xform->matrix[1][3];
    float z = xform->matrix[2][0] * input[0] + xform->matrix[2][1] * input[1] + xform->matrix[2][2] * input[2] + xform->matrix[2][3];
    float w = xform->matrix[3][0] * input[0] + xform->matrix[3][1] * input[1] + xform->matrix[3][2] * input[2] + xform->matrix[3][3];
    
    if (w != 0.0f && w != 1.0f) {
        output[0] = x / w;
        output[1] = y / w;
        output[2] = z / w;
    } else {
        output[0] = x;
        output[1] = y;
        output[2] = z;
    }
}
