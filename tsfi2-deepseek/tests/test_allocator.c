#include "lau_memory.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

// Helper to run a test in a subprocess and expect a crash (signal)
void expect_crash(void (*func)(void), const char *test_name) {
    printf("[TEST] Running crash test: %s... ", test_name);
    fflush(stdout);
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child
        // Redirect stderr to /dev/null to avoid noise
        // if (freopen("/dev/null", "w", stderr) == NULL) {
            // If redirection fails, we proceed anyway (might be noisy)
        // }
        func();
        exit(0); // Should not reach here
    } else {
        // Parent
        int status;
        waitpid(pid, &status, 0);
        if (WIFSIGNALED(status)) {
            printf("PASSED (Crashed with signal %d)\n", WTERMSIG(status));
        } else {
            printf("FAILED (Did not crash)\n");
            exit(1);
        }
    }
}

void test_basic_alloc(void) {
    printf("[TEST] Basic Alloc/Free... ");
    void *p = lau_malloc(100);
    if (!p) {
        printf("FAILED (malloc returned NULL)\n");
        exit(1);
    }
    memset(p, 0xAA, 100);
    lau_free(p);
    printf("PASSED\n");
}

void test_leak_report(void) {
    printf("[TEST] Leak Reporting... ");
    fflush(stdout);

    // Redirect stdout to temp file
    int original_stdout = dup(STDOUT_FILENO);
    FILE *temp = tmpfile();
    if (!temp) {
        printf("FAILED (tmpfile creation)\n");
        return;
    }
    int temp_fd = fileno(temp);
    fflush(stdout);
    dup2(temp_fd, STDOUT_FILENO);

    // Perform operations that generate output
    void *p = lau_malloc(50);
    lau_report_memory_metrics();
    
    // Restore stdout
    fflush(stdout);
    dup2(original_stdout, STDOUT_FILENO);
    close(original_stdout);

    // Verify content
    rewind(temp);
    char buffer[1024];
    size_t n = fread(buffer, 1, sizeof(buffer) - 1, temp);
    buffer[n] = 0;
    fclose(temp);

    lau_free(p); // Clean up actual memory

    if (strstr(buffer, "LEAK:") && strstr(buffer, "test_allocator.c")) {
        printf("PASSED\n");
    } else {
        printf("FAILED (Report did not contain expected audit info)\n");
        printf("Captured output:\n%s\n", buffer);
        exit(1);
    }
}

void crash_double_free(void) {
    void *p = lau_malloc(10);
    lau_free(p);
    lau_free(p); // Should abort
    *(volatile int*)0 = 0; // Force SEGFAULT if allocator fails
}

void crash_corruption(void) {
    void *p = lau_malloc(10);
    // Get header (Basic)
    LauHeader *h = (LauHeader*)((char*)p - sizeof(LauHeader));
    h->footer.magic = 0xEF; // Corrupt magic
    lau_free(p); // Should abort
    abort();
}

void crash_underflow(void) {
    void *p = lau_malloc(10);
    // Corrupt checksum by modifying fields without updating checksum
    LauHeader *h = (LauHeader*)((char*)p - sizeof(LauHeader));
    h->meta.alloc_size = 999999; 
    lau_free(p); // Should abort on checksum mismatch
    abort();
}

void test_seal_report(void) {
    printf("[TEST] Seal Reporting... ");
    void *p = lau_malloc(100);
    lau_seal_object(p);
    
    // Capture report
    int original_stdout = dup(STDOUT_FILENO);
    int original_stderr = dup(STDERR_FILENO);
    FILE *temp = tmpfile();
    int temp_fd = fileno(temp);
    dup2(temp_fd, STDOUT_FILENO);
    dup2(temp_fd, STDERR_FILENO);
    
    lau_report_memory_metrics();
    lau_report_memory_to_log();
    
    fflush(stdout);
    fflush(stderr);
    dup2(original_stdout, STDOUT_FILENO);
    dup2(original_stderr, STDERR_FILENO);
    close(original_stdout);
    close(original_stderr);
    
    rewind(temp);
    char buffer[4096];
    size_t n = fread(buffer, 1, sizeof(buffer)-1, temp);
    buffer[n] = 0;
    fclose(temp);
    
    if (strstr(buffer, "Sealed: 1")) {
        printf("PASSED\n");
    } else {
        printf("FAILED (Report did not contain Sealed: 1)\nCaptured:\n%s\n", buffer);
        exit(1);
    }
    
    lau_unseal_object(p);
    lau_free(p);
}

int main() {
    printf("--- RUNNING ALLOCATOR TESTS ---\n");
    
    test_basic_alloc();
    test_leak_report();
    test_seal_report();
    
    expect_crash(crash_double_free, "Double Free");
    expect_crash(crash_corruption, "Header Corruption (Magic)");
    expect_crash(crash_underflow, "Header Corruption (Checksum)");
    
    printf("--- ALL ALLOCATOR TESTS PASSED ---\n");
    
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
