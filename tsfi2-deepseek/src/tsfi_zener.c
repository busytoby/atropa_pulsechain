#include "tsfi_zener.h"
#include <math.h>
#include <stdlib.h>

void tsfi_zener_init(TsfiZener *zener, double vz0, double temp_coeff, double sample_rate) {
    if (!zener) return;
    zener->Is = 1.0e-12;        // 1 pA forward saturation current
    zener->Isz = 1.0e-5;        // 10 uA breakdown scaling current
    zener->Vt = 0.026;          // 26 mV thermal voltage
    zener->N = 1.5;             // Emission coefficient
    zener->Vz0 = vz0;
    zener->temp_coeff = temp_coeff;
    zener->temp = 0.0;
    zener->thermal_res = 150.0; // 150 deg C / Watt
    zener->cooling_rate = 15.0; // cooling rate
    zener->sample_rate = sample_rate;
    zener->noise_amp = 0.005;   // 5 mV base noise amplitude
    zener->circuit.vs = 0.0;
    zener->circuit.rs = 1000.0;
}

double tsfi_zener_get_vz(const TsfiZener *zener) {
    // Vz shifts linearly with temperature deviation
    double vz = zener->Vz0 + zener->temp_coeff * zener->temp;
    return (vz < 1.0) ? 1.0 : vz; // Clamp to prevent breakdown voltage folding into forward bias
}

double tsfi_zener_tick(TsfiZener *zener, double vs, double rs, double *noise_out) {
    zener->circuit.vs = vs;
    zener->circuit.rs = rs;
    double dt = 1.0 / zener->sample_rate;
    double vz = tsfi_zener_get_vz(zener);
    
    // Solve non-linear current equation using Newton-Raphson:
    // F(vd) = Is * (exp(vd / (N*Vt)) - 1) - Isz * (exp((-vd - Vz)/Vt) - 1) - (vs - vd)/rs = 0
    double vd = 0.0; // Initial guess
    double nvt = zener->N * zener->Vt;
    
    for (int iter = 0; iter < 100; iter++) {
        double exp_f = exp(vd / nvt);
        double exp_r = exp((-vd - vz) / zener->Vt);
        
        double f_vd = zener->Is * (exp_f - 1.0) - zener->Isz * (exp_r - 1.0);
        double f_prime_vd = (zener->Is / nvt) * exp_f + (zener->Isz / zener->Vt) * exp_r;
        
        double F = f_vd - (vs - vd) / rs;
        double F_prime = f_prime_vd + 1.0 / rs;
        
        double diff = F / F_prime;
        // Clamp the step size to prevent numerical divergence/overflow in the exponential terms
        if (diff > 0.1) diff = 0.1;
        if (diff < -0.1) diff = -0.1;
        
        vd -= diff;
        if (fabs(diff) < 1.0e-10) {
            break;
        }
    }
    
    // Calculate diode current: Id = (Vs - Vd) / Rs
    double id = (vs - vd) / rs;
    
    // Update thermal state (power dissipated P = Vd * Id)
    double power = fabs(vd * id);
    zener->temp += dt * (power * zener->thermal_res - zener->temp * zener->cooling_rate);
    
    // Generate avalanche dither noise if the diode is operating in reverse breakdown
    if (vd < -0.9 * vz) {
        // Generate white noise in [-1.0, 1.0]
        double r_noise = 2.0 * ((double)rand() / (double)RAND_MAX) - 1.0;
        // Noise is proportional to breakdown current magnitude
        double current_factor = fmin(5.0, fabs(id) * 100.0);
        *noise_out = r_noise * zener->noise_amp * current_factor;
    } else {
        *noise_out = 0.0;
    }
    
    return vd;
}
