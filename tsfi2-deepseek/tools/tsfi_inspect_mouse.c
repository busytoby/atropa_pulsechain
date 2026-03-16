#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "../inc/lau_telemetry.h"

int main() {
    const char *id = getenv("TSFI_TELEMETRY_ID");
    if (!id) id = "cockpit";

    char shm_name[256];
    snprintf(shm_name, sizeof(shm_name), "/tsfi_telem_%s", id);

    int fd = shm_open(shm_name, O_RDONLY, 0666);
    if (fd == -1) {
        printf("{\"error\": \"Cockpit not running\", \"shutter_fps\": 0}\n");
        return 0;
    }

    LauTelemetryState *telem = mmap(NULL, sizeof(LauTelemetryState), PROT_READ, MAP_SHARED, fd, 0);
    if (telem == MAP_FAILED) {
        printf("{\"error\": \"Map failed\"}\n");
        close(fd);
        return 1;
    }

    printf("{\n");
    printf("  \"shutter_fps\": %u,\n", telem->mouse_scope.shutter_fps);
    printf("  \"velocity_flux\": %.4f,\n", telem->mouse_scope.velocity_flux);
    printf("  \"jerk_metric\": %.4f,\n", telem->mouse_scope.jerk_metric);
    printf("  \"surface_quality\": %.2f,\n", telem->mouse_scope.surface_quality);
    printf("  \"surface_hash\": \"0x%016lX\",\n", telem->mouse_scope.surface_hash);
    printf("  \"last_ts\": %lu\n", telem->mouse_scope.last_shutter_ts);
    printf("}\n");

    munmap(telem, sizeof(LauTelemetryState));
    close(fd);
    return 0;
}