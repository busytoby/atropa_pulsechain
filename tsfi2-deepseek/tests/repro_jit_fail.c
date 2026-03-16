#include "lau_thunk.h"
#include "lau_memory.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

// Target functions
void func_1arg(int *val) {
    *val += 1;
}

void func_2args(int *val, int adder) {
    *val += adder;
}

void func_2args_sub(int *val, int subtractor) {
    *val -= subtractor;
}

#include <sys/mman.h>
#include <unistd.h>

// ... existing imports

int main() {
    printf("[TEST] Initializing ThunkProxy...\n");
    ThunkProxy *p = ThunkProxy_create();
    if (!p) {
        fprintf(stderr, "Failed to create proxy\n");
        return 1;
    }
    
    // Make pool writable first (default usually is, but let's be explicit)
    mprotect(p->thunk_pool, p->pool_size, PROT_READ | PROT_WRITE);

    // --- CASE 1: 1 Argument (Should work currently) ---
    printf("[TEST] Testing 1-arg rebind (Current baseline)...\n");
    int val1 = 10;
    void (*thunk1)(void) = ThunkProxy_emit_baked(p, func_1arg, 1, &val1);
    
    // SEAL MEMORY (Make Executable)
    mprotect(p->thunk_pool, p->pool_size, PROT_READ | PROT_EXEC);
    
    thunk1();
    if (val1 != 11) {
        fprintf(stderr, "FAIL: Initial 1-arg call failed. Got %d\n", val1);
        return 1;
    }
    
    // UNSEAL for next emit
    mprotect(p->thunk_pool, p->pool_size, PROT_READ | PROT_WRITE);

    // --- CASE 2: 2 Arguments (The Vulnerability) ---
    printf("[TEST] Testing 2-arg rebind (Vulnerability check)...\n");
    int val2 = 100;
    int adder = 50;
    
    // Emit thunk: func_2args(&val2, 50)
    void (*thunk2)(void) = ThunkProxy_emit_baked(p, func_2args, 2, &val2, (void*)(long)adder);
    
    // SEAL MEMORY
    mprotect(p->thunk_pool, p->pool_size, PROT_READ | PROT_EXEC);
    
    thunk2();
    if (val2 != 150) {
        fprintf(stderr, "FAIL: Initial 2-arg call failed. Got %d\n", val2);
        return 1;
    }

    printf("[TEST] 2-arg initial call passed. Attempting rebind to 'sub' function...\n");
    
    // REBIND: Swap func_2args with func_2args_sub
    // Note: ThunkProxy_rebind handles its own mprotect (RWX -> RX)
    ThunkProxy_rebind((void*)thunk2, func_2args_sub);
    
    thunk2();
    
    printf("[TEST] Result after rebind: %d\n", val2);
    
    if (val2 == 100) {
        printf("[SUCCESS] Rebind worked (Unexpected for current codebase!)\n");
        ThunkProxy_destroy(p);
        return 0;
    } else if (val2 == 200) {
        printf("[FAIL] Rebind ignored! Function still added 50. (Brittleness confirmed)\n");
        ThunkProxy_destroy(p);
        return 1; // Return error code to signal failure
    } else {
        printf("[FAIL] Undefined behavior! Result: %d\n", val2);
        ThunkProxy_destroy(p);
        return 2;
    }
    ThunkProxy_destroy(p);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
