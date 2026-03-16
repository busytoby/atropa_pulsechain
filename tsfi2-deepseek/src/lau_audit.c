#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdatomic.h>
#include "lau_audit.h"
#include "tsfi_io.h"

static FILE *g_log_file = NULL;
static atomic_flag g_init_lock = ATOMIC_FLAG_INIT;

static void lau_cleanup_log(void) {
    if (g_log_file) {
        tsfi_io_flush(g_log_file);
        fclose(g_log_file);
        g_log_file = NULL;
    }
}

void lau_log_status(const char *status) {
    if (!g_log_file) {
        // Simple spinlock for initialization
        while (atomic_flag_test_and_set(&g_init_lock));
        
        if (!g_log_file) {
            g_log_file = fopen("lau_audit.log", "a");
            if (g_log_file) {
                atexit(lau_cleanup_log);
            }
        }
        
        atomic_flag_clear(&g_init_lock);
        
        if (!g_log_file) return;
    }

    tsfi_io_log(g_log_file, TSFI_LOG_INFO, "AUDIT", "%s", status);
    tsfi_io_flush(g_log_file);
}