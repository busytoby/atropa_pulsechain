#include "tsfi_vactrolpickup.h"
#include <math.h>

void tsfi_vactrolpickup_init(TSFiVactrolPickup *vp, double speed, double led_current) {
    if (!vp) return;
    
    vp->tonewheel_speed = speed;
    vp->vactrol_led_current = led_current;
    vp->vactrol_resistance = 1000.0; // Default dark resistance
    vp->pickup_signal = 0.0;
}

void tsfi_vactrolpickup_update(TSFiVactrolPickup *vp, double dt, double angle_rad) {
    if (!vp) return;
    (void)dt;
    
    // Light output is modulated by the tonewheel teeth (simulated by sine of the rotation angle)
    double teeth_mod = 0.5 * (1.0 + sin(angle_rad));
    double effective_light = vp->vactrol_led_current * teeth_mod;
    
    // Vactrol transfer function: LDR resistance decreases as light intensity increases
    if (effective_light > 0.01) {
        vp->vactrol_resistance = 10000.0 / (effective_light + 1.0) + 100.0;
    } else {
        vp->vactrol_resistance = 100000.0; // High dark resistance
    }
    
    // Output signal is inversely proportional to resistance (simulating a voltage divider)
    vp->pickup_signal = 5.0 * (1000.0 / (vp->vactrol_resistance + 1000.0));
}

double tsfi_vactrolpickup_get_signal(const TSFiVactrolPickup *vp) {
    if (!vp) return 0.0;
    
    return vp->pickup_signal;
}
