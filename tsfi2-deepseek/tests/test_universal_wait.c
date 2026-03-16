#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include "tsfi_io.h"
#include "tsfi_time.h"
#include "lau_memory.h"

uint32_t g_mem_val = 0;

void* async_trigger(void* arg) {
    int *args = (int*)arg;
    int mode = args[0];
    usleep(500000); 
    if (mode == 0) {
        printf("[THREAD] Triggering Memory\n");
        g_mem_val = 0xDEADBEEF;
    } else {
        printf("[THREAD] Triggering Pipe\n");
        int fd = args[1];
        if (write(fd, "T", 1) < 0) perror("write");
    }
    return NULL;
}

int main() {
    printf("=== TSFi Universal Wait Unit Test ===\n");

    int pipe_fds[2];
    if (pipe(pipe_fds) == -1) { perror("pipe"); return 1; }
    fcntl(pipe_fds[0], F_SETFL, O_NONBLOCK);

    int fds[1] = { pipe_fds[0] };
    void* mem_addrs[1] = { &g_mem_val };
    uint32_t mem_masks[1] = { 0 };

    printf("\n[TEST] Waiting for Memory via Universal Wait...\n");
    g_mem_val = 0;
    int args[2] = { 0, 0 };
    pthread_t t1;
    pthread_create(&t1, NULL, async_trigger, args);
    
    int res = tsfi_io_wait_universal(fds, 1, mem_addrs, mem_masks, 1, 2000);
    if (res == 1) { 
        printf("[PASS] Universal Wait detected memory change. g_mem_val=0x%x\n", g_mem_val);
    } else {
        printf("[FAIL] Expected index 1, got %d\n", res);
    }
    pthread_join(t1, NULL);

    printf("\n[TEST] Waiting for Pipe via Universal Wait...\n");
    g_mem_val = 0xDEADBEEF; 
    mem_masks[0] = 0xDEADBEEF;
    int args2[2] = { 1, pipe_fds[1] };
    pthread_create(&t1, NULL, async_trigger, args2);

    res = tsfi_io_wait_universal(fds, 1, mem_addrs, mem_masks, 1, 2000);
    if (res == 0) { 
        char c; if (read(pipe_fds[0], &c, 1) < 0) perror("read");
        printf("[PASS] Universal Wait detected pipe event.\n");
    } else {
        printf("[FAIL] Expected index 0, got %d\n", res);
    }
    pthread_join(t1, NULL);

    printf("\n[TEST] Testing Universal Timeout...\n");
    res = tsfi_io_wait_universal(fds, 1, mem_addrs, mem_masks, 1, 100);
    if (res == -1) {
        printf("[PASS] Universal timeout correctly detected.\n");
    } else {
        printf("[FAIL] Timeout failed, got %d\n", res);
    }

    close(pipe_fds[0]);
    close(pipe_fds[1]);

    printf("\n=== Universal Wait Tests Passed ===\n");

    extern void lau_report_memory_metrics(void);
    extern void lau_free_all_active(void);
    lau_free_all_active();
    lau_report_memory_metrics();
    return 0;
}
