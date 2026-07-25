#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "auncient_sdk.h"

// Parse a VoxPL script line-by-line and compile it into a Transfluxor Registry
bool compile_voxpl_script(auncient_transfluxor_registry_t *reg, const char *script) {
    char script_copy[1024];
    strncpy(script_copy, script, sizeof(script_copy) - 1);
    script_copy[sizeof(script_copy) - 1] = '\0';

    char *line = strtok(script_copy, "\n");
    auncient_transfluxor_word_t current_word;
    bool building_word = false;

    while (line != NULL) {
        // Skip empty lines or comments
        if (line[0] == '\0' || line[0] == '#') {
            line = strtok(NULL, "\n");
            continue;
        }

        char command[32] = "";
        char arg1[64] = "";
        char arg2[64] = "";
        
        int parsed = sscanf(line, "%s %s %s", command, arg1, arg2);
        if (parsed <= 0) {
            line = strtok(NULL, "\n");
            continue;
        }

        if (strcmp(command, "JUMP") == 0) {
            // Commit previous word if we were building one
            if (building_word) {
                if (!auncient_sdk_register_transfluxor_word(reg, &current_word)) {
                    printf("   [COMPILER ERROR] Failed to register word '%s' (possible hash collision).\n", 
                           current_word.name);
                    return false;
                }
                printf("   [COMPILER] Successfully compiled and registered word: '%s'\n", current_word.name);
            }
            
            // Start building new word
            memset(&current_word, 0, sizeof(current_word));
            snprintf(current_word.name, sizeof(current_word.name), "%s", arg1);
            current_word.amplitude = 5.0; // Default Wheeler Jump amplitude
            building_word = true;

            
        } else if (strcmp(command, "FORMANT") == 0 && building_word) {
            current_word.f1 = atof(arg1);
            current_word.f2 = atof(arg2);
            
        } else if (strcmp(command, "DECAY") == 0 && building_word) {
            current_word.decay = atof(arg1);
            
        } else if (strcmp(command, "WMQ") == 0 && building_word) {
            current_word.wmq_cmd = (uint32_t)strtoul(arg1, NULL, 16);
            
        } else if (strcmp(command, "ABI") == 0 && building_word) {
            current_word.abi_op = (uint32_t)strtoul(arg1, NULL, 16);
        }

        line = strtok(NULL, "\n");
    }

    // Commit final word
    if (building_word) {
        if (!auncient_sdk_register_transfluxor_word(reg, &current_word)) {
            printf("   [COMPILER ERROR] Failed to register final word '%s'.\n", current_word.name);
            return false;
        }
        printf("   [COMPILER] Successfully compiled and registered word: '%s'\n", current_word.name);
    }

    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT VOXPL COMPILER TOOL\n");
    printf("=============================================================\n");

    auncient_transfluxor_registry_t registry;
    auncient_sdk_init_transfluxor_registry(&registry);

    // Sample VoxPL source script input
    const char *source_code = 
        "# Target command structures\n"
        "JUMP SPK_LOCK_SCSI\n"
        "FORMANT 440.0 880.0\n"
        "DECAY 0.4\n"
        "WMQ 0x10\n"
        "\n"
        "JUMP SPK_WRITE_LEDGER\n"
        "FORMANT 350.0 700.0\n"
        "DECAY 0.2\n"
        "ABI 0x02\n";

    printf("[COMPILER] Compiling VoxPL source script...\n");
    bool success = compile_voxpl_script(&registry, source_code);
    
    if (success) {
        printf("   ✓ VoxPL Compilation Succeeded. Registered words: %u/%d\n", 
               registry.total_registered, MAX_REGISTRY_WORDS);
    } else {
        printf("   ✗ VoxPL Compilation Failed.\n");
        return 1;
    }

    // Verify compiled registry content
    assert(registry.total_registered == 2);
    assert(strcmp(registry.registered_words[0].name, "SPK_LOCK_SCSI") == 0);
    assert(registry.registered_words[0].f1 == 440.0);
    assert(registry.registered_words[0].wmq_cmd == 0x10);

    assert(strcmp(registry.registered_words[1].name, "SPK_WRITE_LEDGER") == 0);
    assert(registry.registered_words[1].f1 == 350.0);
    assert(registry.registered_words[1].abi_op == 0x02);

    printf("=============================================================\n");
    printf("VOXPL COMPILER AUDIT COMPLETE\n");
    printf("=============================================================\n");

    return 0;
}
