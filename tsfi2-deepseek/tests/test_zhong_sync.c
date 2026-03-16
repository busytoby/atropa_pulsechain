#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdatomic.h>
#include <unistd.h>
#include <assert.h>
#include "tsfi_resonance.h"
#include "tsfi_zhao.h"
#include "lau_memory.h"

#include <pthread.h>
#include "tsfi_wire_firmware.h"
#include "tsfi_raw.h"

static _Atomic uint64_t g_ring_bell = 0;

void* writer_thread(void *arg) {
    (void)arg;
    tsfi_raw_usleep(100000); // 100ms
    printf("[THREAD] Ringing the atomic bell!\n");
    atomic_store_explicit(&g_ring_bell, 1, memory_order_release);
    return NULL;
}

void test_atomic_wait() {
    printf("\n=== TSFi Zhong Atomic Wait Test ===\n");

    tsfi_wire_firmware_init();
    LauWireFirmware *fw = tsfi_wire_firmware_get();
    assert(fw != NULL);

    pthread_t tid;
    pthread_create(&tid, NULL, writer_thread, NULL);

    printf("[TEST] Firmware executing ZHONG_OP_ATOMIC_WAIT (0x30)...\n");
    
    fw->rtl.zhong_timeline_handle = (uint64_t)(uintptr_t)&g_ring_bell;
    fw->rtl.zhong_timeline_wait_val = 1;
    fw->rtl.zhong_op = 0x30;
    fw->rtl.zhong_strobe = true;

    tsfi_wire_firmware_step_peripheral(fw, NULL);
    assert(fw->rtl.zhong_done == true);
    
    fw->rtl.zhong_strobe = false;
    tsfi_wire_firmware_step_peripheral(fw, NULL);

    printf("[PASS] Firmware correctly suspended and awoke via Atomic Ring Bell.\n");
    pthread_join(tid, NULL);
}


static atomic_int g_work_done = 0;

void* worker_func(void* arg) {
    (void)arg;
    atomic_fetch_add(&g_work_done, 1);
    return NULL;
}

int main() {
    printf("=== TSFi Zhong Timeline Sync Test ===\n");
    
    // Simulated GPU Timeline Semaphore in ReBAR memory
    uint64_t *gpu_sem = (uint64_t*)lau_malloc(sizeof(uint64_t));
    *gpu_sem = 100; // Start at value 100
    
    // Create thread waiting for value 200
    LauWireThread *wt = tsfi_wire_thread_create(1, 1.0);
    wt->start_routine = worker_func;
    wt->gpu_sem_addr = gpu_sem;
    wt->gpu_sem_target = 200;
    
    printf("[SYNC] Thread created, waiting for GPU Sem >= 200 (Current: %lu)\n", *gpu_sem);
    
    // Dispatch several times - should yield
    for(int i=0; i<10; i++) {
        printf("[SYNC] Dispatching #%d...\n", i);
        tsfi_wire_thread_dispatch(wt);
        printf("[SYNC] Returned from dispatch #%d.\n", i);
    }
    
    if (atomic_load(&g_work_done) == 0) {
        printf("[PASS] Thread correctly yielded while GPU Sem < 200\n");
    } else {
        printf("[FAIL] Thread executed prematurely!\n");
        return 1;
    }
    
    // Simulate GPU reaching value 200
    printf("[SYNC] Setting GPU Sem to 200...\n");
    *gpu_sem = 200;
    
    // Dispatch - should now execute
    tsfi_wire_thread_dispatch(wt);
    
    if (atomic_load(&g_work_done) == 1) {
        printf("[PASS] Thread correctly executed after GPU Sem reached 200\n");
    } else {
        printf("[FAIL] Thread failed to execute after dependency met!\n");
        return 1;
    }
    
    lau_free(gpu_sem);
    // Cleanup wt header
    lau_unseal_object(wt);
    lau_free(wt);
    
    test_atomic_wait();
    printf("\nAll Zhong Sync Tests Passed!\n");
    extern void tsfi_wire_firmware_teardown(void);
    tsfi_wire_firmware_teardown();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
