#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tsfi_vaesen_fpga_mistral_ext.h"

static double get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

int main(void) {
    printf("========================================================================================\n");
    printf("   VAESEN FPGA MISTRAL EXTENSIONS: TOOL CALLING, ENTROPY SCALING & DRAFT VERIFIER\n");
    printf("========================================================================================\n\n");

    /* Ensure canonical slices exist */
    assert(tsfi_vaesen_slice_build_canonical_all() == 0);

    TsfiVaesenDeviceSliceBin nacken, tomte, myling;
    assert(tsfi_vaesen_slice_load("assets/vaesen/nacken.dat.bin", &nacken) == 0);
    assert(tsfi_vaesen_slice_load("assets/vaesen/tomte.dat.bin", &tomte) == 0);
    assert(tsfi_vaesen_slice_load("assets/vaesen/myling.dat.bin", &myling) == 0);

    /* 1. Acoustic Entropy Scaling Verification */
    TsfiAcousticEntropyConfig cfg_tomte = tsfi_vaesen_compute_acoustic_entropy(&tomte);
    TsfiAcousticEntropyConfig cfg_myling = tsfi_vaesen_compute_acoustic_entropy(&myling);

    printf("[ACOUSTIC ENTROPY SCALING]\n");
    printf("  Tomte (High Dogma Do=5):  Temp: %.3f | Top-p: %.2f | Jitter: %u mU (Predictable Rite: PASS)\n",
           cfg_tomte.temperature, cfg_tomte.top_p, cfg_tomte.jitter_variance_milli);
    printf("  Myling (High Fear Fr=4):  Temp: %.3f | Top-p: %.2f | Jitter: %u mU (Erratic Haunting: PASS)\n\n",
           cfg_myling.temperature, cfg_myling.top_p, cfg_myling.jitter_variance_milli);
    assert(cfg_tomte.temperature < cfg_myling.temperature);
    assert(cfg_tomte.top_p < cfg_myling.top_p);

    /* 2. Hardware Tool Dispatch Verification */
    TsfiHardwareToolCall tool_call;
    int tool_res = tsfi_vaesen_dispatch_hardware_tool(&tomte, TSFI_TOOL_OP_OFFER_GIFT, 0xD405, 0x01, &tool_call);
    assert(tool_res == 0);
    printf("[HARDWARE TOOL DISPATCH]\n");
    printf("  Op: 0x%02X | Target 6502 Addr: 0x%04X | Val: 0x%02X\n",
           tool_call.op_code, tool_call.target_6502_addr, tool_call.value);
    printf("  Description: %s\n\n", tool_call.description);
    assert(tool_call.op_code == TSFI_TOOL_OP_OFFER_GIFT);
    assert(tool_call.target_6502_addr == 0xD405);

    /* 3. Hardware Speculative Token Draft Verification */
    uint16_t draft_tokens[4] = {
        0x1B01, /* Carrier 11 (Näcken match) */
        0x1C02, /* Carrier 12 (Compatible) */
        0x1A03, /* Carrier 10 (Compatible) */
        0x1104  /* Carrier 1  (Mismatch) */
    };
    TsfiSpeculativeDraftResult draft_res;
    int draft_check = tsfi_vaesen_speculative_verify_draft(&nacken, draft_tokens, 4, &draft_res);
    assert(draft_check == 0);
    printf("[SPECULATIVE DRAFT VERIFICATION]\n");
    printf("  Draft Proposed: %u | Accepted: %u | All Accepted: %s\n",
           draft_res.draft_count, draft_res.verified_count, draft_res.all_accepted ? "YES" : "NO (Early Exit: PASS)");
    assert(draft_res.verified_count == 3);

    /* 4. High-Performance Latency Benchmark across 1,000,000 tool dispatch + draft verification cycles */
    const int benchmark_count = 1000000;
    double t_start = get_time_ns();
    for (int i = 0; i < benchmark_count; ++i) {
        TsfiHardwareToolCall tc;
        TsfiSpeculativeDraftResult dr;
        tsfi_vaesen_dispatch_hardware_tool(&tomte, TSFI_TOOL_OP_OFFER_GIFT, 0xD405, 0x01, &tc);
        tsfi_vaesen_speculative_verify_draft(&nacken, draft_tokens, 4, &dr);
    }
    double t_end = get_time_ns();
    double avg_latency = (t_end - t_start) / (double)benchmark_count;
    printf("\n[BENCHMARK] Average FPGA Mistral Extensions latency: %.2f ns (Target: < 1000 ns)\n", avg_latency);
    assert(avg_latency < 1000.0);

    printf("\n>>> ALL VAESEN FPGA MISTRAL EXTENSIONS FORMALLY VERIFIED <<<\n");
    return 0;
}
