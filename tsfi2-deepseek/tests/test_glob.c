#include "tsfi_glob.h"
#include "lau_memory.h"
#include <stdio.h>
#include <assert.h>
#include <unistd.h>

int main() {
    printf("[VET] Testing TSFi Glob...\n");
    alarm(5);
    
    TSFiGlobResult *res = tsfi_glob(".", "*.c");
    
    assert(res != NULL);
    printf("Found %zu files\n", res->count);
    for (size_t i = 0; i < res->count && i < 5; i++) {
        printf("  %s\n", res->paths[i]);
    }
    
    tsfi_glob_free(res);
    
    extern void lau_free_all_active(void);
    lau_free_all_active();
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    printf("[PASS] Glob successful.\n");
    return 0;
}
