#include <stdint.h>
#include <stddef.h>

/**
 * TSFi Ballet Step 4: Glisser (Gliding)
 * Data glides. Zero-copy bridge moves. Friction is zero.
 * This thunk implements the high-speed context switch via R15 manipulation.
 */
void tsfi_ballet_glisser_thunk(void *ctx, void (*task)(void)) {
    if (!ctx || !task) return;
    
    // Switch to the target context (Gliding)
    // We update the R15 register, which acts as the bijective base pointer
    __asm__ volatile (
        "movq %0, %%r15
	"  // Glider: Context sets the R15 anchor
        "call *%1
	"        // Task executes atop the new context
        :
        : "r"(ctx), "r"(task)
        : "r15", "memory"
    );
    
    // Friction is zero; the task executes directly on the target manifold.
}
