#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "lau_telemetry.h"
#include "tsfi_controlnet_shm.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <telemetry_id>\n", argv[0]);
        return 1;
    }

    char shm_name[256];
    snprintf(shm_name, sizeof(shm_name), "/tsfi_telem_%s", argv[1]);

    int fd = shm_open(shm_name, O_RDONLY, 0666);
    if (fd < 0) { perror("shm_open telemetry"); return 1; }

    LauTelemetryState *telem = mmap(NULL, sizeof(LauTelemetryState), PROT_READ, MAP_SHARED, fd, 0);
    if (telem == MAP_FAILED) { perror("mmap telemetry"); return 1; }

    TsfiDynamicGuidance *dgui = tsfi_dgui_shm_create();
    if (!dgui) { fprintf(stderr, "Failed to create DGUI SHM\n"); return 1; }

    printf("[TUNER] Starting Dynamic Guidance Loop (Telem: %s)\n", argv[1]);

    while (1) {
        float symmetry = telem->recip_symmetry;
        
        if (symmetry < 0.5f) {
            dgui->depth_strength = 0.95f;
            dgui->pose_strength = 0.8f;
        } else if (symmetry > 0.8f) {
            dgui->depth_strength = 0.65f;
            dgui->pose_strength = 0.5f;
        } else {
            dgui->depth_strength = 0.8f;
            dgui->pose_strength = 0.6f;
        }

        dgui->cfg_scale = 1.0f + (telem->current_intensity * 2.0f);

        printf("\r[TUNER] Symmetry: %.3f | Depth: %.2f | Pose: %.2f | CFG: %.2f   ", 
               symmetry, dgui->depth_strength, dgui->pose_strength, dgui->cfg_scale);
        fflush(stdout);

        usleep(100000); // 10Hz
    }

    return 0;
}
