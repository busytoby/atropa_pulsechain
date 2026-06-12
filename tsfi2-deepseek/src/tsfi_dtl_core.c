#include "tsfi_dtl_core.h"
#include <math.h>
#include <string.h>

float tsfi_dtl_nand(float v_in1, float v_in2) {
    // Minimum input pulls the diode node low
    float min_in = (v_in1 < v_in2) ? v_in1 : v_in2;
    
    // Threshold voltage: 0.7V BJT base + 1.4V (2 shift diodes) - 0.7V input diode = 1.4V
    float threshold = 1.4f;
    
    if (min_in >= threshold) {
        // Transistor saturates
        return 0.2f; // Vce,sat
    } else {
        // Transistor cut off
        return 5.0f; // Vcc
    }
}

void tsfi_core_init(TsfiMagneticCore *core) {
    memset(core, 0, sizeof(TsfiMagneticCore));
    core->B = -1.0f; // Initial state 0
    core->last_H = 0.0f;
    core->dir = 1;
}

float tsfi_core_update(TsfiMagneticCore *core, float H_current) {
    // H_c = 1.0f (coercivity threshold)
    // If drive current H exceeds H_c, the core magnetization flips to +1.0
    // If drive current H falls below -H_c, it flips to -1.0
    if (H_current > 1.0f) {
        core->B = 1.0f;
    } else if (H_current < -1.0f) {
        core->B = -1.0f;
    }
    // Otherwise, magnetization B is retained (remanence)
    return core->B;
}
