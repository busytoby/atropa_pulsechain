#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "auncient_sdk.h"

// Parse and run diagnostics on a test script
void run_tpu_diagnostics(const char *script_source) {
    printf("--- Beginning TPU Diagnostic Audits ---\n");
    
    auncient_transfluxor_registry_t reg;
    auncient_sdk_init_transfluxor_registry(&reg);

    // Simple line-by-line parser simulation for diagnostics
    char source_copy[1024];
    strncpy(source_copy, script_source, sizeof(source_copy) - 1);
    source_copy[sizeof(source_copy) - 1] = '\0';

    char *line = strtok(source_copy, "\n");
    uint32_t compiled_count = 0;

    while (line != NULL) {
        if (line[0] == '\0' || line[0] == '#') {
            line = strtok(NULL, "\n");
            continue;
        }

        char command[32] = "";
        char name[32] = "";
        double f1 = 0.0, f2 = 0.0, decay = 0.0;
        uint32_t word_id = 0, wmq = 0, abi = 0;

        int parsed = sscanf(line, "%s %s %u %lf %lf %lf %x %x", 
                            command, name, &word_id, &f1, &f2, &decay, &wmq, &abi);

        if (parsed == 8 && strcmp(command, "WORD") == 0) {
            auncient_transfluxor_word_t word;
            bool ok = auncient_sdk_compile_transfluxor_word(
                &word, name, word_id, f1, f2, decay, wmq, abi
            );
            assert(ok);

            ok = auncient_sdk_register_transfluxor_word(&reg, &word);
            if (ok) {
                compiled_count++;
                printf("[AUDIT] Compiled '%s' (ID %u) -> Hash: %lu\n", 
                       word.name, word.word_id, 
                       auncient_sdk_calculate_transfluxor_hash(f1, f2, decay, word_id));
            } else {
                printf("[AUDIT WARNING] Collision detected for '%s' (ID %u).\n", name, word_id);
            }
        }

        line = strtok(NULL, "\n");
    }

    printf("[AUDIT SUMMARY] Total successfully compiled: %u words.\n", compiled_count);
    
    // Dispatch testing
    if (compiled_count > 0) {
        printf("[AUDIT] Simulating loopback dispatches...\n");
        double feedback = 0.0;
        for (uint32_t i = 0; i < reg.total_registered; i++) {
            bool success = auncient_sdk_dispatch_transfluxor_word(&reg, &reg.registered_words[i], &feedback);
            printf("        Word '%s' (ID %u) -> Dispatch: %s | Feedback: %.2fHz\n",
                   reg.registered_words[i].name, reg.registered_words[i].word_id,
                   success ? "SUCCESS" : "FAILED", feedback);
        }
    }
    printf("--- TPU Diagnostic Audits Complete ---\n");
}

int main(void) {
    printf("=============================================================\n");
    printf("ANTIGRAVITY TPU DEVELOPMENT DIAGNOSTIC TOOL\n");
    printf("=============================================================\n");

    // Sample diagnostic script
    const char *test_script = 
        "# Command Format: WORD [name] [id] [f1] [f2] [decay] [wmq_hex] [abi_hex]\n"
        "WORD SPK_LOCK 1 440.0 880.0 0.4 10 00\n"
        "WORD SPK_WRITE 2 350.0 700.0 0.2 00 20\n"
        "WORD SPK_COLLIDE 3 440.0 880.0 0.4 00 00\n"; // Should collide with SPK_LOCK if word_id matches (but word_id=3 prevents it)

    run_tpu_diagnostics(test_script);

    printf("=============================================================\n");
    printf("DIAGNOSTIC PROCESSOR COMPLETE\n");
    printf("=============================================================\n");

    return 0;
}
