#include <stdio.h>
#include <assert.h>
#include "tsfi_schrodinger_quadrature.h"

int main() {
    printf("[TEST] Initializing Schrodinger's Quadrature Circuit...\n");
    
    tsfi_SchrodingerQuadCircuit circuit;
    tsfi_schrodinger_quad_init(&circuit, 9999, 12000);
    
    assert(circuit.Base == 9999);
    assert(circuit.MangleEnabled == true);
    
    printf("[TEST] Stepping circuit for 100 clock cycles...\n");
    for (int i = 0; i < 100; i++) {
        tsfi_schrodinger_quad_step(&circuit, 0.001, 60);
        double f_l, f_r;
        tsfi_schrodinger_quad_resolve(&circuit, &f_l, &f_r);
        assert(f_l >= 100.0 && f_l <= 2200.0);
        assert(f_r >= 100.0 && f_r <= 2200.0);
    }
    
    printf("[SUCCESS] Schrodinger's Quadrature Circuit verified successfully!\n");
    return 0;
}
