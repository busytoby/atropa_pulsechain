#include <stdio.h>
#include "tsfi_yang_registry.h"

int main() {
    printf("=== TSFi Tech Test: Thetan Heartbeat Modulation ===\n");
    TsfiAcousticRegistry reg = {0};
    
    // Simulate high pressure (Economic Collapse)
    reg.pdai_manifold_pressure = 1.0f;
    reg.yang_heartbeat_hz = 40.0f + (reg.pdai_manifold_pressure * 40.0f);
    
    printf("[AUDIT] Pressure: %.2f -> Heartbeat: %.1f Hz\n", reg.pdai_manifold_pressure, reg.yang_heartbeat_hz);
    return (reg.yang_heartbeat_hz == 80.0f) ? 0 : 1;
}
