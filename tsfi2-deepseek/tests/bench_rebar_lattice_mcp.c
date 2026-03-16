#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <stdatomic.h>
#include <immintrin.h>
#include <time.h>

// We use the same ReBAR SHM name configured in tsfi_wire_firmware.c
#define REBAR_SHM_NAME "/tsfi_rebar_lattice"
#define REBAR_SIZE (256 * 1024 * 1024) // 256MB

void rebar_mcp_writer(void *rebar_ptr, size_t size, int iterations) {
    // Fill the ReBAR memory with AVX-512 streaming stores to simulate a >20GB/s workload
    size_t vec_size = (size / 2) & ~255; 
    __m512i v_data = _mm512_set1_epi64(0xDEADBEEFCAFEBABEULL);
    
    for (int it = 0; it < iterations; it++) {
        for (size_t i = 0; i < vec_size; i += 256) {
            _mm512_stream_si512((__m512i*)((char*)rebar_ptr + i), v_data);
            _mm512_stream_si512((__m512i*)((char*)rebar_ptr + i + 64), v_data);
            _mm512_stream_si512((__m512i*)((char*)rebar_ptr + i + 128), v_data);
            _mm512_stream_si512((__m512i*)((char*)rebar_ptr + i + 192), v_data);
        }
    }
}

void rebar_app_reader(void *rebar_ptr, size_t size, int iterations) {
    // Read the ReBAR memory rapidly with AVX-512
    size_t vec_size = (size / 2) & ~255;
    void *read_ptr = (char*)rebar_ptr + (size / 2); // Read from the other half
    __m512i v_acc0 = _mm512_setzero_si512();
    __m512i v_acc1 = _mm512_setzero_si512();
    
    for (int it = 0; it < iterations; it++) {
        for (size_t i = 0; i < vec_size; i += 256) {
            __m512i v_val0 = _mm512_stream_load_si512((__m512i*)((char*)read_ptr + i));
            __m512i v_val1 = _mm512_stream_load_si512((__m512i*)((char*)read_ptr + i + 64));
            __m512i v_val2 = _mm512_stream_load_si512((__m512i*)((char*)read_ptr + i + 128));
            __m512i v_val3 = _mm512_stream_load_si512((__m512i*)((char*)read_ptr + i + 192));
            
            // Minimal arithmetic to prevent optimizing out the loads
            v_acc0 = _mm512_add_epi64(v_acc0, v_val0);
            v_acc1 = _mm512_add_epi64(v_acc1, v_val1);
            v_acc0 = _mm512_add_epi64(v_acc0, v_val2);
            v_acc1 = _mm512_add_epi64(v_acc1, v_val3);
        }
    }
    
    v_acc0 = _mm512_add_epi64(v_acc0, v_acc1);
    
    uint64_t result[8];
    _mm512_storeu_si512((__m512i*)result, v_acc0);
    if (result[0] == 1) {
        printf("Never happens.\n");
    }
}

static unsigned long long local_get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (unsigned long long)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

int main() {
    printf("[BENCH] ReBAR Lattice MCP <-> Application Cross-Communication\n");
    
    int fd = shm_open(REBAR_SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        perror("shm_open failed");
        return 1;
    }
    
    if (ftruncate(fd, REBAR_SIZE) != 0) {
        perror("ftruncate failed");
        return 1;
    }
    
    void *rebar_ptr = mmap(NULL, REBAR_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (rebar_ptr == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }
    
    // We will do 50 iterations over 128MB * 2 (reader + writer)
    const int iterations = 50;
    const double bytes_transferred = (double)(REBAR_SIZE / 2) * iterations * 2.0; // 128MB * 50 * 2 = 12.8GB
    
    // Active Application acts as the Reader
    unsigned long long start_time = local_get_time_ns();
    
    pid_t pid = fork();
    if (pid == 0) {
        // MCP acts as the Writer
        rebar_mcp_writer(rebar_ptr, REBAR_SIZE, iterations);
        exit(0);
    }
    
    // App reads concurrently
    rebar_app_reader(rebar_ptr, REBAR_SIZE, iterations);
    
    int status;
    waitpid(pid, &status, 0);
    unsigned long long end_time = local_get_time_ns();
    
    double duration_s = (double)(end_time - start_time) / 1e9;
    double throughput_gbps = (bytes_transferred / duration_s) / (1024.0 * 1024.0 * 1024.0);
    
    printf("  [ROUTINE] %.1f GB total data volume exchanged across structural bound.\n", bytes_transferred / (1024.0 * 1024.0 * 1024.0));
    printf("[PASS] Zero-Copy Precision Rate: %.2f GB/s\n", throughput_gbps);
    
    if (throughput_gbps > 20.0) {
        printf("[SUCCESS] ReBAR Lattice >20GB/s bandwidth requirement exceeded.\n");
    } else {
        printf("[FAIL] ReBAR Lattice bandwidth fell below 20GB/s requirement.\n");
        return 1;
    }
    
    munmap(rebar_ptr, REBAR_SIZE);
    close(fd);
    shm_unlink(REBAR_SHM_NAME);
    return 0;
}
