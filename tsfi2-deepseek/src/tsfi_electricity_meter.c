#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_electricity_meter.h"
#include "tsfi_knuth_storage.h"

int tsfi_electricity_meter_init(tsfi_electricity_meter_t *meter) {
    if (!meter) return -1;
    memset(meter, 0, sizeof(tsfi_electricity_meter_t));
    printf("[ELECTRICITY METER] Initialized Physical Energy Metering System (Joules / Micro-Joules)\n");
    return 0;
}

int tsfi_electricity_meter_calculate(
    float v_gate,
    float c_farads,
    float i_bus_amps,
    float bus_volts,
    float duration_seconds,
    tsfi_electricity_meter_t *out_meter
) {
    if (!out_meter) return -1;
    memset(out_meter, 0, sizeof(tsfi_electricity_meter_t));

    // 1. FET Gate Capacitance Charge Energy: E_fet = 0.5 * C * V^2
    out_meter->fet_charge_joules = 0.5 * (double)c_farads * (double)v_gate * (double)v_gate;

    // 2. WinchesterMQ SCSI Bus Power Draw: E_scsi = I * V * t
    out_meter->scsi_bus_power_joules = (double)i_bus_amps * (double)bus_volts * (double)duration_seconds;

    // 3. Vulkan 3D Render Energy Allocation
    out_meter->vulkan_render_joules = 0.000050; // 50 Micro-Joules per frame step

    // 4. Sum Total Micro-Joules (1 Joule = 1,000,000 Micro-Joules)
    double total_joules = out_meter->fet_charge_joules + out_meter->scsi_bus_power_joules + out_meter->vulkan_render_joules;
    out_meter->total_micro_joules = total_joules * 1000000.0;

    // 5. Pack (Voltage, Current) into Knuth Base 2i Word
    int32_t x_volts_mv = (int32_t)(v_gate * 1000.0f);
    int32_t y_current_ma = (int32_t)(i_bus_amps * 1000.0f);
    out_meter->knuth_packed_energy = tsfi_knuth_pack_base2i(x_volts_mv, y_current_ma);

    printf("[ELECTRICITY METER] FET Energy: %.6f J | SCSI Energy: %.6f J | Total: %.2f uJ (Knuth Base 2i: 0x%016lX)\n",
           out_meter->fet_charge_joules, out_meter->scsi_bus_power_joules, out_meter->total_micro_joules, (unsigned long)out_meter->knuth_packed_energy);

    return 0; // Physical electricity calculation complete
}
