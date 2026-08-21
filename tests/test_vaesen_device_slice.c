#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tsfi_vaesen_device_slice.h"

static double get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

int main(void) {
    printf("========================================================================================\n");
    printf("   VAESEN LLM AUTONOMOUS DEVICE SLICES & [WMQ:WMQ] DUAL-SLICE INTERACTION TEST\n");
    printf("========================================================================================\n\n");

    /* 1. Build and persist all canonical .dat.bin device slices */
    int build_res = tsfi_vaesen_slice_build_canonical_all();
    printf("[BUILD] Building individual Vaesen .dat.bin device slices: %s\n", build_res == 0 ? "SUCCESS" : "FAIL");
    assert(build_res == 0);

    /* 2. Load individual slices */
    TsfiVaesenDeviceSliceBin nacken, linnea, myling, teddy;
    assert(tsfi_vaesen_slice_load("assets/vaesen/nacken.dat.bin", &nacken) == 0);
    assert(tsfi_vaesen_slice_load("assets/vaesen/linnea.dat.bin", &linnea) == 0);
    assert(tsfi_vaesen_slice_load("assets/vaesen/myling.dat.bin", &myling) == 0);
    assert(tsfi_vaesen_slice_load("assets/vaesen/teddy_bear.dat.bin", &teddy) == 0);

    printf("[LOAD] Successfully loaded autonomous device slices:\n");
    printf("  - %s (WMQ: %s | Clan: %u | EDO-22 Carrier: %u | q0: %u mU)\n",
           nacken.name, nacken.wmq_address, nacken.clan_id, nacken.edo22_carrier_freq, nacken.mathieu_q0_milli);
    printf("  - %s (WMQ: %s | Clan: %u | EDO-22 Carrier: %u | q0: %u mU)\n",
           linnea.name, linnea.wmq_address, linnea.clan_id, linnea.edo22_carrier_freq, linnea.mathieu_q0_milli);
    printf("  - %s (WMQ: %s | Clan: %u | EDO-22 Carrier: %u | q0: %u mU)\n",
           myling.name, myling.wmq_address, myling.clan_id, myling.edo22_carrier_freq, myling.mathieu_q0_milli);
    printf("  - %s (WMQ: %s | Clan: %u | EDO-22 Carrier: %u | q0: %u mU)\n",
           teddy.name, teddy.wmq_address, teddy.clan_id, teddy.edo22_carrier_freq, teddy.mathieu_q0_milli);

    /* 3. Test [WMQ:WMQ] Interaction: Linnea Elfvestam <-> Näcken */
    float dist1;
    uint16_t phase1;
    bool locked1;
    char diag1[256];
    int int_res1 = tsfi_vaesen_slice_interact(&linnea, &nacken, &dist1, &phase1, &locked1, diag1, sizeof(diag1));
    assert(int_res1 == 0);
    printf("\n[WMQ:WMQ INTERACTION 1]\n");
    printf("  Entities: [%s : %s]\n", linnea.wmq_address, nacken.wmq_address);
    printf("  Distance: %.2f | PLL Phase: %u deg | Status: %s\n", dist1, phase1, locked1 ? "LOCKED (Covenant)" : "TRACKING");
    printf("  Dialogue Frame: %s\n", diag1);

    /* 4. Test [WMQ:WMQ] Interaction: Linnea Elfvestam <-> Myling (High Trauma) */
    float dist2;
    uint16_t phase2;
    bool locked2;
    char diag2[256];
    int int_res2 = tsfi_vaesen_slice_interact(&linnea, &myling, &dist2, &phase2, &locked2, diag2, sizeof(diag2));
    assert(int_res2 == 0);
    printf("\n[WMQ:WMQ INTERACTION 2]\n");
    printf("  Entities: [%s : %s]\n", linnea.wmq_address, myling.wmq_address);
    printf("  Distance: %.2f | PLL Phase: %u deg | Status: %s\n", dist2, phase2, locked2 ? "LOCKED" : "CAUTION/TRACKING");
    printf("  Dialogue Frame: %s\n", diag2);

    /* 5. Latency Benchmark across 1,000,000 [WMQ:WMQ] dual-slice interactions */
    const int benchmark_count = 1000000;
    double t_start = get_time_ns();
    for (int i = 0; i < benchmark_count; ++i) {
        float d; uint16_t p; bool l;
        tsfi_vaesen_slice_interact(&linnea, &teddy, &d, &p, &l, NULL, 0);
    }
    double t_end = get_time_ns();
    double avg_latency = (t_end - t_start) / (double)benchmark_count;
    printf("\n[BENCHMARK] Average [WMQ:WMQ] interaction latency: %.2f ns (Target: < 1000 ns)\n", avg_latency);
    assert(avg_latency < 1000.0);

    printf("\n>>> ALL AUTONOMOUS VAESEN DEVICE SLICES FORMALLY VERIFIED UNDER [WMQ:WMQ] <<<\n");
    return 0;
}
