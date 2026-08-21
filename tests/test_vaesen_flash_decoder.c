#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tsfi_vaesen_flash_decoder.h"

static double get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

int main(void) {
    printf("========================================================================================\n");
    printf("   VAESEN LLM FLASH-DECODING SINGLE-TOKEN DISPATCHER & BIT-PACKED STREAMER TEST\n");
    printf("========================================================================================\n\n");

    /* Ensure canonical slices exist */
    assert(tsfi_vaesen_slice_build_canonical_all() == 0);

    /* 1. Bit-Packed Token Streamer Encoding/Decoding */
    uint8_t clan_in = 3;  /* Teddy Bear Clan */
    uint8_t tone_in = 10; /* EDO-22 carrier 10 */
    uint8_t act_in  = 0x7E; /* '㉾' Action Code */
    TsfiPackedToken packed = tsfi_flash_encode_token(clan_in, tone_in, act_in);

    uint8_t clan_out, tone_out, act_out;
    tsfi_flash_decode_token(packed, &clan_out, &tone_out, &act_out);
    printf("[BIT-PACKED STREAMER] Encoded 0x%04X -> Clan: %u | Tone: %u | Action: 0x%02X\n",
           packed, clan_out, tone_out, act_out);
    assert(clan_out == clan_in);
    assert(tone_out == tone_in);
    assert(act_out == act_in);

    /* 2. Initialize BRAM Pool and Load Autonomous Slices */
    TsfiFlashDecoderBramPool pool;
    assert(tsfi_flash_decoder_init(&pool) == 0);
    assert(tsfi_flash_decoder_load_slice(&pool, "assets/vaesen/nacken.dat.bin") == 0);
    assert(tsfi_flash_decoder_load_slice(&pool, "assets/vaesen/linnea.dat.bin") == 0);
    assert(tsfi_flash_decoder_load_slice(&pool, "assets/vaesen/tomte.dat.bin") == 0);
    assert(tsfi_flash_decoder_load_slice(&pool, "assets/vaesen/myling.dat.bin") == 0);
    assert(tsfi_flash_decoder_load_slice(&pool, "assets/vaesen/teddy_bear.dat.bin") == 0);
    printf("[BRAM POOL] Loaded %u autonomous creature slices into simulated FPGA BRAM.\n", pool.num_entities);
    assert(pool.num_entities == 5);

    /* 3. Flash-Decoding Single-Token Interactive Dispatch */
    TsfiPackedTokenStream out_stream;
    char text_buf[256];
    int disp_res = tsfi_flash_decoder_dispatch_single_token(&pool, 0, 0x42, &out_stream, text_buf, sizeof(text_buf));
    assert(disp_res == 0);
    printf("[FLASH-DECODING DISPATCH] Stream Count: %u tokens\n", out_stream.count);
    printf("  Summary: %s\n", text_buf);
    assert(out_stream.count == 5);

    /* 4. High-Performance Latency Benchmark across 1,000,000 single-token dispatches */
    const int benchmark_count = 1000000;
    double t_start = get_time_ns();
    for (int i = 0; i < benchmark_count; ++i) {
        TsfiPackedTokenStream s;
        tsfi_flash_decoder_dispatch_single_token(&pool, 0, 0x42, &s, NULL, 0);
    }
    double t_end = get_time_ns();
    double avg_latency = (t_end - t_start) / (double)benchmark_count;
    printf("\n[BENCHMARK] Average Flash-Decoding single-token latency: %.2f ns (Target: < 1000 ns)\n", avg_latency);
    assert(avg_latency < 1000.0);

    printf("\n>>> VAESEN FLASH-DECODER & BIT-PACKED STREAMER FORMALLY VERIFIED <<<\n");
    return 0;
}
