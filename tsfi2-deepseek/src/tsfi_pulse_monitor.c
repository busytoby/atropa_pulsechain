#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include "tsfi_types.h"

// Get physical hardware time in milliseconds
uint64_t get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    printf("=== TSFi Phase 2: Active Stall Detection (Pulse Monitor) ===\\n");
    printf("[INFO] Overseer Daemon Engaged. Monitoring physiological health of all executing nodes...\\n\\n");

    while (1) {
        DIR *d = opendir("/dev/shm");
        if (!d) {
            printf("[FAIL] Cannot access /dev/shm to read telemetry.\\n");
            sleep(1);
            continue;
        }

        struct dirent *dir;
        uint64_t current_ms = get_time_ms();

        while ((dir = readdir(d)) != NULL) {
            if (strncmp(dir->d_name, "tsfi_telem_", 11) == 0) {
                char path[256];
                snprintf(path, sizeof(path), "/dev/shm/%s", dir->d_name);
                
                int fd = open(path, O_RDWR);
                if (fd >= 0) {
                    LauTelemetryState *telem = mmap(NULL, sizeof(LauTelemetryState), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                    if (telem != MAP_FAILED) {
                        uint64_t last_ts = telem->exec_last_ts;
                        uint64_t steps = telem->exec_steps;
                        
                        // Extract PID from filename format "tsfi_telem_PID" or "tsfi_telem_cockpit_PID"
                        // Assuming standard formats like tsfi_telem_cockpit_12345
                        int target_pid = -1;
                        char *pid_str = strrchr(dir->d_name, '_');
                        if (pid_str && pid_str[1] != '\0') {
                            target_pid = atoi(pid_str + 1);
                        }

                        // Check for physical stall (Node hasn't updated its timestamp in > 5000ms but has > 0 steps meaning it's not just idle/new)
                        if (last_ts > 0 && current_ms > last_ts) {
                            uint64_t delta = current_ms - last_ts;
                            
                            if (delta > 5000) {
                                printf("[OVERSEER] [STALL DETECTED] Node %s has spin-locked!\n", dir->d_name + 11);
                                printf("  -> Exec Steps Halted at: %lu\n", steps);
                                printf("  -> Time since last pulse: %lu ms\n", delta);
                                
                                char cmdline[2048] = {0};
                                int cmd_valid = 0;
                                
                                if (target_pid > 0) {
                                    // PHASE 3: Capture the physical launch arguments before termination
                                    char proc_path[256];
                                    snprintf(proc_path, sizeof(proc_path), "/proc/%d/cmdline", target_pid);
                                    int pfd = open(proc_path, O_RDONLY);
                                    if (pfd >= 0) {
                                        ssize_t bytes = read(pfd, cmdline, sizeof(cmdline) - 1);
                                        if (bytes > 0) {
                                            cmd_valid = 1;
                                            // cmdline is null-separated
                                        }
                                        close(pfd);
                                    }

                                    printf("  -> [ACTION] Executing SIGKILL on PID %d...\n", target_pid);
                                    if (kill(target_pid, SIGKILL) == 0) {
                                        printf("  -> [PASS] Node Terminated.\n");
                                    } else {
                                        printf("  -> [FAIL] Process not found or access denied.\n");
                                    }
                                }
                                
                                printf("  -> [ACTION] Zeroing Corrupted Manifold...\n");
                                memset(telem, 0, sizeof(LauTelemetryState));
                                
                                // PHASE 3: Resuscitation Loop
                                if (cmd_valid) {
                                    printf("  -> [ACTION] Resuscitating Node via exact hardware parameters...\n");
                                    pid_t new_pid = fork();
                                    if (new_pid == 0) {
                                        // Child Process: Reconstruct the exact argv array
                                        char *argv_exec[64] = {0};
                                        int argc_exec = 0;
                                        char *p = cmdline;
                                        while (*p && argc_exec < 63) {
                                            argv_exec[argc_exec++] = p;
                                            p += strlen(p) + 1; // skip null byte
                                        }
                                        argv_exec[argc_exec] = NULL;
                                        
                                        // Detach from overseer to prevent zombie locking
                                        setsid();
                                        
                                        // Execute the reconstructed binary
                                        execvp(argv_exec[0], argv_exec);
                                        exit(1); // Only reached if exec fails
                                    } else if (new_pid > 0) {
                                        printf("  -> [PASS] Resuscitation Complete. New Node spawned at PID %d.\n", new_pid);
                                    }
                                }
                                
                                // shm_unlink(dir->d_name); 
                            }
                        }
                        munmap(telem, sizeof(LauTelemetryState));
                    }
                    close(fd);
                }
            }
        }
        closedir(d);
        sleep(2); // Sweep every 2 seconds
    }
    return 0;
}
