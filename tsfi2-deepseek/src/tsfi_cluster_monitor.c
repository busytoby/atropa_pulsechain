#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdbool.h>
#include "tsfi_types.h"

void cluster_send_command(const char *instance_id, const char *command) {
    char path[256];
    snprintf(path, sizeof(path), "/dev/shm/tsfi_telem_%s", instance_id);

    int fd = open(path, O_RDWR);
    if (fd < 0) {
        printf("[FAIL] Cannot open shared memory for instance %s.\n", instance_id);
        return;
    }

    LauTelemetryState *telem = mmap(NULL, sizeof(LauTelemetryState), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (telem == MAP_FAILED) {
        printf("[FAIL] Cannot map shared memory for instance %s.\n", instance_id);
        close(fd);
        return;
    }

    if (strlen(command) >= sizeof(telem->request_cmd)) {
        printf("[FAIL] Command %s is too long for instance %s.\n", command, instance_id);
    } else {
        strncpy(telem->request_cmd, command, sizeof(telem->request_cmd) - 1);
        telem->request_cmd[sizeof(telem->request_cmd) - 1] = '\0'; // Ensure null termination
        printf("[PASS] Managerial Command '%s' injected into instance %s.\n", command, instance_id);
    }

    munmap(telem, sizeof(LauTelemetryState));
    close(fd);
}

int main(int argc, char **argv) {
    if (argc >= 3) {
        // Managerial Action Mode: ./tsfi_cluster_monitor <INSTANCE_ID> <COMMAND>
        cluster_send_command(argv[1], argv[2]);
        return 0;
    }

    printf("\033[2J\033[H"); // Clear screen
    printf("=== TSFi Unified Cluster Telemetry Monitor ===\\n\\n");
    
    DIR *d = opendir("/dev/shm");
    if (!d) {
        printf("[FAIL] Cannot access /dev/shm to read telemetry.\\n");
        return 1;
    }

    struct dirent *dir;
    int active_instances = 0;

    printf("%-20s | %-10s | %-15s | %-15s | %-20s\\n", "INSTANCE ID", "CPU (%)", "ALLOCATIONS", "PEAK RAM (MB)", "LATEST EVENT");
    printf("----------------------------------------------------------------------------------------------------\\n");

    while ((dir = readdir(d)) != NULL) {
        if (strncmp(dir->d_name, "tsfi_telem_", 11) == 0) {
            char path[256];
            snprintf(path, sizeof(path), "/dev/shm/%s", dir->d_name);
            
            int fd = open(path, O_RDONLY);
            if (fd >= 0) {
                LauTelemetryState *telem = mmap(NULL, sizeof(LauTelemetryState), PROT_READ, MAP_SHARED, fd, 0);
                if (telem != MAP_FAILED) {
                    float cpu_usage = (float)telem->exec_steps;
                    uint64_t allocs = telem->active_allocs;
                    float peak_mb = (float)telem->peak_bytes / (1024.0f * 1024.0f);
                    
                    // Find latest event
                    char latest_event[64] = "None";
                    if (telem->last_directive_str[0] != '\0') {
                        strncpy(latest_event, telem->last_directive_str, sizeof(latest_event) - 1);
                        latest_event[63] = '\0';
                    }

                    char instance_id[64];
                    strncpy(instance_id, dir->d_name + 11, sizeof(instance_id) - 1);
                    instance_id[63] = '\0';

                    printf("%-20s | %-10.2f | %-15lu | %-15.2f | %-20s\\n", 
                           instance_id, cpu_usage, allocs, peak_mb, latest_event);

                    munmap(telem, sizeof(LauTelemetryState));
                    active_instances++;
                }
                close(fd);
            }
        }
    }
    closedir(d);

    if (active_instances == 0) {
        printf("[INFO] No active TSFi telemetry blocks found in /dev/shm.\\n");
    }

    printf("\\n[Kr0wZ] Cluster scan complete.\\n");
    return 0;
}
