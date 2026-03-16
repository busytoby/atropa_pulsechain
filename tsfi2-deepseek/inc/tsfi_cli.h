#ifndef TSFI_CLI_H
#define TSFI_CLI_H

#include "tsfi_types.h"

// Processes a single line of CLI input.
// Returns 0 to continue, 1 to exit.
int tsfi_cli_process_line(WaveSystem *ws, char *line);

#endif // TSFI_CLI_H
