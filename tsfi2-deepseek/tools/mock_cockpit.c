#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "../inc/lau_telemetry.h"

int main(int argc, char **argv) {
    const char *id = "test";
    char shm_name[256];
    snprintf(shm_name, sizeof(shm_name), "/tsfi_telem_%s", id);

    // Create SHM
    shm_unlink(shm_name); 
    int fd = shm_open(shm_name, O_RDWR | O_CREAT, 0666);
    if (fd == -1) { perror("shm_open"); return 1; }
    if (ftruncate(fd, sizeof(LauTelemetryState)) == -1) { perror("ftruncate"); return 1; }
    
    LauTelemetryState *telem = mmap(NULL, sizeof(LauTelemetryState), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (telem == MAP_FAILED) { perror("mmap"); return 1; }
    memset(telem, 0, sizeof(LauTelemetryState));

    printf("Mock Cockpit Ready. SHM: %s\\n", shm_name);

    // Wait for Kr0wZ to attach (simulated by sleep)
    sleep(1);

    // 1. Send BENCH_NOTES
    printf("Sending BENCH_NOTES...\\n");
    snprintf((char*)telem->request_cmd, 63, "BENCH_NOTES");
    
    // Wait for response (Kr0wZ must be triggered via JSON to check)
    // We expect Kr0wZ to clear request_cmd when done.
    int timeout = 20;
    while (telem->request_cmd[0] != 0 && timeout-- > 0) {
        usleep(100000);
    }
    
    if (strstr((char*)telem->kr0wz_msg, "Kr0wZ Online")) {
        printf("[PASS] BENCH_NOTES Response: %s\\n", (char*)telem->kr0wz_msg);
    } else {
        printf("[FAIL] BENCH_NOTES. Msg: %s, Cmd: %s\\n", (char*)telem->kr0wz_msg, (char*)telem->request_cmd);
    }

    // 2. Send BENCH_EVO
    printf("Sending BENCH_EVO...\\n");
    snprintf((char*)telem->request_cmd, 63, "BENCH_EVO");
    timeout = 20;
    while (telem->request_cmd[0] != 0 && timeout-- > 0) {
        usleep(100000);
    }

    if (strstr((char*)telem->kr0wz_msg, "Density:")) {
        printf("[PASS] BENCH_EVO Response: %s\\n", (char*)telem->kr0wz_msg);
    } else {
        printf("[FAIL] BENCH_EVO. Msg: %s\\n", (char*)telem->kr0wz_msg);
    }

    shm_unlink(shm_name);
    return 0;
}