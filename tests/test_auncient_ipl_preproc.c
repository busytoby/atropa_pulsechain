#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// Simulated WinchesterMQ Device Registry
#define REGISTERED_CONSOLE_ADDR 0x01F

// I/PL PARM state
typedef struct {
    char parm_string[64];
    uint32_t parsed_console_addr;
    gate_state_t boot_gate;
} ipl_parm_t;

// preprocessor macro replacement representation
typedef struct {
    char macro_name[16];
    char macro_value[16];
    gate_state_t preproc_gate;
} macro_entry_t;

#define TAG_KERNEL 0x01
#define TAG_USER   0x02

// -------------------------------------------------------------
// 1. I/PL PARM Extraction & Device Registry Audit
// -------------------------------------------------------------
bool ipl_parse_and_validate(ipl_parm_t *parm, uint8_t tag) {
    parm->boot_gate = CUTOFF_STATE;

    if (tag != TAG_KERNEL) return false;

    // Simulate constant-time TRANSLATE character scan to find CONSOLE=
    const char *target = "CONSOLE=";
    char *match = strstr(parm->parm_string, target);
    if (!match) return false;

    // Extract address following target
    char addr_str[4];
    strncpy(addr_str, match + strlen(target), 3);
    addr_str[3] = '\0';

    // Parse hex address
    uint32_t parsed_addr = (uint32_t)strtol(addr_str, NULL, 16);
    parm->parsed_console_addr = parsed_addr;

    // Wortman bounds check: verify console address matches registered console device (0x01F)
    if (parsed_addr != REGISTERED_CONSOLE_ADDR) {
        return false; // Halted: unregistered console device specified
    }

    parm->boot_gate = CONDUC_STATE;
    return true;
}

// -------------------------------------------------------------
// 2. PL/I %DECLARE Preprocessor Replacement Audit
// -------------------------------------------------------------
bool preproc_substitute(macro_entry_t *macro, const char *source, char *dest, size_t dest_len) {
    macro->preproc_gate = CUTOFF_STATE;

    // Horning aliasing check: prevent circular or redundant macro name-value overlaps
    if (strcmp(macro->macro_name, macro->macro_value) == 0) {
        return false; // Aliasing loop caught
    }

    // Simulate macro replacement in source
    char *match = strstr(source, macro->macro_name);
    if (match) {
        size_t prefix_len = match - source;
        // Wortman bounds check: verify expanded replacement fits within target buffer
        size_t total_needed = prefix_len + strlen(macro->macro_value) + strlen(match + strlen(macro->macro_name)) + 1;
        if (total_needed > dest_len) {
            return false; // Buffer overflow blocked
        }

        macro->preproc_gate = CONDUC_STATE;
        strncpy(dest, source, prefix_len);
        dest[prefix_len] = '\0';
        strcat(dest, macro->macro_value);
        strcat(dest, match + strlen(macro->macro_name));
        return true;
    }

    return false;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT I/PL PARM AND PREPROCESSOR VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Test 1: Valid I/PL PARM parsing
    ipl_parm_t parm = {
        .parm_string = "SYSRES=DSN001,CONSOLE=01F",
        .parsed_console_addr = 0,
        .boot_gate = CUTOFF_STATE
    };
    printf("[TEST] Parsing valid I/PL PARM string...\n");
    fflush(stdout);
    bool ok = ipl_parse_and_validate(&parm, TAG_KERNEL);
    assert(ok == true);
    assert(parm.boot_gate == CONDUC_STATE);
    assert(parm.parsed_console_addr == 0x01F);
    printf("   ✓ Console port parsed and validated: 0x%03X\n", parm.parsed_console_addr);
    fflush(stdout);

    // Test 2: Invalid console device specified -> Should cutoff
    strcpy(parm.parm_string, "SYSRES=DSN001,CONSOLE=999");
    printf("[TEST] Parsing invalid I/PL PARM string (unregistered console)...\n");
    fflush(stdout);
    ok = ipl_parse_and_validate(&parm, TAG_KERNEL);
    assert(ok == false);
    assert(parm.boot_gate == CUTOFF_STATE);
    printf("   ✓ Unregistered console address rejected successfully.\n");
    fflush(stdout);

    // Test 3: Preprocessor replacement (valid macro definition)
    macro_entry_t macro = {
        .macro_name = "%DevAddr",
        .macro_value = "01F",
        .preproc_gate = CUTOFF_STATE
    };
    char output_code[128] = "";
    printf("[TEST] Executing macro substitution for %%DevAddr...\n");
    fflush(stdout);
    ok = preproc_substitute(&macro, "PORT = %DevAddr;", output_code, sizeof(output_code));
    assert(ok == true);
    assert(macro.preproc_gate == CONDUC_STATE);
    assert(strcmp(output_code, "PORT = 01F;") == 0);
    printf("   ✓ Substitution successful: %s\n", output_code);
    fflush(stdout);

    // Test 4: Preprocessor aliasing loop detection -> Should cutoff
    strcpy(macro.macro_name, "%Loop");
    strcpy(macro.macro_value, "%Loop"); // Aliased loop definition
    printf("[TEST] Executing preprocessor macro aliasing check...\n");
    fflush(stdout);
    ok = preproc_substitute(&macro, "PORT = %Loop;", output_code, sizeof(output_code));
    assert(ok == false);
    assert(macro.preproc_gate == CUTOFF_STATE);
    printf("   ✓ Preprocessor aliasing collision intercepted successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("I/PL PARM AND PREPROCESSOR TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
