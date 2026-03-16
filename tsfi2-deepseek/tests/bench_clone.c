#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stdatomic.h>
#include <linux/sched.h>

#define STACK_SIZE (1024 * 1024)

static double get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

#define ITERATIONS 500

static int worker_dummy(void *arg) {
    _Atomic int *flag = (_Atomic int *)arg;
    atomic_store_explicit(flag, 1, memory_order_release);
    return 0;
}

static inline pid_t tsfi_raw_clone(int (*fn)(void *), void *child_stack, int flags, void *arg) {
    long ret;
    uintptr_t *stack = (uintptr_t *)child_stack;
    *--stack = (uintptr_t)arg;
    *--stack = (uintptr_t)fn;
    
    __asm__ volatile (
        "syscall\n\t"
        "test %%rax, %%rax\n\t"
        "jnz 1f\n\t"
        "xor %%rbp, %%rbp\n\t"
        "pop %%rax\n\t"
        "pop %%rdi\n\t"
        "call *%%rax\n\t"
        "mov %%rax, %%rdi\n\t"
        "mov $60, %%rax\n\t"
        "syscall\n\t"
        "1:\n\t"
        : "=a" (ret)
        : "a" (56), "D" ((long)flags), "S" (stack)
        : "rcx", "r11", "memory"
    );
    return ret;
}

void bench_glibc_clone() {
    void *stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    void *stack_top = (void*)(((uintptr_t)stack + STACK_SIZE) & ~63);
    _Atomic int flag = 0;

    double start = get_time_ms();
    for (int i = 0; i < ITERATIONS; i++) {
        atomic_store(&flag, 0);
        int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND;
        pid_t pid = clone(worker_dummy, stack_top, flags, &flag);
        if (pid > 0) {
            int status;
            waitpid(pid, &status, __WALL);
        }
    }
    double end = get_time_ms();
    munmap(stack, STACK_SIZE);
    printf("[BENCH] glibc clone(): %.2f ms (%.4f ms/iter)\n", end - start, (end - start) / ITERATIONS);
}

void bench_tsfi_raw_clone() {
    void *stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    void *stack_top = (void*)(((uintptr_t)stack + STACK_SIZE) & ~63);
    _Atomic int flag = 0;

    double start = get_time_ms();
    for (int i = 0; i < ITERATIONS; i++) {
        atomic_store(&flag, 0);
        int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND;
        pid_t pid = tsfi_raw_clone(worker_dummy, stack_top, flags, &flag);
        if (pid > 0) {
            int status;
            waitpid(pid, &status, __WALL);
        }
    }
    double end = get_time_ms();
    munmap(stack, STACK_SIZE);
    printf("[BENCH] tsfi_raw_clone() thunk: %.2f ms (%.4f ms/iter)\n", end - start, (end - start) / ITERATIONS);
}

int main() {
    printf("=== Syscall 56 (clone) Benchmark ===\n");
    bench_glibc_clone();
    bench_tsfi_raw_clone();
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
