#include "tsfi_pulsechain_rpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A standalone TSFi RPC executable that serves as the internal drop-in replacement for curl.
// It bridges the C codebase directly to the PulseChain JSON-RPC API.

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("Usage: tsfi_rpc <call|storage> <address> <hex_data_or_slot>\n");
        return 1;
    }

    const char *action = argv[1];
    const char *address = argv[2];
    const char *data = argv[3];

    char result[8192];

    if (strcmp(action, "call") == 0) {
        if (tsfi_pulse_rpc_call(address, data, result, sizeof(result))) {
            printf("%s\n", result);
            return 0;
        } else {
            printf("Error: RPC Call failed.\n");
            return 1;
        }
    } else if (strcmp(action, "storage") == 0) {
        if (tsfi_pulse_rpc_get_storage_at(address, data, result, sizeof(result))) {
            printf("%s\n", result);
            return 0;
        } else {
            printf("Error: RPC Storage read failed.\n");
            return 1;
        }
    } else {
        printf("Unknown action: %s\n", action);
        return 1;
    }
}
