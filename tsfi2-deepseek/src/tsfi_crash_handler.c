#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>
#include <stdatomic.h>
#include "tsfi_crash_handler.h"

static _Atomic uint32_t *g_fault_flag = NULL;

void tsfi_register_fault_flag(_Atomic uint32_t *flag) {
    g_fault_flag = flag;
}

static void crash_handler(int sig, siginfo_t *info, void *ucontext) {
    (void)ucontext;
    
    if (g_fault_flag) {
        atomic_store(g_fault_flag, 1);
    }

    printf("\n\n[TSFI CRITICAL] Caught fatal signal %d (Address: %p)\n", sig, info->si_addr);
    
    void *array[50];
    int size = backtrace(array, 50);
    printf("[TSFI CRITICAL] Backtrace (%d frames):\n", size);
    backtrace_symbols_fd(array, size, STDOUT_FILENO);
    
    fflush(stdout);
    _exit(128 + sig);
}

void tsfi_install_crash_handler() {
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO | SA_RESETHAND;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = crash_handler;
    
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);
    sigaction(SIGILL, &sa, NULL);
    sigaction(SIGFPE, &sa, NULL);
}
