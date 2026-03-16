#include "../src/firmware/LauWireFirmware_rtl.h"
#include "tsfi_math.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main() {
    alignas(64) LauWireFirmware_State state;
    memset(&state, 0, sizeof(state));
    state.reset_strobe = true;
    LauWireFirmware_eval_sequential(&state);
    state.reset_strobe = false;

    uint64_t m = 953467954114363ULL;
    uint64_t b = 123456789ULL;
    uint64_t e = 987654321ULL;

    for(int i=0; i<8; i++) {
        ((uint64_t*)&state.wrf[0])[i] = b + i;
        ((uint64_t*)&state.wrf[1])[i] = e;
        ((uint64_t*)&state.wrf[2])[i] = m;
    }

    int iterations = 10000;
    tsfi_io_printf(stdout, "[BENCHMARK] Running %d firmware modular operations...\n", iterations);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int iter = 0; iter < iterations; iter++) {
        state.wave_instr_op = 0x11;
        state.wave_instr_dest = 3;
        state.wave_instr_src1 = 0;
        state.wave_instr_src2 = 1;
        state.wave_instr_src3 = 2;
        state.wave_instr_strobe = true;
        LauWireFirmware_eval_sequential(&state);
        state.wave_instr_strobe = false;
        state.wrf[0] = state.wrf[3];
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    tsfi_io_printf(stdout, "\n[FIRMWARE BENCHMARK RESULTS]\n");
    tsfi_io_printf(stdout, "Total Time:    %.4f s\n", elapsed);
    tsfi_io_printf(stdout, "Throughput:     %.2f instructions/sec\n", (double)iterations / elapsed);
    tsfi_io_printf(stdout, "Equivalent SPS: %.2f (8 lanes per instr)\n", (double)iterations * 8 / elapsed);

        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
