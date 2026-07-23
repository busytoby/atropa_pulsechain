#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#define SECURITY_LEVEL_UNCLASSIFIED 0
#define SECURITY_LEVEL_CONFIDENTIAL 1
#define SECURITY_LEVEL_SECRET       2

#define REG_PSG_VOL 61697  // 0xF101 (Requires Confidential)
#define REG_VCE_BASE 62208 // 0xF300 (Requires Secret)

// 1. LALR(1) State Machine transition definitions
typedef struct {
    int state;
    const char *token;
    int next_state;
} LalrTransition;

static LalrTransition lalr_extended_table[] = {
    {0, "BYTE", 1},
    {1, "(", 2},
    {2, "NUMBER", 3},
    {3, ")", 4},
    {4, "=", 5},
    {5, "NUMBER", 6},
    {6, ";", 0} // Cycle back to state 0 for multi-statement parsing
};
#define EXT_TRANSITIONS_COUNT (sizeof(lalr_extended_table)/sizeof(lalr_extended_table[0]))

int lalr_extended_next_state(int current_state, const char *symbol) {
    for (size_t i = 0; i < EXT_TRANSITIONS_COUNT; i++) {
        if (lalr_extended_table[i].state == current_state && strcmp(lalr_extended_table[i].token, symbol) == 0) {
            return lalr_extended_table[i].next_state;
        }
    }
    return -1; // Syntax error
}

// 2. Direct-to-Yul Multi-Statement Emitter (SKELETON)
void emit_yul_register_write(unsigned int address, unsigned int value, char *out_yul) {
    char temp[256];
    if (address == REG_PSG_VOL) {
        sprintf(temp, 
            "// Direct PSG Volume Write\n"
            "storeNamespaced(0xF101, %u)\n", value);
    } else if (address >= REG_VCE_BASE && address <= 62719) {
        unsigned int color_idx = address - REG_VCE_BASE;
        unsigned int r = (value >> 6) & 0x7;
        unsigned int g = (value >> 3) & 0x7;
        unsigned int b = value & 0x7;
        
        sprintf(temp, 
            "// Direct VCE Write\n"
            "let colorIdx := %u\n"
            "let packedVal := add(shl(%u, 6), add(shl(%u, 3), %u))\n"
            "storeNamespaced(add(0xF300, colorIdx), packedVal)\n", 
            color_idx, r, g, b);
    } else {
        sprintf(temp, "storeNamespaced(%u, %u)\n", address, value);
    }
    strcat(out_yul, temp);
}

// 3. Multi-Level Compile-Time Security Governance Gatekeeper (XCOM)
int xcom_compile_extended(const char *source, uint8_t clearance, char *out_yul) {
    printf("[XCOM] Compile-time exhaustive sweep starting...\n");
    out_yul[0] = '\0';
    
    char src_copy[1024];
    strncpy(src_copy, source, sizeof(src_copy) - 1);
    src_copy[sizeof(src_copy) - 1] = '\0';
    
    char *statement = strtok(src_copy, " \t\n\r");
    int state = 0;
    unsigned int active_address = 0;
    
    while (statement != NULL) {
        // Simple token matching
        const char *token = statement;
        if (strncmp(token, "BYTE", 4) == 0) {
            state = lalr_extended_next_state(state, "BYTE");
        } else if (token[0] == '(') {
            state = lalr_extended_next_state(state, "(");
            active_address = atoi(token + 1);
            state = lalr_extended_next_state(state, "NUMBER");
            if (token[strlen(token) - 1] == ')') {
                state = lalr_extended_next_state(state, ")");
            }
        } else if (token[0] == '=') {
            state = lalr_extended_next_state(state, "=");
            unsigned int value = atoi(token + 1);
            state = lalr_extended_next_state(state, "NUMBER");
            
            // Security verification depending on address type
            if (active_address == REG_PSG_VOL) {
                if (clearance < SECURITY_LEVEL_CONFIDENTIAL) {
                    printf("[XCOM] Governance block: PSG Vol requires Confidential (has %u).\n", clearance);
                    return -10;
                }
            } else if (active_address >= REG_VCE_BASE && active_address <= 62719) {
                if (clearance < SECURITY_LEVEL_SECRET) {
                    printf("[XCOM] Governance block: VCE registers require Secret (has %u).\n", clearance);
                    return -11;
                }
            }
            
            // Emit code
            emit_yul_register_write(active_address, value, out_yul);
            
            if (token[strlen(token) - 1] == ';') {
                state = lalr_extended_next_state(state, ";");
            }
        }
        
        statement = strtok(NULL, " \t\n\r");
    }
    
    if (state != 0) {
        printf("[XCOM] Syntax error: Incomplete statement sequence.\n");
        return -12;
    }
    
    return 0;
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("=== TSFi ZMM VM Auncient XPL TWS Exhaustive Test Suite ===\n");

    // Multi-statement program updating both PSG volume and VCE color index 0
    const char *program_src = "BYTE (61697) =15; BYTE (62208) =306;";
    char emitted_yul[2048];

    // Case 1: Secret Clearance (2) - should compile both statements successfully
    printf("\n--- Exhaustive Case 1: User with Secret Clearance (2) ---\n");
    memset(emitted_yul, 0, sizeof(emitted_yul));
    int res1 = xcom_compile_extended(program_src, SECURITY_LEVEL_SECRET, emitted_yul);
    assert(res1 == 0);
    printf("[SKELETON] Emitted Yul code:\n%s\n", emitted_yul);
    assert(strstr(emitted_yul, "storeNamespaced(0xF101, 15)") != NULL);
    assert(strstr(emitted_yul, "storeNamespaced(add(0xF300, colorIdx), packedVal)") != NULL);
    printf("[TWS] Case 1 passed: Multi-statement compiled successfully.\n");

    // Case 2: Confidential Clearance (1) - should fail compiling VCE statement
    printf("\n--- Exhaustive Case 2: User with Confidential Clearance (1) ---\n");
    memset(emitted_yul, 0, sizeof(emitted_yul));
    int res2 = xcom_compile_extended(program_src, SECURITY_LEVEL_CONFIDENTIAL, emitted_yul);
    assert(res2 == -11); // Blocked by VCE clearance validation
    printf("[TWS] Case 2 passed: VCE block verified at Confidential level.\n");

    // Case 3: Unclassified Clearance (0) - should fail compiling PSG statement
    printf("\n--- Exhaustive Case 3: User with Unclassified Clearance (0) ---\n");
    memset(emitted_yul, 0, sizeof(emitted_yul));
    int res3 = xcom_compile_extended(program_src, SECURITY_LEVEL_UNCLASSIFIED, emitted_yul);
    assert(res3 == -10); // Blocked by PSG clearance validation
    printf("[TWS] Case 3 passed: PSG block verified at Unclassified level.\n");

    // Case 4: LALR(1) parser syntax verification cycle
    printf("\n--- Exhaustive Case 4: Multi-statement parsing syntax checks ---\n");
    int state = 0;
    state = lalr_extended_next_state(state, "BYTE");
    state = lalr_extended_next_state(state, "(");
    state = lalr_extended_next_state(state, "NUMBER");
    state = lalr_extended_next_state(state, ")");
    state = lalr_extended_next_state(state, "=");
    state = lalr_extended_next_state(state, "NUMBER");
    state = lalr_extended_next_state(state, ";");
    assert(state == 0); // Cycled back to start state ready for next statement
    printf("[LALR] Case 4 passed: Multi-statement transition loop verified.\n");

    printf("\n=== ALL EXHAUSTIVE TWS UPGRADE TESTS PASSED ===\n");
    return 0;
}
