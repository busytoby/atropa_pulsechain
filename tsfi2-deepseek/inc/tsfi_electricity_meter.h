#ifndef TSFI_ELECTRICITY_METER_H
#define TSFI_ELECTRICITY_METER_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    double fet_charge_joules;      // E = 0.5 * C * V^2
    double scsi_bus_power_joules;  // E = I * V * t
    double vulkan_render_joules;   // E = P * t
    double total_micro_joules;     // Total Energy in Micro-Joules
    uint64_t knuth_packed_energy;  // Base 2i Packed (Voltage, Current)
} tsfi_electricity_meter_t;

/* Initialize Physical Electricity Metering System */
int tsfi_electricity_meter_init(tsfi_electricity_meter_t *meter);

/* Compute total electricity energy draw (Joules) for a transaction step */
int tsfi_electricity_meter_calculate(
    float v_gate,
    float c_farads,
    float i_bus_amps,
    float bus_volts,
    float duration_seconds,
    tsfi_electricity_meter_t *out_meter
);

#endif // TSFI_ELECTRICITY_METER_H
