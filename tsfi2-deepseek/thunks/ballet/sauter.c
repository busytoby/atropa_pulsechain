#include <stdint.h>
#include <stddef.h>

/**
 * TSFi Ballet Step 5: Sauter (Jumping)
 * Instruction jumps. JIT thunk enters. Logic is fast.
 * This thunk ensures low-overhead entry into JIT-compiled manifolds.
 */
void tsfi_ballet_sauter_thunk(void (*target)(void)) {
    if (!target) return;
    
    // Jump into the target JIT thunk
    // We aim for minimal instruction counts to keep logic fast
    __asm__ volatile (
        "jmp *%0"  // Single instruction jump to the target manifold
        :
        : "r"(target)
        : "memory"
    );
}
