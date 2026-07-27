#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"
#include "lau_yul_thunk.h"

#define BENCH_ITERATIONS 50000

int main(void) {
    printf("=========================================================\n");
    printf("AUNCIENT ZMM VM: BTC RAILS OBJECT DEPLOYMENT AND BENCHMARK\n");
    printf("=========================================================\n");

    // Clean previous storage state files
    remove("evm_storage.json");
    remove("tsfi2-deepseek/evm_storage.json");

    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Deploy BtcRailsVM to the ZMM VM
    const char *yul_path = "../solidity/bin/btc_rails_vm.yul";
    FILE *f_yul = fopen(yul_path, "r");
    if (f_yul) {
        fclose(f_yul);
    } else {
        yul_path = "solidity/bin/btc_rails_vm.yul";
    }

    printf("Deploying btc_rails_vm to ZMM...\n");
    char init_cmd[256];
    snprintf(init_cmd, sizeof(init_cmd), "YULINIT \"btc_rails_vm\", \"%s\", 1024", yul_path);
    tsfi_zmm_vm_exec(&vm, init_cmd);

    // 2. Formulate YULEXEC command representing OP_EXECUTE
    // Program: PUSH 5, PUSH 10, ADD, HALT
    // Calldata payload = selector (22137682) + offset (40) + cycles (64) + len (0c) + data (0100000005010000000a0206...)
    const char *exec_cmd = "YULEXEC \"btc_rails_vm\", \"2213768200000000000000000000000000000000000000000000000000000000000000400000000000000000000000000000000000000000000000000000000000000064000000000000000000000000000000000000000000000000000000000000000c0100000005010000000a02060000000000000000000000000000000000000000\"";

    printf("Executing initial transaction verification...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, exec_cmd);

    printf("Output buffer:\n%s\n", vm.output_buffer);

    // Benchmark performance
    printf("Running %d ZMM VM execution loop iterations (command parsing)...\n", BENCH_ITERATIONS);
    fflush(stdout);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < BENCH_ITERATIONS; i++) {
        vm.output_pos = 0;
        tsfi_zmm_vm_exec(&vm, exec_cmd);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_cmd = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    // Decode hex once for the direct thunk execution loop
    const char *hex_data = "2213768200000000000000000000000000000000000000000000000000000000000000400000000000000000000000000000000000000000000000000000000000000064000000000000000000000000000000000000000000000000000000000000000c0100000005010000000a02060000000000000000000000000000000000000000";
    size_t cd_len = 0;
    uint8_t cd_bytes[4096];
    size_t hex_len = strlen(hex_data);
    for (size_t i = 0; i < hex_len; i += 2) {
        unsigned int byteval = 0;
        sscanf(&hex_data[i], "%02x", &byteval);
        cd_bytes[cd_len++] = (uint8_t)byteval;
    }



    printf("Running %d ZMM VM execution loop iterations (direct thunk execution)...\n", BENCH_ITERATIONS * 100);
    fflush(stdout);

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < BENCH_ITERATIONS * 100; i++) {
        lau_yul_thunk_execute("btc_rails_vm", cd_bytes, cd_len, NULL, NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_direct = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    double lps_cmd = BENCH_ITERATIONS / elapsed_cmd;
    double lips_cmd = lps_cmd * 4.0;

    double lps_direct = (BENCH_ITERATIONS * 100) / elapsed_direct;
    double lips_direct = lps_direct * 4.0;

    printf("\nZMM Benchmark Results:\n");
    printf("  [Command Parsing Mode] Elapsed: %.4f s | LIPS: %.2f M LIPS\n", elapsed_cmd, lips_cmd / 1e6);
    printf("  [Direct Thunk Mode]    Elapsed: %.4f s | LIPS: %.2f M LIPS (%.2f G LIPS)\n", elapsed_direct, lips_direct / 1e6, lips_direct / 1e9);
    printf("=========================================================\n");

    tsfi_zmm_vm_destroy(&vm);
    return 0;
}
