#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include "tsfi_types.h"

uint64_t get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
}

int main() {
    int pid = getpid();
    char path[256];
    snprintf(path, sizeof(path), "/tsfi_telem_mock_%d", pid);
    int fd = shm_open(path, O_RDWR | O_CREAT, 0666);
    ftruncate(fd, sizeof(LauTelemetryState));
    LauTelemetryState *t = mmap(NULL, sizeof(LauTelemetryState), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    
    printf("[MOCK] Node %d Online. Simulating healthy execution...\n", pid);
    for (int i=0; i<3; i++) {
        t->exec_steps++;
        t->exec_last_ts = get_time_ms();
        usleep(500000);
    }
    
    printf("[MOCK] Node %d Spin-Locking (Simulating GPU crash)...\n", pid);
    while (1) {
        // Infinite loop without updating telemetry timestamps
        usleep(100000);
    }
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}
