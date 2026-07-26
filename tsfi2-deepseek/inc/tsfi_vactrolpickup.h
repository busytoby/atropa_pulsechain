#ifndef TSFI_VACTROLPICKUP_H
#define TSFI_VACTROLPICKUP_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    double tonewheel_speed;     // Rotation speed in rad/s
    double vactrol_led_current;  // LED drive current in mA
    double vactrol_resistance;   // LDR resistance in ohms
    double pickup_signal;        // Output pickup signal voltage
} TSFiVactrolPickup;

// Initialize the VactrolPickup context
void tsfi_vactrolpickup_init(TSFiVactrolPickup *vp, double speed, double led_current);

// Updates the vactrol resistance and pickup output voltage for a time step
void tsfi_vactrolpickup_update(TSFiVactrolPickup *vp, double dt, double angle_rad);

// Computes the audio signal amplitude displacement modulated by the vactrol
double tsfi_vactrolpickup_get_signal(const TSFiVactrolPickup *vp);

#endif // TSFI_VACTROLPICKUP_H
