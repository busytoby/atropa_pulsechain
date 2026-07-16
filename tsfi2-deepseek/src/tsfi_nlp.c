#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_nlp.h"

int tsfi_nlp_execute_command(TSFiSynthPerfEngine *engine, const char *command_str, TSFiParsedCommand *cmd_out) {
    if (!engine || !command_str || !cmd_out) return -1;

    memset(cmd_out, 0, sizeof(TSFiParsedCommand));
    cmd_out->type = CMD_INVALID;

    // 1. Tabled parsing simulation (Tokenization)
    char cmd_copy[256];
    strncpy(cmd_copy, command_str, sizeof(cmd_copy) - 1);
    
    char *tokens[8];
    int token_count = 0;
    char *tok = strtok(cmd_copy, " ");
    while (tok && token_count < 8) {
        tokens[token_count++] = tok;
        tok = strtok(NULL, " ");
    }

    if (token_count == 0) return -2;

    // 2. Definite Clause Grammar (DCG) Match & CLP Constraints check
    if (strcmp(tokens[0], "play") == 0 && token_count >= 4 && strcmp(tokens[1], "tone") == 0) {
        int wheel_idx = atoi(tokens[2]);
        float amp = (float)atof(tokens[3]);

        // CLP Constraint Verification: Assert wheel index is valid and amplitude is in [0.0, 1.0] bounds
        if (wheel_idx >= 0 && wheel_idx < 4 && amp >= 0.0f && amp <= 1.0f) {
            cmd_out->type = CMD_PLAY_TONE;
            cmd_out->target_wheel_idx = wheel_idx;
            cmd_out->target_amplitude = amp;

            // Execute verified action directly modifying synthesizer registers
            tsfi_synth_akb_process_input(engine->synth, wheel_idx, amp, "prior/active");
            return 0; // Success
        } else {
            return -3; // Constraint Violation (Backtracks)
        }
    } 
    else if (strcmp(tokens[0], "verify") == 0 && token_count >= 4 && strcmp(tokens[1], "path") == 0) {
        const char *src = tokens[2];
        const char *dest = tokens[3];

        // CLP Constraint: Assert path endpoints are not identical
        if (strcmp(src, dest) != 0) {
            cmd_out->type = CMD_VERIFY_PATH;
            strncpy(cmd_out->path_src, src, sizeof(cmd_out->path_src) - 1);
            strncpy(cmd_out->path_dest, dest, sizeof(cmd_out->path_dest) - 1);

            // Tabled lookups: Write dynamic transition edge to memory cache
            tsfi_akb_add_edge(engine->synth->orchestrator->akb, src, dest, 0.95f);
            return 0; // Success
        } else {
            return -3; // Constraint Violation (Backtracks)
        }
    }

    return -4; // Parse Failure
}

int tsfi_nlp_parse_dcg(TSFiSynthPerfEngine *engine, const char **tokens, int token_count, TSFiParsedCommand *cmd_out) {
    if (!engine || !tokens || token_count == 0 || !cmd_out) return -1;

    // Simulate recursive Noun Phrase (NP) tabling lookup:
    int wheel_idx = -1;
    int parsed_tokens = 0;

    if (token_count >= 3 && strcmp(tokens[0], "the") == 0 && strcmp(tokens[2], "tone-wheel") == 0) {
        if (strcmp(tokens[1], "primary") == 0) {
            wheel_idx = 0;
        } else if (strcmp(tokens[1], "secondary") == 0) {
            wheel_idx = 1;
        }
        parsed_tokens = 3;
    }

    if (wheel_idx == -1) return -2;

    // Parse Verb Phrase (VP):
    if (token_count >= (parsed_tokens + 3) && strcmp(tokens[parsed_tokens], "play") == 0 && strcmp(tokens[parsed_tokens + 1], "amplitude") == 0) {
        float amp = (float)atof(tokens[parsed_tokens + 2]);

        // CLP Constraint Check
        if (amp >= 0.0f && amp <= 1.0f) {
            cmd_out->type = CMD_PLAY_TONE;
            cmd_out->target_wheel_idx = wheel_idx;
            cmd_out->target_amplitude = amp;

            tsfi_synth_akb_process_input(engine->synth, wheel_idx, amp, "prior/active");
            return 0;
        } else {
            return -3;
        }
    }

    return -4;
}
