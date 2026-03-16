#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "tsfi_io.h"
#include "lau_memory.h"

uint32_t shared_val = 0;

void* updater_thread(void* arg) {
    (void)arg;
    usleep(500000); // 0.5s
    printf("[THREAD] Updating shared_val to 42\n");
    shared_val = 42;
    return NULL;
}

int main() {
    printf("=== TSFi Assembly Thunk Poll Test ===\n");

    void *addrs[1];
    addrs[0] = &shared_val;
    uint32_t masks[1];
    masks[0] = 0;

    pthread_t thread;
    pthread_create(&thread, NULL, updater_thread, NULL);

    printf("[MAIN] Waiting for shared_val to change via thunk poll...\n");
    int res = tsfi_io_wait_wavefront(addrs, masks, 1, 2000); 

    if (res == 0) {
        printf("[PASS] Thunk detected change at index %d. shared_val=%u\n", res, shared_val);
    } else if (res == -1) {
        printf("[FAIL] Thunk poll timed out.\n");
    } else {
        printf("[FAIL] Unexpected result from thunk: %d\n", res);
    }

    pthread_join(thread, NULL);
    
    uint32_t val2 = 100;
    void *addrs2[2];
    addrs2[0] = &shared_val;
    addrs2[1] = &val2;
    uint32_t masks2[2];
    masks2[0] = 42;
    masks2[1] = 100;
    
    printf("[MAIN] Testing timeout logic (should take ~100ms)...\n");
    res = tsfi_io_wait_wavefront(addrs2, masks2, 2, 100);
    if (res == -1) {
        printf("[PASS] Timeout correctly detected.\n");
    } else {
        printf("[FAIL] Timeout failed, returned index %d\n", res);
    }


    extern void lau_report_memory_metrics(void);
    extern void lau_free_all_active(void);
    lau_free_all_active();
    lau_report_memory_metrics();
    return 0;
}
