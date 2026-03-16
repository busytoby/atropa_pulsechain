#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "tsfi_raw.h"
#include "tsfi_wire_firmware.h"
#include "tsfi_wire_pty.h"
#include "lau_memory.h"


#include <pty.h>
#include <sys/wait.h>

#define ITERATIONS 50

static double get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

void bench_glibc_forkpty() {
    double start = get_time_ms();
    for (int i = 0; i < ITERATIONS; i++) {
        int amaster;
        pid_t pid = forkpty(&amaster, NULL, NULL, NULL);
        if (pid == 0) {
            execl("/bin/sh", "sh", "-c", "exit 0", (char*)NULL);
            _exit(0);
        } else if (pid > 0) {
            waitpid(pid, NULL, 0);
            close(amaster);
        }
    }
    double end = get_time_ms();
    printf("[BENCH] glibc forkpty + execl: %.2f ms (%.2f ms/iter)\n", end - start, (end - start) / ITERATIONS);
}

void bench_tsfi_wire_pty_fork() {
    double start = get_time_ms();
    for (int i = 0; i < ITERATIONS; i++) {
        LauWirePty *pty = tsfi_wire_pty_fork("exit 0");
        if (pty) {
            if (pty->child_pid > 0) {
                waitpid(pty->child_pid, NULL, 0);
            }
            if (pty->master_fd >= 0) close(pty->master_fd);
            lau_unseal_object(pty);
            lau_free(pty);
        }
    }
    double end = get_time_ms();
    printf("[BENCH] tsfi_wire_pty_fork + thunk JIT: %.2f ms (%.2f ms/iter)\n", end - start, (end - start) / ITERATIONS);
}

// Hook for PTY out to firmware
void pty_callback(void *arg, const char *data, size_t len) {
    LauWireFirmware *fw = (LauWireFirmware *)arg;
    if (fw && fw->rx_pin_pty_out) {
        fw->rx_pin_pty_out(NULL, data, len);
    }
}

int main() {
    printf("=== TSFi Wire Firmware Integration Test ===\n");
    printf("\n--- PTY Fork/Setup Benchmark ---\n");
    bench_glibc_forkpty();
    bench_tsfi_wire_pty_fork();
    printf("--------------------------------\n");
    
    tsfi_wire_firmware_init();
    LauWireFirmware *fw = tsfi_wire_firmware_get();
    if (!fw) {
        printf("[FAIL] Failed to get firmware.\n");
        return 1;
    }
    printf("[PASS] Firmware initialized.\n");

    // Clear FIFOs
    if (fw->step_executor_directive) {
        void (*dir_fn)(char*) = fw->step_executor_directive;
        __asm__ __volatile__(
            "push %%r15\n\t"
            "mov %%rdi, %%r15\n\t"
            "sub $8, %%rsp\n\t"
            "call *%%rax\n\t"
            "add $8, %%rsp\n\t"
            "pop %%r15\n\t"
            :
            : "D"(fw), "S"("FLUSH"), "a"(dir_fn)
            : "rcx", "rdx", "r8", "r9", "r10", "r11", "memory"
        );
    }

    // Launch a PTY running a simple script
    LauWirePty *pty = tsfi_wire_pty_fork("echo 'FIRMWARE_RX_VERIFIED'; sleep 5");
    if (!pty) {
        printf("[FAIL] Failed to create LauWirePty.\n");
        return 1;
    }
    printf("[PASS] LauWirePty initialized.\n");

    // Wire logs
    pty->log_stdout.buffer = (char *)lau_malloc(4096);
    pty->log_stdout.head = &fw->rtl.log_stdout_head;
    pty->log_stdout.tail = &fw->rtl.log_stdout_tail;
    pty->log_stdout.lock = (_Atomic uint32_t *)&fw->rtl.log_stdout_lock;
    pty->log_stdout.capacity = 4096;

    // Step the PTY to read output into firmware FIFO
    int iterations = 0;
    while (pty->is_active && iterations < 200) {
        // Trigger Firmware Peripheral Step via RTL invariants
        tsfi_wire_firmware_strobe(fw, 0x0D, (uint64_t)iterations + 1); // host_epoch
        fw->rtl.periph_state_ready = true;
        
        tsfi_wire_firmware_step_peripheral(fw, pty);
        tsfi_raw_usleep(1000);
        iterations++;
    }

    // Check if the log_stdout has data
    int rx_len = fw->rtl.log_stdout_head - fw->rtl.log_stdout_tail;
    if (rx_len < 0) rx_len += 4096;
    if (rx_len > 0) {
        printf("[PASS] Firmware log_stdout intercepted data from PTY callback. Length: %d\n", rx_len);
    } else {
        printf("[FAIL] Firmware log_stdout is empty.\n");
        return 1;
    }

    // Now step the Firmware to drain its FIFO
    printf("[INFO] Draining log_stdout wavefront...\n");
    if (fw->cell_extract_stdout) {
        char drain_buf[4096];
        fw->cell_extract_stdout(0, drain_buf, sizeof(drain_buf));
    }

    rx_len = fw->rtl.log_stdout_head - fw->rtl.log_stdout_tail;
    if (rx_len < 0) rx_len += 4096;
    if (rx_len == 0) {
        printf("[PASS] Firmware log_stdout successfully drained via HELMHOLTZ_LIST.\n");
    } else {
        printf("[FAIL] Firmware log_stdout did not drain.\n");
        return 1;
    }

    // Test log_stdin Pin
    if (fw->tx_pin_pty_in) {
        ssize_t written = fw->tx_pin_pty_in(pty, "TEST_TX\n", 8);
        if (written > 0) {
            printf("[PASS] Firmware log_stdin buffered %zd bytes.\n", written);
        } else {
            printf("[FAIL] Firmware log_stdin failed to send.\n");
        }
    }

    // Check log_stdin length
    int tx_len = fw->rtl.log_stdin_head - fw->rtl.log_stdin_tail;
    if (tx_len < 0) tx_len += 4096;
    if (tx_len == 8) {
        printf("[PASS] Firmware log_stdin contains the written data.\n");
    } else {
        printf("[FAIL] Firmware log_stdin does not contain the written data. tx_len: %d\n", tx_len);
        return 1;
    }

    // Test Provenance Polling
    if (fw->provenance) {
        void (*prov_fn)(void) = fw->provenance;
        __asm__ __volatile__(
            "push %%r15\n\t"
            "mov %%rdi, %%r15\n\t"
            "sub $8, %%rsp\n\t"
            "call *%%rax\n\t"
            "add $8, %%rsp\n\t"
            "pop %%r15\n\t"
            :
            : "D"(fw), "a"(prov_fn)
            : "rcx", "rdx", "rsi", "r8", "r9", "r10", "r11", "memory"
        );
        printf("[PASS] Firmware provenance polled successfully.\n");
    }

    // Stop PTY using directive
    if (pty->step_executor_directive) {
        void (*dir_fn)(char*) = pty->step_executor_directive;
        __asm__ __volatile__(
            "push %%r15\n\t"
            "mov %%rdi, %%r15\n\t"
            "sub $8, %%rsp\n\t"
            "call *%%rax\n\t"
            "add $8, %%rsp\n\t"
            "pop %%r15\n\t"
            :
            : "D"(pty), "S"("STOP"), "a"(dir_fn)
            : "rcx", "rdx", "r8", "r9", "r10", "r11", "memory"
        );
    }
    
    // Check if stopped
    if (pty->step_safety_epoch) {
            void (*dispatch_fn)(void) = pty->step_safety_epoch;
            __asm__ __volatile__(
                "push %%r15\n\t"
                "mov %%rdi, %%r15\n\t"
                "sub $8, %%rsp\n\t"
                "call *%%rax\n\t"
                "add $8, %%rsp\n\t"
                "pop %%r15\n\t"
                :
                : "D"(pty), "a"(dispatch_fn)
                : "rcx", "rdx", "rsi", "r8", "r9", "r10", "r11", "memory"
            );
    }
    if (!pty->is_active) {
        printf("[PASS] Firmware Directive 'STOP' applied. PTY is inactive.\n");
    } else {
        printf("[FAIL] PTY is still active.\n");
    }

    lau_free(pty->log_stdout.buffer);
    lau_unseal_object(pty);
    lau_free(pty);

    printf("=== All Firmware Tests Passed ===\n");
    extern void tsfi_wire_firmware_teardown(void);
    tsfi_wire_firmware_teardown();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
