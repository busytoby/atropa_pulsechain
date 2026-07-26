#ifndef TSFI_NPNPNPVALVE_H
#define TSFI_NPNPNPVALVE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    double npn_charge;          // Charge level of NPN node
    double pnp_charge;          // Charge level of PNP node
    double coupling_capacitance;// Capacitance in Farads
    double valve_conductance;   // Converted conductance output
} TSFiNpnPnpValve;

// Initialize the NpnPnpValve context
void tsfi_npnpnpvalve_init(TSFiNpnPnpValve *valve, double cap);

// Updates the valve charge levels and calculates conductance
void tsfi_npnpnpvalve_update(TSFiNpnPnpValve *valve, double dt, double input_voltage);

// Returns the active conductance factor of the NPN-PNP valve
double tsfi_npnpnpvalve_get_conductance(const TSFiNpnPnpValve *valve);

#endif // TSFI_NPNPNPVALVE_H
