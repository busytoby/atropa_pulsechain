#include "../src/firmware/LauWireFirmware_rtl.h"
#include "tsfi_math.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    LauWireFirmware_State state;
    memset(&state, 0, sizeof(state));
    state.reset_strobe = true;
    LauWireFirmware_eval_sequential(&state);
    state.reset_strobe = false;

    // Use a small prime for testing
    uint64_t m = 953467954114363ULL;
    uint64_t b = 123456789ULL;
    uint64_t e = 987654321ULL;

    // Load into WRF slots
    // slot 0: Base, slot 1: Exp, slot 2: Mod
    for(int i=0; i<8; i++) {
        ((uint64_t*)&state.wrf[0])[i] = b + i; 
        ((uint64_t*)&state.wrf[1])[i] = e;
        ((uint64_t*)&state.wrf[2])[i] = m;
    }

    // Trigger VDYS_MODPOW
    state.wave_instr_op = 0x11;
    state.wave_instr_dest = 3;
    state.wave_instr_src1 = 0;
    state.wave_instr_src2 = 1;
    state.wave_instr_src3 = 2;
    state.wave_instr_strobe = true;
    
    tsfi_io_printf(stdout, "[TEST] Triggering VDYS_MODPOW via Firmware...\n");
    LauWireFirmware_eval_sequential(&state);
    state.wave_instr_strobe = false;

    // Check results
    for(int i=0; i<8; i++) {
        uint64_t res = ((uint64_t*)&state.wrf[3])[i];
        
        TSFiBigInt bn_b, bn_e, bn_m, bn_res;
        tsfi_bn_set_u64(&bn_b, b + i);
        tsfi_bn_set_u64(&bn_e, e);
        tsfi_bn_set_u64(&bn_m, m);
        tsfi_bn_modpow_avx512(&bn_res, &bn_b, &bn_e, &bn_m);
        
        uint64_t expected = bn_res.limbs[0];
        if (res != expected) {
            tsfi_io_printf(stderr, "[FAIL] Lane %d: Result %lu != Expected %lu\n", i, res, expected);
            return 1;
        }
        tsfi_io_printf(stdout, "[PASS] Lane %d: Result %lu verified.\n", i, res);
    }

    tsfi_io_printf(stdout, "[SUCCESS] Firmware Math (VDYS_MODPOW) verified successfully.\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
