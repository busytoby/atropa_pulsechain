#include "tsfi_npnpnpvalve.h"
#include <math.h>

void tsfi_npnpnpvalve_init(TSFiNpnPnpValve *valve, double cap) {
    if (!valve) return;
    
    valve->npn_charge = 0.0;
    valve->pnp_charge = 0.0;
    valve->coupling_capacitance = (cap > 0.0) ? cap : 1e-6;
    valve->valve_conductance = 0.0;
}

void tsfi_npnpnpvalve_update(TSFiNpnPnpValve *valve, double dt, double input_voltage) {
    if (!valve) return;
    if (dt <= 0.0) dt = 0.001;
    
    // NPN node charges up from input voltage
    double npn_delta = (input_voltage - valve->npn_charge) * (1.0 / (1000.0 * valve->coupling_capacitance)) * dt;
    valve->npn_charge += npn_delta;
    
    // PNP node acts as an inverted discharge path coupled via the capacitor
    double pnp_delta = (-input_voltage - valve->pnp_charge) * (1.0 / (1000.0 * valve->coupling_capacitance)) * dt;
    valve->pnp_charge += pnp_delta;
    
    // Valve conductance is modulated by the absolute difference in charge (potential threshold)
    double potential_diff = fabs(valve->npn_charge - valve->pnp_charge);
    valve->valve_conductance = 1.0 / (1.0 + exp(-5.0 * (potential_diff - 0.7))); // Sigmoid switch around 0.7V threshold
}

double tsfi_npnpnpvalve_get_conductance(const TSFiNpnPnpValve *valve) {
    if (!valve) return 0.0;
    
    return valve->valve_conductance;
}
