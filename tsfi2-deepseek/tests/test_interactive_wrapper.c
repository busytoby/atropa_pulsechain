#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>
#include "tsfi_wire_pty.h"
#include "tsfi_raw.h"

// We are going to use our own PTY architecture to test the interactive wrapper!
int main() {
    printf("=== Testing Interactive PTY Wrapper (End-to-End) ===\n");

    // 1. Fork the interactive wrapper itself
    LauWirePty *wrapper_pty = tsfi_wire_pty_fork("./bin/tsfi_interactive_pty \"bash -i\"");
    if (!wrapper_pty) {
        printf("[FAIL] Could not fork interactive wrapper.\n");
        return 1;
    }

    // Give it a moment to boot up and print the header
    tsfi_raw_usleep(100000); // 100ms

    // 2. Read the header to verify it started
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    ssize_t n = read(wrapper_pty->master_fd, buffer, sizeof(buffer) - 1);
    
    if (n > 0) {
        printf("[INFO] Wrapper Output:\n%.*s\n", (int)n, buffer);
        if (strstr(buffer, "Ctrl+] (ASCII 29)") == NULL) {
            printf("[FAIL] Did not see the expected header.\n");
            return 1;
        }
    } else {
        printf("[FAIL] No output from wrapper.\n");
        return 1;
    }

    // 3. Send a command to bash interactively
    printf("[INFO] Injecting command: 'echo TSFI_INTERACTIVE_WORKS'\n");
    const char *cmd = "echo TSFI_INTERACTIVE_WORKS\n";
    tsfi_wire_pty_write(wrapper_pty, cmd, strlen(cmd));

    // Wait for bash to process and echo
    tsfi_raw_usleep(200000); // 200ms

    memset(buffer, 0, sizeof(buffer));
    n = read(wrapper_pty->master_fd, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        printf("[INFO] Wrapper Output:\n%.*s\n", (int)n, buffer);
        if (strstr(buffer, "TSFI_INTERACTIVE_WORKS") == NULL) {
            printf("[FAIL] Did not see the echoed command output.\n");
            return 1;
        }
        printf("[PASS] Interactive command executed and output received.\n");
    }

    // 4. Send the hardware escape sequence (Ctrl+])
    printf("[INFO] Injecting escape sequence (Ctrl+])...\n");
    char escape_seq = 29; // ASCII 29 is Ctrl+]
    tsfi_wire_pty_write(wrapper_pty, &escape_seq, 1);

    // Wait for it to close
    tsfi_raw_usleep(200000); // 200ms

    // Read the shutdown message
    memset(buffer, 0, sizeof(buffer));
    n = read(wrapper_pty->master_fd, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        printf("[INFO] Wrapper Output:\n%.*s\n", (int)n, buffer);
        if (strstr(buffer, "Closing Session") != NULL) {
            printf("[PASS] Escape sequence correctly intercepted and session closed.\n");
        }
    }

    tsfi_wire_pty_step(wrapper_pty);
    if (!wrapper_pty->is_active) {
        printf("[PASS] Wrapper process terminated cleanly.\n");
    } else {
        printf("[FAIL] Wrapper process did not terminate after escape sequence.\n");
        return 1;
    }

    printf("=== Interactive PTY Test Complete ===\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
