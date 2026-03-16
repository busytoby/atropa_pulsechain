#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tsfi_types.h"
#include "tsfi_io.h"

int main() {
    const char *id = getenv("TSFI_TELEMETRY_ID");
    if (!id) { fprintf(stderr, "Missing TSFI_TELEMETRY_ID\n"); return 1; }

    char shm_name[256];
    snprintf(shm_name, sizeof(shm_name), "/tsfi_telem_%s", id);

    int fd = tsfi_shm_open(shm_name, O_RDWR, 0666);
    if (fd == -1) { perror("tsfi_shm_open"); return 1; }

    LauTelemetryState *telem = mmap(NULL, sizeof(LauTelemetryState), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (telem == MAP_FAILED) { perror("mmap"); return 1; }

    telem->magic = LAU_TELEMETRY_MAGIC;
    telem->total_allocs = 1234;
    telem->active_bytes = 1024 * 1024 * 5; // 5MB
    strcpy((char*)telem->last_directive_str, "TEST_DIRECTIVE");
    strcpy((char*)telem->zmm_msg, "ZMM_OK");
    telem->zmm_val = 42;

    telem->header_crc = 0; // Simplified CRC signal

    printf("Mock telemetry provider active for %s\n", id);
    sleep(1);
    
    extern void lau_report_memory_metrics(void);
    extern void lau_free_all_active(void);
    lau_free_all_active();
    lau_report_memory_metrics();
    return 0;
}
