#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

typedef struct {
    TsfiZmmVmState *vm;
    int iterations;
} ThreadArg;

double get_time_in_seconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

void* run_vm_benchmark(void *arg) {
    ThreadArg *ta = (ThreadArg *)arg;
    char cmd[1024];
    // Selector for echo(address,uint256) is 0f2723ea
    sprintf(cmd, "YULEXEC \"target\", \"0f2723ea"
                  "0000000000000000000000000000000000000000000000000000000000000000"
                  "000000000000000000000000000000000000000000000000000000000000002a\"");
    for (int i = 0; i < ta->iterations; i++) {
        ta->vm->output_pos = 0;
        tsfi_zmm_vm_exec(ta->vm, cmd);
    }
    return NULL;
}

int main() {
    printf("=== TSFi ZMM VM Parallel CPU Execution & Speedup Benchmark ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm1, vm2;
    tsfi_zmm_vm_init(&vm1);
    tsfi_zmm_vm_init(&vm2);

    printf("[ZMM] Compiling Target.sol on VM 1 and VM 2...\n");
    tsfi_zmm_vm_exec(&vm1, "YULINIT \"target\", \"../solidity/Target.sol\", 1");
    tsfi_zmm_vm_exec(&vm2, "YULINIT \"target\", \"../solidity/Target.sol\", 2");

    int iterations = 10000;
    printf("[ZMM] Configuration: %d iterations per VM context.\n", iterations);

    ThreadArg arg1 = { &vm1, iterations };
    ThreadArg arg2 = { &vm2, iterations };

    // 1. Sequential Benchmark
    printf("[BENCHMARK] Running sequentially (VM 1 then VM 2)...\n");
    double seq_start = get_time_in_seconds();
    run_vm_benchmark(&arg1);
    run_vm_benchmark(&arg2);
    double seq_end = get_time_in_seconds();
    double seq_time = seq_end - seq_start;
    printf("[BENCHMARK] Sequential time: %.4f seconds\n", seq_time);



    // 2. Parallel Benchmark
    printf("[BENCHMARK] Running in parallel using pthreads...\n");
    double par_start = get_time_in_seconds();
    pthread_t t1, t2;
    pthread_create(&t1, NULL, run_vm_benchmark, &arg1);
    pthread_create(&t2, NULL, run_vm_benchmark, &arg2);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    double par_end = get_time_in_seconds();
    double par_time = par_end - par_start;
    printf("[BENCHMARK] Parallel time: %.4f seconds\n", par_time);

    // 3. Performance analysis
    double speedup = seq_time / par_time;
    printf("[RESULT] Parallel Speedup Factor: %.2fx\n", speedup);
    assert(speedup > 1.0);

    tsfi_zmm_vm_destroy(&vm1);
    tsfi_zmm_vm_destroy(&vm2);
    printf("=== BENCHMARK COMPLETED SUCCESSFULLY ===\n");
    return 0;
}
