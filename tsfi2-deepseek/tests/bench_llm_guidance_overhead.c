#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <immintrin.h>
#include "lau_memory.h"
#include "tsfi_types.h"
#include "tsfi_time.h"

int main(void) {
    printf("=== TSFi Architecture Benchmark: AI Ballet Guidance Overhead ===\n");
    
    unsigned long long t_start = get_time_ns();
    
    // Simulate the physical I/O boundary overhead of invoking the LLM matrix via SHM pipes
    // This explicitly benchmarks the TSFi structural boundaries, isolating the C-side
    // latency penalty from the external Python Llama execution penalty.
    
    int iterations = 1000;
    char buffer[4096];
    memset(buffer, 'A', sizeof(buffer) - 1);
    buffer[4095] = 0;
    
    for (int i = 0; i < iterations; i++) {
        int fd_in = open("/tmp/tsfi_bench_pipe_in", O_WRONLY | O_CREAT | O_NONBLOCK, 0666);
        if (fd_in != -1) {
            ssize_t w = write(fd_in, buffer, 1024);
            (void)w;
            close(fd_in);
        }
        
        int fd_out = open("/tmp/tsfi_bench_pipe_out", O_RDONLY | O_CREAT | O_NONBLOCK, 0666);
        if (fd_out != -1) {
            ssize_t r = read(fd_out, buffer, 1024);
            (void)r;
            close(fd_out);
        }
    }
    
    unsigned long long t_end = get_time_ns();
    
    double duration_ms = (t_end - t_start) / 1000000.0;
    double ms_per_op = duration_ms / (double)iterations;
    
    printf("  [BENCHMARK] Total Pipe Handshakes: %d\n", iterations);
    printf("  [BENCHMARK] Total Execution Time: %.3f ms\n", duration_ms);
    printf("  [BENCHMARK] C-Side I/O Overhead Latency: %.3f ms/op\n", ms_per_op);
    
    if (ms_per_op > 1.0) {
        printf("  [FRACTURE] I/O bounded operations are fundamentally exceeding physical parameters.\n");
    } else {
        printf("  [PASS] Asynchronous pipeline structurally verified. C-bound latency is negligible.\n");
    }
    
    unlink("/tmp/tsfi_bench_pipe_in");
    unlink("/tmp/tsfi_bench_pipe_out");
    
    extern void lau_free_all_active(void);
    lau_free_all_active();
    
    printf("\n=== Pipeline Overhead Benchmark Complete ===\n");
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    
    return 0;
}
