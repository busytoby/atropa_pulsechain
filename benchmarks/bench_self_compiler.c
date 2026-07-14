#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include "../scripts/libantigravity_interop.h"

static double get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec * 1000.0 + (double)tv.tv_usec / 1000.0;
}

int main(void) {
    printf("=== RUNNING SELF-COMPILER PERFORMANCE BENCHMARK ===\n");
    
    // 1. Benchmark JIT Compilation Latency
    double t_start = get_time_ms();
    int compile_status = system("bash ../assets/self_compiler.dat.bin > /dev/null 2>&1");
    double t_end = get_time_ms();
    
    if (compile_status != 0) {
        printf("Error: self_compiler.dat.bin execution failed.\n");
        return 1;
    }
    double compilation_ms = t_end - t_start;
    printf("- Self-extraction and JIT compilation latency: %.2f ms\n", compilation_ms);

    // 2. Benchmark Transaction Audit Logging (FNV-1a Chain)
    void *dummy_payload = (void*)0xdeadbeef;
    uint64_t args[2] = { 10, 20 };
    char audit_file[128];
    snprintf(audit_file, sizeof(audit_file), "../assets/audit_log_%p.dat.bin", dummy_payload);
    remove(audit_file); // Ensure starting clean
    
    t_start = get_time_ms();
    int iterations = 1000;
    for (int i = 0; i < iterations; i++) {
        interop_agent_log_transaction(dummy_payload, 0x11111111, args, 2);
    }
    t_end = get_time_ms();
    double log_ms = t_end - t_start;
    printf("- Audit logging latency (%d writes): %.2f ms (%.3f us/write)\n", iterations, log_ms, (log_ms * 1000.0) / iterations);
    
    // 3. Benchmark Audit Log Bottom-Up Verification
    t_start = get_time_ms();
    uint64_t final_hash = 0;
    int verified_count = interop_agent_verify_audit_log(dummy_payload, &final_hash);
    t_end = get_time_ms();
    double verify_ms = t_end - t_start;
    if (verified_count != iterations) {
        printf("Error: Verification count mismatch: %d vs %d\n", verified_count, iterations);
        remove(audit_file);
        return 2;
    }
    printf("- Bottom-up audit verify latency (%d blocks): %.2f ms (%.3f us/block)\n", verified_count, verify_ms, (verify_ms * 1000.0) / verified_count);
    
    remove(audit_file);
    printf("=== BENCHMARK COMPLETED SUCCESSFULLY ===\n");
    return 0;
}
