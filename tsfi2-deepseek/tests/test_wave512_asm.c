#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_wave512_asm.h"
#include "lau_memory.h"

int main() {
    printf("=== Wave512 Assembly Compiler Test ===\n");
    Wave512State state;
    tsfi_wave512_init(&state);
    
    tsfi_wave512_exec(&state, 
        "WLOAD ALPHA, 10.0\n"
        "WLOAD BETA, 5.0\n"
        "WADD W3, ALPHA, BETA\n"
        "WDUMP W3\n"
        "CMPGT COND, ALPHA, BETA\n"
        "JNZ COND, LABEL_GT\n"
        "WLOAD W4, 1.0\n"
        "JMP LABEL_END\n"
        "LABEL_GT: WLOAD W4, 99.0\n"
        "LABEL_END: WDUMP W4\n"
    );
    
    printf("Output buffer:\n%s", state.output_buffer);
    
    // ALPHA is allocated to a variable register (likely W8 or similar)
    // Beta is also allocated
    // W3 should print [15.00...]
    // COND should be true (all elements 1.00)
    // Branch should follow to LABEL_GT and set W4 to 99.00
    if (strstr(state.output_buffer, "W3: [15.00") &&
        strstr(state.output_buffer, "W4: [99.00")) {
        printf("[PASS] Wave512 Assembly execution with statement table & symbols validated.\n");
    } else {
        printf("[FAIL] Output mismatch.\n");
        return 1;
    }
    
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    return 0;
}
