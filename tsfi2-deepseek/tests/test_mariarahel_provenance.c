#include "tsfi_pulsechain_rpc.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void print_storage(const char *address, int slot) {
    char hex_slot[64];
    snprintf(hex_slot, sizeof(hex_slot), "0x%x", slot);
    
    char result[1024];
    if (tsfi_pulse_rpc_get_storage_at(address, hex_slot, result, sizeof(result))) {
        printf("Slot %d: %s\n", slot, result);
    } else {
        printf("Slot %d: [FAILED TO READ]\n", slot);
    }
}

int main() {
    printf("[TSFI PULSECHAIN C-NATIVE PROVENANCE SCAN]\n");
    printf("Target Token: mariarahel (0xD32c39fEE49391c7952d1b30b15921b0D3b42E69)\n\n");

    // We know from our previous discovery that Dysnomia tokens store parent links in Slot 9.
    // I will write a C loop to traverse the CHATLOG hierarchy purely in C using my POSIX socket RPC client.
    
    char current_address[64] = "0xD32c39fEE49391c7952d1b30b15921b0D3b42E69";
    char next_address[1024];
    
    for (int step = 0; step < 6; step++) {
        printf("--- STEP %d ---\n", step);
        printf("Address: %s\n", current_address);
        
        // Let's get the name() of the current address if it's an ERC20 to identify it
        char result[1024];
        if (tsfi_pulse_rpc_call(current_address, "0x06fdde03", result, sizeof(result))) {
            if (strcmp(result, "0x") != 0 && strlen(result) > 130) {
                // Decode hex to ascii in pure C
                char ascii[256] = {0};
                int ascii_len = 0;
                // The string data starts after the 0x (2 chars) + offset (64 chars) + length (64 chars) = 130 chars
                for (size_t i = 130; i < strlen(result) && i < 130 + 128; i += 2) {
                    char hex_byte[3] = {result[i], result[i+1], '\0'};
                    int val = (int)strtol(hex_byte, NULL, 16);
                    if (val > 31 && val < 127) {
                        ascii[ascii_len++] = (char)val;
                    }
                }
                printf("Name: %s\n", ascii);
            }
        }
        
        // Read Slot 9 to find the parent
        if (tsfi_pulse_rpc_get_storage_at(current_address, "0x9", next_address, sizeof(next_address))) {
            printf("Slot 9 (Parent Link): %s\n", next_address);
            
            // Format next_address to standard 0x... 40 char hex
            // next_address is something like 0x000000000000000000000000965b0d74591bf30327075a247c47dbf487dcff08
            if (strlen(next_address) >= 66) {
                snprintf(current_address, sizeof(current_address), "0x%s", next_address + 26);
            } else {
                printf("Reached end of chain or invalid address format.\n");
                break;
            }
        } else {
            printf("Failed to read Slot 9.\n");
            break;
        }
        printf("\n");
    }

    return 0;
}
