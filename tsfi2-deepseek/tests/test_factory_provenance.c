#include "tsfi_pulsechain_rpc.h"
#include <stdio.h>
#include <string.h>

int main() {
    printf("[TSFI PULSECHAIN C-NATIVE FACTORY SCAN]\n");
    printf("Target Factory: 0xbA6CcD38992839aEE20D5bF9125b1d94190b091C\n\n");
    
    char result[1024];
    if (tsfi_pulse_rpc_get_storage_at("0xbA6CcD38992839aEE20D5bF9125b1d94190b091C", "0x0", result, sizeof(result))) {
        printf("Factory Slot 0 (Hardcoded Implementation): %s\n", result);
    } else {
        printf("Failed to read Factory Slot 0.\n");
    }

    // Checking if the factory has code
    if (tsfi_pulse_rpc_call("0xbA6CcD38992839aEE20D5bF9125b1d94190b091C", "0x06fdde03", result, sizeof(result))) {
        if (strcmp(result, "0x") == 0) {
            printf("Factory is an Externally Owned Account (No Code).\n");
        } else {
            printf("Factory is a Smart Contract.\n");
        }
    } else {
        // If the eth_call fails completely, but we read storage, it's a contract that just doesn't implement name().
        printf("Factory is a Smart Contract (Call reverted).\n");
    }

    return 0;
}
