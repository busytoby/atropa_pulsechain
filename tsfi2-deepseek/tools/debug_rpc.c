#include "tsfi_pulsechain_rpc.h"
#include "tsfi_tls.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    printf("=== Live RPC Debugger (Logs Test) ===\n");
    
    char block_buf[128] = {0};
    const char *block_payload = "{\"jsonrpc\":\"2.0\",\"method\":\"eth_blockNumber\",\"params\":[],\"id\":1}";
    if (!tsfi_pulse_rpc_exec_raw(block_payload, block_buf, sizeof(block_buf))) {
        printf("Failed to get block number\n");
        return 1;
    }
    
    uint64_t current_block = strtoull(block_buf, NULL, 16);
    printf("Latest Block: %lu (0x%lx)\n", current_block, current_block);
    
    char logs_payload[512];
    snprintf(logs_payload, sizeof(logs_payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_getLogs\",\"params\":[{\"fromBlock\":\"0x%lx\",\"toBlock\":\"0x%lx\",\"topics\":[\"0xd78ad95fa46c994b6551d0da85fc275fe613ce37657fb8d5e3d130840159d822\"]}],\"id\":1}",
             current_block, current_block);
             
    static char logs_buf[262144] = {0};
    if (tsfi_pulse_rpc_exec_raw(logs_payload, logs_buf, sizeof(logs_buf))) {
        printf("SUCCESS! Logs buffer length: %zu\n", strlen(logs_buf));
        if (strlen(logs_buf) > 100) {
            printf("Start: %.100s\n", logs_buf);
        } else {
            printf("Value: %s\n", logs_buf);
        }
    } else {
        printf("FAILED to query logs\n");
    }
    return 0;
}
