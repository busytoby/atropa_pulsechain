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

    uint64_t m1 = 953467954114363ULL;
    uint64_t m2 = 953467954114361ULL;
    uint64_t b = 123456789ULL;
    uint64_t e = 987654321ULL;

    tsfi_io_printf(stdout, "[BARRIER] Starting Interleaved Session Test...\n");

    for (int iter = 0; iter < 10; iter++) {
        for(int i=0; i<8; i++) {
            ((uint64_t*)&state.wrf[0])[i] = b + i + iter;
            ((uint64_t*)&state.wrf[1])[i] = e;
            ((uint64_t*)&state.wrf[2])[i] = m1;
        }
        state.wave_instr_op = 0x11;
        state.wave_instr_dest = 3;
        state.wave_instr_src1 = 0;
        state.wave_instr_src2 = 1;
        state.wave_instr_src3 = 2;
        state.wave_instr_strobe = true;
        LauWireFirmware_eval_sequential(&state);
        state.wave_instr_strobe = false;
        
        for(int i=0; i<8; i++) {
            uint64_t res = ((uint64_t*)&state.wrf[3])[i];
            TSFiBigInt bn_b, bn_e, bn_m, bn_res;
            tsfi_bn_set_u64(&bn_b, b + i + iter);
            tsfi_bn_set_u64(&bn_e, e);
            tsfi_bn_set_u64(&bn_m, m1);
            tsfi_bn_modpow_avx512(&bn_res, &bn_b, &bn_e, &bn_m);
            if (res != bn_res.limbs[0]) {
                tsfi_io_printf(stderr, "[FAIL] Session A, Iter %d, Lane %d mismatched.\n", iter, i);
                return 1;
            }
        }

        for(int i=0; i<8; i++) {
            ((uint64_t*)&state.wrf[4])[i] = b + i + iter;
            ((uint64_t*)&state.wrf[5])[i] = e;
            ((uint64_t*)&state.wrf[6])[i] = m2;
        }
        state.wave_instr_op = 0x11;
        state.wave_instr_dest = 7;
        state.wave_instr_src1 = 4;
        state.wave_instr_src2 = 5;
        state.wave_instr_src3 = 6;
        state.wave_instr_strobe = true;
        LauWireFirmware_eval_sequential(&state);
        state.wave_instr_strobe = false;

        for(int i=0; i<8; i++) {
            uint64_t res = ((uint64_t*)&state.wrf[7])[i];
            TSFiBigInt bn_b, bn_e, bn_m, bn_res;
            tsfi_bn_set_u64(&bn_b, b + i + iter);
            tsfi_bn_set_u64(&bn_e, e);
            tsfi_bn_set_u64(&bn_m, m2);
            tsfi_bn_modpow_avx512(&bn_res, &bn_b, &bn_e, &bn_m);
            if (res != bn_res.limbs[0]) {
                tsfi_io_printf(stderr, "[FAIL] Session B, Iter %d, Lane %d mismatched.\n", iter, i);
                return 1;
            }
        }
        tsfi_io_printf(stdout, "Iteration %d passed (Cache swapped correctly).\n", iter);
    }

    tsfi_io_printf(stdout, "[SUCCESS] Batch Barrier / Cache Invalidation verified.\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
