#ifndef TSFI_NLP_H
#define TSFI_NLP_H

#include "tsfi_synth_perf.h"

// Parsed Command Types
typedef enum {
    CMD_PLAY_TONE,   // "play tone <index> <amplitude>"
    CMD_VERIFY_PATH, // "verify path <src> <dest>"
    CMD_INVALID
} TSFiCommandType;

// NLP Parsed State
typedef struct {
    TSFiCommandType type;
    int target_wheel_idx;
    float target_amplitude;
    char path_src[64];
    char path_dest[64];
} TSFiParsedCommand;

// Parse a natural language command and execute CLP constraint verification
int tsfi_nlp_execute_command(TSFiSynthPerfEngine *engine, const char *command_str, TSFiParsedCommand *cmd_out);

#endif // TSFI_NLP_H
