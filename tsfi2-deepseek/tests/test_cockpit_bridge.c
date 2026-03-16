#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include "tsfi_types.h"
#include "tsfi_io.h"

int main() {
    printf("=== TSFi Cockpit Bridge Unit Test ===\n");

    int p[2];
    if (pipe(p) == -1) { perror("pipe"); return 1; }

    pid_t pid = fork();
    if (pid == 0) {
        close(p[1]);
        dup2(p[0], STDIN_FILENO);
        execl("./bin/tsfi_cockpit", "./bin/tsfi_cockpit", "sleep", "10", (char *)NULL);
        perror("execl");
        exit(1);
    }

    close(p[0]);

    char shm_name[256];
    snprintf(shm_name, sizeof(shm_name), "/tsfi_telem_cockpit_%d", pid);

    LauTelemetryState *telem = NULL;
    int fd = -1;
    for (int i = 0; i < 200; i++) {
        fd = tsfi_shm_open(shm_name, O_RDONLY, 0666);
        if (fd != -1) break;
        usleep(10000);
    }

    if (fd == -1) {
        printf("[FAIL] Cockpit failed to create SHM %s\n", shm_name);
        kill(pid, SIGTERM);
        return 1;
    }

    telem = mmap(NULL, sizeof(LauTelemetryState), PROT_READ, MAP_SHARED, fd, 0);
    if (telem == MAP_FAILED) { perror("mmap"); kill(pid, SIGTERM); return 1; }

    const char *cmd_read = "read test_file.c\n";
    write(p[1], cmd_read, strlen(cmd_read));
    usleep(200000);

    printf("[DEBUG] request_cmd: %s\n", telem->request_cmd);
    if (strstr(telem->request_cmd, "read_file") && strstr(telem->request_cmd, "test_file.c")) {
        printf("[PASS] 'read' command bridged to JSON-RPC.\n");
    } else {
        printf("[FAIL] 'read' command NOT bridged correctly.\n");
        kill(pid, SIGTERM);
        return 1;
    }

    const char *cmd_grep = "grep TSFI_VERSION\n";
    write(p[1], cmd_grep, strlen(cmd_grep));
    usleep(200000);

    printf("[DEBUG] request_cmd: %s\n", telem->request_cmd);
    if (strstr(telem->request_cmd, "grep_search") && strstr(telem->request_cmd, "TSFI_VERSION")) {
        printf("[PASS] 'grep' command bridged to JSON-RPC.\n");
    } else {
        printf("[FAIL] 'grep' command NOT bridged correctly.\n");
        kill(pid, SIGTERM);
        return 1;
    }

    const char *cmd_wave = "wave Hello World\n";
    write(p[1], cmd_wave, strlen(cmd_wave));
    usleep(200000);

    printf("[DEBUG] request_cmd: %s\n", telem->request_cmd);
    if (strstr(telem->request_cmd, "tsfi_wave") && strstr(telem->request_cmd, "Hello World")) {
        printf("[PASS] 'wave' command bridged to JSON-RPC.\n");
    } else {
        printf("[FAIL] 'wave' command NOT bridged correctly.\n");
        kill(pid, SIGTERM);
        return 1;
    }

    kill(pid, SIGTERM);
    waitpid(pid, NULL, 0);
    munmap(telem, sizeof(LauTelemetryState));
    close(fd);
    tsfi_shm_unlink(shm_name);

    printf("=== Cockpit Bridge Test Passed ===\n");

    extern void lau_report_memory_metrics(void);
    extern void lau_free_all_active(void);
    lau_free_all_active();
    lau_report_memory_metrics();
    return 0;
}
