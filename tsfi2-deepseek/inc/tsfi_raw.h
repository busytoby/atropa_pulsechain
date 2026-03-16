#ifndef TSFI_RAW_H
#define TSFI_RAW_H

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <time.h>

// --- Internal CPU Set (Matches Linux 1024-bit mask) ---
typedef struct {
    uint64_t __bits[1024 / 64];
} tsfi_cpu_set_t;

#define TSFI_CPU_ZERO(s) do { for(int _i=0; _i<16; _i++) (s)->__bits[_i] = 0; } while(0)
#define TSFI_CPU_SET(n, s) do { if((n) < 1024) (s)->__bits[(n)/64] |= (1UL << ((n)%64)); } while(0)

// --- Raw Syscall Wrappers (Pure Assembly) ---

// sched_setaffinity (203)
static inline int tsfi_raw_sched_setaffinity(pid_t pid, size_t cpusetsize, const tsfi_cpu_set_t *mask) {
    long ret;
    __asm__ volatile (
        "syscall\n\t"
        : "=a" (ret)
        : "a" (203), "D" (pid), "S" (cpusetsize), "d" (mask)
        : "rcx", "r11", "memory"
    );
    return (int)ret;
}

// futex (202) - Raw synchronization barrier
static inline int tsfi_raw_futex_wait(int *uaddr, int val) {
    long ret;
    register long r10 __asm__("r10") = 0; // timeout struct
    __asm__ volatile (
        "syscall\n\t"
        : "=a" (ret)
        : "a" (202), "D" (uaddr), "S" (0), "d" (val), "r" (r10) // 0 = FUTEX_WAIT
        : "rcx", "r11", "memory"
    );
    return (int)ret;
}

// clone (56) - Advanced Thread Spawning with TLS and TID tracking
static inline pid_t tsfi_raw_clone_thread(int (*fn)(void *), void *child_stack, int flags, void *arg, int *ptid, void *tls, int *ctid) {
    long ret;
    uintptr_t *stack = (uintptr_t *)child_stack;
    *--stack = (uintptr_t)arg;
    *--stack = (uintptr_t)fn;
    
    register long r10 __asm__("r10") = (long)ctid;
    register long r8 __asm__("r8") = (long)tls;
    
    __asm__ volatile (
        "syscall\n\t"
        "test %%rax, %%rax\n\t"
        "jnz 1f\n\t"
        "xor %%rbp, %%rbp\n\t"
        "pop %%rax\n\t"
        "pop %%rdi\n\t"
        "call *%%rax\n\t"
        "mov %%rax, %%rdi\n\t"
        "mov $60, %%rax\n\t" // exit
        "syscall\n\t"
        "1:\n\t"
        : "=a" (ret)
        : "a" (56), "D" ((long)flags), "S" (stack), "d" (ptid), "r" (r10), "r" (r8)
        : "rcx", "r11", "memory"
    );
    return (pid_t)ret;
}

// clone (56) - Simplified for internal use
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
        "mov $60, %%rax\n\t" // exit
        "syscall\n\t"
        "1:\n\t"
        : "=a" (ret)
        : "a" (56), "D" ((long)flags), "S" (stack)
        : "rcx", "r11", "memory"
    );
    return (pid_t)ret;
}

// clock_gettime (228)
extern int (*g_vdso_clock_gettime)(int, struct timespec *);

static inline int tsfi_raw_clock_gettime(int clk_id, struct timespec *tp) {
    if (g_vdso_clock_gettime) return g_vdso_clock_gettime(clk_id, tp);
    long ret;
    __asm__ volatile (
        "syscall\n\t"
        : "=a" (ret)
        : "a" (228), "D" (clk_id), "S" (tp)
        : "rcx", "r11", "memory"
    );
    return (int)ret;
}

// usleep replacement via nanosleep (35)
static inline int tsfi_raw_usleep(unsigned int usec) {
    struct timespec ts;
    ts.tv_sec = usec / 1000000;
    ts.tv_nsec = (usec % 1000000) * 1000;
    long ret;
    __asm__ volatile (
        "syscall\n\t"
        : "=a" (ret)
        : "a" (35), "D" (&ts), "S" (0)
        : "rcx", "r11", "memory"
    );
    return (int)ret;
}

#endif // TSFI_RAW_H