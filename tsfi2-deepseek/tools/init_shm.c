#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "inc/tsfi_yang_registry.h"

int main() {
    int fd = shm_open("/tsfi_test_acoustic_space", O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        printf("[FAIL] Failed to create shm\n");
        return 1;
    }
    ftruncate(fd, sizeof(TsfiAcousticRegistry));
    
    TsfiAcousticRegistry *reg = mmap(NULL, sizeof(TsfiAcousticRegistry), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (reg == MAP_FAILED) {
        printf("[FAIL] Failed to mmap shm\n");
        close(fd);
        return 1;
    }

    // Initialize with sane defaults
    memset(reg, 0, sizeof(TsfiAcousticRegistry));
    reg->pdai_manifold_pressure = 1.0f;
    strcpy(reg->agent_monologue, "Awaiting DeepSeek Input...");
    strcpy(reg->genie_poetic_observation, "Awaiting Genie Input...");
    reg->alligator_form_ratio = 1.0f;
    reg->spider_texture_sharpness = 100.0f;

    printf("[SUCCESS] Acoustic Proof Space allocated and initialized at /dev/shm/tsfi_test_acoustic_space.\n");
    printf("[INFO] The SHM block is now persistent. Press Ctrl+C to destroy it.\n");
    
    // Hold the process open indefinitely so the SHM block persists for the Python scripts
    while(1) {
        sleep(10);
    }
    
    // Cleanup (only reached if broken out)
    munmap(reg, sizeof(TsfiAcousticRegistry));
    close(fd);
    shm_unlink("/tsfi_test_acoustic_space");
    return 0;
}
